#pragma once
#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interface
{
    class ThreadConfigPanel
    {
      public:
        static void Init();
        static void Destroy();
        static void __stdcall Render();

      private:
        struct ActorState
        {
            uint32_t formId{};
            bool cardOpen{ true };
        };

        static void OnRandomScene();
        static void OnMoveScene();
        static void OnAutoPlaySet(bool state);
        static void OnNextPermutation(RE::Actor* actor);
        static void OnSetExpression(RE::Actor* actor, const Registry::Expression* expr);
        static void OnSetVoice(RE::Actor* actor, const Registry::Voice* voice);
        static void OnSetActorAlpha(RE::Actor* actor, int alphaInt);

        static void RenderActorCard(RE::Actor* actor, ActorState& state);

        inline static bool isVisible{ false };
        inline static std::vector<ActorState> s_actorStates;
        inline static std::vector<RE::Actor*> s_sortedActors;

        inline static bool s_threadSectionOpen{ true };
        inline static bool s_actorsSectionOpen{ true };
    };
}
