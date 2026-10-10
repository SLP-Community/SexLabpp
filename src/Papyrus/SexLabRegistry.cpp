#include "SexLabRegistry.h"

#include "Registry/Library.h"
#include "Util/StringUtil.h"

namespace Papyrus::SexLabRegistry
{
#define SCENE(argRet)                                    \
    const auto lib = Registry::Library::GetSingleton();  \
    const auto scene = lib->GetSceneById(a_id);          \
    if (!scene) {                                        \
        a_vm->TraceStack("Invalid scene id", a_stackID); \
        return argRet;                                   \
    }

#define STAGE(argRet)                                    \
    SCENE(argRet);                                       \
    const auto stage = scene->GetStageById(a_stage);     \
    if (!stage) {                                        \
        a_vm->TraceStack("Invalid stage id", a_stackID); \
        return argRet;                                   \
    }

#define POSITION(argRet)                                     \
    const auto positions = stage->GetPositions();            \
    if (n < 0 || static_cast<size_t>(n) >= positions.size()) { \
        a_vm->TraceStack("Invalid position idx", a_stackID); \
        return argRet;                                       \
    }                                                        \
    const auto position = positions[n];

#define POSITION_META(argRet)                                \
    if (n < 0 || n >= scene->GetPositions().size()) {        \
        a_vm->TraceStack("Invalid position idx", a_stackID); \
        return argRet;                                       \
    }

    int32_t GetRaceID(STATICARGS, RE::Actor* a_actor)
    {
        if (!a_actor) {
            a_vm->TraceStack("Cannot get race id of a none ref", a_stackID);
            return 0;
        }
        const auto racekey = Registry::RaceKey(a_actor);
        if (!racekey.IsValid()) {
            const auto err = std::format("Invalid race key for actor {}", a_actor->formID);
            a_vm->TraceStack(err.c_str(), a_stackID);
            return 0;
        }
        return static_cast<int32_t>(racekey);
    }

    int32_t MapRaceKeyToID(STATICARGS, RE::BSFixedString a_racekey)
    {
        const auto racekey = Registry::RaceKey(a_racekey);
        if (!racekey.IsValid()) {
            const auto err = std::format("Invalid race key {}", a_racekey);
            a_vm->TraceStack(err.c_str(), a_stackID);
            return 0;
        }
        return static_cast<int32_t>(racekey);
    }

    std::vector<int32_t> GetRaceIDA(STATICARGS, RE::Actor* a_actor)
    {
        const auto key = GetRaceID(a_vm, a_stackID, nullptr, a_actor);
        if (key == 0) {
            return {};
        }
        std::vector<int32_t> ret{ key };
        const auto alternate = Registry::RaceKey(Registry::RaceKey::Value(key)).GetMetaRace();
        if (alternate.IsValid()) {
            ret.push_back(static_cast<int32_t>(alternate));
        }
        return ret;
    }

    std::vector<int32_t> MapRaceKeyToIDA(STATICARGS, RE::BSFixedString a_racekey)
    {
        const auto key = MapRaceKeyToID(a_vm, a_stackID, nullptr, a_racekey);
        if (key == 0) {
            return {};
        }
        std::vector<int32_t> ret{ key };
        const Registry::RaceKey racekey{ a_racekey };
        const auto alternate = racekey.GetMetaRace();
        if (alternate.IsValid()) {
            ret.push_back(static_cast<int32_t>(alternate.value));
        }
        return ret;
    }

    RE::BSFixedString GetRaceKey(STATICARGS, RE::Actor* a_actor)
    {
        if (!a_actor) {
            a_vm->TraceStack("Cannot get race key of a none ref", a_stackID);
            return 0;
        }
        const Registry::RaceKey racekey{ a_actor };
        if (!racekey.IsValid()) {
            const auto err = std::format("Invalid race key for race {:X}", a_actor->formID);
            a_vm->TraceStack(err.c_str(), a_stackID);
            return 0;
        }
        return racekey.AsString();
    }

    RE::BSFixedString GetRaceKeyByRace(STATICARGS, RE::TESRace* a_race)
    {
        if (!a_race) {
            a_vm->TraceStack("Cannot get race key of a none race", a_stackID);
            return 0;
        }
        const Registry::RaceKey racekey{ a_race };
        if (!racekey.IsValid()) {
            const auto err = std::format("Invalid race key for race {:X}", a_race->formID);
            a_vm->TraceStack(err.c_str(), a_stackID);
            return 0;
        }
        return racekey.AsString();
    }

    RE::BSFixedString MapRaceIDToRaceKey(RE::StaticFunctionTag*, int32_t a_raceid)
    {
        return Registry::RaceKey(Registry::RaceKey::Value(a_raceid)).AsString();
    }

