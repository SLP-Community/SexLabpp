#include "Stats.h"

#include "Registry/Define/RaceKey.h"
#include "Registry/Define/Sex.h"

namespace Registry::Statistics
{
	ActorStats::ActorStats(RE::Actor* owner) :
		_stats(StatisticID::Total)
	{
		const auto base = owner->GetActorBase();
		if (const auto relations = base ? base->relationships : nullptr) {
			Registry::Sexuality s = Sexuality::None;
			for (auto&& rs : *relations) {
				if (!rs)
					continue;
				if (!rs->level.all(RE::BGSRelationship::RELATIONSHIP_LEVEL::kLover))
					continue;
				const auto other = rs->npc1 == base ? rs->npc2 : rs->npc1;
				if (!other)
					continue;
				bool samesex = base->GetSex() == other->GetSex();
				switch (s) {
				case Sexuality::Hetero:
					if (samesex) {
						s = Sexuality::Bi;
						break;
					}
					continue;	 // Next iteration
				case Sexuality::Homo:
					if (!samesex) {
						s = Sexuality::Bi;
						break;
					}
					continue;
				default:
					s = samesex ? Sexuality::Homo : Sexuality::Hetero;
					continue;
				}
				break;
			}
			if (s != Sexuality::Bi && s != Sexuality::None) {
				const auto chance = s == Sexuality::Hetero ?
															Settings::fPercentageHetero + (Settings::fPercentageHomo / 2) :
															Settings::fPercentageHomo + (Settings::fPercentageHetero / 2);
				if (chance >= Random::draw<float>(0, 99)) {
					s = Sexuality::Bi;
				}
			}
			switch (s) {
			case Sexuality::Bi:
				_stats[StatisticID::Sexuality] = owner->IsPlayerRef() ? 75.0f : Random::draw(Settings::fPercentageHomo, 100.0f - Settings::fPercentageHetero);
				break;
			case Sexuality::Hetero:
				_stats[StatisticID::Sexuality] = Random::draw(100.0f - Settings::fPercentageHetero, 100.0f);
				break;
			case Sexuality::Homo:
				_stats[StatisticID::Sexuality] = Random::draw(1.0f, Settings::fPercentageHomo);
				break;
			default:
				_stats[StatisticID::Sexuality] = Random::draw(1.0f, 100.0f);
				break;
			}
		} else {
			_stats[StatisticID::Sexuality] = owner->IsPlayerRef() ? 75.0f : Random::draw(1.0f, 100.0f);
		}
	}

	ActorStats::ActorStats(SKSE::SerializationInterface* a_intfc) :
		_stats(StatisticID::Total), _custom({})
	{
		for (size_t i = 0; i < StatisticID::Total; i++) {
			a_intfc->ReadRecordData(_stats[i]);
		}
		size_t numRegs;
		a_intfc->ReadRecordData(numRegs);

		std::string key;
		size_t alternative;
		for (size_t i = 0; i < numRegs; i++) {
			stl::read_string(a_intfc, key);
			a_intfc->ReadRecordData(alternative);
			if (alternative == 0) {
				float obj;
				a_intfc->ReadRecordData(obj);
				_custom[key] = obj;
			} else {
				std::string obj;
				stl::read_string(a_intfc, obj);
				_custom[key] = obj;
			}
		}
	}

	void ActorStats::SetStatistic(StatisticID key, float value)
	{
		_stats[key] = value;
	}

	void ActorStats::AddStatistic(StatisticID key, float value)
	{
		_stats[key] += value;
	}

	float ActorStats::GetStatistic(StatisticID key) const
	{
		return _stats[key];
	}

	std::vector<RE::BSFixedString> ActorStats::GetEveryCustomID() const
	{
		const auto v = std::views::keys(_custom);
		return std::vector<RE::BSFixedString>{ v.begin(), v.end() };
	}

	std::optional<float> ActorStats::GetCustomFlt(const RE::BSFixedString& key) const
	{
		return GetCustom<float>(key);
	}

	std::optional<RE::BSFixedString> ActorStats::GetCustomStr(const RE::BSFixedString& key) const
	{
		return GetCustom<RE::BSFixedString>(key);
	}

	void ActorStats::SetCustomFlt(const RE::BSFixedString& key, float value)
	{
		_custom[key] = value;
	}

	void ActorStats::SetCustomStr(const RE::BSFixedString& key, RE::BSFixedString value)
	{
		_custom[key] = value;
	}

	bool ActorStats::HasCustom(const RE::BSFixedString& key) const
	{
		return _custom.contains(key);
	}

