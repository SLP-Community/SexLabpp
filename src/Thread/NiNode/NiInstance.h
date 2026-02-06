#pragma once

#include "NiActor.h"
#include "NiInteraction.h"
#include "Registry/Define/Animation.h"

namespace Thread::NiNode
{
	class NiInstance
	{
		constexpr static inline int8_t IDX_INVALID = std::numeric_limits<int8_t>::max();
		constexpr static inline int8_t IDX_UNSPECIFIED = -1;

	  public:
		struct PairInteractionState
		{
			std::array<NiInteraction, NiInteraction::NUM_TYPES> interactions{};
			float lastUpdateTime = 0.0f;
		};

	  public:
		NiInstance(const std::vector<RE::Actor*>& a_positions, const Registry::Scene* a_scene);
		~NiInstance() = default;

		void Update(float a_timeStamp);

		/// @brief Iterate interactions matching criteria, callback receives (actorA, actorB, interaction)
		void ForEachInteraction(const std::function<void(RE::ActorPtr, RE::ActorPtr, const NiInteraction&)>& callback,
		  RE::FormID a_idA = 0, RE::FormID a_idB = 0, NiInteraction::Type a_type = NiInteraction::Type::None) const;

		/// @brief Wrapper functions for ForEachInteraction
		std::vector<const NiInteraction*> GetInteractions(RE::FormID a_idA, RE::FormID a_idB, NiInteraction::Type a_type) const;
		std::vector<RE::Actor*> GetInteractionPartners(RE::FormID a_idA, NiInteraction::Type a_type) const;
		std::vector<RE::Actor*> GetInteractionPartnersRev(RE::FormID a_idB, NiInteraction::Type a_type) const;

	  private:
		int8_t GetActorIndex(RE::FormID id) const;

		void EvaluateRuleBased(PairInteractionState& state, const NiActor& a, const NiActor& b) const;
		void UpdateHysteresis(PairInteractionState& state, float currentTime);

	  private:
		std::vector<NiActor> positions;
		std::vector<std::pair<std::pair<int8_t, int8_t>, PairInteractionState>> states;

		mutable std::mutex _m;
	};

}  // namespace Thread::NiNode