    std::vector<RE::BSFixedString> GetRaceKeyA(STATICARGS, RE::Actor* a_actor)
    {
        const auto ids = GetRaceIDA(a_vm, a_stackID, nullptr, a_actor);
        std::vector<RE::BSFixedString> ret{};
        for (auto&& id : ids) {
            const auto rk = Registry::RaceKey(Registry::RaceKey::Value(id));
            ret.push_back(rk.AsString());
        }
        return ret;
    }

    std::vector<RE::BSFixedString> GetRaceKeyByRaceA(STATICARGS, RE::TESRace* a_race)
    {
        if (!a_race) {
            a_vm->TraceStack("Cannot get racekeys from none race", a_stackID);
            return {};
        }
        const Registry::RaceKey key{ a_race };
        if (!key.IsValid())
            return {};
        const auto variant = key.GetMetaRace();
        if (!variant.IsValid())
            return { key.AsString() };
        return { key.AsString(), variant.AsString() };
    }

    std::vector<RE::BSFixedString> MapRaceIDToRaceKeyA(RE::StaticFunctionTag*, int32_t a_raceid)
    {
        const Registry::RaceKey key{ Registry::RaceKey::Value(a_raceid) };
        const auto key1 = key.AsString();
        if (key1.empty())
            return {};
        const auto variant = key.GetMetaRace();
        if (!variant.IsValid())
            return { key1 };
        return { key1, variant.AsString() };
    }

    std::vector<RE::BSFixedString> GetAllRaceKeys(RE::StaticFunctionTag*, bool a_ignoreambiguous)
    {
        return Registry::RaceKey::GetAllRaceKeys(a_ignoreambiguous);
    }

    int32_t GetSex(STATICARGS, RE::Actor* a_actor, bool a_ignoreoverwrite)
    {
        if (!a_actor) {
            a_vm->TraceStack("Cannot get sex from none ref", a_stackID);
            return 0;
        }
        const bool humanoid = a_actor->IsHumanoid();
        auto sex = Registry::GetSex(a_actor, a_ignoreoverwrite);
        switch (sex) {
        case Registry::Sex::Male:
            return humanoid ? 0 : 3;
        case Registry::Sex::Female:
            return humanoid                  ? 1 :
                   Settings::bCreatureGender ? 4 :
                                               3;
        case Registry::Sex::Futa:
            return humanoid                  ? 2 :
                   Settings::bCreatureGender ? 4 :
                                               3;
        default:
            return 0;
        }
    }

    std::vector<RE::BSFixedString> LookupScenes(STATICARGS,
        std::vector<RE::Actor*> a_positions, std::string a_tags, RE::Actor* a_submissive, FurniturePreference a_furniturepref, RE::TESObjectREFR* a_center)
    {
        auto argSubmissive{ a_submissive ? std::vector<RE::Actor*>{ a_submissive } : std::vector<RE::Actor*>{} };
        return LookupScenesA(a_vm, a_stackID, nullptr, a_positions, a_tags, argSubmissive, a_furniturepref, a_center);
    }

    std::vector<RE::BSFixedString> LookupScenesA(STATICARGS,
        std::vector<RE::Actor*> a_positions, std::string a_tags, std::vector<RE::Actor*> a_submissives, FurniturePreference a_furniturepref, RE::TESObjectREFR* a_center)
    {
        if (a_positions.empty() || std::ranges::find(a_positions, nullptr) != a_positions.end()) {
            a_vm->TraceStack("Cannot lookup animations without actors", a_stackID);
            return {};
        }
        if (std::ranges::find(a_submissives, nullptr) != a_submissives.end()) {
            a_vm->TraceStack("None actor in submissives", a_stackID);
            return {};
        }
        const auto lib = Registry::Library::GetSingleton();
        const auto tags = Util::StringSplit(a_tags, ",");
        auto scenes = lib->LookupScenes(a_positions, tags, a_submissives);
        if (const auto pretrim = scenes.size()) {
            logger::info("Lookup found {} Scenes. Validating by center preference...", pretrim);
            if (a_center) {
                const auto details = lib->GetFurnitureDetails(a_center);
                if (details) {
                    std::erase_if(scenes, [&](const Registry::Animation::Scene* a_scene) {
                        return !a_scene->IsCompatibleFurniture(details);
                    });
                }
            } else if (a_furniturepref == FurniturePreference::Prefer) {
                const auto where = std::remove_if(scenes.begin(), scenes.end(), [&](const Registry::Animation::Scene* a_scene) {
                    return !a_scene->RequiresFurniture();
                });
                if (where != scenes.begin()) {
                    scenes.erase(where, scenes.end());
                } else {
                    logger::info("Validating Center; Prefering furnitures but no furniture animations in set");
                }
            } else if (a_furniturepref == FurniturePreference::Disallow) {
                std::erase_if(scenes, [&](const Registry::Animation::Scene* a_scene) {
                    return a_scene->RequiresFurniture();
                });
            }
            logger::info("Validated Center; Returning {}/{} scenes", scenes.size(), pretrim);
        }
        std::vector<RE::BSFixedString> ret{};
        ret.reserve(scenes.size());
        for (auto&& scene : scenes)
            ret.push_back(RE::BSFixedString{ scene->GetId().data() });
        return ret;
    }

