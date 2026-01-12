#include <iostream>
#include <chrono>
#include <thread>
#include "../sdk/game/classes.hpp"
#include "../sdk/memory/memory.h"
#include "../sdk/math/math.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

bool g_Running = true;
int g_ScreenWidth = 1920;
int g_ScreenHeight = 1080;

void RenderGUI()
{
    uintptr_t localUnit = 0;
    uint8_t localTeamNum;
    Vector3 position{};
    Matrix3x3 rotation{};
    Vector3 bbmin{};
    Vector3 bbmax{};
    float Invulnerable;
    bool isInvulnerable;
    ViewMatrix mat{};
    Vector3 cameraPos{};

    ReadMemory(PLocalUnit, localUnit);
    if (!localUnit)
        return;

    ReadMemory(localUnit + Offset::Unit::Position, position);
    ReadMemory(localUnit + Offset::Unit::RotationMatrix, rotation);
    ReadMemory(localUnit + Offset::Unit::BBMin, bbmin);
    ReadMemory(localUnit + Offset::Unit::BBMax, bbmax);
    ReadMemory(localUnit + Offset::Unit::TeamNum, localTeamNum);
    ReadMemory(CCamera + Offset::Camera::ViewMatrix, mat);
    ReadMemory(CCamera + Offset::Camera::Position, cameraPos);


    uintptr_t unitListEntry;
    unsigned int unitCount;

    ReadMemory(CGame + Offset::Game::UnitList, unitListEntry);
    ReadMemory(CGame + Offset::Game::UnitCount, unitCount);

    uintptr_t pCurrentUnit;
    uintptr_t currentUnit;

    for (unsigned int i = 0; i < unitCount; i++)
    {
        pCurrentUnit = unitListEntry + sizeof(uintptr_t) * i;
        ReadMemory(pCurrentUnit, currentUnit);

        if (currentUnit == 0 && currentUnit == localUnit) continue;

        uint8_t teamNum;
        ReadMemory(currentUnit + Offset::Unit::TeamNum, teamNum);

        if (teamNum == localTeamNum)
            continue;

        uintptr_t unitPlayer;
        ReadMemory(currentUnit + Offset::Unit::Player, unitPlayer);

        if (!unitPlayer)
            continue;
        
        uint8_t playerGuiState;
        ReadMemory(unitPlayer + Offset::Player::GuiState, playerGuiState);

        if (playerGuiState != 2)
            continue;

        uintptr_t unitInfo;
        ReadMemory(currentUnit + Offset::Unit::UnitInfo, unitInfo);

        std::string unitType;
        ReadRemoteCStringPtr(unitInfo + Offset::UnitInfo::UnitType, unitType);

        if (unitType.find("exp_tank") == std::string::npos &&
            unitType.find("exp_SPAA") == std::string::npos &&
            unitType.find("exp_heavy_tank") == std::string::npos) {
            continue;
        }

        std::string nickName;
        std::string unitName;

        uint8_t reloadTimer;

        ReadMemory(currentUnit + Offset::Unit::ReloadTimer, reloadTimer);
        ReadCString(unitPlayer + Offset::Player::Name, nickName);
        ReadRemoteCStringPtr(unitInfo + Offset::UnitInfo::ShortName, unitName);

        ReadMemory(currentUnit + Offset::Unit::Position, position);
        ReadMemory(currentUnit + Offset::Unit::RotationMatrix, rotation);
        ReadMemory(currentUnit + Offset::Unit::BBMin, bbmin);
        ReadMemory(currentUnit + Offset::Unit::BBMax, bbmax);
        ReadMemory(currentUnit + Offset::Unit::IsInvulnerable, isInvulnerable);
        ReadMemory(currentUnit + Offset::Unit::InvulnerableTimer, Invulnerable);

        DrawESP(position, bbmin, bbmax, rotation, mat, Invulnerable, reloadTimer, nickName, unitName);
    }
}

bool InitGUI()
{
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    
    // ✅ OVERLAY НАСТРОЙКИ
    
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);  // Прозрачность
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);               // Без рамок
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);                 // Поверх других окон
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);               // Нельзя изменить размер
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);           // Не забирать фокус
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
    
    auto monitor = glfwGetPrimaryMonitor();
    
    // Получить размер экрана
    int screen_width = 1920;
    int screen_height = 1080;
    if (monitor)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        screen_width = mode->width;
        screen_height = mode->height;
    }

    float main_scale = monitor ? ImGui_ImplGlfw_GetContentScaleForMonitor(monitor) : 1.0f;
    
    // Создать окно БЕЗ fullscreen (nullptr вместо monitor)
    GLFWwindow* window = glfwCreateWindow(screen_width, screen_height, "Overlay", nullptr, nullptr);
    if (window == nullptr)
        return false;

    // Позиция в левом верхнем углу
    glfwSetWindowPos(window, 0, 0);
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.IniFilename = nullptr; // ✅ Не сохранять настройки ImGui

    ImGui::StyleColorsDark();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ✅ ВЫЗОВ ТВОЕЙ ФУНКЦИИ ОТРИСОВКИ
        RenderGUI();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        
        // ✅ ПРОЗРАЧНЫЙ ФОН (важно для overlay)
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return true;
}