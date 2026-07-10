#include "ElementCtrlPanel.h"
#include "Papyrus/SexLabUtil.h"

namespace Thread::Interface
{
    void ElementCtrlPanel::Init()
    {
        isVisible = true;
        SceneHUD::winElementCtrl->IsOpen = true;
    }

    void ElementCtrlPanel::Destroy()
    {
        isVisible = false;
        if (SceneHUD::winElementCtrl) SceneHUD::winElementCtrl->IsOpen = false;
    }

    void ElementCtrlPanel::OnScaleChange(float val)
    {
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (!inst) return;
        inst->SetThreadProperty<float>("VarUI_MenuScaleMult", std::clamp(val, 0.5f, 2.5f));
        ScaleUI::InvalidateScale();
    }

    void __stdcall ElementCtrlPanel::Render()
    {
        if (!isVisible || !SceneHUD::IsActive()) return;
        if (!SceneHUD::IsPanelOpen(IdxTabPanel::kElementCtrl)) return;
        if (!SceneHUD::focusMode) return;

        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (!inst) return;

        auto* io = ImGuiMCP::GetIO();
        ImGuiMCP::SetNextWindowPos(
            ImGuiMCP::ImVec2{io->DisplaySize.x - ScaleUI::pxScale(40.0f), io->DisplaySize.y * 0.5f},
            ImGuiMCP::ImGuiCond_Always, ImGuiMCP::ImVec2{1.0f, 0.5f});
        ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{ScaleUI::pxScale(220.0f), 0.0f}, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(10.0f) / ImGuiMCP::GetFontSize());

        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
            ImGuiMCP::ImGuiWindowFlags_NoMove     | ImGuiMCP::ImGuiWindowFlags_NoCollapse |
            ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize;

        if (!ImGuiMCP::Begin("Elements##slpp_ECM", nullptr, kFlags)) { ImGuiMCP::End(); return; }

        // Scale slider only commits once the slider is released.
        float sAdj = inst->GetThreadProperty<float>("VarUI_MenuScaleMult");
        ImGuiMCP::SetNextItemWidth(-1.0f);
        ImGuiMCP::SliderFloat("##slpp_ecmScale", &sAdj, 0.5f, 2.5f, "Scale %.2fx");
        if (ImGuiMCP::IsItemDeactivatedAfterEdit())
            OnScaleChange(sAdj);

        ImGuiMCP::Separator();

        // "Elements" section
        ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());
        const char* arrow = s_elementSectionOpen ? "\xe2\x96\xbc" : "\xe2\x96\xb2";  // ▼ / ▲
        char hdrBuf[32];
        std::snprintf(hdrBuf, sizeof(hdrBuf), "%s Toggle HUD Elements", arrow);
        if (ImGuiMCP::Selectable(hdrBuf, false, 0, ImGuiMCP::ImVec2{0, ScaleUI::pxScale(22.0f)}))
            s_elementSectionOpen = !s_elementSectionOpen;

        if (s_elementSectionOpen) {
            ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(10.0f) / ImGuiMCP::GetFontSize());

            bool state_gameHud = inst->GetThreadProperty<bool>("ElementUI_GameHUD");
            bool state_AnimSpeed = inst->GetThreadProperty<bool>("ElementUI_AnimSpeed");
            bool state_EnjBars = inst->GetThreadProperty<bool>("ElementUI_EnjBars");

            if (ImGuiMCP::Checkbox("Game HUD", &state_gameHud)) {
                inst->SetThreadProperty<bool>("ElementUI_GameHUD", state_gameHud);
                Papyrus::SexLabUtil::HideElementsGameHUD(nullptr, !state_gameHud);

            } else if (ImGuiMCP::Checkbox("Anim Speed Overlay", &state_AnimSpeed)) {
                inst->SetThreadProperty<bool>("ElementUI_AnimSpeed", state_AnimSpeed);
                SceneHUD::OnOverlayToggle(IdxHudElement::kAnimSpeedOverlay, state_AnimSpeed);

            } else if (ImGuiMCP::Checkbox("Enj Bars Overlay", &state_EnjBars)) {
                inst->SetThreadProperty<bool>("ElementUI_EnjBars", state_EnjBars);
                SceneHUD::OnOverlayToggle(IdxHudElement::kEnjBarsOverlay, state_EnjBars);
            }

            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{0.0f, ScaleUI::pxScale(4.0f)});

            struct Row { const char* label; const char* prop; IdxTabPanel panel; };
            Row panelRows[] = {
                { "Thread Config Panel", "ElementUI_ThreadConfig",  IdxTabPanel::kThreadConfig },
                { "Scene Select Panel",  "ElementUI_SceneSelect",   IdxTabPanel::kSceneSelect  },
                { "Offset Adjust Panel", "ElementUI_OffsetAdjust",  IdxTabPanel::kOffsetAdjust },
            };
            for (auto& row : panelRows) {
                bool state = inst->GetThreadProperty<bool>(row.prop);
                if (ImGuiMCP::Checkbox(row.label, &state)) {
                    inst->SetThreadProperty<bool>(row.prop, state);
                    if (!state && SceneHUD::IsPanelOpen(row.panel))
                        SceneHUD::CloseAllPanels();
                }
            }
        }

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();
    }
}
