#include <inttypes.h>
#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstring>
#include "../sdk/memory/memory.h"

class offset_c
{
private:
    std::string name_space;
    std::map<std::string, uintptr_t> offsets;
    std::vector<offset_c> children;
public:
    offset_c(std::string namespz)
    {
        name_space = namespz;
    };
    void add(std::string name, uintptr_t offset)
    {
        offsets.emplace(name, offset);
    };
    void add_child(offset_c child)
    {
        children.emplace_back(child);
    }
    void dump(std::ofstream& outfile, int offset = 0)
    {
        if (offset == 0)
            outfile << "//Generated using bditt's War Thunder dumper! https://github.com/bditt/WarThunder-Offset-Dumper " << std::endl;
        std::string spacer = "";
        for (int i = 0; i < offset; i++)
        {
            spacer += " ";
        }
        
        outfile << spacer << "namespace " << name_space << std::endl;
        outfile << spacer << "{" << std::endl;
        auto offset_space = spacer;
        offset_space += " ";
        for (auto& offset : offsets)
        {
            if (offset.second != 0)
                outfile << offset_space << "constexpr uintptr_t " << offset.first << " = 0x" << std::hex << offset.second << std::dec << "; " << std::endl;
            else
                outfile << offset_space << "//constexpr uintptr_t " << offset.first << " = 0x" << std::hex << offset.second << std::dec << "; // FAILED TO GRAB" << std::endl;
        }
        for (auto& child : children)
        {
            child.dump(outfile, offset + 1);
        }
        outfile << spacer << "}" << std::endl;
    }
};

class cBallistics
{
public:
    int64_t N0000027F; //0x0000
}; //Size: 0x0208


struct offset_info
{
    std::string name;
    uintptr_t offset;
};

bool IsValidPointer(uintptr_t ptr)
{
    if (ptr == 0 || ptr <= 0x100000)
    {
        return false;
    }
    
    // Basic sanity check - check if we can read 1 byte
    uint8_t test;
    return ReadMemory(ptr, test);
}

bool are_floats_equal(float a, float b, float epsilon = 0.001f)
{
    return std::fabs(a - b) < epsilon;
}

bool are_doubles_equal(double a, double b, double epsilon = 0.001)
{
    return std::abs(a - b) < epsilon;
}

namespace cgame
{
    uintptr_t find_unit_count_ptr(uintptr_t cgame, uintptr_t local_unit)
    {
        for (uintptr_t offset = 0x0; offset < 0x1000; offset += 0x1)
        {
            uintptr_t value1;
            if (!ReadMemory(cgame + offset, value1))
                continue;
                
            if (IsValidPointer(value1)) {
                for (uintptr_t offset2 = 0x0; offset2 < 0x1000; offset2 += 0x1)
                {
                    uintptr_t value2;
                    if (!ReadMemory(value1 + offset2, value2))
                        continue;
                        
                    if (IsValidPointer(value2))
                    {
                        if (value2 == local_unit)
                        {
                            return offset;
                        }
                    }
                }
            }
        }
        return 0;
    }
    
    uintptr_t find_camera_ptr(uintptr_t cgame)
    {
        for (uintptr_t offset = 0x500; offset < 0x2000; offset += 0x4)
        {
            uintptr_t addr;
            if (!ReadMemory(cgame + offset, addr))
                continue;
                
            if (IsValidPointer(addr))
            {
                for (uintptr_t offset2 = 0x0; offset2 < 0x1000; offset2 += 0x8)
                {
                    if (!IsValidPointer(addr + offset2))
                        continue;
                    
                    std::string found_text;
                    if (ReadRemoteCStringPtr(addr + offset2, found_text, 0x50))
                    {
                        if (!found_text.empty())
                        {
                            bool invalid_string = false;
                            for (size_t i = 0; i < found_text.size(); i++)
                            {
                                if (!isalpha(found_text[i]) && found_text[i] != '_')
                                {
                                    invalid_string = true;
                                    break;
                                }
                            }
                            if (invalid_string)
                                continue;
                                
                            if (found_text.find("lense_color") != std::string::npos) {
                                return offset;
                            }
                        }
                    }
                }
            }
        }
        return 0;
    }
    