    bool ValidateScene(STATICARGS,
        RE::BSFixedString a_sceneid, std::vector<RE::Actor*> a_positions, std::string a_tags, RE::Actor* a_submissive)
    {
        auto argSubmissive{ a_submissive ? std::vector<RE::Actor*>{ a_submissive } : std::vector<RE::Actor*>{} };
        return ValidateSceneA(a_vm, a_stackID, nullptr, a_sceneid, a_positions, a_tags, argSubmissive);
    }

    bool ValidateSceneA(STATICARGS,
        RE::BSFixedString a_sceneid, std::vector<RE::Actor*> a_positions, std::string a_tags, std::vector<RE::Actor*> a_submissives)
    {
        return !ValidateScenesA(a_vm, a_stackID, nullptr, { a_sceneid }, a_positions, a_tags, a_submissives).empty();
    }

    std::vector<RE::BSFixedString> ValidateScenes(STATICARGS,
        std::vector<RE::BSFixedString> a_sceneids, std::vector<RE::Actor*> a_positions, std::string a_tags, RE::Actor* a_submissive)
    {
        auto argSubmissive{ a_submissive ? std::vector<RE::Actor*>{ a_submissive } : std::vector<RE::Actor*>{} };
        return ValidateScenesA(a_vm, a_stackID, nullptr, a_sceneids, a_positions, a_tags, argSubmissive);
    }

    std::vector<RE::BSFixedString> ValidateScenesA(STATICARGS,
        std::vector<RE::BSFixedString> a_sceneids, std::vector<RE::Actor*> a_positions, std::string a_tags, std::vector<RE::Actor*> a_submissives)
    {
        if (a_positions.empty()) {
            a_vm->TraceStack("Cannot validate scenes against an empty position array", a_stackID);
            return {};
        }
        if (std::find(a_positions.begin(), a_positions.end(), nullptr) != a_positions.end()) {
            a_vm->TraceStack("Array contains none", a_stackID);
            return {};
        }
        if (std::find(a_submissives.begin(), a_submissives.end(), nullptr) != a_submissives.end()) {
            a_vm->TraceStack("Array contains none", a_stackID);
            return {};
        }
        if (a_sceneids.empty()) {
            return {};
        }
        std::vector<RE::BSFixedString> ret{};
        ret.reserve(a_sceneids.size());
        const auto fragments = Registry::ActorFragment::MakeFragmentList(a_positions, a_submissives);
        const auto tagdetail = Registry::TagDetails{ a_tags };
        const auto lib = Registry::Library::GetSingleton();
        for (auto&& sceneid : a_sceneids) {
            const auto scene = lib->GetSceneById(sceneid);
            if (!scene) {
                a_vm->TraceStack("Invalid scene id ", a_stackID);
                break;
            }
            if (!scene->IsCompatibleTags(tagdetail))
                continue;
            if (scene->FindAssignments(fragments).empty())
                continue;
            ret.push_back(sceneid);
        }
        logger::info("Validated Scenes, return {}/{} Scenes", ret.size(), a_sceneids.size());
        return ret;
    }

    bool SortByScene(STATICARGS, RE::reference_array<RE::Actor*> a_positions, RE::Actor* a_victim, std::string a_sceneid)
    {
        if (a_positions.empty() || std::ranges::find(a_positions, nullptr) != a_positions.end()) {
            a_vm->TraceStack("Array is empty or contains none", a_stackID);
            return false;
        }
        const auto lib = Registry::Library::GetSingleton();
        const auto scene = lib->GetSceneById(a_sceneid);
        if (!scene) {
            a_vm->TraceStack("Invalid scene id ", a_stackID);
            return false;
        }
        std::vector<RE::Actor*> positions{ a_positions.begin(), a_positions.end() };
        const auto fragments = Registry::ActorFragment::MakeFragmentList(positions, { a_victim });
        const auto ret = scene->FindAssignments(fragments);
        if (ret.empty())
            return false;
        const auto& arr = ret.front();
        for (size_t i = 0; i < arr.size(); i++) {
            a_positions[i] = arr.at(i);
        }
        return true;
    }

    RE::BSFixedString GetSceneByName(RE::StaticFunctionTag*, RE::BSFixedString a_name)
    {
        auto ret = Registry::Library::GetSingleton()->GetSceneByName(a_name);
        return ret ? RE::BSFixedString{ ret->GetId().data() } : "";
    }

