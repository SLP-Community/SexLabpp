#pragma once

#include <shared_mutex>

namespace Thread::Collision
{
	class CollisionHandler :
	  public Singleton<CollisionHandler>
	{
	  public:
		static void Install();

		static void AddActor(RE::FormID a_actor);
		static void RemoveActor(RE::FormID a_actor);
		static void Clear();

	  private:
		static inline std::shared_mutex _mutex;
		static inline std::vector<RE::FormID> _cache;

		static void Hook_ApplyMovementDelta(RE::Actor* a_actor, float a_delta);
		static inline REL::Relocation<decltype(Hook_ApplyMovementDelta)> _originalApplyMovementDelta;
	};

}  // namespace Thread::Collision
