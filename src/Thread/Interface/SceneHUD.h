#pragma once

#include "SKSEMenuFramework.h"
#include "Thread/Interface/UI/Scale.h"
#include "Thread/Interface/UI/Theme.h"
#include "Thread/Thread.h"
#include "Util/Script.h"

namespace Thread::Interface
{
    enum class HudElement : std::uint8_t
    {
        kAnimationSpeed,
        kEnjoymentBars,
    };

    enum class PanelId : std::uint8_t
    {
        kNone,
        kThreadConfig,
        kSceneSelect,
        kOffsetAdjust,
        kElementControl,
    };

    class SceneHUD final
    {
      public:
        // This integration enters UI lifecycle, update, and render callbacks on the game thread.
        // Component state is therefore intentionally ordinary, non-atomic state.
        static SceneHUD& GetSingleton();

        bool Register();
        void Init(RE::TESQuest* a_quest);
        void Destroy();

        void SetFocus(bool a_focused);
        void ToggleFocus() { SetFocus(!_focused); }
        void OpenPanel(PanelId a_panel);
        void CloseAllPanels();
        void OnOverlayToggle(HudElement a_element, bool a_visible);

        void UpdateStageTimer(float a_duration, float a_timer);
        void UpdateHighlightedPartner(RE::Actor* a_partner);
        void UpdateEnjoyment(RE::Actor* a_actor, float a_enjoyment, RE::BSFixedString a_interactions);
        void RegisterRaiseEnjoymentAttempt(RE::Actor* a_actor, float a_nextTimeCycle);
        void OnStageChanged();
        void RebuildSceneList();

        [[nodiscard]] bool IsActive() const { return _linkedThread != nullptr; }
        [[nodiscard]] bool IsFocused() const { return _focused; }
        [[nodiscard]] bool IsPanelOpen(PanelId a_panel) const { return _activePanel == a_panel; }
        [[nodiscard]] RE::TESQuest* GetLinkedThread() const { return _linkedThread; }
        [[nodiscard]] Instance* GetThreadInstance() const { return _linkedThread ? Instance::GetInstance(_linkedThread) : nullptr; }
        [[nodiscard]] const Script::ObjectPtr& GetThreadScript() const { return _threadScript; }
        [[nodiscard]] const Script::CallbackPtr& GetCallback() const { return _callback; }
        [[nodiscard]] UI::Scale& GetScale() { return _scale; }

      private:
        SceneHUD() = default;

        RE::TESQuest* _linkedThread{};
        Script::ObjectPtr _threadScript{};
        Script::CallbackPtr _callback{};
        UI::Scale _scale;
        PanelId _activePanel{ PanelId::kNone };
        bool _registered{ false };
        bool _focused{ false };
    };

    namespace ScaleUI
    {
        inline float pxScale(float a_units) { return SceneHUD::GetSingleton().GetScale().Px(a_units); }
        inline float pxTextScale(float a_units) { return SceneHUD::GetSingleton().GetScale().TextPx(a_units); }
        inline float pxScaleClamp(float a_minUnits, float a_percent, float a_maxUnits, float a_axisSize)
        {
            return SceneHUD::GetSingleton().GetScale().Clamp(a_minUnits, a_percent, a_maxUnits, a_axisSize);
        }
    }

}
