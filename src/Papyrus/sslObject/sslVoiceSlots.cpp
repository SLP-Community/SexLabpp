#include "sslVoiceSlots.h"

#include "Registry/Library.h"

namespace Papyrus::VoiceSlots
{
    namespace BaseVoice
    {
        bool GetEnabled(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id)
        {
            const auto vs = Registry::Library::GetSingleton();
            auto it = vs->GetVoiceById(a_id);
            if (!it) {
                a_vm->TraceStack("Invalid voice form", a_stackID);
                return false;
            }
            return it->enabled;
        }

        void SetEnabled(RE::StaticFunctionTag*, RE::BSFixedString a_id, bool a_enabled)
        {
            const auto vs = Registry::Library::GetSingleton();
            vs->SetVoiceEnabled(a_id, a_enabled);
        }

        std::vector<RE::BSFixedString> GetVoiceTags(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id)
        {
            const auto vs = Registry::Library::GetSingleton();
            auto it = vs->GetVoiceById(a_id);
            if (!it) {
                a_vm->TraceStack("Invalid voice form", a_stackID);
                return {};
            }
            return it->tags.AsVector();
        }

        int GetCompatibleSex(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id)
        {
            const auto vs = Registry::Library::GetSingleton();
            auto it = vs->GetVoiceById(a_id);
            if (!it) {
                a_vm->TraceStack("Invalid voice form", a_stackID);
                return {};
            }
            return it->sex == RE::SEXES::kNone ? -1 : it->sex;
        }

        std::vector<RE::BSFixedString> GetCompatibleRaces(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id)
        {
            const auto vs = Registry::Library::GetSingleton();
            auto it = vs->GetVoiceById(a_id);
            if (!it) {
                a_vm->TraceStack("Invalid voice form", a_stackID);
                return {};
            }
            std::vector<RE::BSFixedString> ret{};
            for (auto&& r : it->races) {
                ret.push_back(r.AsString());
            }
            return ret;
        }

        RE::TESSound* GetSoundObject(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id, int a_strength, RE::BSFixedString a_scene, int a_idx, bool a_muffled)
        {
            const auto lib = Registry::Library::GetSingleton();
            auto scene = lib->GetSceneById(a_scene);
            if (!scene) {
                a_vm->TraceStack("Invalid scene id", a_stackID);
                return nullptr;
            }
            if (scene->CountPositions() <= static_cast<uint32_t>(a_idx)) {
                a_vm->TraceStack("Invalid position idx", a_stackID);
                return nullptr;
            }
            REX::EnumSet<Registry::VoiceAnnotation> annotation;
            if (scene->CountSubmissives() > 0) {
                if (scene->GetNthPosition(a_idx)->IsSubmissive())
                    annotation.set(Registry::VoiceAnnotation::Submissive);
                else
                    annotation.set(Registry::VoiceAnnotation::Dominant);
            }
            if (a_muffled)
                annotation.set(Registry::VoiceAnnotation::Muffled);
            return Registry::Library::GetSingleton()->PickSound(a_id, static_cast<uint32_t>(a_strength), annotation);
        }

        RE::TESSound* GetSoundObjectLeg(RE::StaticFunctionTag*, RE::BSFixedString a_id, int a_idx)
        {
            return Registry::Library::GetSingleton()->PickSound(a_id, Registry::LegacyVoice(a_idx));
        }

        RE::TESSound* GetOrgasmSound(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id, RE::BSFixedString a_scene, int a_idx, bool a_muffled)
        {
            const auto lib = Registry::Library::GetSingleton();
            auto scene = lib->GetSceneById(a_scene);
            if (!scene) {
                a_vm->TraceStack("Invalid scene id", a_stackID);
                return nullptr;
            }
            if (scene->CountPositions() <= static_cast<uint32_t>(a_idx)) {
                a_vm->TraceStack("Invalid position idx", a_stackID);
                return nullptr;
            }
            REX::EnumSet<Registry::VoiceAnnotation> annotation;
            if (scene->CountSubmissives() > 0) {
                if (scene->GetNthPosition(a_idx)->IsSubmissive())
                    annotation.set(Registry::VoiceAnnotation::Submissive);
                else
                    annotation.set(Registry::VoiceAnnotation::Dominant);
            }
            if (a_muffled)
                annotation.set(Registry::VoiceAnnotation::Muffled);
            return Registry::Library::GetSingleton()->PickOrgasmSound(a_id, annotation);
        }

        RE::BSFixedString GetDisplayName(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id)
        {
            const auto vs = Registry::Library::GetSingleton();
            auto it = vs->GetVoiceById(a_id);
            if (!it) {
                a_vm->TraceStack("Invalid voice form", a_stackID);
                return "";
            }
            return it->displayName.empty() ? it->name : it->displayName;
        }

        bool InitializeVoiceObject(RE::StaticFunctionTag*, RE::BSFixedString a_id)
        {
            return Registry::Library::GetSingleton()->CreateVoice(a_id);
        }

