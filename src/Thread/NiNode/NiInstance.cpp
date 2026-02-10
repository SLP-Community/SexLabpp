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
		const auto idxA = GetActorIndex(a_idA);
		const auto idxB = GetActorIndex(a_idB);
		if ((a_idA != 0 && idxA == IDX_INVALID) || (a_idB != 0 && idxB == IDX_INVALID)) {
			logger::error("ForEachInteraction: Actor IDs {:X} or {:X} not found", a_idA, a_idB);
			return;
		}

		std::scoped_lock lk{ _m };
		for (auto& [pair, state] : states) {
			auto [first, second] = pair;
			if (idxA != first && idxA != IDX_UNSPECIFIED)
				continue;
			if (idxB != second && idxB != IDX_UNSPECIFIED)
				continue;
			const auto& interactions = a_type != NiType::Type::None ? std::span(&state.interactions[static_cast<size_t>(a_type)], 1) : std::span(state.interactions);
			for (auto& interaction : interactions) {
				callback(positions[first].actor, positions[second].actor, interaction);
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
			if (a.IsSex(Registry::Sex::Female)) {
				state.interactions[NiType::Vaginal] = EvaluateVaginal(mA, mB);
				state.interactions[NiType::Grinding] = EvaluateGrinding(mA, mB);
			}
			state.interactions[NiType::Anal] = EvaluateAnal(mA, mB);
		}
		if (a != b) {
			return;
		}
		state.interactions[NiType::Kissing] = EvaluateKissing(mA, mB);
	}

	void NiInstance::UpdateHysteresis(PairInteractionState& state, float a_timeStamp)
	{
		const float delta = a_timeStamp - state.lastUpdateTime;
		for (auto&& interaction : state.interactions) {
			if (!interaction.descriptor) {
				interaction.active = false;
				interaction.timeActive = 0.0f;
				continue;
			}
			const float confRaw = interaction.descriptor->Predict();
			const float conf = NiMath::Sigmoid(confRaw);
			assert(conf >= 0.0f && conf <= 1.0f);
			const auto doActive = !interaction.active && conf >= Settings::fEnterThreshold;
			const auto doInactive = interaction.active && conf < Settings::fExitThreshold;
			if (doActive || doInactive) {
				interaction.active = doActive;
				interaction.timeActive = 0.0f;
			} else {
				interaction.timeActive += delta;
			}
		}
	}

}  // namespace Thread::NiNode
