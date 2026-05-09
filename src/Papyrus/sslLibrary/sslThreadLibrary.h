#pragma once

#include "LegacyData.h"

namespace Papyrus::ThreadLibrary
{
    std::vector<RE::TESObjectREFR*> FindBeds(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* a_center, float a_radius, float a_radiusz);
    int32_t GetBedTypeImpl(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* a_reference);
    bool IsBed(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* a_reference);

    std::vector<RE::Actor*> FindAvailableActors(VM* a_vm, StackID a_stackID, RE::TESQuest*, RE::TESObjectREFR* a_center, float a_radius, LegacySex a_targetsex,
        RE::Actor* ignore_ref01, RE::Actor* ignore_ref02, RE::Actor* ignore_ref03, RE::Actor* ignore_ref04, RE::BSFixedString a_targetrace);
    RE::Actor* FindAvailableActor(VM* a_vm, StackID a_stackID, RE::TESQuest*, RE::TESObjectREFR* a_center, float a_radius, LegacySex a_targetsex,
        RE::Actor* ignore_ref01, RE::Actor* ignore_ref02, RE::Actor* ignore_ref03, RE::Actor* ignore_ref04, RE::BSFixedString a_targetrace);
    RE::Actor* FindAvailableActorInFaction(VM* a_vm, StackID a_stackID, RE::TESQuest*, RE::TESFaction* a_faction, RE::TESObjectREFR* a_center, float a_radius, LegacySex a_targetsex,
        RE::Actor* ignore_ref01, RE::Actor* ignore_ref02, RE::Actor* ignore_ref03, RE::Actor* ignore_ref04, bool a_hasfaction, RE::BSFixedString a_targetrace, bool a_samefloor);
    RE::Actor* FindAvailableActorWornForm(VM* a_vm, StackID a_stackID, RE::TESQuest*, uint32_t a_slotmask, RE::TESObjectREFR* a_center, float a_radius, LegacySex a_targetsex,
        RE::Actor* ignore_ref01, RE::Actor* ignore_ref02, RE::Actor* ignore_ref03, RE::Actor* ignore_ref04, bool a_recognizenostrip, bool a_iswearing, RE::BSFixedString a_targetrace,
        bool a_samefloor);

    std::vector<RE::Actor*> FindAvailablePartners(VM* a_vm, StackID a_stackID, RE::TESQuest*,
        std::vector<RE::Actor*> a_positions, int a_total, int a_males, int a_females, float a_radius);
    std::vector<RE::Actor*> FindAnimationPartnersImpl(VM* a_vm, StackID a_stackID, RE::TESQuest*,
        RE::BSFixedString a_sceneid, RE::TESObjectREFR* a_center, float a_radius, std::vector<RE::Actor*> a_includes);

    std::vector<RE::Actor*> SortActorsByAnimationImpl(VM* a_vm, StackID a_stackID, RE::TESQuest*,
        RE::BSFixedString a_sceneid, std::vector<RE::Actor*> a_positions, std::vector<RE::Actor*> a_submissives);

    bool IsActorTrackedImpl(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);
    void TrackActorImpl(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_callback, bool a_dotrack);
    void TrackFactionImpl(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESFaction* a_faction, RE::BSFixedString a_callback, bool a_dotrack);
    std::vector<std::string> GetAllTrackingEvents(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_hook);

    inline bool Register(VM* a_vm)
    {
        REGISTERFUNC(FindBeds, "sslThreadLibrary", false);
        REGISTERFUNC(GetBedTypeImpl, "sslThreadLibrary", false);
        REGISTERFUNC(IsBed, "sslThreadLibrary", false);

        REGISTERFUNC(FindAvailableActors, "sslThreadLibrary", false);
        REGISTERFUNC(FindAvailableActor, "sslThreadLibrary", false);
        REGISTERFUNC(FindAvailableActorInFaction, "sslThreadLibrary", false);
        REGISTERFUNC(FindAvailableActorWornForm, "sslThreadLibrary", false);

        REGISTERFUNC(FindAvailablePartners, "sslThreadLibrary", false);
        REGISTERFUNC(FindAnimationPartnersImpl, "sslThreadLibrary", false);

        REGISTERFUNC(SortActorsByAnimationImpl, "sslThreadLibrary", true);

        REGISTERFUNC(IsActorTrackedImpl, "sslThreadLibrary", true);
        REGISTERFUNC(TrackActorImpl, "sslThreadLibrary", true);
        REGISTERFUNC(TrackFactionImpl, "sslThreadLibrary", true);
        REGISTERFUNC(GetAllTrackingEvents, "sslThreadLibrary", true);

        return true;
    }

}  // namespace Papyrus::ThreadLibrary