    bool SortBySceneA(STATICARGS, RE::reference_array<RE::Actor*> a_positions, std::vector<RE::Actor*> a_victims, std::string a_sceneid)
    {
        if (a_positions.empty() || std::ranges::find(a_positions, nullptr) != a_positions.end()) {
            a_vm->TraceStack("Position Array is empty or contains none", a_stackID);
            return false;
        }
        if (std::ranges::find(a_victims, nullptr) != a_victims.end()) {
            a_vm->TraceStack("Victim Array contains none", a_stackID);
            return false;
        }
        const auto lib = Registry::Library::GetSingleton();
        const auto scene = lib->GetSceneById(a_sceneid);
        if (!scene) {
            a_vm->TraceStack("Invalid scene id ", a_stackID);
            return false;
        }
        std::vector<RE::Actor*> positions{ a_positions.begin(), a_positions.end() };
        const auto fragments = Registry::ActorFragment::MakeFragmentList(positions, a_victims);
        const auto ret = scene->FindAssignments(fragments);
        if (ret.empty())
            return false;
        const auto& arr = ret.front();
        for (size_t i = 0; i < arr.size(); i++) {
            a_positions[i] = arr.at(i);
        }
        return true;
    }

    int32_t SortBySceneEx(STATICARGS, RE::reference_array<RE::Actor*> a_positions, RE::Actor* a_victim, std::vector<std::string> a_sceneids)
    {
        if (a_positions.empty() || std::ranges::find(a_positions, nullptr) != a_positions.end()) {
            a_vm->TraceStack("Array is empty or contains none", a_stackID);
            return -1;
        }
        const auto lib = Registry::Library::GetSingleton();
        for (size_t i = 0; i < a_sceneids.size(); i++) {
            const auto scene = lib->GetSceneById(a_sceneids[i]);
            if (!scene) {
                a_vm->TraceStack("Invalid scene id ", a_stackID);
                break;
            }
            std::vector<RE::Actor*> positions{ a_positions.begin(), a_positions.end() };
            const auto fragments = Registry::ActorFragment::MakeFragmentList(positions, { a_victim });
            const auto result = scene->FindAssignments(fragments);
            if (result.empty())
                continue;
            const auto& arr = result.front();
            for (size_t n = 0; n < arr.size(); n++) {
                a_positions[n] = arr.at(n);
            }
            return static_cast<int32_t>(i);
        }
        return -1;
    }

    int32_t SortBySceneExA(STATICARGS, RE::reference_array<RE::Actor*> a_positions, std::vector<RE::Actor*> a_victims, std::vector<std::string> a_sceneids)
    {
        if (a_positions.empty() || std::ranges::find(a_positions, nullptr) != a_positions.end()) {
            a_vm->TraceStack("Array is empty or contains none", a_stackID);
            return -1;
        }
        if (std::ranges::find(a_victims, nullptr) != a_victims.end()) {
            a_vm->TraceStack("Array is empty or contains none", a_stackID);
            return -1;
        }
        const auto lib = Registry::Library::GetSingleton();
        for (size_t i = 0; i < a_sceneids.size(); i++) {
            const auto scene = lib->GetSceneById(a_sceneids[i]);
            if (!scene) {
                a_vm->TraceStack("Invalid scene id ", a_stackID);
                break;
            }
            std::vector<RE::Actor*> positions{ a_positions.begin(), a_positions.end() };
            const auto fragments = Registry::ActorFragment::MakeFragmentList(positions, a_victims);
            const auto result = scene->FindAssignments(fragments);
            if (result.empty())
                continue;
            const auto& arr = result.front();
            for (size_t n = 0; n < arr.size(); n++) {
                a_positions[n] = arr.at(n);
            }
            return static_cast<int32_t>(i);
        }
        return -1;
    }

    bool SceneExists(RE::StaticFunctionTag*, RE::BSFixedString a_sceneid)
    {
        return Registry::Library::GetSingleton()->GetSceneById(a_sceneid);
    }

    std::vector<RE::BSFixedString> SceneExistA(RE::StaticFunctionTag*, std::vector<RE::BSFixedString> a_sceneids)
    {
        const auto lib = Registry::Library::GetSingleton();
        std::vector<RE::BSFixedString> ret{};
        ret.reserve(a_sceneids.size());
        for (auto&& id : a_sceneids) {
            if (!lib->GetSceneById(id))
                continue;
            ret.push_back(id);
        }
        return ret;
    }

    bool StageExists(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage)
    {
        SCENE(false);
        return scene->GetStageById(a_stage) != nullptr;
    }

    bool IsSceneEnabled(STATICARGS, RE::BSFixedString a_id)
    {
        SCENE(false);
        return scene->IsEnabled();
    }

