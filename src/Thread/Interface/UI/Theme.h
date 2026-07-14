#pragma once

#include "SKSEMenuFramework.h"

namespace Thread::Interface::UI::Theme
{
    struct Color final
    {
        static constexpr ImGuiMCP::ImU32 textPrimary = IM_COL32(221, 216, 208, 255);
        static constexpr ImGuiMCP::ImU32 textSecondary = IM_COL32(176, 168, 152, 255);
        static constexpr ImGuiMCP::ImU32 textMuted = IM_COL32(136, 128, 120, 255);
        static constexpr ImGuiMCP::ImU32 accent = IM_COL32(112, 184, 112, 255);

        static constexpr ImGuiMCP::ImU32 surfacePanel = IM_COL32(20, 20, 22, 245);
        static constexpr ImGuiMCP::ImU32 surfaceHovered = IM_COL32(36, 36, 40, 245);
        static constexpr ImGuiMCP::ImU32 surfacePressed = IM_COL32(34, 34, 36, 245);
        static constexpr ImGuiMCP::ImU32 surfaceActive = IM_COL32(22, 25, 23, 245);
        static constexpr ImGuiMCP::ImU32 separator = IM_COL32(55, 55, 58, 128);
        static constexpr ImGuiMCP::ImU32 shadow = IM_COL32(0, 0, 0, 210);
        static constexpr ImGuiMCP::ImU32 shadowSoft = IM_COL32(0, 0, 0, 90);
        static constexpr ImGuiMCP::ImU32 transparent = IM_COL32(0, 0, 0, 0);
        static constexpr ImGuiMCP::ImU32 selectionText = IM_COL32(144, 176, 200, 255);
        static constexpr ImGuiMCP::ImU32 selectionFill = IM_COL32(30, 40, 30, 128);
        static constexpr ImGuiMCP::ImU32 cardHeader = IM_COL32(24, 24, 30, 140);
        static constexpr ImGuiMCP::ImU32 cardIdle = IM_COL32(10, 10, 14, 89);

        static constexpr ImGuiMCP::ImU32 borderSubtle = IM_COL32(92, 90, 86, 105);
        static constexpr ImGuiMCP::ImU32 borderHovered = IM_COL32(145, 142, 136, 150);
        static constexpr ImGuiMCP::ImU32 borderActive = IM_COL32(112, 184, 112, 220);
    };

    struct FontSize final
    {
        static constexpr float detail = 7.5f;
        static constexpr float smallText = 8.0f;
        static constexpr float metadata = 8.5f;
        static constexpr float caption = 9.0f;
        static constexpr float compact = 9.5f;
        static constexpr float body = 10.0f;
        static constexpr float overlay = 13.0f;
    };

    struct Spacing final
    {
        static constexpr float xxs = 2.0f;
        static constexpr float xs = 4.0f;
        static constexpr float sm = 6.0f;
        static constexpr float md = 8.0f;
        static constexpr float lg = 12.0f;
        static constexpr float xl = 16.0f;
    };

    struct Geometry final
    {
        static constexpr float roundingSmall = 2.0f;
        static constexpr float roundingPanel = 5.0f;
        static constexpr float borderThin = 1.0f;
        static constexpr float panelTabWidth = 78.0f;
        static constexpr float panelTabGap = 8.0f;
    };