        void FinalizeVoiceObject(RE::StaticFunctionTag*, RE::BSFixedString a_id)
        {
            Registry::Library::GetSingleton()->WriteVoiceToFile(a_id);
        }

        void SetSoundObjectLeg(RE::StaticFunctionTag*, RE::BSFixedString a_id, int a_idx, RE::TESSound* a_set)
        {
            Registry::Library::GetSingleton()->SetVoiceSound(a_id, Registry::LegacyVoice(a_idx), a_set);
        }

        void SetVoiceTags(RE::StaticFunctionTag*, RE::BSFixedString a_id, std::vector<RE::BSFixedString> a_newtags)
        {
            Registry::Library::GetSingleton()->SetVoiceTags(a_id, a_newtags);
        }

        void SetCompatibleSex(RE::StaticFunctionTag*, RE::BSFixedString a_id, int a_set)
        {
            Registry::Library::GetSingleton()->SetVoiceSex(a_id, a_set == -1 ? RE::SEXES::kNone : RE::SEXES::SEX(a_set));
        }

        void SetCompatibleRaces(RE::StaticFunctionTag*, RE::BSFixedString a_id, std::vector<RE::BSFixedString> a_set)
        {
            std::vector<Registry::RaceKey> races{};
            races.reserve(a_set.size());
            for (auto&& r : a_set) {
                const Registry::RaceKey rk{ r };
                if (!rk.IsValid())
                    continue;
                races.push_back(rk);
            }
            Registry::Library::GetSingleton()->SetVoiceRace(a_id, races);
        }

    }  // namespace BaseVoice

    RE::BSFixedString SelectVoice(RE::StaticFunctionTag*, RE::Actor* a_actor)
    {
        return SelectVoiceByTags(nullptr, a_actor, "");
    }

    RE::BSFixedString SelectVoiceByTags(RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_tags)
    {
        auto s = Registry::Library::GetSingleton();
        auto v = a_actor ? s->GetVoice(a_actor, Registry::TagDetails{ a_tags.data() }) : s->GetVoice(Registry::TagDetails{ a_tags.data() });
        return v ? v->name : "";
    }

    RE::BSFixedString SelectVoiceByTagsA(RE::StaticFunctionTag*, RE::Actor* a_actor, std::vector<std::string_view> a_tags)
    {
        auto s = Registry::Library::GetSingleton();
        auto v = a_actor ? s->GetVoice(a_actor, { a_tags }) : s->GetVoice({ a_tags });
        return v ? v->name : "";
    }

    RE::BSFixedString GetSavedVoice(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
    {
        if (!a_actor) {
            a_vm->TraceStack("Actor is none", a_stackID);
            return "";
        }
        auto v = Registry::Library::GetSingleton()->GetSavedVoice(a_actor->GetFormID());
        return v ? v->name : "";
    }

    void StoreVoice(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_voice)
    {
        if (!a_actor) {
            a_vm->TraceStack("Actor is none", a_stackID);
            return;
        }
        Registry::Library::GetSingleton()->SaveVoice(a_actor->GetFormID(), a_voice);
    }

    void DeleteVoice(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor)
    {
        if (!a_actor) {
            a_vm->TraceStack("Actor is none", a_stackID);
            return;
        }
        Registry::Library::GetSingleton()->ClearVoice(a_actor->GetFormID());
    }

    std::vector<RE::BSFixedString> GetAllVoices(RE::StaticFunctionTag*, RE::BSFixedString a_racekey)
    {
        auto rk = a_racekey.empty() ? Registry::RaceKey{ Registry::RaceKey::None } : Registry::RaceKey{ a_racekey };
        return Registry::Library::GetSingleton()->GetAllVoiceIds(rk);
    }

    std::vector<RE::Actor*> GetAllCachedUniqueActorsSorted(RE::StaticFunctionTag*, RE::Actor* a_sndprio)
    {
        auto saved = Registry::Library::GetSingleton()->GetSavedActors();
        std::erase_if(saved, [&](RE::Actor* it) {
            if (it->IsPlayerRef() || a_sndprio && it->formID == a_sndprio->formID)
                return true;
            auto base = it->GetActorBase();
            return !base || !base->IsUnique();
        });
        std::ranges::sort(saved, [&](RE::Actor* a, RE::Actor* b) {
            return std::strcmp(a->GetDisplayFullName(), b->GetDisplayFullName()) < 0;
        });
        std::vector<RE::Actor*> ret{ RE::PlayerCharacter::GetSingleton() };
        if (auto base = a_sndprio ? a_sndprio->GetActorBase() : nullptr) {
            if (base->IsUnique())
                ret.push_back(a_sndprio);
        }
        ret.insert_range(ret.end(), saved);
        return ret;
    }

    RE::BSFixedString SelectVoiceByRace(RE::StaticFunctionTag*, RE::BSFixedString a_racekey)
    {
        auto v = Registry::Library::GetSingleton()->GetVoice(Registry::RaceKey{ a_racekey });
        return v ? v->name : "";
    }

}  // namespace Papyrus::VoiceSlots