    void SetSceneEnabled(STATICARGS, RE::BSFixedString a_id, bool a_enabled)
    {
        const auto foundScene = Registry::Library::GetSingleton()->EditScene(a_id, [&](auto scene) {
            scene->SetEnabled(a_enabled);
        });
        if (!foundScene) {
            a_vm->TraceStack("Invalid scene id", a_stackID);
            return;
        }
    }

    RE::BSFixedString GetSceneName(STATICARGS, RE::BSFixedString a_id)
    {
        SCENE("");
        return RE::BSFixedString{ scene->GetName().data() };
    }

    bool IsCompatibleCenter(STATICARGS, RE::BSFixedString a_id, RE::TESObjectREFR* a_center)
    {
        SCENE(false);
        if (!a_center) {
            a_vm->TraceStack("None center ref", a_stackID);
            return false;
        }
        const auto details = Registry::Library::GetSingleton()->GetFurnitureDetails(a_center);
        return scene->IsCompatibleFurniture(details);
    }

    std::vector<RE::BSFixedString> GetSceneTags(STATICARGS, RE::BSFixedString a_id)
    {
        SCENE({});
        return scene->GetTags().AsVector();
    }

    std::vector<RE::BSFixedString> GetCommonTags(STATICARGS, std::vector<RE::BSFixedString> a_ids)
    {
        const auto lib = Registry::Library::GetSingleton();
        bool first = true;
        Registry::TagData ret{};
        for (auto&& sceneid : a_ids) {
            const auto scene = lib->GetSceneById(sceneid);
            if (!scene) {
                a_vm->TraceStack("Invalid scene id", a_stackID);
                break;
            }
            if (first) {
                first = false;
                ret.AddTag(scene->GetTags());
            } else {
                ret.IntersectTags(scene->GetTags());
            }
        }
        return ret.AsVector();
    }

    bool HasSceneTag(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_tag)
    {
        SCENE(false);
        return scene->GetTags().HasTag(a_tag);
    }

    bool HasSceneTagA(STATICARGS, RE::BSFixedString a_id, std::vector<std::string_view> a_tags)
    {
        SCENE(false);
        const auto details = Registry::TagDetails(a_tags);
        return scene->IsCompatibleTags(details);
    }

    bool AddSceneTag(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_tag)
    {
        return Registry::Library::GetSingleton()->EditScene(a_id, [&](auto scene) {
            scene->GetTags().AddAnnotation(a_tag);
        });
    }

    bool RemoveSceneTag(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_tag)
    {
        return Registry::Library::GetSingleton()->EditScene(a_id, [&](auto scene) {
            scene->GetTags().RemoveAnnotation(a_tag);
        });
    }

    std::vector<RE::BSFixedString> GetStageTags(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage)
    {
        STAGE({});
        return stage->GetTags().AsVector();
    }

    bool HasStageTag(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, RE::BSFixedString a_tag)
    {
        STAGE(false);
        return stage->GetTags().HasTag(a_tag);
    }

    bool HasStageTagA(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, std::vector<std::string_view> a_tags)
    {
        STAGE(false);
        const auto details = Registry::TagDetails{ a_tags };
        return details.MatchTags(stage->GetTags());
    }

    bool AddStageTag(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, RE::BSFixedString a_tag)
    {
        return Registry::Library::GetSingleton()->EditScene(a_id, [&](auto scene) {
            const auto stage = scene->GetStageById(a_stage);
            if (!stage) {
                return;
            }
            stage->GetTags().AddAnnotation(a_tag);
        });
    }

    bool RemoveStageTag(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, RE::BSFixedString a_tag)
    {
        return Registry::Library::GetSingleton()->EditScene(a_id, [&](auto scene) {
            const auto stage = scene->GetStageById(a_stage);
            if (!stage) {
                return;
            }
            stage->GetTags().RemoveAnnotation(a_tag);
        });
    }

    std::vector<RE::BSFixedString> GetPositionTags(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, int n)
    {
        STAGE({});
        POSITION({});
        return position.GetTags().AsVector();
    }

    bool HasPositionTag(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, int n, RE::BSFixedString a_tag)
    {
        STAGE(false);
        POSITION(false);
        return position.GetTags().HasTag(a_tag);
    }

    bool HasPositionTagA(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, int n, std::vector<std::string_view> a_tags)
    {
        STAGE(false);
        POSITION(false);
        const auto details = Registry::TagDetails{ a_tags };
        return details.MatchTags(position.GetTags());
    }

    bool AddPositionTag(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, int n, RE::BSFixedString a_tag)
    {
        return Registry::Library::GetSingleton()->EditScene(a_id, [&](auto scene) {
            const auto stage = scene->GetStageById(a_stage);
            if (!stage) {
                return;
            }
            if (n < 0 || static_cast<size_t>(n) >= stage->GetPositions().size()) {
                return;
            }
            stage->GetPositions()[n].GetTags().AddAnnotation(a_tag);
        });
    }

