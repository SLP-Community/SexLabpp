#pragma once

#include "Registry/Stats.h"

namespace Papyrus::ActorStats
{
    static inline constexpr auto Purity{ "SSLLEGACY_Purity" };
    static inline constexpr auto Lewdness{ "SSLLEGACY_Lewd" };
    static inline constexpr auto Foreplay{ "SSLLEGACY_Foreplay" };

    enum class LegacyStatistics
    {
        L_Foreplay,
        XP_Vaginal,
        XP_Anal,
        XP_Oral,
        L_Pure,
        L_Lewd,
        Times_Males,
        Times_Females,
        Times_Creatures,
        Times_Masturbation,
        Times_Aggressor,
        Times_Victim,
        SexCount,   // how often the actor had intercourse in general
        PlayerSex,  // how often the actor had intercourse with the player
        Sexuality,
        TimeSpent,  // time spent in scenes, there is no reference in what scale this is measured
        LastSex_RealTime,
        LastSex_GameTime,
        Times_VaginalCount,
        Times_AnalCount,
        Times_OralCount,

        Total
    };

    std::vector<RE::Actor*> GetAllTrackedActors(RE::StaticFunctionTag*);
    std::vector<RE::Actor*> GetAllTrackedUniqueActorsSorted(RE::StaticFunctionTag*);
    void SetStatistic(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, int id, float a_value);
    float GetStatistic(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, int id);
    int GetSexuality(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);
    void SetSexuality(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, int mapping);
    int MapSexuality(RE::StaticFunctionTag*, float a_sexuality);

    std::vector<RE::BSFixedString> GetAllCustomStatIDs(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);
    bool HasCustomStat(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_stat);
    void SetCustomStatFlt(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_stat, float a_value);
    void SetCustomStatStr(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_stat, RE::BSFixedString a_value);
    float GetCustomStatFlt(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_stat, float a_default);
    RE::BSFixedString GetCustomStatStr(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_stat, RE::BSFixedString a_default);
    void DeleteCustomStat(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_stat);

    std::vector<RE::Actor*> GetAllEncounters(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);
    std::vector<RE::Actor*> GetAllEncounteredVictims(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);
    std::vector<RE::Actor*> GetAllEncounteredAssailants(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);
    RE::Actor* GetMostRecentEncounter(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, int a_encountertype);
    void AddEncounter(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::Actor* a_partner, int a_encountertype);
    float GetLastEncounterTime(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::Actor* a_partner);
    int GetTimesMet(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::Actor* a_partner);
    int GetTimesVictimzed(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::Actor* a_assailant);
    int GetTimesAssaulted(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::Actor* a_victim);
    int GetTimesSubmissive(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::Actor* a_partner);
    int GetTimesDominant(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::Actor* a_partner);

    void ResetStatistics(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);

    std::vector<RE::BSFixedString> GetEveryStatisticID(RE::StaticFunctionTag*);
    float GetLegacyStatistic(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, int id);
    std::vector<float> GetAllLegycSkills(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);
    void SetLegacyStatistic(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, int id, float a_value);

    inline bool Register(VM* a_vm)
    {
        REGISTERFUNC(GetAllTrackedActors, "SexLabStatistics", true);
        REGISTERFUNC(GetAllTrackedUniqueActorsSorted, "SexLabStatistics", true);
        REGISTERFUNC(SetStatistic, "SexLabStatistics", true);
        REGISTERFUNC(GetStatistic, "SexLabStatistics", true);

        REGISTERFUNC(GetAllCustomStatIDs, "SexLabStatistics", true);
        REGISTERFUNC(HasCustomStat, "SexLabStatistics", true);
        REGISTERFUNC(SetCustomStatFlt, "SexLabStatistics", true);
        REGISTERFUNC(SetCustomStatStr, "SexLabStatistics", true);
        REGISTERFUNC(GetCustomStatFlt, "SexLabStatistics", true);
        REGISTERFUNC(GetCustomStatStr, "SexLabStatistics", true);
        REGISTERFUNC(DeleteCustomStat, "SexLabStatistics", true);

        REGISTERFUNC(GetAllEncounters, "SexLabStatistics", true);
        REGISTERFUNC(GetAllEncounteredVictims, "SexLabStatistics", true);
        REGISTERFUNC(GetAllEncounteredAssailants, "SexLabStatistics", true);
        REGISTERFUNC(GetMostRecentEncounter, "SexLabStatistics", true);
        REGISTERFUNC(AddEncounter, "SexLabStatistics", true);
        REGISTERFUNC(GetLastEncounterTime, "SexLabStatistics", true);
        REGISTERFUNC(GetTimesMet, "SexLabStatistics", true);
        REGISTERFUNC(GetTimesVictimzed, "SexLabStatistics", true);
        REGISTERFUNC(GetTimesAssaulted, "SexLabStatistics", true);
        REGISTERFUNC(GetTimesSubmissive, "SexLabStatistics", true);
        REGISTERFUNC(GetTimesDominant, "SexLabStatistics", true);

        REGISTERFUNC(GetSexuality, "SexLabStatistics", true);
        REGISTERFUNC(SetSexuality, "SexLabStatistics", true);
        REGISTERFUNC(MapSexuality, "SexLabStatistics", true);

        REGISTERFUNC(GetEveryStatisticID, "sslActorStats", true);
        REGISTERFUNC(GetLegacyStatistic, "sslActorStats", true);
        REGISTERFUNC(GetAllLegycSkills, "sslActorStats", true);
        REGISTERFUNC(SetLegacyStatistic, "sslActorStats", true);

        return true;
    }
}  // namespace Papyrus::ActorStats
