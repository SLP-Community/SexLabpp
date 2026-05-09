#include "sslThreadLibrary.h"

#include "Registry/Define/Animation.h"
#include "Registry/Define/Fragment.h"
#include "Registry/Define/Furniture.h"
#include "Registry/Define/RaceKey.h"
#include "Registry/Library.h"
#include "Registry/Validation.h"
#include "Serialize.h"
#include "Util/World.h"

namespace Papyrus::ThreadLibrary
{
    std::vector<RE::TESObjectREFR*> FindBeds(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* a_center, float a_radius, float a_radiusZ)
    {
        if (!a_center) {
            a_vm->TraceStack("Cannot find refs from a none center", a_stackID);
            return {};
        } else if (a_radius < 0.0f) {
            a_vm->TraceStack("Cannot find refs within a negative radius", a_stackID);
            return {};
        }
        const auto center = a_center->GetPosition();
        std::vector<RE::TESObjectREFR*> ret{};
        Util::ForEachObjectInRange(a_center, a_radius, [&](RE::TESObjectREFR* ref) {
            if (!ref || !ref->GetBaseObject()->Is(RE::FormType::Furniture) && a_radiusZ > 0.0f ? (std::fabs(center.z - ref->GetPosition().z) <= a_radiusZ) : true)
                if (Registry::FurnitureType::IsBedType(ref))
                    ret.push_back(ref);
            return RE::BSContainer::ForEachResult::kContinue;
        });
        std::sort(ret.begin(), ret.end(), [&](RE::TESObjectREFR* a_refA, RE::TESObjectREFR* a_refB) {
            return center.GetDistance(a_refA->GetPosition()) < center.GetDistance(a_refB->GetPosition());
        });
        return ret;
    }

    int32_t GetBedTypeImpl(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* a_reference)
    {
        if (!a_reference) {
            a_vm->TraceStack("Reference is none", a_stackID);
            return false;
        }
        return static_cast<int32_t>(Registry::FurnitureType::GetBedType(a_reference));
    }

    bool IsBed(VM* a_vm, RE::VMStackID a_stackID, RE::StaticFunctionTag*, RE::TESObjectREFR* a_reference)
    {
        if (!a_reference) {
            a_vm->TraceStack("Reference is none", a_stackID);
            return false;
        }
        return Registry::FurnitureType::IsBedType(a_reference);
    }

    std::vector<RE::Actor*> FindAvailableActors(VM* a_vm, StackID a_stackID, RE::TESQuest*, RE::TESObjectREFR* a_center, float a_radius, LegacySex a_targetsex,
        RE::Actor* ignore_ref01, RE::Actor* ignore_ref02, RE::Actor* ignore_ref03, RE::Actor* ignore_ref04, RE::BSFixedString a_targetrace)
    {
        if (!a_center) {
            a_vm->TraceStack("Cannot find actor from a none reference", a_stackID);
            return {};
        } else if (a_targetsex < LegacySex::None || a_targetsex > LegacySex::CrtFemale) {
            const auto err = std::format("Invalid target sex. Argument should be in [{}; {}]", std::to_underlying(LegacySex::None), std::to_underlying(LegacySex::CrtFemale));
            a_vm->TraceStack(err.c_str(), a_stackID);
            return {};
        } else if (a_radius < 0) {
            a_vm->TraceStack("Cannot find actor in negative radius", a_stackID);
            return {};
        }
        const Registry::RaceKey targetrace{ a_targetrace.empty() ? "Human" : a_targetrace };
        const auto targetsex = [&]() {
            if (a_targetsex >= LegacySex::CrtMale || !targetrace.Is(Registry::RaceKey::Human)) {
                if (!Settings::bCreatureGender || a_targetsex == LegacySex::Male || a_targetsex == LegacySex::CrtMale) {
                    return LegacySex::CrtMale;
                } else if (a_targetsex == LegacySex::Female) {
                    return LegacySex::CrtFemale;
                }
            }
            return a_targetsex;
        }();
        std::vector<RE::Actor*> ret{};
        const auto& highactors = RE::ProcessLists::GetSingleton()->highActorHandles;
        for (auto&& handle : highactors) {
            const auto& actor = handle.get();
            if (!actor ||
                actor.get() == ignore_ref01 ||
                actor.get() == ignore_ref02 ||
                actor.get() == ignore_ref03 ||
                actor.get() == ignore_ref04)
                continue;

            if (!targetrace.IsCompatibleWith(actor.get()))
                continue;
            if (targetsex != LegacySex::None && GetLegacySex(actor.get()) != targetsex)
                continue;
            if (!Registry::IsValidActor(actor.get()))
                continue;

            ret.push_back(actor.get());
        }
        return ret;
    }

    RE::Actor* FindAvailableActor(VM* a_vm, StackID a_stackID, RE::TESQuest*, RE::TESObjectREFR* a_center, float a_radius, LegacySex a_targetsex,
        RE::Actor* ignore_ref01, RE::Actor* ignore_ref02, RE::Actor* ignore_ref03, RE::Actor* ignore_ref04, RE::BSFixedString a_targetrace)
    {
        auto valids = FindAvailableActors(a_vm, a_stackID, nullptr, a_center, a_radius, a_targetsex, ignore_ref01, ignore_ref02, ignore_ref03, ignore_ref04, a_targetrace);
        return valids.empty() ? nullptr : valids[0];
    }

