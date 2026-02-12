#include "NiInstance.h"

#include "NiInteraction.h"

namespace Thread::NiNode
{
	NiInstance::NiInstance(const std::vector<RE::Actor*>& a_positions, const Registry::Scene* a_scene)
	{
		assert(a_positions.size() <= std::numeric_limits<int8_t>::max());
		positions.reserve(a_positions.size());
		states.reserve(a_positions.size() * (a_positions.size() - 1));
		for (size_t i = 0; i < a_positions.size(); i++) {
			const auto sex = a_scene->GetNthPosition(i)->data.GetSex().get();
			positions.emplace_back(a_positions[i], sex);
			for (size_t n = 0; n < a_positions.size(); n++) {
				states.emplace_back(
				  std::make_pair(static_cast<int8_t>(i), static_cast<int8_t>(n)),
				  PairInteractionState{});
			}
		}
	}

	void NiInstance::Update(float a_timeStamp)
	{
		std::unique_lock lk{ _m, std::defer_lock };
		if (!lk.try_lock()) {
			return;
		}

		for (auto& niActor : positions) {
			niActor.CaptureSnapshot(a_timeStamp);
		}

		for (auto& [pair, state] : states) {
			auto& posA = positions[pair.first];
			auto& posB = positions[pair.second];

			EvaluateRuleBased(state, posA, posB);
			UpdateHysteresis(state, a_timeStamp);
			state.lastUpdateTime = a_timeStamp;
		}
	}

	std::vector<const NiInteraction*> NiInstance::GetInteractions(RE::FormID a_idA, RE::FormID a_idB, NiType::Type a_type) const
	{
		std::vector<const NiInteraction*> ret{};
		ForEachInteraction([&](RE::ActorPtr, RE::ActorPtr, const NiInteraction& interaction) {
			if (interaction.active) ret.push_back(&interaction);
		},
		  a_idA, a_idB, a_type);
		return ret;
	}

	std::vector<RE::Actor*> NiInstance::GetInteractionPartners(RE::FormID a_idA, NiType::Type a_type) const
	{
		std::vector<RE::Actor*> ret{};
		ForEachInteraction([&](RE::ActorPtr, RE::ActorPtr b, const NiInteraction& interaction) {
			if (!interaction.active) return;
			if (std::ranges::contains(ret, b.get()))
				return;
			ret.push_back(b.get());
		},
		  a_idA, 0, a_type);
		return ret;
	}

	std::vector<RE::Actor*> NiInstance::GetInteractionPartnersRev(RE::FormID a_idB, NiType::Type a_type) const
	{
		std::vector<RE::Actor*> ret{};
		ForEachInteraction([&](RE::ActorPtr a, RE::ActorPtr, const NiInteraction& interaction) {
			if (!interaction.active) return;
			if (std::ranges::contains(ret, a.get()))
				return;
			ret.push_back(a.get());
		},
		  0, a_idB, a_type);
		return ret;
	}

	void NiInstance::ForEachInteraction(
	  const std::function<void(RE::ActorPtr, RE::ActorPtr, const NiInteraction&)>& callback,
	  RE::FormID a_idA,
	  RE::FormID a_idB,
	  NiType::Type a_type) const
	{
		ForEachCluster([&](RE::ActorPtr a, RE::ActorPtr b, const NiInteractionCluster& cluster) {
			for (const auto& interaction : cluster.interactions) {
				if (interaction.GetType() != a_type && a_type != NiType::Type::None)
					continue;
				callback(a, b, interaction);
			}
		},
		  a_idA, a_idB, a_type != NiType::Type::None ? NiType::GetClusterForType(a_type) : NiType::Cluster::None);
	}

	void NiInstance::ForEachCluster(const std::function<void(RE::ActorPtr, RE::ActorPtr, const NiInteractionCluster&)>& callback,
	  RE::FormID a_idA, RE::FormID a_idB, NiType::Cluster a_cluster) const
	{
		const auto idxA = GetActorIndex(a_idA);
		const auto idxB = GetActorIndex(a_idB);
		if ((a_idA != 0 && idxA == IDX_INVALID) || (a_idB != 0 && idxB == IDX_INVALID)) {
			logger::error("ForEachCluster: Actor IDs {:X} or {:X} not found", a_idA, a_idB);
			return;
		}

		std::scoped_lock lk{ _m };
		for (auto& [pair, state] : states) {
			auto [first, second] = pair;
			if (idxA != first && idxA != IDX_UNSPECIFIED)
				continue;
			if (idxB != second && idxB != IDX_UNSPECIFIED)
				continue;
			const auto& clusters =
			  a_cluster != NiType::Cluster::None ?
				std::span(&state.interactionClusters[static_cast<size_t>(a_cluster)], 1) :
				std::span(state.interactionClusters);
			for (auto& cluster : clusters) {
				callback(positions[first].actor, positions[second].actor, cluster);
			}
		}
	}

	int8_t NiInstance::GetActorIndex(RE::FormID id) const
	{
		if (id == 0) {
			return IDX_UNSPECIFIED;
		}
		const auto it = std::ranges::find(positions, id, [](const auto& actor) { return actor.actor->GetFormID(); });
		if (it == positions.end()) {
			logger::error("GetActorIndex: Actor ID {:X} not found", id);
			return IDX_INVALID;
		}
		return static_cast<int8_t>(std::distance(positions.begin(), it));
	}

	void NiInstance::EvaluateRuleBased(PairInteractionState& state, const NiActor& a, const NiActor& b) const
	{
		const auto& mA = a.Motion();
		const auto& mB = b.Motion();
		if (!mA.HasSufficientData() || !mB.HasSufficientData())
			return;

		if (b.IsSex(Registry::Sex::Male)) {
			state.interactionClusters[static_cast<size_t>(NiType::Cluster::Crotch)] = EvaluateCrotchInteractions(mA, mB);
			state.interactionClusters[static_cast<size_t>(NiType::Cluster::Head)] = EvaluateHeadInteractions(mA, mB);
		}
		if (a == b) {
			return;
		}
		state.interactionClusters[static_cast<size_t>(NiType::Cluster::KissingCl)] = EvaluateKissingCluster(mA, mB);
	}

	void NiInstance::UpdateHysteresis(PairInteractionState&, float)
	{
		// const float delta = a_timeStamp - state.lastUpdateTime;
		// TODO: impl
		// for (auto&& interaction : state.interactions) {
		// 	if (!interaction.descriptor) {
		// 		interaction.active = false;
		// 		interaction.timeActive = 0.0f;
		// 		continue;
		// 	}
		// 	const float confRaw = interaction.descriptor->Predict();
		// 	const float conf = NiMath::Sigmoid(confRaw);
		// 	assert(conf >= 0.0f && conf <= 1.0f);
		// 	const auto doActive = !interaction.active && conf >= Settings::fEnterThreshold;
		// 	const auto doInactive = interaction.active && conf < Settings::fExitThreshold;
		// 	if (doActive || doInactive) {
		// 		interaction.active = doActive;
		// 		interaction.timeActive = 0.0f;
		// 	} else {
		// 		interaction.timeActive += delta;
		// 	}
		// }
	}

}  // namespace Thread::NiNode
