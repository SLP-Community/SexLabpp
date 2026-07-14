#include "ElementCtrlPanel.h"
#include "Papyrus/SexLabUtil.h"
#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interface
{
    using UI::SetWindowFontSize;

    ElementCtrlPanel& ElementCtrlPanel::GetSingleton()
    {
        static ElementCtrlPanel singleton;
        return singleton;
    }

    bool ElementCtrlPanel::Register()
    {
        return RegisterWindow(RenderCallback);
    }

    void ElementCtrlPanel::Open() { Show(); }
    void ElementCtrlPanel::Close() { Hide(); }

    void __stdcall ElementCtrlPanel::RenderCallback()
    {
        GetSingleton().Render();
    }

    void ElementCtrlPanel::OnScaleChange(float a_value)
    {
        auto& hud = SceneHUD::GetSingleton();
        auto* inst = hud.GetThreadInstance();
        if (!inst)
            return;
        const float value = std::clamp(a_value, 0.5f, 2.5f);
        inst->SetThreadProperty<float>("VarUI_MenuScaleMult", value);
        hud.GetScale().SetMultiplier(value);
    }

    void ElementCtrlPanel::Render()
    {
        auto& hud = SceneHUD::GetSingleton();
        if (!IsVisible() || !hud.IsActive() || !hud.IsPanelOpen(PanelId::kElementControl) || !hud.IsFocused())
            return;

        auto* inst = hud.GetThreadInstance();
        if (!inst)
            return;
        auto& scale = hud.GetScale();

        auto* io = ImGuiMCP::GetIO();
        const float panelOffset = scale.Px(UI::Theme::Geometry::panelTabWidth + UI::Theme::Geometry::panelTabGap);
        ImGuiMCP::SetNextWindowPos(
            ImGuiMCP::ImVec2{ io->DisplaySize.x - panelOffset, io->DisplaySize.y * 0.5f },
            ImGuiMCP::ImGuiCond_Always, ImGuiMCP::ImVec2{ 1.0f, 0.5f });
        ImGuiMCP::SetNextWindowSize({ scale.Px(220.0f), 0.0f }, ImGuiMCP::ImGuiCond_Always);

        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
            ImGuiMCP::ImGuiWindowFlags_NoMove | ImGuiMCP::ImGuiWindowFlags_NoCollapse |
            ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize;

        if (!ImGuiMCP::Begin("Elements##slpp_ECM", nullptr, kFlags)) {
            ImGuiMCP::End();
            return;
        }
        SetWindowFontSize(scale.Px(UI::Theme::FontSize::body));

        // Scale slider only commits once the slider is released.
        float sAdj = inst->GetThreadProperty<float>("VarUI_MenuScaleMult");
        ImGuiMCP::SetNextItemWidth(-1.0f);
        ImGuiMCP::SliderFloat("##slpp_ecmScale", &sAdj, 0.5f, 2.5f, "Scale %.2fx");
        if (ImGuiMCP::IsItemDeactivatedAfterEdit())
            OnScaleChange(sAdj);

        ImGuiMCP::Separator();

        // "Elements" section
        SetWindowFontSize(scale.Px(UI::Theme::FontSize::caption));
        const char* arrow = _elementSectionOpen ? "\xe2\x96\xbc" : "\xe2\x96\xb2";  // ▼ / ▲
        char hdrBuf[32];
        std::snprintf(hdrBuf, sizeof(hdrBuf), "%s Toggle HUD Elements", arrow);
        if (ImGuiMCP::Selectable(hdrBuf, false, 0, { 0, scale.Px(22.0f) }))
            _elementSectionOpen = !_elementSectionOpen;

        if (_elementSectionOpen) {
            SetWindowFontSize(scale.Px(UI::Theme::FontSize::body));

            bool state_gameHud = inst->GetThreadProperty<bool>("ElementUI_GameHUD");
            bool state_AnimSpeed = inst->GetThreadProperty<bool>("ElementUI_AnimSpeed");
            bool state_EnjBars = inst->GetThreadProperty<bool>("ElementUI_EnjBars");

            if (ImGuiMCP::Checkbox("Game HUD", &state_gameHud)) {
                inst->SetThreadProperty<bool>("ElementUI_GameHUD", state_gameHud);
                Papyrus::SexLabUtil::HideElementsGameHUD(nullptr, !state_gameHud);

            } else if (ImGuiMCP::Checkbox("Anim Speed Overlay", &state_AnimSpeed)) {
                inst->SetThreadProperty<bool>("ElementUI_AnimSpeed", state_AnimSpeed);
                hud.OnOverlayToggle(HudElement::kAnimationSpeed, state_AnimSpeed);

            } else if (ImGuiMCP::Checkbox("Enj Bars Overlay", &state_EnjBars)) {
                inst->SetThreadProperty<bool>("ElementUI_EnjBars", state_EnjBars);
                hud.OnOverlayToggle(HudElement::kEnjoymentBars, state_EnjBars);
            }

            ImGuiMCP::Dummy({ 0.0f, scale.Px(UI::Theme::Spacing::xs) });

            struct Row
            {
                const char* label;
                const char* property;
                PanelId panel;
            };
            constexpr std::array panelRows{
                Row{ "Thread Config Panel", "ElementUI_ThreadConfig", PanelId::kThreadConfig },
                Row{ "Scene Select Panel", "ElementUI_SceneSelect", PanelId::kSceneSelect },
                Row{ "Offset Adjust Panel", "ElementUI_OffsetAdjust", PanelId::kOffsetAdjust },
            };
            for (const auto& row : panelRows) {
                bool state = inst->GetThreadProperty<bool>(row.property);
                if (ImGuiMCP::Checkbox(row.label, &state)) {
                    inst->SetThreadProperty<bool>(row.property, state);
                    if (!state && hud.IsPanelOpen(row.panel))
                        hud.CloseAllPanels();
                }
            }
        }

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();
    }
}
