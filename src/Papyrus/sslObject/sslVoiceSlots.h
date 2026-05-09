#pragma once

namespace Papyrus::VoiceSlots
{
    namespace BaseVoice
    {
        bool GetEnabled(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id);
        void SetEnabled(RE::StaticFunctionTag*, RE::BSFixedString a_id, bool a_enabled);

        std::vector<RE::BSFixedString> GetVoiceTags(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id);
        int GetCompatibleSex(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id);
        std::vector<RE::BSFixedString> GetCompatibleRaces(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id);

        RE::TESSound* GetSoundObject(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id, int a_strength, RE::BSFixedString a_scene, int a_idx, bool a_muffled);
        RE::TESSound* GetSoundObjectLeg(RE::StaticFunctionTag*, RE::BSFixedString a_id, int a_idx);
        RE::TESSound* GetOrgasmSound(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id, RE::BSFixedString a_scene, int a_idx, bool a_muffled);

        RE::BSFixedString GetDisplayName(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id);

        bool InitializeVoiceObject(RE::StaticFunctionTag*, RE::BSFixedString a_id);
        void FinalizeVoiceObject(RE::StaticFunctionTag*, RE::BSFixedString a_id);

        void SetSoundObjectLeg(RE::StaticFunctionTag*, RE::BSFixedString a_id, int a_idx, RE::TESSound* a_set);
        void SetVoiceTags(RE::StaticFunctionTag*, RE::BSFixedString a_id, std::vector<RE::BSFixedString> a_newtags);
        void SetCompatibleSex(RE::StaticFunctionTag*, RE::BSFixedString a_id, int a_set);
        void SetCompatibleRaces(RE::StaticFunctionTag*, RE::BSFixedString a_id, std::vector<RE::BSFixedString> a_set);

        inline bool Register(VM* a_vm)
        {
            REGISTERFUNC(GetEnabled, "sslBaseVoice", true);
            REGISTERFUNC(SetEnabled, "sslBaseVoice", true);
            REGISTERFUNC(GetVoiceTags, "sslBaseVoice", true);
            REGISTERFUNC(GetCompatibleSex, "sslBaseVoice", true);
            REGISTERFUNC(GetCompatibleRaces, "sslBaseVoice", true);
            REGISTERFUNC(GetSoundObject, "sslBaseVoice", true);
            REGISTERFUNC(GetSoundObjectLeg, "sslBaseVoice", true);
            REGISTERFUNC(GetOrgasmSound, "sslBaseVoice", true);
            REGISTERFUNC(GetDisplayName, "sslBaseVoice", true);
            REGISTERFUNC(InitializeVoiceObject, "sslBaseVoice", true);
            REGISTERFUNC(FinalizeVoiceObject, "sslBaseVoice", true);
            REGISTERFUNC(SetSoundObjectLeg, "sslBaseVoice", true);
            REGISTERFUNC(SetVoiceTags, "sslBaseVoice", true);
            REGISTERFUNC(SetCompatibleSex, "sslBaseVoice", true);
            REGISTERFUNC(SetCompatibleRaces, "sslBaseVoice", true);

            return true;
        }
    }

    RE::BSFixedString SelectVoice(RE::StaticFunctionTag*, RE::Actor* a_actor);
    RE::BSFixedString SelectVoiceByTags(RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_tags);
    RE::BSFixedString SelectVoiceByTagsA(RE::StaticFunctionTag*, RE::Actor* a_actor, std::vector<std::string_view> a_tags);

    RE::BSFixedString GetSavedVoice(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);
    void StoreVoice(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, RE::BSFixedString a_voice);
    void DeleteVoice(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);

    std::vector<RE::BSFixedString> GetAllVoices(RE::StaticFunctionTag*, RE::BSFixedString a_racekey);
    std::vector<RE::Actor*> GetAllCachedUniqueActorsSorted(RE::StaticFunctionTag*, RE::Actor* a_sndprio);
    RE::BSFixedString SelectVoiceByRace(RE::StaticFunctionTag*, RE::BSFixedString a_racekey);

    inline bool Register(VM* a_vm)
    {
        REGISTERFUNC(SelectVoice, "sslVoiceSlots", true);
        REGISTERFUNC(SelectVoiceByTags, "sslVoiceSlots", true);
        REGISTERFUNC(SelectVoiceByTagsA, "sslVoiceSlots", true);

        REGISTERFUNC(GetSavedVoice, "sslVoiceSlots", true);
        REGISTERFUNC(StoreVoice, "sslVoiceSlots", true);
        REGISTERFUNC(DeleteVoice, "sslVoiceSlots", true);

        REGISTERFUNC(GetAllVoices, "sslVoiceSlots", true);
        REGISTERFUNC(GetAllCachedUniqueActorsSorted, "sslVoiceSlots", true);
        REGISTERFUNC(SelectVoiceByRace, "sslVoiceSlots", true);

        return BaseVoice::Register(a_vm);
    }

}  // namespace Papyrus::VoiceSlots
