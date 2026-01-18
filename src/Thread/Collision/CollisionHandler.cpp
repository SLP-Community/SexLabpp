#include "CollisionHandler.h"

namespace Thread::Collision
{

	void CollisionHandler::Install()
	{
		// SE: 36359, AE: 37350 Todo: Test VR...
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(36359, 37350) };

		// std::uintptr_t offset = REL::Module::GetRuntime() != REL::Module::Runtime::AE ? 0xF0 : 0xFB;

		auto& trampoline = SKSE::GetTrampoline();
		_originalApplyMovementDelta = trampoline.write_call<5>(target.address() + 0xFB, Hook_ApplyMovementDelta);

		logger::info("CollisionHandler hooks installed.");
	}

	void CollisionHandler::AddActor(RE::FormID a_actor)
	{
		std::unique_lock lock(_mutex);
		if (!std::ranges::contains(_cache, a_actor)) {
			_cache.push_back(a_actor);
		}
	}

	void CollisionHandler::RemoveActor(RE::FormID a_actor)
	{
		std::unique_lock lock(_mutex);
		auto it = std::ranges::find(_cache, a_actor);
		if (it != _cache.end()) {
			_cache.erase(it);
		}
	}

	void CollisionHandler::Clear()
	{
		std::unique_lock lock(_mutex);
		_cache.clear();
	}

	void CollisionHandler::Hook_ApplyMovementDelta(RE::Actor* a_actor, float a_delta)
	{
		if (a_actor) {
			std::shared_lock lock(_mutex);

			if (std::ranges::contains(_cache, a_actor->GetFormID())) {
				return;
			}
		}

		_originalApplyMovementDelta(a_actor, a_delta);
	}

}  // namespace Thread::Collision
