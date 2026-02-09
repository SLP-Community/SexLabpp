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
		static void ConfigureControllerForNoCollision(RE::bhkCharacterController* a_controller);
		static void DisableRigidBodyPhysics(RE::Actor* a_actor);
		static void RestoreRigidBodyPhysics(RE::Actor* a_actor);
		static void SetFootIKEnabled(RE::Actor* a_actor, bool a_enabled);

		static bool* IsCollisionEnabled(
		  RE::hkpCollidableCollidableFilter* a_this,
		  bool* a_result,
		  const RE::hkpCollidable* a_collidableA,
		  const RE::hkpCollidable* a_collidableB);

		static void Hook_ApplyMovementDelta(RE::Actor* a_actor, float a_delta);

		static inline REL::Relocation<decltype(IsCollisionEnabled)> _IsCollisionEnabled;
		static inline REL::Relocation<decltype(Hook_ApplyMovementDelta)> _originalApplyMovementDelta;

		static inline std::vector<RE::FormID> _cache;
		static inline std::shared_mutex _mutex;
	};
}
