#pragma once

#include "DebugDraw.h"
#include "Thread/Interface/UI/Scale.h"
#include "Thread/Interface/UI/Theme.h"
#include "Thread/Interface/UI/Window.h"
#include "Thread/Thread.h"
#include "Util/Script.h"

#include <memory>

namespace Thread::Interface
{
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
        void OpenPanel(PanelId a_panel);
        void CloseAllPanels();
        void SetRenderEnabled(bool a_enabled) { _renderEnabled = a_enabled; }

        void UpdateStageTimer(float a_duration, float a_timer);
        void UpdateHighlightedPartner(RE::Actor* a_partner);
        void UpdateEnjoyment(RE::Actor* a_actor, float a_enjoyment, RE::BSFixedString a_interactions);
        void RegisterRaiseEnjoymentAttempt(RE::Actor* a_actor, float a_nextTimeCycle);
        void RefreshStageOffsets();
        void RebuildSceneList();

        [[nodiscard]] bool IsActive() const { return _linkedThread != nullptr; }
        [[nodiscard]] bool ShouldRender() const { return IsActive() && !RE::UI::GetSingleton()->GameIsPaused() && _renderEnabled; }
        [[nodiscard]] bool IsFocused() const { return _focused; }
        [[nodiscard]] bool IsPanelOpen(PanelId a_panel) const { return _activePanel == a_panel; }
        [[nodiscard]] RE::TESQuest* GetLinkedThread() const { return _linkedThread; }
        [[nodiscard]] SceneHUD* GetForThread(RE::TESQuest* a_quest) { return a_quest && _linkedThread == a_quest ? this : nullptr; }
        [[nodiscard]] Instance* GetThreadInstance() const { return _linkedThread ? Instance::GetInstance(_linkedThread) : nullptr; }
        [[nodiscard]] const Script::ObjectPtr& GetThreadScript() const { return _threadScript; }
        [[nodiscard]] const Script::CallbackPtr& GetCallback() const { return _callback; }
        [[nodiscard]] UI::Scale& GetScale() { return _scale; }
        [[nodiscard]] DebugDraw& GetDebugDraw() { return _debugDraw; }

      private:
        struct Elements;

        SceneHUD() = default;
        ~SceneHUD();

        static void __stdcall RenderCallback();
        void Render();

        RE::TESQuest* _linkedThread{};
        Script::ObjectPtr _threadScript{};
        Script::CallbackPtr _callback{};
        UI::Scale _scale;
        UI::FrameworkWindow _window;
        DebugDraw _debugDraw;
        std::unique_ptr<Elements> _elements;
        PanelId _activePanel{ PanelId::kNone };
        bool _registered{ false };
        bool _focused{ false };
        bool _renderEnabled{ true };
    };
}