    namespace camera
    {
        uintptr_t find_camera_position_ptr(uintptr_t camera)
        {
            for (uintptr_t offset = 0x0; offset < 0x1000; offset += 0x1)
            {
                Vector3 value;
                if (!ReadMemory(camera + offset, value))
                    continue;
                    
                if (are_floats_equal(value.x, 24566.830f, 0.01f) &&
                    are_floats_equal(value.y, 38.168f, 0.01f) &&
                    are_floats_equal(value.z, 1485.826f, 0.01f))
                {
                    return offset;
                }
            }
            return 0;
        }
        
        uintptr_t find_camera_matrix_ptr(uintptr_t camera)
        {
            for (uintptr_t offset = 0x0; offset < 0x1000; offset += 0x1)
            {
                Vector3 value, value2;
                if (!ReadMemory(camera + offset, value))
                    continue;
                if (!ReadMemory(camera + offset + 0xC, value2))
                    continue;

                std::cout << "------------" << std::endl;
                std::cout << value.x << " " << value.y << " " << value.z << std::endl;
                std::cout << value2.x << " " << value2.y << " " << value2.z << std::endl;
                std::cout << "------------" << std::endl;
                    
                if (are_floats_equal(value.x, -0.063f) &&
                    are_floats_equal(value.y, 0.031f) &&
                    are_floats_equal(value2.x, -0.998f) &&
                    are_floats_equal(value2.z, 1.778f))
                {
                    return offset;
                }
            }
            return 0;
        }
    }
    
    uintptr_t find_ballistics_ptr(uintptr_t cgame)
    {
        for (uintptr_t offset = 0x300; offset < 0x1000; offset += 0x4)
        {
            uintptr_t pointer;
            if (!ReadMemory(cgame + offset, pointer))
                continue;
                
            if (IsValidPointer(pointer))
            {
                if (pointer > 0x1000000 && pointer != 0x10101010000)
                {
                    int64_t val1;
                    if (ReadMemory(pointer + offsetof(cBallistics, N0000027F), val1))
                    {
                        if (val1 == -4294967296)
                        {
                            return offset;
                        }
                    }
                }
            }
        }
        return 0;
    }
    
    namespace ballistics
    {
        uintptr_t find_bombpred_ptr(uintptr_t ballistics)
        {
            for (uintptr_t offset = 0x0000; offset < 0x2000; offset += 0x1)
            {
                Vector3 value;
                if (!ReadMemory(ballistics + offset, value))
                    continue;
                    
                if (are_floats_equal(value.x, 24549.592f) &&
                    are_floats_equal(value.y, 30.420f) &&
                    are_floats_equal(value.z, 1484.755f))
                {
                    return offset;
                }
            }
            return 0;
        }
        
        uintptr_t find_roundvelocity_ptr(uintptr_t ballistics)
        {
            for (uintptr_t offset = 0x1000; offset < 0x2000; offset += 0x4)
            {
                float value;
                if (!ReadMemory(ballistics + offset, value))
                    continue;
                    
                if (are_floats_equal(value, 1030.000f))
                {
                    return offset;
                }
            }
            return 0;
        }
        
        uintptr_t find_roundmass_ptr(uintptr_t ballistics)
        {
            for (uintptr_t offset = 0x1000; offset < 0x2000; offset += 0x4)
            {
                float value;
                if (!ReadMemory(ballistics + offset, value))
                    continue;
                    
                if (are_floats_equal(value, 0.102f))
                {
                    return offset;
                }
            }
            return 0;
        }
        
        uintptr_t find_roundcaliber_ptr(uintptr_t ballistics)
        {
            for (uintptr_t offset = 0x1000; offset < 0x2000; offset += 0x4)
            {
                float value;
                if (!ReadMemory(ballistics + offset, value))
                    continue;
                    
                if (are_floats_equal(value, 0.020f))
                {
                    return offset;
                }
            }
            return 0;
        }
        
        uintptr_t find_roundlength_ptr(uintptr_t ballistics)
        {
            for (uintptr_t offset = 0x1000; offset < 0x2000; offset += 0x4)
            {
                float value;
                if (!ReadMemory(ballistics + offset, value))
                    continue;
                    
                if (are_floats_equal(value, 0.460f))
                {
                    return offset;
                }
            }
            return 0;
        }
        
