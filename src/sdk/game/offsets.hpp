#pragma once

#include <cstdint>

namespace Offset {
    namespace Game
    {
        const uintptr_t UnitList = 0x340; // pointer
        const uintptr_t UnitCount = 0x350; // unsigned int
        const uintptr_t ListCapacity = 0x354; // unsigned int
        const uintptr_t Ballistics = 0x408; // pointer
        const uintptr_t CameraPtr = 0x5F0; // pointer
    } // namespace Game

    namespace Ballistics {
        const uintptr_t Velocity = 0x1F20; // float
        const uintptr_t Mass = 0x1F2C; // float
        const uintptr_t Length = 0x1F34; // float
        const uintptr_t Caliber = 0x1F30; // float
    }

    namespace Camera {
        const uintptr_t Position = 0x58; // Vector3
        const uintptr_t ViewMatrix = 0x1B8; // Matrix4x4
    }

    namespace Unit
    {
        const uintptr_t InvulnerableTimer = 0xC6C; // float
        const uintptr_t IsInvulnerable = 0xC90; // bool
        const uintptr_t ReloadTimer = 0x8E8; // uint8
        const uintptr_t TeamNum = 0xDE8; // uint8
        const uintptr_t BBMin = 0x230; // Vector3
        const uintptr_t BBMax = 0x23C; // Vector3
        const uintptr_t RotationMatrix = 0xB14; // Matrix3x3
        const uintptr_t Position = 0xB38; // Vector3
        const uintptr_t UnitInfo = 0xDF8; // pointer
        const uintptr_t Player = 0xD70; // pointer
    } // namespace Unit   

    namespace UnitInfo 
    {
        const uintptr_t UnitName = 0x8; // char*
        const uintptr_t ModelName = 0x10; //char*
        const uintptr_t ModelPath = 0x18; //char*
        const uintptr_t FullName = 0x20; //char*
        const uintptr_t ShortName = 0x28; //char*
        const uintptr_t UnitChain = 0x30; //char*
        const uintptr_t UnitType = 0x38; //char*
        const uintptr_t FullName2 = 0x40; //char*
    }

    namespace Player
    {
        const uintptr_t GuiState = 0x6D0; // uint8
        const uintptr_t Unit = 0x8D0; // ptr to Unit, need to sub -1
        const uintptr_t Name = 0xB0;
    }
}