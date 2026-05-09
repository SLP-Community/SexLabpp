#pragma once

#include <shared_mutex>

#include "Registry/Define/RaceKey.h"
#include "Registry/Define/Sex.h"

namespace Registry::Statistics
{
    struct ActorStats
    {
        enum StatisticID
        {
            LastUpdate_GameTime,
            SecondsInScene,

            XP_Vaginal,
            XP_Anal,
            XP_Oral,

            PartnersMale,
            PartnersFemale,
            PartnersFuta,
            PartnersCreature,
            TimesOral,
            TimesVaginal,
            TimesAnal,
            TimesMasturbated,
            TimesSubmissive,
            TimesDominant,
            TimesTotal,

            Sexuality,
            Arousal,

            Total
        };
        ActorStats(SKSE::SerializationInterface* a_intfc);
        ActorStats(RE::Actor* owner);
        ~ActorStats() = default;

        void SetStatistic(StatisticID key, float value);
        void AddStatistic(StatisticID key, float value);
        float GetStatistic(StatisticID key) const;
        std::vector<RE::BSFixedString> GetEveryCustomID() const;

        bool HasCustom(const RE::BSFixedString& key) const;
        std::optional<float> GetCustomFlt(const RE::BSFixedString& key) const;
        std::optional<RE::BSFixedString> GetCustomStr(const RE::BSFixedString& key) const;
        void SetCustomFlt(const RE::BSFixedString& key, float value);
        void SetCustomStr(const RE::BSFixedString& key, RE::BSFixedString value);
        void RemoveCustomStat(const RE::BSFixedString& key);

        void Save(SKSE::SerializationInterface* a_intfc);

      private:
        template <class T>
        std::optional<T> GetCustom(const RE::BSFixedString& key) const
        {
            const auto it = _custom.find(key);
            if (it == _custom.end())
                return std::nullopt;

            auto& ret = it->second;
            if (!std::holds_alternative<T>(ret))
                return std::nullopt;

            return std::get<T>(ret);
        }

        std::vector<float> _stats{};
        std::map<RE::BSFixedString, std::variant<float, RE::BSFixedString>, FixedStringCompare> _custom{};
    };

    struct ActorEncounter
    {
        /// Types goes id1 -> id2. E.g. if Victim, then id1 = Victim => id2 = aggressor
        enum class EncounterType
        {
            Any = 0,
            Victim = 1,
            Aggressor = 2,
            Submissive = 3,
            Dominant = 4
        };

        struct EncounterObj
        {
            EncounterObj(RE::Actor* obj);
            EncounterObj(SKSE::SerializationInterface* a_intfc);

            void Save(SKSE::SerializationInterface* a_intfc);

            RE::FormID id;
            RaceKey race;
            Sex sex;
        };

      public:
        ActorEncounter(RE::Actor* fst, RE::Actor* snd, EncounterType a_type);
        ActorEncounter(SKSE::SerializationInterface* a_intfc);
        ~ActorEncounter() = default;

        void Update(EncounterType a_type);

        std::pair<const EncounterObj&, const EncounterObj&> GetParticipants() const { return { npc1, npc2 }; }
        const ActorEncounter::EncounterObj* GetPartner(RE::Actor* a_actor) const;
        float GetLastTimeMet() const { return _lastmet; }
        uint8_t GetTimesMet() const { return _timesmet; }
        uint8_t GetTimesSubmissive(RE::FormID a_id) const;
        uint8_t GetTimesDominant(RE::FormID a_id) const;
        uint8_t GetTimesVictim(RE::FormID a_id) const;
        uint8_t GetTimesAssailant(RE::FormID a_id) const;

        void Save(SKSE::SerializationInterface* a_intfc);

      private:
        EncounterObj npc1;
        EncounterObj npc2;

        float _lastmet;
        uint8_t _timesmet;
        uint8_t _timessubmissive;
        uint8_t _timesdominant;
        uint8_t _timesvictim;
        uint8_t _timesaggressor;
    };

    class StatisticsData :
      public Singleton<StatisticsData>,
      public RE::BSTEventSink<RE::TESDeathEvent>,
      public RE::BSTEventSink<RE::TESResetEvent>
    {
        using EventResult = RE::BSEventNotifyControl;

      public:
        std::vector<RE::Actor*> GetTrackedActors() const;
        ActorStats& GetStatistics(RE::Actor* a_key);
        ActorEncounter* GetEncounter(RE::Actor* fst, RE::Actor* snd);
        std::vector<ActorEncounter>::iterator GetEncounterIter(RE::Actor* fst, RE::Actor* snd);
        void DeleteStatistics(RE::FormID a_key);

        bool ForEachStatistic(std::function<bool(ActorStats&)> a_func);
        bool ForEachEncounter(std::function<bool(ActorEncounter&)> a_func);

        void AddEncounter(RE::Actor* fst, RE::Actor* snd, ActorEncounter::EncounterType a_type);
        RE::Actor* GetMostRecentEncounter(RE::Actor* a_actor, ActorEncounter::EncounterType a_type);

        int GetNumberEncounters(RE::Actor* a_actor);
        int GetNumberEncounters(RE::Actor* a_actor, ActorEncounter::EncounterType a_type);
        int GetNumberEncounters(RE::Actor* a_actor, std::function<bool(const ActorEncounter::EncounterObj&)> a_pred);
        int GetNumberEncounters(RE::Actor* a_actor, ActorEncounter::EncounterType a_type, std::function<bool(const ActorEncounter::EncounterObj&)> a_pred);

        EventResult ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>*) override;
        EventResult ProcessEvent(const RE::TESResetEvent* a_event, RE::BSTEventSource<RE::TESResetEvent>*) override;

        void Register();
        void Save(SKSE::SerializationInterface* a_intfc);
        void Load(SKSE::SerializationInterface* a_intfc);
        void Revert(SKSE::SerializationInterface* a_intfc);

      private:
        mutable std::shared_mutex _m{};
        std::vector<ActorEncounter> _encounters;
        std::map<RE::FormID, ActorStats> _data;
    };

}  // namespace Registry::Statistics
