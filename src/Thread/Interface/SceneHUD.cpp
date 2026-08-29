#include "SceneHUD.h"
#include "Elements/AnimSpeedOverlay.h"
#include "Elements/ElementCtrlPanel.h"
#include "Elements/EnjBarsOverlay.h"
#include "Elements/OffsetAdjustPanel.h"
#include "Elements/PseudoPanelStack.h"
#include "Elements/SceneSelectPanel.h"
#include "Elements/ThreadConfigPanel.h"

namespace Thread::Interface
{
    struct SceneHUD::Elements
    {
        AnimSpeedOverlay animSpeedOverlay;
        EnjBarsOverlay enjoymentBarsOverlay;
        ThreadConfigPanel threadConfigPanel;
        SceneSelectPanel sceneSelectPanel;
        OffsetAdjustPanel offsetAdjustPanel;
        ElementCtrlPanel elementCtrlPanel;
        PseudoPanelStack pseudoPanelStack;
    };

    SceneHUD& SceneHUD::GetSingleton()
    {
        static SceneHUD singleton;
        return singleton;
    }

    SceneHUD::~SceneHUD() = default;

    bool SceneHUD::Register()
    {
        if (_registered)
            return true;
        if (!SKSEMenuFramework::IsInstalled()) {
            logger::warn("SceneHUD::Register >> SKSE Menu Framework not installed");
            return false;
        }
        if (!_window.Register(RenderCallback, false)) {
            logger::error("SceneHUD::Register >> AddWindow failed");
            return false;
        }

        _registered = true;
        logger::info("SceneHUD::Register >> UI window registered");
        return true;
    }

    void SceneHUD::Init(RE::TESQuest* a_quest)
    {
        if (!a_quest)
            return;
        if (!_registered) {
            logger::warn("SceneHUD::Init >> UI window is not registered");
            return;
        }
        if (IsActive())
            Destroy();

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
            scaleMultiplier = 1.0f;
            instance->SetThreadProperty<float>("VarUI_MenuScaleMult", scaleMultiplier);
        }
        _scale.SetMultiplier(scaleMultiplier);

        float textScaleMultiplier = instance->GetThreadProperty<float>("VarUI_TextScaleMult");
        if (textScaleMultiplier <= 0.0f) {
            textScaleMultiplier = 1.0f;
            instance->SetThreadProperty<float>("VarUI_TextScaleMult", textScaleMultiplier);
        }
        _scale.SetTextMultiplier(textScaleMultiplier);

