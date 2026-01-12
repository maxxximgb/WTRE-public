#include "sdk/game/init.hpp"
#include "gui/init.hpp"

#include <thread>
#include <chrono>

#include "dumper/dumper.hpp"
#include "sdk/classes.h"

int main() {
    if (!InitSDK()) {
        printf("Failed to init SDK!");
        return 1;
    }

    // uintptr_t localUnit;
    // ReadMemory(PLocalUnit, localUnit);
    // uintptr_t turretPtr = unit::find_turret_ptr(localUnit);

    // uintptr_t turret;
    // ReadMemory(localUnit + turretPtr, turret);

    // uintptr_t weaponInfo = unit::turret::find_weapon_information_ptr(turret);
    // std::cout << std::hex << turret << std::endl;
    // std::cout << std::hex << weaponInfo << std::endl;

    // return 0;

    if (!InitGUI()) {
        printf("Failed to init GUI!\n");
        return 1;
    }

    while (true) {
        RenderGUI();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}