#include "Fragment.h"

#include "Registry/Util/Scale.h"
#include "Registry/Define/Sex.h"

namespace Registry
{
	ActorFragment::Value RaceKeyToValue(RaceKey a) { return ActorFragment::Value(static_cast<ActorFragment::ValueType>(a.value) << 3); }

	ActorFragment::ActorFragment(REX::EnumSet<Sex> a_sex, RaceKey a_race, float a_scale, bool a_vampire, bool a_submissive, bool a_unconscious) :
		actor(nullptr), value(Value::None), scale(a_scale)
	{
		if (a_sex.all(Sex::Male))
			value.set(Male);
		if (a_sex.all(Sex::Female))
			value.set(Female);
		switch (a_race) {
		case RaceKey::None:
			throw std::runtime_error("Cannt build fragment from Actor: Invalid RaceKey");
		case RaceKey::Human:
			value.set(Human);
			if (a_vampire) {
				value.set(Vampire);
			}
			if (a_sex.all(Sex::Futa)) {
				value.set(Futa);
			}
			break;
		default:
			value.set(RaceKeyToValue(a_race));
			break;
		}
		if (a_unconscious)
			value.set(Unconscious);
		if (a_submissive) {
			value.set(Submissive);
		}
	}

	ActorFragment::ActorFragment(RE::Actor* a_actor, bool a_submissive) :
		actor(a_actor), value(Value::None), scale(Scale::GetSingleton()->GetScale(a_actor))
	{
		const auto sex = Registry::GetSex(a_actor);
		switch (sex) {
		case Sex::Female:
			value.set(Female);
			break;
		case Sex::Male:
			value.set(Male);
			break;
		case Sex::Futa:
			value.set(Futa);
			break;
		default:
			throw std::runtime_error(std::format("Cannt build fragment from Actor {:X}: Invalid Sex: {}", a_actor->GetFormID(), std::to_underlying(sex)));
		}
		const RaceKey race{ a_actor };
		switch (race.value) {
		case RaceKey::None:
			throw std::runtime_error(std::format("Cannt build fragment from Actor {:X}: Invalid RaceKey", a_actor->GetFormID()));
		case RaceKey::Human:
			value.set(Human);
			if (a_actor->HasKeywordWithType(RE::DefaultObjectID::kKeywordVampire)) {
				value.set(Vampire);
			}
			break;
		default:
			value.set(RaceKeyToValue(race));
			break;
		}
		// Check for IsUnconcious(), which only exists in ActorState.h
		if (a_submissive, a_actor->IsDead() || a_actor->AsActorState()->IsUnconscious() || a_actor->GetActorValueMax(RE::ActorValue::kVariable05) < 0)	// || a_actor->IsUnconscious())
			value.set(Unconscious);
		if (a_submissive) {
			value.set(Submissive);
		}
	}