	void ActorStats::RemoveCustomStat(const RE::BSFixedString& key)
	{
		_custom.erase(key);
	}

	void ActorStats::Save(SKSE::SerializationInterface* a_intfc)
	{
		for (auto&& stat : _stats) {
			if (!a_intfc->WriteRecordData(stat)) {
				logger::error("Failed to save statistic");
				continue;
			}
		}
		if (!a_intfc->WriteRecordData(_custom.size())) {
			logger::error("Failed to save number of custom statistics ({})", _custom.size());
			return;
		}
		for (auto&& [id, stat] : _custom) {
			if (!stl::write_string(a_intfc, id)) {
				logger::error("Failed to save custom statistic {}", id);
				continue;
			}
			if (std::holds_alternative<float>(stat)) {
				a_intfc->WriteRecordData(0);
				if (!a_intfc->WriteRecordData(stat)) {
					logger::error("Failed to save custom statistic {}", id);
					continue;
				}
			} else {
				a_intfc->WriteRecordData(1);
				auto save = std::get<RE::BSFixedString>(stat);
				if (!stl::write_string(a_intfc, save)) {
					logger::error("Failed to save custom statistic {}", id);
					continue;
				}
			}
		}
	}

	ActorEncounter::EncounterObj::EncounterObj(RE::Actor* obj) :
		id(obj->GetFormID()), sex(Registry::GetSex(obj)) {}

	ActorEncounter::EncounterObj::EncounterObj(SKSE::SerializationInterface* a_intfc)
	{
		a_intfc->ReadRecordData(id);
		a_intfc->ReadRecordData(race);
		a_intfc->ReadRecordData(sex);
	}

	void ActorEncounter::EncounterObj::Save(SKSE::SerializationInterface* a_intfc)
	{
		a_intfc->WriteRecordData(id);
		a_intfc->WriteRecordData(race);
		a_intfc->WriteRecordData(sex);
	}

	ActorEncounter::ActorEncounter(RE::Actor* fst, RE::Actor* snd, EncounterType a_type) :
		npc1(fst), npc2(snd), _lastmet(0), _timesmet(0), _timesaggressor(0), _timesvictim(0)
	{
		Update(a_type);
	}

	ActorEncounter::ActorEncounter(SKSE::SerializationInterface* a_intfc) :
		npc1(a_intfc), npc2(a_intfc)
	{
		if (!a_intfc->ResolveFormID(npc1.id, npc1.id) || !a_intfc->ResolveFormID(npc2.id, npc2.id)) {
			throw std::exception("Unable to update encounter formid");
		}
		a_intfc->ReadRecordData(_lastmet);
		a_intfc->ReadRecordData(_timesmet);
		a_intfc->ReadRecordData(_timesaggressor);
		a_intfc->ReadRecordData(_timesvictim);
		a_intfc->ReadRecordData(_timesdominant);
		a_intfc->ReadRecordData(_timessubmissive);
	}

	const ActorEncounter::EncounterObj* ActorEncounter::GetPartner(RE::Actor* a_actor) const
	{
		if (a_actor->formID == npc1.id)
			return &npc2;
		if (a_actor->formID == npc2.id)
			return &npc1;
		return nullptr;
	}

	uint8_t ActorEncounter::GetTimesSubmissive(RE::FormID a_id) const
	{
		return a_id == npc1.id ? _timessubmissive :
					 a_id == npc2.id ? _timesdominant :
														 0;
	}

	uint8_t ActorEncounter::GetTimesDominant(RE::FormID a_id) const
	{
		return a_id == npc1.id ? _timesdominant :
					 a_id == npc2.id ? _timessubmissive :
														 0;
	}

	uint8_t ActorEncounter::GetTimesVictim(RE::FormID a_id) const
	{
		return a_id == npc1.id ? _timesvictim :
					 a_id == npc2.id ? _timesaggressor :
														 0;
	}

	uint8_t ActorEncounter::GetTimesAssailant(RE::FormID a_id) const
	{
		return a_id == npc1.id ? _timesaggressor :
					 a_id == npc2.id ? _timesvictim :
														 0;
	}

	void ActorEncounter::Update(EncounterType a_type)
	{
		_lastmet = RE::Calendar::GetSingleton()->GetCurrentGameTime();
		_timesmet++;
		switch (a_type) {
		case EncounterType::Aggressor:
			_timesaggressor++;
			__fallthrough;
		case EncounterType::Dominant:
			_timesdominant++;
			break;
		case EncounterType::Victim:
			_timesvictim++;
			__fallthrough;
		case EncounterType::Submissive:
			_timessubmissive++;
			break;
		}
	}