        uintptr_t find_telecontrol_ptr(uintptr_t ballistics)
        {
            for (uintptr_t offset = 0x0; offset < 0x1000; offset += 0x8)
            {
                std::string found_text;
                if (ReadRemoteCStringPtr(ballistics + offset, found_text, 0x50))
                {
                    if (!found_text.empty() && found_text.find("telecontrol") != std::string::npos)
                    {
                        return offset - (2 * 0x8);
                    }
                }
            }
            return 0;
        }
        
        namespace telecontrol
        {
            uintptr_t find_gameui_ptr(uintptr_t telecontrol)
            {
                for (uintptr_t offset = 0x0; offset < 0x1000; offset += 0x8)
                {
                    uintptr_t found_ptr;
                    if (!ReadMemory(telecontrol + offset, found_ptr))
                        continue;
                        
                    if (IsValidPointer(found_ptr))
                    {
                        std::string found_text;
                        if (ReadRemoteCStringPtr(found_ptr, found_text, 0x50))
                        {
                            if (!found_text.empty() && found_text.find("ui/gameuiskin") != std::string::npos)
                            {
                                return offset + (2 * 0x8);
                            }
                        }
                    }
                }
                return 0;
            }
            
            namespace gameui
            {
                uintptr_t find_mouseposition_ptr(uintptr_t gameui)
                {
                    // Try to get screen resolution from environment or use default
                    int monitor_width = 1920;  // Default
                    int monitor_height = 1080; // Default
                    
                    // Try to read from environment variables
                    const char* display_env = std::getenv("DISPLAY");
                    if (display_env)
                    {
                        // Could implement X11 screen detection here if needed
                        // For now, use defaults
                    }
                    
                    monitor_width /= 2;
                    monitor_height /= 2;
                    std::cout << "Monitor Width: " << monitor_width << " Monitor Height: " << monitor_height << std::endl;
                    
                    for (uintptr_t offset = 0x0; offset < 0x1000; offset += 0x4)
                    {
                        float mouse_x, mouse_y;
                        if (!ReadMemory(gameui + offset, mouse_x))
                            continue;
                        if (!ReadMemory(gameui + offset + 0x4, mouse_y))
                            continue;
                            
                        if (offset == 0xd60)
                        {
                            std::cout << "!!!!!!!! CORRECT !!!!!!" << std::endl;
                            std::cout << "X: " << mouse_x << " Y: " << mouse_y << std::endl;
                        }
                        
                        if (mouse_x > (monitor_width - 1) && mouse_x < (monitor_width + 1))
                        {
                            std::cout << "Found: " << std::hex << offset << std::dec << "\nX: " << mouse_x << " Y: " << mouse_y << std::endl;
                            if (mouse_y > (monitor_width - 100) && mouse_y < (monitor_width + 100))
                                return offset;
                        }
                    }
                    return 0;
                }
            }
        }
    }
}

namespace localplayer
{
    std::vector<offset_info> find_localplayer_offsets(uintptr_t localplayer)
    {
        std::unordered_map<std::string, offset_info> found_offset;
        std::vector<offset_info> offsetinfo;
        
        for (uintptr_t offset = 0x0; offset < 0x4000; offset += 0x1)
        {
            std::string found_text;
            if (ReadRemoteCStringPtr(localplayer + offset, found_text, 0x50))
            {
                if (!found_text.empty() && found_text.size() > 5)
                {
                    bool invalid_string = false;
                    for (size_t i = 0; i < found_text.size(); i++)
                    {
                        if (!isalpha(found_text[i]))
                        {
                            invalid_string = true;
                            break;
                        }
                    }
                    if (invalid_string)
                        continue;
                        
                    if (found_offset.find(found_text) != found_offset.end())
                    {
                        std::cout << "Duplicate: " << found_text << " | " << std::hex << offset + 0x18 << std::dec << std::endl;
                        continue;
                    }
                    
                    offset_info info;
                    info.name = found_text;
                    info.offset = offset + 0x18;
                    found_offset.emplace(found_text, info);
                    offsetinfo.emplace_back(info);
                }
            }
        }
        return offsetinfo;
    }
    
