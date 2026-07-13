#include "PseudoPanelStack.h"

namespace Thread::Interface
{
    void PseudoPanelStack::Init()
    {
        if (!SceneHUD::winPanelStack) return;
        isVisible = true;
        SceneHUD::winPanelStack->IsOpen = true;
    }

    void PseudoPanelStack::Destroy()
    {
        isVisible = false;
        if (SceneHUD::winPanelStack) {
            SceneHUD::winPanelStack->BlockUserInput = false;
            SceneHUD::winPanelStack->IsOpen = false;
        }
    }

    struct TabDef { IdxTabPanel IdxPanel; const char* label; const char* enableProp; };
    static constexpr TabDef kTabs[5] = {
        { IdxTabPanel::kThreadConfig, "General",  "ElementUI_ThreadConfig" },
        { IdxTabPanel::kSceneSelect,  "Scenes",   "ElementUI_SceneSelect"  },
        { IdxTabPanel::kOffsetAdjust, "Offsets",  "ElementUI_OffsetAdjust" },
        { IdxTabPanel::kElementCtrl,  "Elements",  nullptr },
        { IdxTabPanel::kNone,         "Return",    nullptr },
    };

    void __stdcall PseudoPanelStack::Render()
    {
        // Selectable tabs provide mouse, keyboard, and gamepad access to the focus UI.
        if (!isVisible || !SceneHUD::IsActive()) return;
        if (!SceneHUD::focusMode) return;

        auto* io = ImGuiMCP::GetIO();
        const float dw = io->DisplaySize.x;
        const float dh = io->DisplaySize.y;

        const float tabW   = ScaleUI::pxScale(ScaleUI::PanelTabWidth);
        const float tabH   = ScaleUI::pxScale(26.0f);
        const float tabGap = ScaleUI::pxScale(4.0f);

        constexpr int count = 5;

        // Skip any tab whose overlay has been disabled from the Elements panel;
        // Re-center the remaining ones as a group so there's no gap left behind.
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        std::array<int, count> visible{};
        int visibleCount = 0;
        for (int i = 0; i < count; ++i) {
            const bool enabled = !kTabs[i].enableProp || !inst || inst->GetThreadProperty<bool>(kTabs[i].enableProp);
            if (enabled) visible[visibleCount++] = i;
        }
        if (visibleCount == 0) return;

        const float stride = tabH + tabGap;
        const float stackH = visibleCount * tabH + (visibleCount - 1) * tabGap;

        ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{ dw - tabW, dh * 0.5f - stackH * 0.5f }, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{ tabW, stackH }, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetNextWindowBgAlpha(0.0f);  // each tab paints its own background via style colours

        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
            ImGuiMCP::ImGuiWindowFlags_NoMove     | ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
            ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_NoBackground;

        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_WindowPadding, ImGuiMCP::ImVec2{0.0f, 0.0f});
        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_ItemSpacing, ImGuiMCP::ImVec2{0.0f, 0.0f});
        if (!ImGuiMCP::Begin("##slpp_PanelStack", nullptr, kFlags)) {
            ImGuiMCP::End();
            ImGuiMCP::PopStyleVar(2);
            return;
        }

        SetWindowFontSize(ScaleUI::pxScale(9.0f));

        for (int vi = 0; vi < visibleCount; ++vi) {
            const int i = visible[vi];
            const bool isActiveTab = kTabs[i].IdxPanel != IdxTabPanel::kNone && SceneHUD::IsPanelOpen(kTabs[i].IdxPanel);

            ImGuiMCP::PushID(i);
            ImGuiMCP::SetCursorPos(ImGuiMCP::ImVec2{ 0.0f, vi * stride });

            // Idle background with plain dark tint drawn underneath -> keeps an inactive unhovered tab faintly visible.
            auto* dl = ImGuiMCP::GetWindowDrawList();
            const ImGuiMCP::ImVec2 tMin = ImGuiMCP::GetCursorScreenPos();
            const ImGuiMCP::ImVec2 tMax{ tMin.x + tabW, tMin.y + tabH };
            if (!isActiveTab)
                ImGuiMCP::ImDrawListManager::AddRectFilled(dl, tMin, tMax, ColorUI::BgTab, ScaleUI::pxScale(4.0f), 0);

            // An invisible-label selectable provides the real user interactions like hovering and opening.
            // The visible label is drawn separately on top so it can be centered and shadowed exactly as intended.
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Header, ColorUI::BgTabHover);
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_HeaderHovered, ColorUI::BgTabHover);
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_HeaderActive, ColorUI::BgTabHover);

            if (ImGuiMCP::Selectable("##slpp_ppsTab", isActiveTab, 0, ImGuiMCP::ImVec2{ tabW, tabH })) {
                if (kTabs[i].IdxPanel == IdxTabPanel::kNone)
                    SceneHUD::SetFocus(false);
                else
                    SceneHUD::OpenPanel(kTabs[i].IdxPanel);
            }

            ImGuiMCP::PopStyleColor(3);

            ImGuiMCP::ImDrawListManager::AddRect(dl, tMin, tMax,
                IM_COL32(80, 90, 110, isActiveTab ? 160 : 77), ScaleUI::pxScale(4.0f), 0, 1.0f);

            const ImGuiMCP::ImVec2 lblSz = ImGuiMCP::CalcTextSize(kTabs[i].label);
            const ImGuiMCP::ImVec2 lblPos{ tMin.x + (tabW - lblSz.x) * 0.5f, tMin.y + (tabH - lblSz.y) * 0.5f };
            DrawTextShadowed(dl, lblPos, isActiveTab ? ColorUI::TextPrimary : ColorUI::TextSecond, kTabs[i].label);

            ImGuiMCP::PopID();
        }

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();
        ImGuiMCP::PopStyleVar(2);

        // Close on release so the framework receives the matching key-up before input capture is disabled.
        if (SceneHUD::focusMode && ImGuiMCP::IsKeyReleased(ImGuiMCP::ImGuiKey_Escape))
            SceneHUD::SetFocus(false);
    }

}  // namespace Thread::SceneHUD
