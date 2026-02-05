#pragma once

#include "NiInstance.h"

namespace Thread::NiNode
{
	class NiUpdate
	{
	  public:
		static void Install();
		static float GetDeltaTime();

		static std::shared_ptr<NiInstance> Register(RE::FormID a_id, std::vector<RE::Actor*> a_positions, const Registry::Scene* a_scene) noexcept;
		static void Unregister(RE::FormID a_id) noexcept;

	  private:
		static void OnFrameUpdate(RE::PlayerCharacter* a_this);
		static inline REL::Relocation<decltype(OnFrameUpdate)> _OnFrameUpdate;

		static inline float time = 0.0f;
		static inline std::mutex _m{};
		static inline std::vector<std::pair<RE::FormID, std::shared_ptr<NiInstance>>> _instances{};
	};

}  // namespace Thread::NiNode