	RaceKey ActorFragment::GetRace() const
	{
		if (value.any(Human)) {
			return RaceKey::Human;
		}
		return RaceKey::Value((value.underlying() >> 3) & 0b1111'11);
	}

	REX::EnumSet<Sex> ActorFragment::GetSex() const
	{
		REX::EnumSet<Sex> ret{};
#define SET_SEX(s)  \
	if (value.any(s)) \
		ret.set(Sex::s);
		SET_SEX(Male);
		SET_SEX(Female);
		SET_SEX(Futa);
#undef SET
		return ret;
	}

	int32_t ActorFragment::GetCompatibilityScore(const ActorFragment& a_fragment) const
	{
		int32_t score = 0;
		const auto raceKey = GetRace();
		const auto raceKeyIn = a_fragment.GetRace();
		switch (raceKey) {
		case RaceKey::Canine:
			if (!raceKeyIn.IsAnyOf(raceKey.value, RaceKey::Dog, RaceKey::Wolf, RaceKey::Fox))
				return 0;
			break;
		case RaceKey::BoarAny:
			if (!raceKeyIn.IsAnyOf(raceKey.value, RaceKey::BoarSingle, RaceKey::BoarMounted))
				return 0;
			break;
		case RaceKey::Human:
			if (IsVampire() == a_fragment.IsVampire())
				score += Settings::iWeightVampire;
			__fallthrough;
		default:
			if (raceKeyIn != raceKey)
				return 0;
			break;
		}
		const auto sex = GetSex();
		const auto sexIn = a_fragment.GetSex();
		if (sex.any(sexIn.get())) {
			score += Settings::iWeightSexStrict;
		} else if (sexIn.any(Sex::Female, Sex::Futa)) {
			score += Settings::iWeightSexLight;
		} else {
			score += Settings::iWeightSexMismatch;
		}
		score += IsUnconscious() == a_fragment.IsUnconscious() ? Settings::iWeightUnconscious : 0;
		score += IsSubmissive() == a_fragment.IsSubmissive() ? Settings::iWeightSubmissive : 0;
		score += std::abs(scale - a_fragment.scale) <= Settings::fScaleTolerance ? Settings::iWeightScale : 0;
		return score < Settings::iScoreAcceptThreshold ? 0 : score;
	}

	std::vector<ActorFragment> ActorFragment::Split() const
	{
		if (!IsAbstract()) {
			return { *this };
		}
		constexpr REX::EnumSet<Value> CONFLICT_FREE{ Human }; // , Submissive, Unconscious };
		const auto defaultFragment = value & CONFLICT_FREE;

		const auto raceKey = GetRace();
		std::vector<REX::EnumSet<ActorFragment::Value>> ret{};
		// Add bit to all fragments
		const auto Add = [&](REX::EnumSet<Value> a_bit) {
			for (auto&& it : ret) {
				it |= a_bit;
			}
		};
		// For each fragment, duplicate it and add the bit to the new fragments
		const auto Duplicate = [&](REX::EnumSet<Value> a_bit) {
			const auto max = ret.size();
			ret.reserve(max * 2);
			for (size_t i = 0; i < max; i++) {
				ret.push_back(ret[i] | a_bit);
			}
		};
		// For each fragment, duplicate it once for each item in the list and add the default bit to the originals
		const auto RaceVariant = [&](std::initializer_list<RaceKey> a_list) {
			const auto max = ret.size();
			ret.reserve(max * (a_list.size() + 1));
			for (size_t i = 0; i < max; i++) {
				for (auto&& rk : a_list) {
					ret.push_back(ret[i] | RaceKeyToValue(rk));
				}
				ret[i] |= RaceKeyToValue(raceKey);
			}
		};

		if (value.any(Male))
			ret.push_back(defaultFragment | Value::Male);
		if (value.any(Female))
			ret.push_back(defaultFragment | Value::Female);
		switch (raceKey) {
		case RaceKey::Human:
			if (value.any(Futa))
				ret.push_back(defaultFragment | Value::Futa);
			// if (IsVampire()) {
			// 	Add(Value::Vampire);
			// } else {
				Duplicate(Value::Vampire);
			// }
			break;
		case RaceKey::Canine:
			RaceVariant({ RaceKey::Dog, RaceKey::Wolf });
			break;
		case RaceKey::BoarAny:
			RaceVariant({RaceKey::BoarSingle, RaceKey::BoarMounted});
			break;
		default:
			Add(RaceKeyToValue(GetRace()));
			break;
		}
		Duplicate(Value::Submissive);
		Duplicate(Value::Unconscious);
		return std::ranges::fold_left(ret, std::vector<ActorFragment>{}, [&](auto&& acc, auto&& it) {
			return (acc.emplace_back(it), acc);
		});
	}

	ActorFragment::FragmentHash ActorFragment::MakeFragmentHash(std::vector<ActorFragment> a_fragments)
	{
		assert(a_fragments.size() <= MAX_ACTOR_COUNT);
		std::sort(a_fragments.begin(), a_fragments.end());
		FragmentHash ret = a_fragments.front().value.underlying();
		for (size_t i = 1; i < MAX_ACTOR_COUNT; i++) {
			ret <<= MAX_FRAGMENT_BITS;
			if (i < a_fragments.size()) {
				ret |= a_fragments[i].value.underlying();
			}
		}
		return ret;
	}

	std::vector<ActorFragment> ActorFragment::MakeFragmentList(std::vector<RE::Actor*> a_actors, std::vector<RE::Actor*> a_submissives)
	{
		std::vector<ActorFragment> fragments;
		fragments.reserve(a_actors.size());
		for (auto&& actor : a_actors) {
			fragments.emplace_back(actor, std::ranges::contains(a_submissives, actor));
		}
		return fragments;
	}

}	 // namespace Registry
