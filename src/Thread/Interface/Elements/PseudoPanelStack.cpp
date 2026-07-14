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
        const float tabH   = ScaleUI::pxScale(30.0f);
        const float tabGap = ScaleUI::pxScale(5.0f);
        const float railPad = ScaleUI::pxScale(3.0f);

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
        const float railH = stackH + railPad * 2.0f;

        ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{ dw - tabW - railPad, dh * 0.5f - railH * 0.5f }, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{ tabW + railPad, railH }, ImGuiMCP::ImGuiCond_Always);
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
            ImGuiMCP::SetCursorPos(ImGuiMCP::ImVec2{ railPad, railPad + vi * stride });

            auto* dl = ImGuiMCP::GetWindowDrawList();
            const ImGuiMCP::ImVec2 tMin = ImGuiMCP::GetCursorScreenPos();
            const ImGuiMCP::ImVec2 tMax{ tMin.x + tabW, tMin.y + tabH };

            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Header, IM_COL32(0, 0, 0, 0));
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 0));
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_HeaderActive, IM_COL32(0, 0, 0, 0));
            const bool clicked = ImGuiMCP::Selectable("##slpp_ppsTab", isActiveTab, 0, ImGuiMCP::ImVec2{ tabW, tabH });
            const bool highlighted = ImGuiMCP::IsItemHovered() || ImGuiMCP::IsItemFocused();
            const bool pressed = ImGuiMCP::IsItemActive();

            ImGuiMCP::PopStyleColor(3);

            const float rounding = ScaleUI::pxScale(5.0f);
            const auto roundLeft = ImGuiMCP::ImDrawFlags_RoundCornersLeft;
            const ImGuiMCP::ImU32 background = isActiveTab ? IM_COL32(22, 25, 23, 245) :
                pressed ? IM_COL32(34, 34, 36, 245) : highlighted ? IM_COL32(28, 28, 30, 245) : ColorUI::BgPanel;
            const ImGuiMCP::ImU32 border = isActiveTab ? IM_COL32(112, 184, 112, 220) :
                highlighted ? IM_COL32(145, 142, 136, 150) : IM_COL32(92, 90, 86, 105);
            const ImGuiMCP::ImU32 text = isActiveTab ? ColorUI::TextPrimary :
                highlighted ? ColorUI::TextPrimary : ColorUI::TextSecond;

            ImGuiMCP::ImDrawListManager::AddRectFilled(dl,
                ImGuiMCP::ImVec2{tMin.x - ScaleUI::pxScale(1.0f), tMin.y + ScaleUI::pxScale(1.0f)},
                ImGuiMCP::ImVec2{tMax.x, tMax.y + ScaleUI::pxScale(1.0f)},
                IM_COL32(0, 0, 0, 90), rounding, roundLeft);
            ImGuiMCP::ImDrawListManager::AddRectFilled(dl, tMin, tMax, background, rounding, roundLeft);
            ImGuiMCP::ImDrawListManager::AddRect(dl, tMin, tMax, border, rounding, roundLeft, ScaleUI::pxScale(1.0f));

            const float accentW = ScaleUI::pxScale(isActiveTab ? 3.0f : 1.0f);
            ImGuiMCP::ImDrawListManager::AddLine(dl,
                ImGuiMCP::ImVec2{tMin.x + accentW * 0.5f, tMin.y + rounding},
                ImGuiMCP::ImVec2{tMin.x + accentW * 0.5f, tMax.y - rounding},
                isActiveTab ? ColorUI::BadgeGreen : border, accentW);

            const ImGuiMCP::ImVec2 lblSz = ImGuiMCP::CalcTextSize(kTabs[i].label);
            const ImGuiMCP::ImVec2 lblPos{ tMin.x + ScaleUI::pxScale(13.0f), tMin.y + (tabH - lblSz.y) * 0.5f };
            DrawTextShadowed(dl, lblPos, text, kTabs[i].label);

            if (clicked) {
                if (kTabs[i].IdxPanel == IdxTabPanel::kNone)
                    SceneHUD::SetFocus(false);
                else
                    SceneHUD::OpenPanel(kTabs[i].IdxPanel);
            }

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