	void ActorEncounter::Save(SKSE::SerializationInterface* a_intfc)
	{
		npc1.Save(a_intfc);
		npc2.Save(a_intfc);
		a_intfc->WriteRecordData(_lastmet);
		a_intfc->WriteRecordData(_timesmet);
		a_intfc->WriteRecordData(_timesaggressor);
		a_intfc->WriteRecordData(_timesvictim);
		a_intfc->WriteRecordData(_timesdominant);
		a_intfc->WriteRecordData(_timessubmissive);
	}

	void StatisticsData::Register()
	{
		const auto script = RE::ScriptEventSourceHolder::GetSingleton();
		script->AddEventSink<RE::TESDeathEvent>(this);
		script->AddEventSink<RE::TESResetEvent>(this);
	}

	std::vector<RE::Actor*> StatisticsData::GetTrackedActors() const
	{
		const std::shared_lock lock{ _m };
		std::vector<RE::Actor*> ret{};
		ret.reserve(_data.size());
		for (auto&& [id, _] : _data) {
			const auto act = RE::TESForm::LookupByID<RE::Actor>(id);
			if (!act)
				continue;
			ret.push_back(act);
		}
		return ret;
	}

	ActorStats& StatisticsData::GetStatistics(RE::Actor* a_key)
	{
		const std::shared_lock lock{ _m };
		auto where = _data.find(a_key->GetFormID());
		if (where == _data.end()) {
			_data.insert(std::make_pair(a_key->GetFormID(), ActorStats{ a_key }));
			return _data.at(a_key->GetFormID());
		}
		return where->second;
	}

	ActorEncounter* StatisticsData::GetEncounter(RE::Actor* fst, RE::Actor* snd)
	{
		const auto where = GetEncounterIter(fst, snd);
		return where == _encounters.end() ? nullptr : &(*where);
	}

	std::vector<ActorEncounter>::iterator StatisticsData::GetEncounterIter(RE::Actor* fst, RE::Actor* snd)
	{
		return std::ranges::find_if(_encounters, [&](const ActorEncounter& enc) {
			const auto& [a, b] = enc.GetParticipants();
			return a.id == fst->formID && b.id == snd->formID || b.id == fst->formID && a.id == snd->formID;
		});
	}

	void StatisticsData::DeleteStatistics(RE::FormID a_key)
	{
		const std::unique_lock lock{ _m };
		_data.erase(a_key);
		std::erase_if(_encounters, [&](auto& encounter) {
			const auto [fst, snd] = encounter.GetParticipants();
			return fst.id == a_key || snd.id == a_key;
		});
	}

	bool StatisticsData::ForEachStatistic(std::function<bool(ActorStats&)> a_func)
	{
		const std::unique_lock lock{ _m };
		for (auto&& [_, statistic] : _data) {
			if (a_func(statistic))
				return true;
		}
		return false;
	}

	bool StatisticsData::ForEachEncounter(std::function<bool(ActorEncounter&)> a_func)
	{
		const std::unique_lock lock{ _m };
		for (auto&& encounter : _encounters) {
			if (a_func(encounter))
				return true;
		}
		return false;
	}

	void StatisticsData::AddEncounter(RE::Actor* fst, RE::Actor* snd, ActorEncounter::EncounterType a_type)
	{
		const std::unique_lock lock{ _m };
		if (auto enc = GetEncounterIter(fst, snd); enc != _encounters.end()) {
			if (enc->GetParticipants().first.id == snd->formID) {
				switch (a_type) {
				case ActorEncounter::EncounterType::Aggressor:
					a_type = ActorEncounter::EncounterType::Victim;
					break;
				case ActorEncounter::EncounterType::Victim:
					a_type = ActorEncounter::EncounterType::Aggressor;
					break;
				default:
					a_type = ActorEncounter::EncounterType::Any;
					break;
				}
			}
			enc->Update(a_type);
			std::iter_swap(enc, _encounters.end() - 1);
			return;
		}
		_encounters.emplace_back(fst, snd, a_type);
	}

	RE::Actor* StatisticsData::GetMostRecentEncounter(RE::Actor* a_actor, ActorEncounter::EncounterType a_type)
	{
		const std::shared_lock lock{ _m };
		for (auto it = _encounters.rbegin(); it != _encounters.rend(); it++) {
			const auto partnerobj = it->GetPartner(a_actor);
			const auto partner = partnerobj ? RE::TESForm::LookupByID<RE::Actor>(partnerobj->id) : nullptr;
			if (!partner)
				continue;
			switch (a_type) {
			case ActorEncounter::EncounterType::Any:
				return partner;
			case ActorEncounter::EncounterType::Victim:
				if (it->GetTimesVictim(a_actor->formID) > 0) {
					return partner;
				}
				break;
			case ActorEncounter::EncounterType::Aggressor:
				if (it->GetTimesAssailant(a_actor->formID) > 0) {
					return partner;
				}
				break;
			case ActorEncounter::EncounterType::Submissive:
				if (it->GetTimesSubmissive(a_actor->formID) > 0) {
					return partner;
				}
				break;
			case ActorEncounter::EncounterType::Dominant:
				if (it->GetTimesDominant(a_actor->formID) > 0) {
					return partner;
				}
				break;
			}
		}
		return nullptr;
	}

