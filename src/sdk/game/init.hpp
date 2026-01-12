#include "../memory/memory.h"
#include "../math/math.hpp"
#include "signatures.hpp"
#include "classes.hpp"
#include "offsets.hpp"

#include <cstdint>

bool InitSDK() {
    if (!OpenProcess("aces")) {
        return false;
    }

    uintptr_t ptnCGame = PatternScan(signatures::ptr2CGame);
    if (ptnCGame == 0) 
        return false;

    int32_t disp;
    if (!ReadMemory<int32_t>(ptnCGame + 3, disp))
        return false;

    uintptr_t target = ptnCGame + 7 + (intptr_t)disp;

    if (!ReadMemory<uintptr_t>(target, CGame))
        return false;

    printf("cGame at 0x%lx\n", (unsigned long)CGame);

    if (!ReadMemory(CGame + Offset::Game::CameraPtr, CCamera))
        return false;
    
    if (!ReadMemory(CGame + Offset::Game::Ballistics, CBallistics))
        return false;

    uintptr_t ptnLocalUnit = PatternScan(signatures::ptr2LocalUnit);
    if (ptnLocalUnit == 0)
        return false;

    if (!ReadMemory<int32_t>(ptnLocalUnit + 3, disp))
        return false;

    PLocalUnit = ptnLocalUnit + 7 + (intptr_t)disp;  

    printf("pLocalUnit = 0x%lx\n", (unsigned long)PLocalUnit);

    uintptr_t localUnit;
    if (!ReadMemory(PLocalUnit, localUnit))
        return false;

    printf("localUnit at 0x%lx\n", (unsigned long)localUnit);

    if (!ReadMemory(localUnit + Offset::Unit::Player, CLocalPlayer))
        return false;

    printf("CLocalPlayer at 0x%lx\n", (unsigned long)CLocalPlayer);

    return true;
}