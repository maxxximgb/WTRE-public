#pragma once


#include <sys/uio.h>
#include<sys/types.h>
#include <dirent.h>
#include <immintrin.h>

#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "../classes.h"



bool OpenProcess(const char* name);

uintptr_t PatternScan(const char* pattern);

bool ReadCString(std::uint64_t address, std::string& out, size_t maxLen = 256);

bool ReadRemoteCStringPtr(std::uint64_t fieldAddress, std::string& out, size_t maxLen = 256);

template<typename T>
bool ReadMemory(std::uint64_t address, T& dst);

template<typename T>
bool WriteMemory(std::uint64_t address, const T& value);