    bool RemovePositionTag(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, int n, RE::BSFixedString a_tag)
    {
        return Registry::Library::GetSingleton()->EditScene(a_id, [&](auto scene) {
            const auto stage = scene->GetStageById(a_stage);
            if (!stage) {
                return;
            }
            if (n < 0 || static_cast<size_t>(n) >= stage->GetPositions().size()) {
                return;
            }
            stage->GetPositions()[n].GetTags().RemoveAnnotation(a_tag);
        });
    }

    RE::BSFixedString GetAnimationEvent(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, int n)
    {
        STAGE("");
        if (n < 0 || static_cast<size_t>(n) >= stage->GetPositions().size()) {
            a_vm->TraceStack("GetAnimationEvent: Index out of bounds", 1);
            return "";
        }
        return stage->GetAnimationEvent(n);
    }

    std::vector<RE::BSFixedString> GetAnimationEventA(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage)
    {
        STAGE({});
        return stage->GetAnimationEvents();
    }

    RE::BSFixedString GetStartAnimation(STATICARGS, RE::BSFixedString a_id)
    {
        SCENE("");
        const auto start = scene->GetStageById("");
        return start ? RE::BSFixedString{ start->GetId().data() } : "";
    }

    int32_t GetNumStages(STATICARGS, RE::BSFixedString a_id)
    {
        SCENE(-1);
        return static_cast<int32_t>(scene->GetNumStages());
    }

    std::vector<RE::BSFixedString> GetAllstages(STATICARGS, RE::BSFixedString a_id)
    {
        SCENE({});
        std::vector<RE::BSFixedString> ret{};
        scene->ForEachStage([&](const auto& stage) {
            ret.push_back(RE::BSFixedString{ stage->GetId().data() });
            return false;
        });
        return ret;
    }

    RE::BSFixedString BranchTo(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, int n)
    {
        STAGE("");
        const auto& edges = stage->GetOutgoingEdges();
        if (n < 0 || static_cast<size_t>(n) >= edges.size()) {
            return "";
        }
        return RE::BSFixedString{ edges[n]->GetId().data() };
    }

    int32_t GetNumBranches(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage)
    {
        STAGE(0);
        return static_cast<int32_t>(stage->GetOutgoingEdges().size());
    }

    std::vector<RE::BSFixedString> GetPathMin(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage)
    {
        STAGE({});
        const auto path = stage->GetShortestPath();
        std::vector<RE::BSFixedString> ret{};
        for (auto&& p : path) {
            ret.push_back(RE::BSFixedString{ p->GetId().data() });
        }
        return ret;
    }

    std::vector<RE::BSFixedString> GetPathMax(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage)
    {
        STAGE({});
        const auto path = stage->GetLongestPath();
        std::vector<RE::BSFixedString> ret{};
        for (auto&& p : path) {
            ret.push_back(RE::BSFixedString{ p->GetId().data() });
        }
        return ret;
    }

    int32_t GetActorCount(STATICARGS, RE::BSFixedString a_id)
    {
        SCENE(false);
        return scene->GetNumPositions();
    }

    bool IsSimilarPosition(STATICARGS, RE::BSFixedString a_id, int n, int m)
    {
        SCENE(false);
        if (n < 0 || m < 0 || n >= scene->GetPositions().size() || m >= scene->GetPositions().size()) {
            a_vm->TraceStack("Invalid position idx", a_stackID);
            return false;
        }
        return scene->GetPositions()[n].CanFillPosition(scene->GetPositions()[m]);
    }

    bool CanFillPosition(STATICARGS, RE::BSFixedString a_id, int n, RE::Actor* a_actor)
    {
        if (!a_actor) {
            a_vm->TraceStack("Actor is none", a_stackID);
            return false;
        }
        SCENE(false);
        POSITION_META(false);
        return scene->GetPositions()[n].CanFillPosition(a_actor);
    }

    std::vector<RE::BSFixedString> GetFixedLengthStages(STATICARGS, RE::BSFixedString a_id)
    {
        SCENE({});
        const auto stages = scene->GetFixedLengthStages();
        std::vector<RE::BSFixedString> ret{};
        for (auto&& stage : stages) {
            ret.push_back(RE::BSFixedString{ stage->GetId().data() });
        }
        return ret;
    }

    float GetFixedLength(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage)
    {
        STAGE(0);
        return stage->GetFixedDuration() / 1000.0f;
    }

    std::vector<RE::BSFixedString> GetClimaxStages(STATICARGS, RE::BSFixedString a_id, int32_t n)
    {
        SCENE({});
        if (n >= scene->GetPositions().size()) {
            a_vm->TraceStack("Invalid position idx", a_stackID);
            return {};
        }
        const auto stages = scene->GetClimaxStages();
        std::vector<RE::BSFixedString> ret{};
        for (auto&& stage : stages) {
            if (n == -1 || stage->GetPositions()[n].IsClimax())
                ret.push_back(RE::BSFixedString{ stage->GetId().data() });
        }
        return ret;
    }

