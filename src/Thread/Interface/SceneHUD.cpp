#include "SceneHUD.h"
#include "FurnSelectMenu.h"
#include "Elements/AnimSpeedOverlay.h"
#include "Elements/EnjBarsOverlay.h"
#include "Elements/OffsetAdjustPanel.h"
#include "Elements/SceneSelectPanel.h"
#include "Elements/ThreadConfigPanel.h"
#include "Elements/ElementCtrlPanel.h"
#include "Elements/PseudoPanelStack.h"

namespace Thread::Interface
{
    bool SceneHUD::Register()
    {
        if (!SKSEMenuFramework::IsInstalled()) {
            logger::warn("SceneHUD::Register >> SKSE Panel Framework not installed");
            return false;
        }

        winAnimSpeed = SKSEMenuFramework::AddWindow(AnimSpeedOverlay::Render, false);
        winEnjBars = SKSEMenuFramework::AddWindow(EnjBarsOverlay::Render, false);
        winThreadConfig = SKSEMenuFramework::AddWindow(ThreadConfigPanel::Render, false);
        winSceneSelect = SKSEMenuFramework::AddWindow(SceneSelectPanel::Render, false);
        winOffsetAdjust = SKSEMenuFramework::AddWindow(OffsetAdjustPanel::Render, false);
        winElementCtrl = SKSEMenuFramework::AddWindow(ElementCtrlPanel::Render, false);
        winPanelStack = SKSEMenuFramework::AddWindow(PseudoPanelStack::Render, false);

        if (!winAnimSpeed || !winEnjBars || !winThreadConfig || !winSceneSelect ||
            !winOffsetAdjust || !winElementCtrl || !winPanelStack) {
            logger::error("SceneHUD::Register >> one or more AddWindow calls failed");
            return false;
        }

        winAnimSpeed->IsOpen = false;
        winEnjBars->IsOpen = false;
        winThreadConfig->IsOpen = false;
        winSceneSelect->IsOpen = false;
        winOffsetAdjust->IsOpen = false;
        winElementCtrl->IsOpen = false;
        winPanelStack->IsOpen = false;

        if (!FurnSelectMenu::Register()) return false;

        logger::info("SceneHUD::Register >> all UI windows registered");
        return true;
    }

    void SceneHUD::Init(RE::TESQuest* a_qst)
    {
        if (!a_qst) return;
        linkedThread = a_qst;
        threadScript = Script::GetScriptObject(linkedThread, "sslThreadController");
        auto* inst = Instance::GetInstance(linkedThread);
        if (!inst) {
            logger::warn("SceneHUD::Init >> thread instance is null");
            linkedThread = nullptr;
            threadScript = nullptr;
            return;
        }

        if (inst->GetThreadProperty<float>("VarUI_MenuScaleMult") <= 0.0f)
            inst->SetThreadProperty<float>("VarUI_MenuScaleMult", 1.5f);

        ScaleUI::InvalidateScale();
        ScaleUI::RecomputeScale();

        activePanel = IdxTabPanel::kNone;
        focusMode = false;

        AnimSpeedOverlay::Init();
        EnjBarsOverlay::Init();

        logger::info("SceneHUD::Init >> scene UI active");
    }

    void SceneHUD::Destroy()
    {
        if (!linkedThread) return;
        AnimSpeedOverlay::Destroy();
        EnjBarsOverlay::Destroy();
        ThreadConfigPanel::Destroy();
        SceneSelectPanel::Destroy();
        OffsetAdjustPanel::Destroy();
        ElementCtrlPanel::Destroy();
        PseudoPanelStack::Destroy();
        activePanel = IdxTabPanel::kNone;
        linkedThread = nullptr;
        threadScript = nullptr;
        logger::info("SceneHUD::Destroy >> scene UI deactivated");
    }

    void SceneHUD::SetFocus(bool state)
    {
        if (focusMode == state) return;
        focusMode = state;
        winPanelStack->IsOpen = state;
        if (!state) CloseAllPanels();
    }

    void SceneHUD::CloseAllPanels()
    {
        if (activePanel == IdxTabPanel::kThreadConfig) ThreadConfigPanel::Destroy();
        if (activePanel == IdxTabPanel::kSceneSelect) SceneSelectPanel::Destroy();
        if (activePanel == IdxTabPanel::kOffsetAdjust) OffsetAdjustPanel::Destroy();
        if (activePanel == IdxTabPanel::kElementCtrl) ElementCtrlPanel::Destroy();
        activePanel = IdxTabPanel::kNone;
    }

    void SceneHUD::OpenPanel(IdxTabPanel idxPanel)
    {
        CloseAllPanels();
        if (activePanel == idxPanel) return;
        activePanel = idxPanel;
        switch (idxPanel) {
        case IdxTabPanel::kThreadConfig: ThreadConfigPanel::Init(); break;
        case IdxTabPanel::kSceneSelect: SceneSelectPanel::Init(); break;
        case IdxTabPanel::kOffsetAdjust: OffsetAdjustPanel::Init(); break;
        case IdxTabPanel::kElementCtrl: ElementCtrlPanel::Init(); break;
        default: break; }
    }

    void SceneHUD::OnOverlayToggle(IdxHudElement idxElement, bool state)
    {
        switch (idxElement) {
        case IdxHudElement::kAnimSpeedOverlay: state ? AnimSpeedOverlay::Init() : AnimSpeedOverlay::Destroy(); break;
        case IdxHudElement::kEnjBarsOverlay: state ? EnjBarsOverlay::Init() : EnjBarsOverlay::Destroy(); break;
        default: break; }
    }

}  // namespace Thread::SceneHUD
