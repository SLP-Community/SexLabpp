#pragma once

#include <shared_mutex>
#include <vector>

namespace Thread::Collision
{
    class CollisionHandler final : public Singleton<CollisionHandler>
    {
      public:
        static void Install();
        static void AddActor(RE::FormID a_actor);
        static void RemoveActor(RE::FormID a_actor);
        static void Clear();

        [[nodiscard]] static bool HasActor(RE::FormID a_actor);

      private:
        static void SetActorLocked(RE::Actor* a_actor, bool a_locked);
        static void SetFootIKEnabled(RE::Actor* a_actor, bool a_enabled);

        static void Hook_ApplyMovementDelta(RE::Actor* a_actor, float a_delta);

        static inline REL::Relocation<decltype(Hook_ApplyMovementDelta)> _originalApplyMovementDelta;

        static inline std::vector<RE::FormID> _cache;
        static inline std::shared_mutex _mutex;
    };
}
