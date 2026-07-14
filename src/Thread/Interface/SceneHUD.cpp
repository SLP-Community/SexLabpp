#include "SceneHUD.h"
#include "Elements/AnimSpeedOverlay.h"
#include "Elements/ElementCtrlPanel.h"
#include "Elements/EnjBarsOverlay.h"
#include "Elements/OffsetAdjustPanel.h"
#include "Elements/PseudoPanelStack.h"
#include "Elements/SceneSelectPanel.h"
#include "Elements/ThreadConfigPanel.h"
#include "FurnSelectMenu.h"

namespace Thread::Interface
{
    namespace
    {
        struct PanelEntry
        {
            PanelId id;
            UI::Panel* panel;
        };

        const std::array<PanelEntry, 4>& GetPanels()
        {
            static const std::array panels{
                PanelEntry{ PanelId::kThreadConfig, &ThreadConfigPanel::GetSingleton() },
                PanelEntry{ PanelId::kSceneSelect, &SceneSelectPanel::GetSingleton() },
                PanelEntry{ PanelId::kOffsetAdjust, &OffsetAdjustPanel::GetSingleton() },
                PanelEntry{ PanelId::kElementControl, &ElementCtrlPanel::GetSingleton() },
            };
            return panels;
        }

        UI::Panel* FindPanel(PanelId a_id)
        {
            const auto& panels = GetPanels();
            const auto entry = std::ranges::find(panels, a_id, &PanelEntry::id);
            return entry == panels.end() ? nullptr : entry->panel;
        }
    }

    SceneHUD& SceneHUD::GetSingleton()
    {
        static SceneHUD singleton;
        return singleton;
    }

    bool SceneHUD::Register()
    {
        if (!SKSEMenuFramework::IsInstalled()) {
            logger::warn("SceneHUD::Register >> SKSE Menu Framework not installed");
            return false;
        }

        const std::array results{
            AnimSpeedOverlay::GetSingleton().Register(),
            EnjBarsOverlay::GetSingleton().Register(),
            ThreadConfigPanel::GetSingleton().Register(),
            SceneSelectPanel::GetSingleton().Register(),
            OffsetAdjustPanel::GetSingleton().Register(),
            ElementCtrlPanel::GetSingleton().Register(),
            PseudoPanelStack::GetSingleton().Register(),
            FurnSelectMenu::GetSingleton().Register(),
        };

        if (!std::ranges::all_of(results, std::identity{})) {
            logger::error("SceneHUD::Register >> one or more UI windows failed to register");
            return false;
        }

        _registered = true;
        logger::info("SceneHUD::Register >> all UI windows registered");
        return true;
    }

    void SceneHUD::Init(RE::TESQuest* a_quest)
    {
        if (!a_quest)
            return;
        if (!_registered) {
            logger::warn("SceneHUD::Init >> UI windows are not registered");
            return;
        }

        _linkedThread = a_quest;
        _threadScript = Script::GetScriptObject(_linkedThread, "sslThreadController");
        auto* instance = GetThreadInstance();
        if (!instance) {
            logger::warn("SceneHUD::Init >> thread instance is null");
            _linkedThread = nullptr;
            _threadScript = nullptr;
            return;
        }

        float scaleMultiplier = instance->GetThreadProperty<float>("VarUI_MenuScaleMult");
        if (scaleMultiplier <= 0.0f) {
            scaleMultiplier = 1.5f;
            instance->SetThreadProperty<float>("VarUI_MenuScaleMult", scaleMultiplier);
        }
        _scale.SetMultiplier(scaleMultiplier);

        _activePanel = PanelId::kNone;
        _focused = false;
        AnimSpeedOverlay::GetSingleton().Init();
        EnjBarsOverlay::GetSingleton().Init();
        logger::info("SceneHUD::Init >> scene UI active");
    }

    void SceneHUD::Destroy()
    {
        if (!IsActive())
            return;

        SetFocus(false);
        AnimSpeedOverlay::GetSingleton().Destroy();
        EnjBarsOverlay::GetSingleton().Destroy();
        for (const auto& entry : GetPanels())
            entry.panel->Close();
        PseudoPanelStack::GetSingleton().Close();

        _activePanel = PanelId::kNone;
        _linkedThread = nullptr;
        _threadScript = nullptr;
        logger::info("SceneHUD::Destroy >> scene UI deactivated");
    }

    void SceneHUD::SetFocus(bool a_focused)
    {
        if (_focused == a_focused)
            return;

        _focused = a_focused;
        auto& panelStack = PseudoPanelStack::GetSingleton();
        panelStack.SetInputBlocking(a_focused);
        if (a_focused) {
            panelStack.Open();
        } else {
            CloseAllPanels();
            panelStack.Close();
        }
    }

    void SceneHUD::CloseAllPanels()
    {
        if (auto* panel = FindPanel(_activePanel))
            panel->Close();
        _activePanel = PanelId::kNone;
    }

    void SceneHUD::OpenPanel(PanelId a_panel)
    {
        if (_activePanel == a_panel) {
            CloseAllPanels();
            return;
        }

        CloseAllPanels();
        if (auto* panel = FindPanel(a_panel)) {
            _activePanel = a_panel;
            panel->Open();
        }
    }

    void SceneHUD::OnOverlayToggle(HudElement a_element, bool a_visible)
    {
        switch (a_element) {
        case HudElement::kAnimationSpeed:
            a_visible ? AnimSpeedOverlay::GetSingleton().Init() : AnimSpeedOverlay::GetSingleton().Destroy();
            break;
        case HudElement::kEnjoymentBars:
            a_visible ? EnjBarsOverlay::GetSingleton().Init() : EnjBarsOverlay::GetSingleton().Destroy();
            break;
        default:
            break;
        }
    }

    void SceneHUD::UpdateStageTimer(float a_duration, float a_timer)
    {
        AnimSpeedOverlay::GetSingleton().UpdateStageTimer(a_duration, a_timer);
    }

    void SceneHUD::UpdateHighlightedPartner(RE::Actor* a_partner)
    {
        EnjBarsOverlay::GetSingleton().UpdateHighlightedPartner(a_partner);
    }

    void SceneHUD::UpdateEnjoyment(RE::Actor* a_actor, float a_enjoyment, RE::BSFixedString a_interactions)
    {
        EnjBarsOverlay::GetSingleton().UpdateSlider(a_actor, a_enjoyment, a_interactions);
    }

    void SceneHUD::RegisterRaiseEnjoymentAttempt(RE::Actor* a_actor, float a_nextTimeCycle)
    {
        EnjBarsOverlay::GetSingleton().RegisterRaiseEnjAttempt(a_actor, a_nextTimeCycle);
    }

    void SceneHUD::OnStageChanged()
    {
        OffsetAdjustPanel::GetSingleton().OnStageChanged();
    }

    void SceneHUD::RebuildSceneList()
    {
        auto& panel = SceneSelectPanel::GetSingleton();
        panel.RebuildEntries();
        panel.RebuildFilter();
    }
}