    uintptr_t find_localunit_ptr(uintptr_t localplayer)
    {
        for (uintptr_t offset = 0x0; offset < 0x1000; offset += 0x1)
        {
            std::string found_text;
            if (ReadRemoteCStringPtr(localplayer + offset, found_text, 0x50))
            {
                if (!found_text.empty() && found_text.find("ownedUnitRef") != std::string::npos)
                {
                    return offset + (3 * 0x8);
                }
            }
        }
        return 0;
    }
}

namespace unit
{
    std::vector<offset_info> find_unit_offsets(uintptr_t localunit)
    {
        std::unordered_map<std::string, offset_info> found_offset;
        std::vector<offset_info> offsetinfo;
        
        for (uintptr_t offset = 0x0; offset < 0x4000; offset += 0x1)
        {
            std::string found_text;
            if (ReadRemoteCStringPtr(localunit + offset, found_text, 0x50))
            {
                if (!found_text.empty() && found_text.size() > 5)
                {
                    bool invalid_string = false;
                    for (size_t i = 0; i < found_text.size(); i++)
                    {
                        if (!isalpha(found_text[i]))
                        {
                            invalid_string = true;
                            break;
                        }
                    }
                    if (invalid_string)
                        continue;
                        
                    if (found_offset.find(found_text) != found_offset.end())
                    {
                        std::cout << "Duplicate: " << found_text << " | " << std::hex << offset + 0x18 << std::dec << std::endl;
                        continue;
                    }
                    
                    offset_info info;
                    info.name = found_text;
                    info.offset = offset + 0x18;
                    found_offset.emplace(found_text, info);
                    offsetinfo.emplace_back(info);
                }
            }
        }
        return offsetinfo;
    }
    
    uintptr_t find_bbmin_ptr(uintptr_t localunit)
    {
        for (uintptr_t offset = 0x0; offset < 0x1000; offset += 0x1)
        {
            Vector3 value;
            if (!ReadMemory(localunit + offset, value))
                continue;
                
            if (are_floats_equal(value.x, -7.562f) &&
                are_floats_equal(value.y, -2.739f) &&
                are_floats_equal(value.z, -6.199f))
            {
                return offset;
            }
        }
        return 0;
    }
    
    uintptr_t find_bbmax_ptr(uintptr_t localunit)
    {
        for (uintptr_t offset = 0x0; offset < 0x1000; offset += 0x1)
        {
            Vector3 value;
            if (!ReadMemory(localunit + offset, value))
                continue;
                
            if (are_floats_equal(value.x, 7.571f) && 
                are_floats_equal(value.y, 3.812f) && 
                are_floats_equal(value.z, 6.213f))
            {
                return offset;
            }
        }
        return 0;
    }
    
    uintptr_t find_position_ptr(uintptr_t localunit)
    {
        for (uintptr_t offset = 0x0; offset < 0x2000; offset += 0x1)
        {
            Vector3 value;
            if (!ReadMemory(localunit + offset, value))
                continue;
                
            if (are_floats_equal(value.x, 24549.592f) &&
                are_floats_equal(value.y, 35.843f) &&
                are_floats_equal(value.z, 1484.755f))
            {
                return offset;
            }
        }
        return 0;
    }
    
    uintptr_t find_info_ptr(uintptr_t localunit)
    {
        for (uintptr_t offset = 0x0000; offset < 0x2000; offset += 0x4)
        {
            uintptr_t addr;
            if (!ReadMemory(localunit + offset, addr))
                continue;
                
            if (IsValidPointer(addr))
            {
                for (uintptr_t offset2 = 0x0; offset2 < 0x500; offset2 += 0x4)
                {
                    std::string found_text;
                    if (ReadRemoteCStringPtr(addr + offset2, found_text, 0x50))
                    {
                        if (!found_text.empty() && found_text.find("A-7K") != std::string::npos)
                        {
                            return offset;
                        }
                    }
                }
            }
        }
        return 0;
    }
    
    uintptr_t find_turret_ptr(uintptr_t localunit)
    {
        for (uintptr_t offset = 0x000; offset < 0x2000; offset += 0x4)
        {
            uintptr_t addr;
            if (!ReadMemory(localunit + offset, addr))
                continue;
                
            if (IsValidPointer(addr))
            {
                for (uintptr_t offset2 = 0x0; offset2 < 0x500; offset2 += 0x1)
                {
                    std::string found_text;
                    if (ReadRemoteCStringPtr(addr + offset2, found_text, 0x50))
                    {
                        if (!found_text.empty() && found_text.find("optic1_turret") != std::string::npos)
                        {
                            return offset;
                        }
                    }
                }
            }
        }
        return 0;
    }
    
