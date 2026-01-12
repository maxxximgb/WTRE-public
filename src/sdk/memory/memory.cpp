#include "memory.h"

static pid_t PID = -1;
static uintptr_t MODULE_START = 0;
static uintptr_t MODULE_END = 0;

bool OpenProcess(const char* name) {
    DIR* dir = opendir("/proc");
    dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (ent->d_type != DT_DIR)
            continue;
        pid_t pid = std::atoi(ent->d_name);
        char path[64];
        std::snprintf(path, sizeof(path), "/proc/%d/comm", pid);
        FILE* f = std::fopen(path, "r");
        if (!f)
            continue;
        char comm[64];
        if (std::fgets(comm, sizeof(comm), f)) {
            comm[strcspn(comm, "\n")] = 0;
            if (std::strcmp(comm, name) == 0) {
                std::fclose(f);
                closedir(dir);
                PID = pid;
                
                std::snprintf(path, sizeof(path), "/proc/%d/maps", pid);
                std::ifstream maps(path);
                std::string line;
                while (std::getline(maps, line)) {
                    if (line.find("r-xp") != std::string::npos && MODULE_START == 0) {
                        std::istringstream iss(line);
                        char dash;
                        iss >> std::hex >> MODULE_START >> dash >> MODULE_END;
                        break;
                    }
                }
                return MODULE_START != 0;
            }
        }
        std::fclose(f);
    }
    closedir(dir);
    return false;
}

template<typename T>
bool ReadMemory(std::uint64_t address, T& dst) {
    if (PID <= 0) return false;

    iovec local_iov{
        .iov_base = &dst,
        .iov_len  = sizeof(T)
    };

    iovec remote_iov{
        .iov_base = reinterpret_cast<void*>(address),
        .iov_len  = sizeof(T)
    };

    const ssize_t n = process_vm_readv(
        PID,
        &local_iov, 1,
        &remote_iov, 1,
        0
    );

    return n == static_cast<ssize_t>(sizeof(T));
}


template<typename T>
bool WriteMemory(std::uint64_t address, const T& value) {
    iovec local { (void*)&value, sizeof(T) };
    iovec remote{ (void*)address, sizeof(T) };

    return process_vm_writev(PID, &local, 1, &remote, 1, 0)
           == (ssize_t)sizeof(T);
}


uintptr_t PatternScan(const char* pattern) {
    if (PID <= 0 || !MODULE_START) return 0;
    
    uint8_t bytes[64], mask[64];
    size_t len = 0;
    for (const char* p = pattern; *p && len < 64; ) {
        while (*p == ' ') ++p;
        if (!*p) break;
        if (*p == '?') {
            bytes[len] = 0;
            mask[len++] = 0;
            ++p;
        } else {
            bytes[len] = ((*p & 0xDF) >= 'A' ? (*p & 0xDF) - 'A' + 10 : *p - '0') << 4;
            if (*++p) bytes[len] |= (*p & 0xDF) >= 'A' ? (*p & 0xDF) - 'A' + 10 : *p - '0';
            mask[len++] = 1;
            ++p;
        }
    }
    
    alignas(32) uint8_t buf[0x200000];
    __m256i first = _mm256_set1_epi8(bytes[0]);
    
    for (uintptr_t addr = MODULE_START; addr < MODULE_END; addr += sizeof(buf) - len) {
        size_t sz = addr + sizeof(buf) > MODULE_END ? MODULE_END - addr : sizeof(buf);
        iovec l{buf, sz}, r{(void*)addr, sz};
        if (process_vm_readv(PID, &l, 1, &r, 1, 0) <= 0) continue;
        
        for (size_t i = 0; i <= sz - len; i += 32) {
            uint32_t m = mask[0] ? _mm256_movemask_epi8(_mm256_cmpeq_epi8(
                _mm256_loadu_si256((__m256i*)(buf + i)), first)) : 1;
            
            while (m) {
                size_t pos = __builtin_ctz(m);
                m &= m - 1;
                size_t j = 1;
                for (; j < len && (!mask[j] || buf[i + pos + j] == bytes[j]); ++j);
                if (j == len) return addr + i + pos;
            }
        }
    }
    return 0;
}


bool ReadCString(std::uint64_t address, std::string& out, size_t maxLen)
{
    out.clear();
    if (PID <= 0 || address == 0) return false;

    // Читаем кусками, чтобы не делать 1 sys-call на байт
    constexpr size_t CHUNK = 64;
    std::vector<char> buf(CHUNK);

    size_t readTotal = 0;
    while (readTotal < maxLen)
    {
        size_t want = std::min(CHUNK, maxLen - readTotal);

        iovec local{ .iov_base = buf.data(), .iov_len = want };
        iovec remote{ .iov_base = (void*)(address + readTotal), .iov_len = want };

        ssize_t n = process_vm_readv(PID, &local, 1, &remote, 1, 0);
        if (n <= 0) return false;

        for (ssize_t i = 0; i < n; i++)
        {
            char c = buf[(size_t)i];
            if (c == '\0') return true;
            out.push_back(c);
        }

        readTotal += (size_t)n;
        if ((size_t)n < want) break; // меньше прочитали — дальше может быть unmapped
    }

    // не нашли \0 в пределах maxLen
    return true;
}

bool ReadRemoteCStringPtr(std::uint64_t fieldAddress, std::string& out, size_t maxLen)
{
    std::uint64_t strPtr = 0;
    if (!ReadMemory<std::uint64_t>(fieldAddress, strPtr) || strPtr == 0) {
        out.clear();
        return false;
    }
    return ReadCString(strPtr, out, maxLen);
}


// memory.cpp (в самом низу файла)
template bool ReadMemory<uint8_t>(uintptr_t, uint8_t&);
template bool ReadMemory<uint16_t>(uintptr_t, uint16_t&);
template bool ReadMemory<uint32_t>(uintptr_t, uint32_t&);
template bool ReadMemory<uint64_t>(uintptr_t, uint64_t&);
template bool ReadMemory<bool>(uintptr_t, bool&);
template bool ReadMemory<char>(uintptr_t, char&);
template bool ReadMemory<int>(uintptr_t, int&);
template bool ReadMemory<long>(uintptr_t, long&);
template bool ReadMemory<float>(uintptr_t, float&);
template bool ReadMemory<double>(uintptr_t, double&);
template bool ReadMemory<Matrix4x4>(uintptr_t, Matrix4x4&);
template bool ReadMemory<Matrix3x3>(uintptr_t, Matrix3x3&);
template bool ReadMemory<ViewMatrix>(uintptr_t, ViewMatrix&);
template bool ReadMemory<Vector3>(uintptr_t, Vector3&);
template bool ReadMemory<Vector3d>(uintptr_t, Vector3d&);

template bool WriteMemory<uint32_t>(uintptr_t, const uint32_t&);
template bool WriteMemory<uint64_t>(uintptr_t, const uint64_t&);