	int StatisticsData::GetNumberEncounters(RE::Actor* a_actor)
	{
		return GetNumberEncounters(a_actor, ActorEncounter::EncounterType::Any, [](auto&) { return true; });
	}
	int StatisticsData::GetNumberEncounters(RE::Actor* a_actor, ActorEncounter::EncounterType a_type)
	{
		return GetNumberEncounters(a_actor, a_type, [](auto&) { return true; });
	}
	int StatisticsData::GetNumberEncounters(RE::Actor* a_actor, std::function<bool(const ActorEncounter::EncounterObj&)> a_pred)
	{
		return GetNumberEncounters(a_actor, ActorEncounter::EncounterType::Any, a_pred);
	}
	int StatisticsData::GetNumberEncounters(RE::Actor* a_actor, ActorEncounter::EncounterType a_type, std::function<bool(const ActorEncounter::EncounterObj&)> a_pred)
	{
		const std::shared_lock lock{ _m };
		int ret = 0;
		for (auto&& encounter : _encounters) {
			const auto partner = encounter.GetPartner(a_actor);
			if (!partner || !a_pred(*partner))
				continue;
			switch (a_type) {
			case ActorEncounter::EncounterType::Any:
				ret += encounter.GetTimesMet();
				break;
			case ActorEncounter::EncounterType::Victim:
				ret += encounter.GetTimesVictim(a_actor->formID);
				break;
			case ActorEncounter::EncounterType::Aggressor:
				ret += encounter.GetTimesAssailant(a_actor->formID);
				break;
			case ActorEncounter::EncounterType::Submissive:
				ret += encounter.GetTimesSubmissive(a_actor->formID);
				break;
			case ActorEncounter::EncounterType::Dominant:
				ret += encounter.GetTimesDominant(a_actor->formID);
				break;
			}
		}
		return ret;
	}

	StatisticsData::EventResult StatisticsData::ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>*)
	{
		if (!a_event || !a_event->actorDying)
			return EventResult::kContinue;

		DeleteStatistics(a_event->actorDying->formID);
		return EventResult::kContinue;
	}

	StatisticsData::EventResult StatisticsData::ProcessEvent(const RE::TESResetEvent* a_event, RE::BSTEventSource<RE::TESResetEvent>*)
	{
		if (!a_event || !a_event->object || !a_event->object->IsNot(RE::FormType::ActorCharacter))
			return EventResult::kContinue;

		DeleteStatistics(a_event->object->formID);
		return EventResult::kContinue;
	}

	void StatisticsData::Save(SKSE::SerializationInterface* a_intfc)
	{
		const std::shared_lock lock{ _m };
		if (!a_intfc->WriteRecordData(_data.size())) {
			logger::error("Failed to save number of saved statistics ({})", _data.size());
			return;
		}
		for (auto&& [id, data] : _data) {
			if (!a_intfc->WriteRecordData(id)) {
				logger::error("Failed to save reg ({:X})", id);
				continue;
			}
			try {
				data.Save(a_intfc);
			} catch (const std::exception& e) {
				logger::error("{}", e.what());
			}
		}
		logger::info("Saved {} statistics", _data.size());
	}

	void StatisticsData::Load(SKSE::SerializationInterface* a_intfc)
	{
		const std::unique_lock lock{ _m };
		_data.clear();
		std::size_t numRegs;
		a_intfc->ReadRecordData(numRegs);

		RE::FormID formID;
		for (size_t i = 0; i < numRegs; i++) {
			a_intfc->ReadRecordData(formID);
			if (!a_intfc->ResolveFormID(formID, formID)) {
				logger::warn("Error reading formID ({:X})", formID);
				continue;
			}
			_data.insert(std::make_pair(formID, ActorStats{ a_intfc }));
		}
		logger::info("Loaded {} statistics", _data.size());
	}

	void StatisticsData::Revert(SKSE::SerializationInterface*)
	{
		const std::unique_lock lock{ _m };
		_data.clear();
	}


}	 // namespace Registry::Statistics
