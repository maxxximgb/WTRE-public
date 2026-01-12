#pragma once
#include <array>
#include <cmath>
#include <utility>
#include <algorithm>
#include "../classes.h"
#include "../../deps/imgui/imgui.h"

static inline bool WorldToScreen(
    const Vector3& in,
    Vector3& out,
    const ViewMatrix& m
) noexcept
{
    int w, h;
    w = 1920;
    h = 1080;
    
    const float clipX = in.x * m[0][0] + in.y * m[1][0] + in.z * m[2][0] + m[3][0];
    const float clipY = in.x * m[0][1] + in.y * m[1][1] + in.z * m[2][1] + m[3][1];
    const float clipZ = in.x * m[0][2] + in.y * m[1][2] + in.z * m[2][2] + m[3][2];
    float clipW = in.x * m[0][3] + in.y * m[1][3] + in.z * m[2][3] + m[3][3];
    
    if (clipW <= 0.001f) return false;
    
    clipW = 1.0f / clipW;
    float nx = clipX * clipW;
    float ny = clipY * clipW;
    float nz = clipZ * clipW;
    
    out.x = (w / 2.0f) * (nx + 1.0f);
    out.y = -(h / 2.0f) * (ny - 1.0f);
    out.z = nz;
    
    return true;
}

void DrawLine(const Vector3& p1, const Vector3& p2, float Invulnerable)
{
    // Определить цвет в зависимости от неуязвимости
    ImU32 color;
    if (Invulnerable > 0.f)
    {
        // Зеленый цвет для неуязвимых (с прозрачностью 78%)
        color = IM_COL32(0, 255, 0, 199);
    }
    else
    {
        // Красный цвет для обычных
        color = IM_COL32(255, 0, 0, 255);
    }
    
    // Получить DrawList для рисования на заднем фоне (без окна)
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    
    // Нарисовать линию
    draw_list->AddLine(
        ImVec2(p1.x, p1.y),
        ImVec2(p2.x, p2.y),
        color,
        2.0f  // Толщина линии
    );
}