    std::vector<int32_t> GetClimaxingActors(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage)
    {
        STAGE({});
        std::vector<int32_t> ret{};
        const auto& stagePositions = stage->GetPositions();
        for (int32_t i = 0; i < static_cast<int32_t>(stagePositions.size()); i++) {
            if (stagePositions[i].IsClimax()) {
                ret.push_back(i);
            }
        }
        return ret;
    }

    std::vector<RE::BSFixedString> GetEndingStages(STATICARGS, RE::BSFixedString a_id)
    {
        SCENE({});
        const auto stages = scene->GetEndingStages();
        std::vector<RE::BSFixedString> ret{};
        for (auto&& stage : stages) {
            ret.push_back(RE::BSFixedString{ stage->GetId().data() });
        }
        return ret;
    }

    int32_t GetPositionSex(STATICARGS, RE::BSFixedString a_id, int n)
    {
        SCENE(0);
        POSITION_META(0);
        return static_cast<int32_t>(scene->GetPositions()[n].GetSexPapyrus());
    }

    std::vector<int32_t> GetPositionSexA(STATICARGS, RE::BSFixedString a_id)
    {
        SCENE({});
        std::vector<int32_t> ret{};
        ret.reserve(scene->GetPositions().size());
        for (auto&& position : scene->GetPositions()) {
            const auto sex = static_cast<int32_t>(position.GetSexPapyrus());
            ret.push_back(sex);
        }
        return ret;
    }

    int32_t GetRaceIDPosition(STATICARGS, RE::BSFixedString a_id, int n)
    {
        SCENE(0);
        POSITION_META(0);
        return static_cast<int32_t>(scene->GetPositions()[n].get().GetRace());
    }

    std::vector<int32_t> GetRaceIDPositionA(STATICARGS, RE::BSFixedString a_id)
    {
        SCENE({});
        std::vector<int32_t> ret{};
        ret.reserve(scene->GetPositions().size());
        for (auto&& position : scene->GetPositions()) {
            const auto race = static_cast<int32_t>(position.get().GetRace());
            ret.push_back(race);
        }
        return ret;
    }

    RE::BSFixedString GetRaceKeyPosition(STATICARGS, RE::BSFixedString a_id, int n)
    {
        SCENE("");
        POSITION_META("");
        return scene->GetPositions()[n].get().GetRace().AsString();
    }

    std::vector<RE::BSFixedString> GetRaceKeyPositionA(STATICARGS, RE::BSFixedString a_id)
    {
        SCENE({});
        std::vector<RE::BSFixedString> ret{};
        ret.reserve(scene->GetPositions().size());
        for (auto&& position : scene->GetPositions()) {
            const auto race = position.get().GetRace().AsString();
            ret.push_back(race);
        }
        return ret;
    }

    std::vector<float> GetSceneOffset(STATICARGS, RE::BSFixedString a_id)
    {
        std::vector<float> argRet{ 0, 0, 0, 0 };
        SCENE(argRet);
        return scene->GetFurnitureOffset().GetOffset().AsVector();
    }

    std::vector<float> GetSceneOffsetRaw(STATICARGS, RE::BSFixedString a_id)
    {
        std::vector<float> argRet{ 0, 0, 0, 0 };
        SCENE(argRet);
        return scene->GetFurnitureOffset().GetRawOffset().AsVector();
    }

    void SetSceneOffset(STATICARGS, RE::BSFixedString a_id, float a_value, Registry::CoordinateType a_idx)
    {
        if (a_idx < Registry::CoordinateType::X || a_idx >= Registry::CoordinateType::Total) {
            a_vm->TraceStack("Invalid offset idx", a_stackID);
            return;
        }
        const auto& func = [&](auto scene) {
            scene->GetFurnitureOffset().SetOffset(a_value, a_idx);
        };
        const auto foundScene = Registry::Library::GetSingleton()->EditScene(a_id, func);
        if (!foundScene) {
            a_vm->TraceStack("Invalid scene id", a_stackID);
            return;
        }
    }

    void SetSceneOffsetA(STATICARGS, RE::BSFixedString a_id, std::vector<float> a_newoffset)
    {
        if (a_newoffset.size() < Registry::CoordinateType::Total) {
            a_vm->TraceStack("New offsets are of incorrect size", a_stackID);
            return;
        }
        const auto& func = [&](auto scene) {
            const Registry::Coordinate coordinate{ a_newoffset };
            scene->GetFurnitureOffset().SetOffset(coordinate);
        };
        const auto foundScene = Registry::Library::GetSingleton()->EditScene(a_id, func);
        if (!foundScene) {
            a_vm->TraceStack("Invalid scene id", a_stackID);
            return;
        }
    }

