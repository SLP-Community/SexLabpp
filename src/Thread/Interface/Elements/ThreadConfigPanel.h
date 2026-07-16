#pragma once
#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interface
{
    class ThreadConfigPanel final
    {
      public:
        void Open(SceneHUD& a_hud);
        void Close();
        void Render(SceneHUD& a_hud);

      private:
        struct ActorState
        {
            uint32_t formId{};
            bool cardOpen{ true };
        };

        void OnRandomScene(SceneHUD& a_hud);
        void OnMoveScene(SceneHUD& a_hud);
        void OnAutoPlaySet(SceneHUD& a_hud, bool a_state);
        void OnNextPosition(SceneHUD& a_hud, RE::Actor* a_actor);
        void OnSetExpression(SceneHUD& a_hud, RE::Actor* a_actor, const Registry::Expression* a_expression);
        void OnSetVoice(SceneHUD& a_hud, RE::Actor* a_actor, const Registry::Voice* a_voice);
        void OnSetActorAlpha(RE::Actor* a_actor, int a_alpha);

        void RenderActorCard(SceneHUD& a_hud, RE::Actor* a_actor, ActorState& a_state);

        std::vector<ActorState> _actorStates;
        std::vector<RE::Actor*> _sortedActors;

        bool _threadSectionOpen{ true };
        bool _actorsSectionOpen{ true };
    };
}