    RE::Actor* FindAvailableActorInFaction(VM* a_vm, StackID a_stackID, RE::TESQuest*, RE::TESFaction* a_faction, RE::TESObjectREFR* a_center, float a_radius, LegacySex a_targetsex,
        RE::Actor* ignore_ref01, RE::Actor* ignore_ref02, RE::Actor* ignore_ref03, RE::Actor* ignore_ref04, bool a_hasfaction, RE::BSFixedString a_targetrace, bool a_samefloor)
    {
        if (!a_faction) {
            a_vm->TraceStack("Cannot find actor in none faction", a_stackID);
            return nullptr;
        }
        auto valids = FindAvailableActors(a_vm, a_stackID, nullptr, a_center, a_radius, a_targetsex, ignore_ref01, ignore_ref02, ignore_ref03, ignore_ref04, a_targetrace);
        for (auto&& actor : valids) {
            if (a_samefloor && std::fabs(a_center->GetPositionZ() - actor->GetPositionZ()) > 200)
                continue;
            if (actor->IsInFaction(a_faction) != a_hasfaction)
                continue;

            return actor;
        }
        return nullptr;
    }

    RE::Actor* FindAvailableActorWornForm(VM* a_vm, StackID a_stackID, RE::TESQuest*, uint32_t a_slotmask, RE::TESObjectREFR* a_center, float a_radius, LegacySex a_targetsex,
        RE::Actor* ignore_ref01, RE::Actor* ignore_ref02, RE::Actor* ignore_ref03, RE::Actor* ignore_ref04, bool a_recognizenostrip, bool a_shouldwear, RE::BSFixedString a_targetrace,
        bool a_samefloor)
    {
        if (a_slotmask == 0) {
            a_vm->TraceStack("Cannot find actor from worn form without slotmask", a_stackID);
            return nullptr;
        }
        const auto slotmask = RE::BGSBipedObjectForm::BipedObjectSlot(a_slotmask);
        const auto valids = FindAvailableActors(a_vm, a_stackID, nullptr, a_center, a_radius, a_targetsex, ignore_ref01, ignore_ref02, ignore_ref03, ignore_ref04, a_targetrace);
        for (auto&& actor : valids) {
            if (a_samefloor && std::fabs(a_center->GetPositionZ() - actor->GetPositionZ()) > 200)
                continue;
            const auto armo = actor->GetWornArmor(slotmask);
            if (static_cast<bool>(armo) != a_shouldwear)
                continue;
            if (a_recognizenostrip) {
                const auto kywdform = armo ? armo->As<RE::BGSKeywordForm>() : nullptr;
                if (kywdform && kywdform->ContainsKeywordString("NoStrip"))
                    continue;
            }

            return actor;
        }
        return nullptr;
    }

    std::vector<RE::Actor*> FindAvailablePartners(VM* a_vm, StackID a_stackID, RE::TESQuest*,
        std::vector<RE::Actor*> a_positions, int a_total, int a_males, int a_females, float a_radius)
    {
        if (a_positions.size() >= a_total) {
            return a_positions;
        }
        const auto valids = FindAvailableActors(a_vm, a_stackID, nullptr,
            a_positions.empty() ? RE::PlayerCharacter::GetSingleton() : a_positions[0],
            a_radius,
            LegacySex::None,
            a_positions.size() > 0 ? a_positions[0] : nullptr,
            a_positions.size() > 1 ? a_positions[1] : nullptr,
            a_positions.size() > 2 ? a_positions[2] : nullptr,
            a_positions.size() > 3 ? a_positions[3] : nullptr,
            "");

        if (valids.empty()) {
            return a_positions;
        }
        auto genders = GetLegacySex(a_positions);
        for (auto&& actor : valids) {
            int targetsex;
            if (genders[LegacySex::Male] < a_males) {
                targetsex = LegacySex::Male;
            } else if (genders[LegacySex::Female] < a_females) {
                targetsex = LegacySex::Female;
            } else {
                targetsex = LegacySex::None;
            }
            const auto sex = GetLegacySex(actor);
            if (targetsex == LegacySex::None || sex == targetsex) {
                a_positions.push_back(actor);
                if (a_positions.size() == a_total) {
                    return a_positions;
                } else {
                    genders[sex]++;
                }
            }
        }
        return a_positions;
    }

