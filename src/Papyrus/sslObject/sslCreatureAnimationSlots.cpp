#include "sslCreatureAnimationSlots.h"

#include "Registry/Define/RaceKey.h"
#include "Registry/Library.h"

namespace Papyrus::CreatureAnimationSlots
{
    std::vector<RE::BSFixedString> GetByRaceKeyTagsImpl(VM* a_vm, StackID a_stackID, RE::TESQuest*,
        int32_t a_actorcount,
        RE::BSFixedString a_racekey,
        std::vector<std::string_view> a_tags)
    {
        const Registry::RaceKey racekey{ a_racekey };
        if (!racekey.IsValid()) {
            a_vm->TraceStack("Invalid racekey", a_stackID);
            return {};
        }
        Registry::TagDetails tagdetails{ a_tags };
        std::vector<RE::BSFixedString> ret{};
        ret.reserve(256);
        Registry::Library::GetSingleton()->ForEachScene([&](const Registry::Scene* a_scene) {
            if (!a_scene->IsEnabled() || a_scene->IsPrivate())
                return false;
            if (a_actorcount > -1 && a_scene->positions.size() != a_actorcount)
                return false;
            if (!a_scene->IsCompatibleTags(tagdetails))
                return false;
            for (auto&& position : a_scene->positions) {
                if (position.data.GetRace().IsCompatibleWith(racekey)) {
                    ret.push_back(a_scene->id);
                    break;
                }
            }
            return ret.size() == ret.capacity();
        });
        return ret;
    }

    std::vector<RE::BSFixedString> GetByCreatureActorsTagsImpl(VM* a_vm, StackID a_stackID, RE::TESQuest*,
        int32_t a_actorcount, std::vector<RE::Actor*> a_creatures, std::vector<std::string_view> a_tags)
    {
        if (a_actorcount <= 0 || a_actorcount > Registry::ActorFragment::MAX_ACTOR_COUNT) {
            const auto err = std::format("Actorcount should be between 1 and {} but was {}", Registry::ActorFragment::MAX_ACTOR_COUNT, a_actorcount);
            a_vm->TraceStack(err.c_str(), a_stackID);
            return {};
        }
        if (a_creatures.empty() || std::find(a_creatures.begin(), a_creatures.end(), nullptr) != a_creatures.end()) {
            a_vm->TraceStack("Array contained none values", a_stackID);
            return {};
        }
        if (a_creatures.size() > a_actorcount) {
            a_vm->TraceStack("Actor array should be less or equal than actor count but was greater", a_stackID);
            return {};
        }

        Registry::TagDetails tagdetails{ a_tags };
        std::vector<RE::BSFixedString> ret{};
        ret.reserve(256);
        Registry::Library::GetSingleton()->ForEachScene([&](const Registry::Scene* a_scene) {
            if (!a_scene->IsEnabled() || a_scene->IsPrivate())
                return false;
            if (a_scene->positions.size() != a_actorcount)
                return false;
            if (!a_scene->IsCompatibleTags(tagdetails))
                return false;

            int32_t reqtrue = static_cast<int32_t>(a_creatures.size());
            std::vector<bool> control(a_actorcount, false);
            for (auto&& creature : a_creatures) {
                const Registry::RaceKey racekey{ creature };
                if (!racekey.IsValid()) {
                    a_vm->TraceStack("Invalid racekey", a_stackID);
                    ret.clear();
                    return true;
                }
                if (racekey.Is(Registry::RaceKey::Human)) {
                    reqtrue -= 1;
                    continue;
                }
                for (size_t i = 0; i < a_scene->positions.size(); i++) {
                    if (control[i])
                        continue;
                    const auto& position = a_scene->positions[i];
                    if (position.data.GetRace().IsCompatibleWith(racekey)) {
                        control[i] = true;
                        break;
                    }
                }
            }
            if (reqtrue != std::count(control.begin(), control.end(), true)) {
                return false;
            }
            ret.push_back(a_scene->id);
            return ret.size() == ret.capacity();
        });
        return ret;
    }

    std::vector<RE::BSFixedString> GetByRaceGendersTagsImpl(VM* a_vm, StackID a_stackID, RE::TESQuest*,
        int32_t a_actorcount,
        RE::BSFixedString a_racekey,
        int32_t a_malecrt,
        int32_t a_femalecrt,
        std::vector<std::string_view> a_tags)
    {
        if (a_actorcount <= 0 || a_actorcount > Registry::ActorFragment::MAX_ACTOR_COUNT) {
            const auto err = std::format("Actorcount should be between 1 and {} but was {}", Registry::ActorFragment::MAX_ACTOR_COUNT, a_actorcount);
            a_vm->TraceStack(err.c_str(), a_stackID);
            return {};
        }
        if (a_malecrt + a_femalecrt > a_actorcount) {
            a_vm->TraceStack("Expecting more creatures than total positions", a_stackID);
            return {};
        }
        const Registry::RaceKey racekey{ a_racekey };
        if (!racekey.IsValid()) {
            a_vm->TraceStack("Invalid racekey", a_stackID);
            return {};
        }
        Registry::TagDetails tagdetails{ a_tags };
        std::vector<RE::BSFixedString> ret;
        ret.reserve(256);
        Registry::Library::GetSingleton()->ForEachScene([&](const Registry::Scene* a_scene) {
            if (!a_scene->IsEnabled() || a_scene->IsPrivate())
                return false;
            if (a_scene->positions.size() != a_actorcount)
                return false;
            if (!a_scene->IsCompatibleTags(tagdetails))
                return false;
            bool has_race = false;
            for (auto&& position : a_scene->positions) {
                if (position.data.GetRace().IsCompatibleWith(racekey)) {
                    has_race = true;
                    break;
                }
            }
            if (!has_race) {
                return false;
            }
            bool match_gender = a_scene->Legacy_IsCompatibleSexCountCrt(a_malecrt, a_femalecrt);
            if (!match_gender) {
                return false;
            }
            ret.push_back(a_scene->id);
            return ret.size() == ret.capacity();
        });
        return ret;
    }

    RE::TESRace* GetRaceByEditorID(RE::StaticFunctionTag*, RE::BSFixedString a_id)
    {
        return RE::TESForm::LookupByEditorID<RE::TESRace>(a_id);
    }

    std::vector<RE::BSFixedString> GetAllRaceIDs(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_racekey)
    {
        const auto retVal = GetAllRaces(a_vm, a_stackID, nullptr, a_racekey);
        std::vector<RE::BSFixedString> ret{};
        for (auto&& race : retVal) {
            ret.push_back(race->formEditorID);
        }
        return ret;
    }

    std::vector<RE::TESRace*> GetAllRaces(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_racekey)
    {
        a_vm->TraceStack("Calling legacy function GetAllRaces(). It is highly recommended to no longer use this function.", a_stackID);
        const Registry::RaceKey matchkey{ a_racekey };
        if (!matchkey.IsValid()) {
            const auto err = std::format("Invalid RaceKey: ", a_racekey.c_str());
            a_vm->TraceStack(err.c_str(), a_stackID);
            return {};
        }
        std::vector<RE::TESRace*> ret{};
        const auto& races = RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESRace>();
        for (auto&& race : races) {
            if (!race)
                continue;
            const Registry::RaceKey racekey{ race };
            if (!racekey.IsCompatibleWith(matchkey))
                continue;
            ret.push_back(race);
        }
        return ret;
    }

}  // namespace Papyrus::CreatureAnimationSlots