    void ResetSceneOffset(STATICARGS, RE::BSFixedString a_id)
    {
        const auto foundScene = Registry::Library::GetSingleton()->EditScene(a_id, [&](auto scene) {
            scene->GetFurnitureOffset().ResetOffset();
        });
        if (!foundScene) {
            a_vm->TraceStack("Invalid scene id", a_stackID);
            return;
        }
    }

    std::vector<float> GetStageOffset(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, int n)
    {
        std::vector<float> argRet{ 0, 0, 0, 0 };
        STAGE(argRet);
        POSITION(argRet);
        return position.GetOffset().GetOffset().AsVector();
    }

    std::vector<float> GetStageOffsetRaw(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, int n)
    {
        std::vector<float> argRet{ 0, 0, 0, 0 };
        STAGE(argRet);
        POSITION(argRet);
        return position.GetOffset().GetRawOffset().AsVector();
    }

    void SetStageOffset(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, int n, float a_value, Registry::CoordinateType a_idx)
    {
        bool fouundScene = Registry::Library::GetSingleton()->EditScene(a_id, [&](auto scene) {
            POSITION_META((void)0);
            if (a_idx < Registry::CoordinateType::X || a_idx >= Registry::CoordinateType::Total) {
                a_vm->TraceStack("Invalid offset idx", a_stackID);
                return;
            }
            if (a_stage.empty()) {
                scene->ForEachStage([&](Registry::Animation::Stage* a_stage) {
                    a_stage->GetPositions()[n].GetOffset().SetOffset(a_value, a_idx);
                    return false;
                });
            } else {
                const auto stage = scene->GetStageById(a_stage);
                if (!stage) {
                    a_vm->TraceStack("Invalid stage id", a_stackID);
                    return;
                }
                stage->GetPositions()[n].GetOffset().SetOffset(a_value, a_idx);
            }
        });
        if (!fouundScene) {
            a_vm->TraceStack("Invalid scene id", a_stackID);
            return;
        }
    }

    void SetStageOffsetA(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, int n, std::vector<float> a_newoffset)
    {
        bool foundScene = Registry::Library::GetSingleton()->EditScene(a_id, [&](auto scene) {
            POSITION_META((void)0);
            if (a_newoffset.size() < Registry::CoordinateType::Total) {
                a_vm->TraceStack("New offsets are of incorrect size", a_stackID);
                return;
            }
            const Registry::Coordinate coordinate{ a_newoffset };
            if (a_stage.empty()) {
                scene->ForEachStage([&](Registry::Animation::Stage* a_stage) {
                    a_stage->GetPositions()[n].GetOffset().SetOffset(coordinate);
                    return false;
                });
            } else {
                const auto stage = scene->GetStageById(a_stage);
                if (!stage) {
                    a_vm->TraceStack("Invalid stage id", a_stackID);
                    return;
                }
                stage->GetPositions()[n].GetOffset().SetOffset(coordinate);
            }
        });
        if (!foundScene) {
            a_vm->TraceStack("Invalid scene id", a_stackID);
            return;
        }
    }

    void ResetStageOffset(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, int n)
    {
        bool foundScene = !Registry::Library::GetSingleton()->EditScene(a_id, [&](auto scene) {
            const auto stage = scene->GetStageById(a_stage);
            if (!stage) {
                a_vm->TraceStack("Invalid stage id", a_stackID);
                return;
            }
            POSITION_META((void)0);
            stage->GetPositions()[n].GetOffset().ResetOffset();
        });
        if (!foundScene) {
            a_vm->TraceStack("Invalid scene id", a_stackID);
            return;
        }
    }

    void ResetStageOffsetA(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage)
    {
        const auto foundScene = Registry::Library::GetSingleton()->EditScene(a_id, [&](auto scene) {
            const auto stage = scene->GetStageById(a_stage);
            if (!stage) {
                a_vm->TraceStack("Invalid stage id", a_stackID);
                return;
            }
            for (auto&& pos : stage->GetPositions()) {
                pos.GetOffset().ResetOffset();
            }
        });
        if (!foundScene) {
            a_vm->TraceStack("Invalid scene id", a_stackID);
            return;
        }
    }

    int32_t GetStripData(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage, int n)
    {
        STAGE(0);
        POSITION(0);
        return position.GetStrips().underlying();
    }

    std::vector<int32_t> GetStripDataA(STATICARGS, RE::BSFixedString a_id, RE::BSFixedString a_stage)
    {
        STAGE({});
        std::vector<int32_t> ret{};
        ret.reserve(stage->GetPositions().size());
        for (auto&& position : stage->GetPositions()) {
            ret.push_back(position.GetStrips().underlying());
        }
        return ret;
    }

}  // namespace Papyrus::SexLabRegistry