    std::vector<RE::Actor*> FindAnimationPartnersImpl(VM* a_vm, StackID a_stackID, RE::TESQuest*,
        RE::BSFixedString a_sceneid, RE::TESObjectREFR* a_center, float a_radius, std::vector<RE::Actor*> a_includes)
    {
        const auto scene = Registry::Library::GetSingleton()->GetSceneById(a_sceneid);
        if (!scene) {
            a_vm->TraceStack("Cannot find actors for a none scene", a_stackID);
            return {};
        }
        std::vector<RE::Actor*> ret{};
        auto valids = FindAvailableActors(a_vm, a_stackID, nullptr,
            a_center,
            a_radius,
            LegacySex::None,
            a_includes.size() > 0 ? a_includes[0] : nullptr,
            a_includes.size() > 1 ? a_includes[1] : nullptr,
            a_includes.size() > 2 ? a_includes[2] : nullptr,
            a_includes.size() > 3 ? a_includes[3] : nullptr,
            "");

        for (auto&& position : scene->positions) {
            RE::Actor* fill = nullptr;
            for (auto& include : a_includes) {
                if (include && position.CanFillPosition(include)) {
                    include = nullptr;
                    fill = include;
                    break;
                }
            }
            if (fill) {
                continue;
            }
            for (auto& valid : valids) {
                if (valid && position.CanFillPosition(valid)) {
                    valid = nullptr;
                    fill = valid;
                    break;
                }
            }
            if (!fill) {
                return {};
            }
        }
        return ret;
    }

    std::vector<RE::Actor*> SortActorsByAnimationImpl(VM* a_vm, StackID a_stackID, RE::TESQuest*,
        RE::BSFixedString a_sceneid, std::vector<RE::Actor*> a_positions, std::vector<RE::Actor*> a_submissives)
    {
        if (a_positions.empty() || std::ranges::find(a_positions, nullptr) != a_positions.end()) {
            a_vm->TraceStack("Array is empty or contains none", a_stackID);
            return {};
        }
        if (std::ranges::find(a_submissives, nullptr) != a_submissives.end()) {
            a_vm->TraceStack("Array is empty or contains none", a_stackID);
            return {};
        }
        const auto library = Registry::Library::GetSingleton();
        const auto scene = library->GetSceneById(a_sceneid);
        if (!scene) {
            a_vm->TraceStack("Cannot sort actors by a none scene", a_stackID);
            return a_positions;
        }
        auto subcount = scene->CountSubmissives();
        std::vector<Registry::ActorFragment> fragments;
        for (auto&& actor : a_positions) {
            const auto submissive = subcount > 0 && std::find(a_submissives.begin(), a_submissives.end(), actor) != a_submissives.end();
            if (submissive)
                subcount--;
            const auto fragment = Registry::ActorFragment{ actor, submissive };
            fragments.push_back(fragment);
        }
        auto ret = scene->FindAssignments(fragments);
        return ret.empty() ? a_positions : ret.front();
    }

    bool IsActorTrackedImpl(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
    {
        if (!a_actor) {
            a_vm->TraceStack("Cannot check track status on a none reference", a_stackID);
            return false;
        }
        if (a_actor->IsPlayerRef()) {
            return true;
        }
        const auto data = Tracking::GetSingleton();
        if (data->_actors.contains(a_actor->formID)) {
            return true;
        }

        bool ret = false;
        a_actor->VisitFactions([&](auto fac, auto rank) {
            if (!fac || rank < 0)
                return false;

            if (data->_factions.contains(fac->formID)) {
                ret = true;
                return true;
            }
            return false;
        });
        return ret;
    }

    void TrackActorImpl(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_callback, bool a_dotrack)
    {
        if (!a_actor) {
            a_vm->TraceStack("Cannot track on a none reference", a_stackID);
            return;
        }
        auto data = Tracking::GetSingleton();
        if (a_dotrack) {
            data->Add(data->_actors, a_actor->formID, a_callback);
        } else {
            data->Remove(data->_actors, a_actor->formID, a_callback);
        }
    }

    void TrackFactionImpl(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESFaction* a_faction, RE::BSFixedString a_callback, bool a_dotrack)
    {
        if (!a_faction) {
            a_vm->TraceStack("Cannot track on a none faction", a_stackID);
            return;
        }
        auto data = Tracking::GetSingleton();
        if (a_dotrack) {
            data->Add(data->_factions, a_faction->formID, a_callback);
        } else {
            data->Remove(data->_factions, a_faction->formID, a_callback);
        }
    }

    std::vector<std::string> GetAllTrackingEvents(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_hook)
    {
        if (!a_actor) {
            a_vm->TraceStack("Cannot get tracking events from a none reference", a_stackID);
            return {};
        }
        const auto suffix = a_hook.empty() ? "" : std::format("_{}", a_hook.data());
        auto data = Tracking::GetSingleton();
        std::vector<std::string> ret{};
        if (a_actor->IsPlayerRef()) {
            ret.push_back(std::format("PlayerTrack{}", suffix));
        }
        const auto where = data->_actors.find(a_actor->formID);
        if (where != data->_actors.end()) {
            for (auto&& event : where->second) {
                ret.push_back(std::format("{}{}", event, suffix));
            }
        }
        a_actor->VisitFactions([&](auto fac, auto rank) {
            if (!fac || rank < 0)
                return false;

            const auto it = data->_factions.find(fac->formID);
            if (it == data->_factions.end())
                return false;

            for (auto&& event : it->second) {
                ret.push_back(std::format("{}{}", event, suffix));
            }
            return false;
        });
        return ret;
    }

}  // namespace Papyrus::ThreadLibrary
