#pragma once
#include "Thread/Interface/SceneHUD.h"
#include "Thread/Interface/UI/Window.h"

namespace Thread::Interface
{
    class ThreadConfigPanel final : public UI::WindowComponent, public UI::Panel
    {
      public:
        static ThreadConfigPanel& GetSingleton();

        bool Register();
        void Open() override;
        void Close() override;

      private:
        ThreadConfigPanel() = default;

        struct ActorState
        {
            uint32_t formId{};
            bool cardOpen{ true };
        };

        static void __stdcall RenderCallback();
        void Render();
        void OnRandomScene();
        void OnMoveScene();
        void OnAutoPlaySet(bool a_state);
        void OnNextPermutation(RE::Actor* a_actor);
        void OnSetExpression(RE::Actor* a_actor, const Registry::Expression* a_expression);
        void OnSetVoice(RE::Actor* a_actor, const Registry::Voice* a_voice);
        void OnSetActorAlpha(RE::Actor* a_actor, int a_alpha);

        void RenderActorCard(RE::Actor* a_actor, ActorState& a_state);

        std::vector<ActorState> _actorStates;
        std::vector<RE::Actor*> _sortedActors;

        bool _threadSectionOpen{ true };
        bool _actorsSectionOpen{ true };
    };
}