        _debugDraw.Clear();
        _activePanel = PanelId::kNone;
        _focused = false;
        _elements = std::make_unique<Elements>();
        _elements->enjoymentBarsOverlay.Init(*instance);
        _window.SetBlocksInput(false);
        _window.Open();
        logger::info("SceneHUD::Init >> scene UI active");
    }

    void SceneHUD::Destroy()
    {
        if (!IsActive() && !_elements)
            return;

        _window.SetBlocksInput(false);
        _window.Close();
        _elements.reset();
        _debugDraw.Clear();
        _activePanel = PanelId::kNone;
        _linkedThread = nullptr;
        _threadScript = nullptr;
        logger::info("SceneHUD::Destroy >> scene UI deactivated");
    }

    void __stdcall SceneHUD::RenderCallback()
    {
        GetSingleton().Render();
    }

    void SceneHUD::Render()
    {
        if (!_elements || !ShouldRender())
            return;
        auto* instance = GetThreadInstance();
        if (!instance)
            return;

        if (instance->GetThreadProperty<bool>("ElementUI_AnimSpeed"))
            _elements->animSpeedOverlay.Render(*this);
        if (instance->GetThreadProperty<bool>("ElementUI_EnjBars") &&
            instance->GetThreadProperty<bool>("VarUI_SeparateOrgasm") &&
            instance->GetThreadProperty<bool>("VarUI_HasPlayer")) {
            _elements->enjoymentBarsOverlay.Render(*this);
        }

        _debugDraw.Render();

        if (!_focused)
            return;

        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_WindowRounding, _scale.Px(UI::Theme::Geometry.roundingPanel));
        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_FrameRounding, _scale.Px(UI::Theme::Geometry.roundingSmall));
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_WindowBg, UI::Theme::Color.panelBackground);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Border, UI::Theme::Color.panelBorder);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Button, UI::Theme::Color.buttonIdle);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ButtonHovered, UI::Theme::Color.buttonHovered);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ButtonActive, UI::Theme::Color.buttonPressed);
        _elements->pseudoPanelStack.Render(*this);
        switch (_activePanel) {
        case PanelId::kThreadConfig:
            _elements->threadConfigPanel.Render(*this);
            break;
        case PanelId::kSceneSelect:
            _elements->sceneSelectPanel.Render(*this);
            break;
        case PanelId::kOffsetAdjust:
            _elements->offsetAdjustPanel.Render(*this);
            break;
        case PanelId::kElementControl:
            _elements->elementCtrlPanel.Render(*this);
            break;
        default:
            break;
        }
        _elements->elementCtrlPanel.RenderThemeEditor(*this);
        ImGuiMCP::PopStyleColor(5);
        ImGuiMCP::PopStyleVar(2);
    }

    void SceneHUD::SetFocus(bool a_focused)
    {
        if (a_focused && (!_elements || !IsActive()))
            return;
        if (_focused == a_focused)
            return;

        _focused = a_focused;
        _window.SetBlocksInput(a_focused);
        if (!a_focused)
            CloseAllPanels();
    }

    void SceneHUD::CloseAllPanels()
    {
        if (_elements) {
            switch (_activePanel) {
            case PanelId::kThreadConfig:
                _elements->threadConfigPanel.Close();
                break;
            case PanelId::kSceneSelect:
                _elements->sceneSelectPanel.Close();
                break;
            case PanelId::kOffsetAdjust:
                _elements->offsetAdjustPanel.Close();
                break;
            case PanelId::kElementControl:
                _elements->elementCtrlPanel.Close();
                break;
            default:
                break;
            }
        }
        _activePanel = PanelId::kNone;
    }

    void SceneHUD::OpenPanel(PanelId a_panel)
    {
        if (!_elements || !_focused)
            return;
        if (_activePanel == a_panel) {
            CloseAllPanels();
            return;
        }

        CloseAllPanels();
        switch (a_panel) {
        case PanelId::kThreadConfig:
            _elements->threadConfigPanel.Open(*this);
            break;
        case PanelId::kSceneSelect:
            _elements->sceneSelectPanel.Open(*this);
            break;
        case PanelId::kOffsetAdjust:
            _elements->offsetAdjustPanel.Open(*this);
            break;
        case PanelId::kElementControl:
            _elements->elementCtrlPanel.Open(*this);
            break;
        default:
            return;
        }
        _activePanel = a_panel;
    }

    void SceneHUD::UpdateStageTimer(float a_duration, float a_timer)
    {
        if (_elements)
            _elements->animSpeedOverlay.UpdateStageTimer(a_duration, a_timer);
    }

    void SceneHUD::UpdateHighlightedPartner(RE::Actor* a_partner)
    {
        if (_elements)
            _elements->enjoymentBarsOverlay.UpdateHighlightedPartner(a_partner);
    }

    void SceneHUD::UpdateEnjoyment(RE::Actor* a_actor, float a_enjoyment, RE::BSFixedString a_interactions)
    {
        if (_elements)
            _elements->enjoymentBarsOverlay.UpdateSlider(a_actor, a_enjoyment, a_interactions);
    }

    void SceneHUD::RegisterRaiseEnjoymentAttempt(RE::Actor* a_actor, float a_nextTimeCycle)
    {
        if (!_elements)
            return;
        auto* instance = GetThreadInstance();
        if (instance && instance->GetThreadProperty<bool>("ElementUI_EnjBars") &&
            instance->GetThreadProperty<bool>("VarUI_SeparateOrgasm")) {
            _elements->enjoymentBarsOverlay.RegisterRaiseEnjAttempt(*this, a_actor, a_nextTimeCycle);
        }
    }

    void SceneHUD::RefreshStageOffsets()
    {
        if (_elements && _activePanel == PanelId::kOffsetAdjust)
            _elements->offsetAdjustPanel.RefreshStageOffsets(*this);
    }

    void SceneHUD::RebuildSceneList()
    {
        if (!_elements)
            return;
        _elements->sceneSelectPanel.RebuildEntries(*this);
        _elements->sceneSelectPanel.RebuildFilter();
    }
}