static inline void DrawESP(
    const Vector3& position,
    const Vector3& bbmin,
    const Vector3& bbmax,
    const Matrix3x3& rotation,
    const ViewMatrix& viewProj,
    float Invulnerable,
    uint8_t reloadTimer,
    std::string nickName,
    std::string unitName
)
{
    int w = 0, h = 0;
    w = 1920;
    h = 1080;
    if (w <= 0 || h <= 0) return;

    // Вычисление осей бокса
    Vector3 ax[6];
    ax[0] = Vector3{ rotation[0][0], rotation[0][1], rotation[0][2] }.Scale(bbmin.x);
    ax[1] = Vector3{ rotation[1][0], rotation[1][1], rotation[1][2] }.Scale(bbmin.y);
    ax[2] = Vector3{ rotation[2][0], rotation[2][1], rotation[2][2] }.Scale(bbmin.z);
    ax[3] = Vector3{ rotation[0][0], rotation[0][1], rotation[0][2] }.Scale(bbmax.x);
    ax[4] = Vector3{ rotation[1][0], rotation[1][1], rotation[1][2] }.Scale(bbmax.y);
    ax[5] = Vector3{ rotation[2][0], rotation[2][1], rotation[2][2] }.Scale(bbmax.z);

    // Вычисление промежуточных точек
    Vector3 temp[6];
    temp[0] = position + ax[2];
    temp[1] = position + ax[5];
    temp[2] = temp[0] + ax[3];
    temp[3] = temp[1] + ax[3];
    temp[4] = temp[0] + ax[0];
    temp[5] = temp[1] + ax[0];

    // 8 вершин бокса
    Vector3 v[8];
    v[0] = temp[2] + ax[1];
    v[1] = temp[2] + ax[4];
    v[2] = temp[3] + ax[4];
    v[3] = temp[3] + ax[1];
    v[4] = temp[4] + ax[1];
    v[5] = temp[4] + ax[4];
    v[6] = temp[5] + ax[4];
    v[7] = temp[5] + ax[1];

    // Преобразование всех вершин в экранные координаты
    Vector3 screenPoints[8];
    bool allVisible = true;
    for (int i = 0; i < 8; i++)
    {
        if (!WorldToScreen(v[i], screenPoints[i], viewProj))
        {
            allVisible = false;
            break;
        }
    }

    if (!allVisible) return;

    // Рисование 12 рёбер куба
    Vector3 p1, p2;
    for (int i = 0; i < 4; i++)
    {
        // Верхняя грань
        if (WorldToScreen(v[i], p1, viewProj) && WorldToScreen(v[(i + 1) & 3], p2, viewProj))
            DrawLine(p1, p2, Invulnerable);
        // Нижняя грань
        if (WorldToScreen(v[4 + i], p1, viewProj) && WorldToScreen(v[4 + ((i + 1) & 3)], p2, viewProj))
            DrawLine(p1, p2, Invulnerable);
        // Вертикальные рёбра
        if (WorldToScreen(v[i], p1, viewProj) && WorldToScreen(v[4 + i], p2, viewProj))
            DrawLine(p1, p2, Invulnerable);
    }

    // Вычисление 2D bounding box на экране
    float minX = screenPoints[0].x, maxX = screenPoints[0].x;
    float minY = screenPoints[0].y, maxY = screenPoints[0].y;
    for (int i = 1; i < 8; i++)
    {
        if (screenPoints[i].x < minX) minX = screenPoints[i].x;
        if (screenPoints[i].x > maxX) maxX = screenPoints[i].x;
        if (screenPoints[i].y < minY) minY = screenPoints[i].y;
        if (screenPoints[i].y > maxY) maxY = screenPoints[i].y;
    }

    float boxWidth = maxX - minX;
    float boxHeight = maxY - minY;
    auto* drawList = ImGui::GetBackgroundDrawList();

    // ========== НАД КОРОБКОЙ: ник и unitName ==========
    ImVec2 nickSize = ImGui::CalcTextSize(nickName.c_str());
    ImVec2 unitSize = ImGui::CalcTextSize(unitName.c_str());
    
    drawList->AddText(
        ImVec2(minX + boxWidth / 2 - nickSize.x / 2, minY - 30),
        IM_COL32(255, 255, 255, 255),
        nickName.c_str()
    );
    drawList->AddText(
        ImVec2(minX + boxWidth / 2 - unitSize.x / 2, minY - 15),
        IM_COL32(200, 200, 200, 255),
        unitName.c_str()
    );

    // ========== СПРАВА: Invulnerable (полоска + число) ==========
    if (Invulnerable > 0.0f)
    {
        float invBarWidth = 5.0f;
        float invBarHeight = boxHeight;
        float invBarX = maxX + 8;
        float invBarY = minY;

        // Фон полоски
        drawList->AddRectFilled(
            ImVec2(invBarX, invBarY),
            ImVec2(invBarX + invBarWidth, invBarY + invBarHeight),
            IM_COL32(30, 30, 30, 200)
        );

        // Заполнение (предполагаем Invulnerable от 0 до 100)
        float invPercent = std::clamp(Invulnerable / 50.0f, 0.0f, 1.0f);
        ImU32 invColor = IM_COL32(255, 215, 0, 255); // Золотой
        drawList->AddRectFilled(
            ImVec2(invBarX, invBarY + invBarHeight * (1.0f - invPercent)),
            ImVec2(invBarX + invBarWidth, invBarY + invBarHeight),
            invColor
        );

        // Обводка
        drawList->AddRect(
            ImVec2(invBarX, invBarY),
            ImVec2(invBarX + invBarWidth, invBarY + invBarHeight),
            IM_COL32(0, 0, 0, 255)
        );

        // Текст (число)
        char invText[32];
        snprintf(invText, sizeof(invText), "%.0f", Invulnerable);
        ImVec2 invTextSize = ImGui::CalcTextSize(invText);
        drawList->AddText(
            ImVec2(invBarX + invBarWidth + 5, invBarY + invBarHeight / 2 - invTextSize.y / 2),
            IM_COL32(255, 255, 255, 255),
            invText
        );
    }

    // ========== ПОД КОРОБКОЙ: reloadTimer (полоска + число) ==========
    if (reloadTimer > 0)
    {
        float timerBarWidth = boxWidth;
        float timerBarHeight = 5.0f;
        float timerBarX = minX;
        float timerBarY = maxY + 8;

        // Фон полоски
        drawList->AddRectFilled(
            ImVec2(timerBarX, timerBarY),
            ImVec2(timerBarX + timerBarWidth, timerBarY + timerBarHeight),
            IM_COL32(30, 30, 30, 200)
        );

        // Заполнение (предполагаем reloadTimer от 0 до 100, или используйте свой максимум)
        float timerPercent = std::clamp(reloadTimer / 15.0f, 0.0f, 1.0f);
        ImU32 timerColor = IM_COL32(255, 69, 58, 255); // Красный
        drawList->AddRectFilled(
            ImVec2(timerBarX, timerBarY),
            ImVec2(timerBarX + timerBarWidth * timerPercent, timerBarY + timerBarHeight),
            timerColor
        );

        // Обводка
        drawList->AddRect(
            ImVec2(timerBarX, timerBarY),
            ImVec2(timerBarX + timerBarWidth, timerBarY + timerBarHeight),
            IM_COL32(0, 0, 0, 255)
        );
    }
}