    namespace turret
    {
        uintptr_t find_weapon_information_ptr(uintptr_t turret)
        {
            for (uintptr_t offset = 0x0; offset < 0x2000; offset += 0x4)
            {
                uintptr_t addr;
                if (!ReadMemory(turret + offset, addr))
                    continue;
                    
                if (IsValidPointer(addr) && addr != 0x959B9E0)
                {
                    for (uintptr_t offset2 = 0x0; offset2 < 0x1000; offset2 += 0x1)
                    {
                        Vector3 value;
                        if (!ReadMemory(addr + offset2, value))
                            continue;
                            
                        if (fabsf(value.x - 1996.494f) <= 10.0f &&
                            fabsf(value.y - 18.718f) <= 10.0f && value.x != 2048)  // +2104.729f потому что z отрицательный
                        {
                            return offset;
                        }
                    }
                }
            }
            return 0;
        }
        
        namespace weapon_information
        {
            uintptr_t find_weapon_position_ptr(uintptr_t weapon_info)
            {
                for (uintptr_t offset = 0x0; offset < 0x1000; offset += 0x1)
                {
                    Vector3 value;
                    if (!ReadMemory(weapon_info + offset, value))
                        continue;
                        
                    if (are_floats_equal(value.x, 24549.592f) &&
                        are_floats_equal(value.y, 35.843f) &&
                        are_floats_equal(value.z, 1484.755f))
                    {
                        return offset;
                    }
                }
                return 0;
            }
        }
    }
    
    uintptr_t find_groundmovement_ptr(uintptr_t localunit)
    {
        for (uintptr_t offset = 0x1500; offset < 0x4000; offset += 0x1)
        {
            std::string found_text;
            if (ReadRemoteCStringPtr(localunit + offset, found_text, 0x50))
            {
                if (!found_text.empty() && found_text.size() > 5)
                {
                    bool invalid_string = false;
                    for (size_t i = 0; i < found_text.size(); i++)
                    {
                        if (!isalpha(found_text[i]))
                        {
                            invalid_string = true;
                            break;
                        }
                    }
                    if (invalid_string)
                        continue;
                        
                    if (!found_text.empty() && found_text.find("timeToRearm") != std::string::npos)
                    {
                        for (uintptr_t offset2 = 0x0; offset2 < 0x1000; offset2 += 0x1)
                        {
                            uint64_t value;
                            if (ReadMemory(localunit + offset + offset2, value))
                            {
                                if (value == 0xBF800000BF800000)
                                {
                                    return offset + offset2;
                                }
                            }
                        }
                    }
                }
            }
        }
        return 0;
    }
    
    uintptr_t find_rotation_matrix_ptr(uintptr_t localunit)
    {
        for (uintptr_t offset = 0x0; offset < 0x1000; offset += 0x1)
        {
            Vector3 value, value2;
            if (!ReadMemory(localunit + offset, value))
                continue;
            if (!ReadMemory(localunit + offset + 0xC, value2))
                continue;
                
            if (are_floats_equal(value.x, -0.993f) &&
                are_floats_equal(value.y, 0.093f) &&
                are_floats_equal(value.z, -0.063f) &&
                are_floats_equal(value2.x, 0.093f) &&
                are_floats_equal(value2.y, 0.995f) &&
                are_floats_equal(value2.z, 0.005f))
            {
                return offset;
            }
        }
        return 0;
    }
    
    namespace airmovement
    {
        uintptr_t find_velocity_ptr(uintptr_t airmovement)
        {
            for (uintptr_t offset = 0x0; offset < 0x2000; offset += 0x1)
            {
                Vector3d value;
                if (!ReadMemory(airmovement + offset, value))
                    continue;
                    
                if (are_doubles_equal(value.x, -0.035) &&
                    are_doubles_equal(value.y, -0.003) &&
                    are_doubles_equal(value.z, -0.002))
                {
                    return offset;
                }
            }
            return 0;
        }
    }
}