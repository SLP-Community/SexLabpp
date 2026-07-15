#include "PseudoPanelStack.h"
#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interface
{
    using UI::DrawTextShadowed;
    using UI::SetWindowFontSize;

    PseudoPanelStack& PseudoPanelStack::GetSingleton()
    {
        static PseudoPanelStack singleton;
        return singleton;
    }

    bool PseudoPanelStack::Register()
    {
        return RegisterWindow(RenderCallback);
    }

    void PseudoPanelStack::Open() { Show(); }

    void PseudoPanelStack::Close()
    {
        SetBlocksInput(false);
        Hide();
    }

    void __stdcall PseudoPanelStack::RenderCallback()
    {
        GetSingleton().Render();
    }

    struct TabDef
    {
        PanelId panel;
        const char* label;
        const char* enableProperty;
    };
    static constexpr std::array kTabs{
        TabDef{ PanelId::kThreadConfig, "General", "ElementUI_ThreadConfig" },
        TabDef{ PanelId::kSceneSelect, "Scenes", "ElementUI_SceneSelect" },
        TabDef{ PanelId::kOffsetAdjust, "Offsets", "ElementUI_OffsetAdjust" },
        TabDef{ PanelId::kElementControl, "Elements", nullptr },
        TabDef{ PanelId::kNone, "Return", nullptr },
    };

    void PseudoPanelStack::Render()
    {
        // Selectable tabs provide mouse, keyboard, and gamepad access to the focus UI.
        auto& hud = SceneHUD::GetSingleton();
        if (!IsVisible() || !hud.IsActive() || !hud.IsFocused())
            return;
        auto& scale = hud.GetScale();

        auto* io = ImGuiMCP::GetIO();
        const float dw = io->DisplaySize.x;
        const float dh = io->DisplaySize.y;

        const float tabW = scale.Px(UI::Theme::Geometry::panelTabWidth);
        const float tabH = std::max(scale.Px(30.0f),
            scale.TextPx(UI::Theme::FontSize::caption) + scale.Px(UI::Theme::Spacing::xs));
        const float tabGap = scale.Px(5.0f);
        const float railPad = scale.Px(3.0f);

        constexpr auto count = kTabs.size();

        // Skip any tab whose overlay has been disabled from the Elements panel;
        // Re-center the remaining ones as a group so there's no gap left behind.
        auto* inst = hud.GetThreadInstance();
        std::array<std::size_t, count> visible{};
        std::size_t visibleCount = 0;
        for (std::size_t i = 0; i < count; ++i) {
            const bool enabled = !kTabs[i].enableProperty || !inst || inst->GetThreadProperty<bool>(kTabs[i].enableProperty);
            if (enabled)
                visible[visibleCount++] = i;
        }
        if (visibleCount == 0)
            return;

        const float stride = tabH + tabGap;
        const float stackH = static_cast<float>(visibleCount) * tabH + static_cast<float>(visibleCount - 1) * tabGap;
        const float railH = stackH + railPad * 2.0f;

        ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{ dw - tabW - railPad, dh * 0.5f - railH * 0.5f }, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{ tabW + railPad, railH }, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetNextWindowBgAlpha(0.0f);  // each tab paints its own background via style colours

        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
            ImGuiMCP::ImGuiWindowFlags_NoMove | ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
            ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_NoBackground;

        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_WindowPadding, ImGuiMCP::ImVec2{ 0.0f, 0.0f });
        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_ItemSpacing, ImGuiMCP::ImVec2{ 0.0f, 0.0f });
        if (!ImGuiMCP::Begin("##slpp_PanelStack", nullptr, kFlags)) {
            ImGuiMCP::End();
            ImGuiMCP::PopStyleVar(2);
            return;
        }

        SetWindowFontSize(scale.TextPx(UI::Theme::FontSize::caption));
        bool closeFocus = false;

        for (std::size_t visibleIndex = 0; visibleIndex < visibleCount; ++visibleIndex) {
            const auto index = visible[visibleIndex];
            const bool isActiveTab = kTabs[index].panel != PanelId::kNone && hud.IsPanelOpen(kTabs[index].panel);

            ImGuiMCP::PushID(static_cast<int>(index));
            ImGuiMCP::SetCursorPos({ railPad, railPad + static_cast<float>(visibleIndex) * stride });

            auto* dl = ImGuiMCP::GetWindowDrawList();
            const ImGuiMCP::ImVec2 tMin = ImGuiMCP::GetCursorScreenPos();
            const ImGuiMCP::ImVec2 tMax{ tMin.x + tabW, tMin.y + tabH };

            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Header, UI::Theme::Color::transparent);
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_HeaderHovered, UI::Theme::Color::transparent);
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_HeaderActive, UI::Theme::Color::transparent);
            const bool clicked = ImGuiMCP::Selectable("##slpp_ppsTab", isActiveTab, 0, ImGuiMCP::ImVec2{ tabW, tabH });
            const bool highlighted = ImGuiMCP::IsItemHovered() || ImGuiMCP::IsItemFocused();
            const bool pressed = ImGuiMCP::IsItemActive();

            ImGuiMCP::PopStyleColor(3);

            const float rounding = scale.Px(UI::Theme::Geometry::roundingPanel);
            const auto roundLeft = ImGuiMCP::ImDrawFlags_RoundCornersLeft;
            const auto background = isActiveTab ? UI::Theme::Color::surfaceActive :
                                    pressed     ? UI::Theme::Color::surfacePressed :
                                    highlighted ? UI::Theme::Color::surfaceHovered :
                                                  UI::Theme::Color::surfacePanel;
            const auto border = isActiveTab ? UI::Theme::Color::borderActive :
                                highlighted ? UI::Theme::Color::borderHovered :
                                              UI::Theme::Color::borderSubtle;
            const auto text = isActiveTab || highlighted ? UI::Theme::Color::textPrimary : UI::Theme::Color::textSecondary;

            ImGuiMCP::ImDrawListManager::AddRectFilled(dl,
                ImGuiMCP::ImVec2{ tMin.x - scale.Px(1.0f), tMin.y + scale.Px(1.0f) },
                ImGuiMCP::ImVec2{ tMax.x, tMax.y + scale.Px(1.0f) },
                UI::Theme::Color::shadowSoft, rounding, roundLeft);
            ImGuiMCP::ImDrawListManager::AddRectFilled(dl, tMin, tMax, background, rounding, roundLeft);
            ImGuiMCP::ImDrawListManager::AddRect(dl, tMin, tMax, border, rounding, roundLeft,
                scale.Px(UI::Theme::Geometry::borderThin));

            const float accentW = scale.Px(isActiveTab ? 3.0f : 1.0f);
            ImGuiMCP::ImDrawListManager::AddLine(dl,
                ImGuiMCP::ImVec2{ tMin.x + accentW * 0.5f, tMin.y + rounding },
                ImGuiMCP::ImVec2{ tMin.x + accentW * 0.5f, tMax.y - rounding },
                isActiveTab ? UI::Theme::Color::accent : border, accentW);

            const ImGuiMCP::ImVec2 lblSz = ImGuiMCP::CalcTextSize(kTabs[index].label);
            const ImGuiMCP::ImVec2 lblPos{ tMin.x + scale.Px(13.0f), tMin.y + (tabH - lblSz.y) * 0.5f };
            DrawTextShadowed(dl, lblPos, text, kTabs[index].label);

            if (clicked) {
                if (kTabs[index].panel == PanelId::kNone)
                    closeFocus = true;
                else
                    hud.OpenPanel(kTabs[index].panel);
            }

            ImGuiMCP::PopID();
        }

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();
        ImGuiMCP::PopStyleVar(2);

        // Close on release so the framework receives the matching key-up before input capture is disabled.
        if (hud.IsFocused() && ImGuiMCP::IsKeyReleased(ImGuiMCP::ImGuiKey_Escape))
            closeFocus = true;
        // (Hotkey bug workaround) Use the controller path so Papyrus hotkey gating and timer state stay synchronized with C++ focus
        if (closeFocus)
            Script::DispatchMethodCall(hud.GetThreadScript(), "ToggleFocusSceneHUD", hud.GetCallback());
    }

}  // namespace Thread::Interface
