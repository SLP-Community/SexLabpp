#include "PseudoPanelStack.h"

namespace Thread::Interface
{
    void PseudoPanelStack::Init()
    {
        isVisible = true;
        SceneHUD::winPanelStack->IsOpen = true;
    }

    void PseudoPanelStack::Destroy()
    {
        isVisible = false;
        if (SceneHUD::winPanelStack) SceneHUD::winPanelStack->IsOpen = false;
    }

    struct TabDef { IdxTabPanel IdxPanel; const char* label; const char* enableProp; };
    static constexpr TabDef kTabs[4] = {
        { IdxTabPanel::kThreadConfig, "General",  "ElementUI_ThreadConfig" },
        { IdxTabPanel::kSceneSelect,  "Scenes",   "ElementUI_SceneSelect"  },
        { IdxTabPanel::kOffsetAdjust, "Offsets",  "ElementUI_OffsetAdjust" },
        { IdxTabPanel::kElementCtrl,  "Elements",  nullptr },
    };

    void __stdcall PseudoPanelStack::Render()
    {
        // This stack contains selectable widgets for the 4 panels/pull-tabs making them
        // accessible through keyboard/gamepad navigation, in addition to mouse hit-testing.
        if (!isVisible || !SceneHUD::IsActive()) return;
        if (!SceneHUD::focusMode) return;

        auto* io = ImGuiMCP::GetIO();
        const float dw = io->DisplaySize.x;
        const float dh = io->DisplaySize.y;

        const float tabW   = ScaleUI::pxScale(28.0f);
        const float tabGap = ScaleUI::pxScale(4.0f);

        constexpr int count = 4;

        // Each tab is sized to fit its label at a fixed font size plus padding;
        // The tallest label sets the height used for all 4 so they line up evenly.
        // This only needs recomputing when the scale factor changes, not every frame.
        static float s_cachedTabH = 0.0f;
        static float s_cachedForFactor = -1.0f;
        const float curFactor = ScaleUI::s_scaleFactor;
        if (s_cachedForFactor != curFactor) {
            ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());
            float maxH = 0.0f;
            for (auto& t : kTabs)
                maxH = std::max(maxH, ImGuiMCP::CalcTextSize(t.label).x);
            ImGuiMCP::SetWindowFontScale(1.0f);
            s_cachedTabH = maxH + ScaleUI::pxScale(20.0f);  // padding: 10s top + 10s bottom
            s_cachedForFactor = curFactor;
        }
        const float tabH = s_cachedTabH;

        // Skip any tab whose overlay has been disabled from the Elements panel;
        // Re-center the remaining ones as a group so there's no gap left behind.
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        std::array<int, 4> visible{};
        int visibleCount = 0;
        for (int i = 0; i < count; ++i) {
            const bool enabled = !inst || inst->GetThreadProperty<bool>(kTabs[i].enableProp) || !kTabs[i].enableProp;
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

        if (!ImGuiMCP::Begin("##slpp_PanelStack", nullptr, kFlags)) { ImGuiMCP::End(); return; }

        ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());

        for (int vi = 0; vi < visibleCount; ++vi) {
            const int i = visible[vi];
            const bool isActiveTab = SceneHUD::IsPanelOpen(kTabs[i].IdxPanel);

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

            if (ImGuiMCP::Selectable("##slpp_ppsTab", isActiveTab, 0, ImGuiMCP::ImVec2{ tabW, tabH }))
                SceneHUD::OpenPanel(kTabs[i].IdxPanel);

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

        // Clicks outside the tabs and outside open panel windows closes the accordion; ESC does the same.
        if (SceneHUD::activePanel != IdxTabPanel::kNone &&
            ImGuiMCP::IsMouseClicked(ImGuiMCP::ImGuiMouseButton_Left) &&
            !ImGuiMCP::IsWindowHovered(ImGuiMCP::ImGuiHoveredFlags_AnyWindow)) {
            SceneHUD::CloseAllPanels();
        }
        if (SceneHUD::activePanel != IdxTabPanel::kNone && ImGuiMCP::IsKeyPressed(ImGuiMCP::ImGuiKey_Escape))
            SceneHUD::CloseAllPanels();
    }

}  // namespace Thread::SceneHUD