    struct Enjoyment final
    {
        static constexpr ImGuiMCP::ImU32 normalLow = IM_COL32(122, 40, 40, 255);
        static constexpr ImGuiMCP::ImU32 normalHigh = IM_COL32(208, 104, 88, 255);
        static constexpr ImGuiMCP::ImU32 overflowLow = IM_COL32(138, 96, 16, 255);
        static constexpr ImGuiMCP::ImU32 overflowHigh = IM_COL32(232, 184, 64, 255);
        static constexpr ImGuiMCP::ImU32 negativeLow = IM_COL32(30, 58, 88, 255);
        static constexpr ImGuiMCP::ImU32 negativeHigh = IM_COL32(72, 128, 192, 255);
        static constexpr ImGuiMCP::ImU32 zoneIdle = IM_COL32(50, 155, 60, 41);
        static constexpr ImGuiMCP::ImU32 zoneActive = IM_COL32(60, 185, 65, 71);
        static constexpr ImGuiMCP::ImU32 zoneBorder = IM_COL32(90, 200, 70, 128);
        static constexpr ImGuiMCP::ImU32 zoneFocused = IM_COL32(110, 250, 90, 242);
        static constexpr ImGuiMCP::ImU32 needle = IM_COL32(200, 216, 184, 255);
        static constexpr ImGuiMCP::ImU32 needleActive = IM_COL32(144, 248, 120, 255);
        static constexpr ImGuiMCP::ImU32 hit = IM_COL32(96, 204, 80, 255);
        static constexpr ImGuiMCP::ImU32 miss = IM_COL32(224, 96, 80, 255);
        static constexpr ImGuiMCP::ImU32 frameShadow = IM_COL32(0, 0, 0, 150);
        static constexpr ImGuiMCP::ImU32 frameSurface = IM_COL32(16, 16, 18, 255);
        static constexpr ImGuiMCP::ImU32 frameBorder = IM_COL32(40, 40, 48, 255);
        static constexpr ImGuiMCP::ImU32 frameRim = IM_COL32(255, 255, 255, 20);
        static constexpr ImGuiMCP::ImU32 frameShine = IM_COL32(255, 255, 255, 10);
        static constexpr ImGuiMCP::ImU32 zoneCenter = IM_COL32(80, 180, 60, 51);
        static constexpr ImGuiMCP::ImU32 zoneCenterActive = IM_COL32(100, 230, 80, 77);
        static constexpr ImGuiMCP::ImU32 feedbackHit = IM_COL32(60, 200, 80, 89);
        static constexpr ImGuiMCP::ImU32 feedbackMiss = IM_COL32(200, 60, 40, 97);
        static constexpr ImGuiMCP::ImU32 targetBorder = IM_COL32(106, 96, 85, 255);
    };

    struct Offset final
    {
        static constexpr ImGuiMCP::ImU32 fill = IM_COL32(160, 160, 160, 56);
        static constexpr ImGuiMCP::ImU32 needle = IM_COL32(176, 168, 152, 255);
        static constexpr ImGuiMCP::ImU32 needleActive = IM_COL32(221, 216, 208, 255);
        static constexpr ImGuiMCP::ImU32 track = IM_COL32(255, 255, 255, 10);
        static constexpr ImGuiMCP::ImU32 trackBorder = IM_COL32(58, 58, 58, 128);
        static constexpr ImGuiMCP::ImU32 centerTick = IM_COL32(255, 255, 255, 26);
        static constexpr ImGuiMCP::ImU32 separator = IM_COL32(38, 38, 38, 115);
    };

    struct Animation final
    {
        static constexpr ImGuiMCP::ImU32 timerTrack = IM_COL32(10, 10, 12, 178);
        static constexpr ImGuiMCP::ImU32 timerEdge = IM_COL32(255, 255, 255, 38);
        static constexpr ImGuiMCP::ImU32 timerCenter = IM_COL32(255, 255, 255, 217);
    };

    inline ImGuiMCP::ImVec4 ToVec4(ImGuiMCP::ImU32 a_color)
    {
        return {
            static_cast<float>((a_color >> IM_COL32_R_SHIFT) & 0xFF) / 255.0f,
            static_cast<float>((a_color >> IM_COL32_G_SHIFT) & 0xFF) / 255.0f,
            static_cast<float>((a_color >> IM_COL32_B_SHIFT) & 0xFF) / 255.0f,
            static_cast<float>((a_color >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f,
        };
    }
}

namespace Thread::Interface::UI
{
    inline void SetWindowFontSize(float a_size)
    {
        const auto* font = ImGuiMCP::GetFont();
        ImGuiMCP::SetWindowFontScale(font && font->FontSize > 0.0f ? a_size / font->FontSize : 1.0f);
    }

    inline void DrawTextShadowed(ImGuiMCP::ImDrawList* a_drawList, ImGuiMCP::ImVec2 a_position,
        ImGuiMCP::ImU32 a_color, const char* a_text)
    {
        ImGuiMCP::ImDrawListManager::AddText(a_drawList, { a_position.x - 1, a_position.y - 1 }, Theme::Color::shadow, a_text, nullptr);
        ImGuiMCP::ImDrawListManager::AddText(a_drawList, { a_position.x + 1, a_position.y - 1 }, Theme::Color::shadow, a_text, nullptr);
        ImGuiMCP::ImDrawListManager::AddText(a_drawList, { a_position.x - 1, a_position.y + 1 }, Theme::Color::shadow, a_text, nullptr);
        ImGuiMCP::ImDrawListManager::AddText(a_drawList, { a_position.x + 1, a_position.y + 1 }, Theme::Color::shadow, a_text, nullptr);
        ImGuiMCP::ImDrawListManager::AddText(a_drawList, a_position, a_color, a_text, nullptr);
    }
}
