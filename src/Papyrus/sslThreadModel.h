#pragma once

#include "Registry/Define/Animation.h"

namespace Papyrus::ThreadModel
{
    enum FurniStatus
    {
        Disallow = 0,
        Allow = 1,
        Prefer = 2,
    };

    namespace ActorAlias
    {
        RE::BSFixedString GetActorVoice(ALIASARGS);
        RE::BSFixedString GetActorExpression(ALIASARGS);
        void SetActorVoiceImpl(ALIASARGS, RE::BSFixedString a_voice);
        void SetActorExpressionImpl(ALIASARGS, RE::BSFixedString a_expression);

        void SetActorCollisions(ALIASARGS, bool a_enable);

        std::vector<RE::TESForm*> StripByData(ALIASARGS, int32_t a_stripdata, std::vector<uint32_t> a_defaults, std::vector<uint32_t> a_overwrite);
        std::vector<RE::TESForm*> StripByDataEx(ALIASARGS, int32_t a_stripdata, std::vector<uint32_t> a_defaults, std::vector<uint32_t> a_overwrite, std::vector<RE::TESForm*> a_mergewith);

        void EnjBarsUpdateSlider(ALIASARGS, float a_enjoyment, RE::BSFixedString a_interactions);
        void RegisterRaiseEnjAttempt(ALIASARGS, float a_nextTimeCycle);

        inline bool Register(VM* a_vm)
        {
            REGISTERFUNC(GetActorVoice, "sslActorAlias", false);
            REGISTERFUNC(GetActorExpression, "sslActorAlias", false);
            REGISTERFUNC(SetActorVoiceImpl, "sslActorAlias", false);
            REGISTERFUNC(SetActorExpressionImpl, "sslActorAlias", false);

            REGISTERFUNC(SetActorCollisions, "sslActorAlias", false);

            REGISTERFUNC(StripByData, "sslActorAlias", false);
            REGISTERFUNC(StripByDataEx, "sslActorAlias", false);

            REGISTERFUNC(EnjBarsUpdateSlider, "sslActorAlias", false);
            REGISTERFUNC(RegisterRaiseEnjAttempt, "sslActorAlias", false);

            return true;
        }
    }  // namespace ActorAlias

    RE::BSFixedString GetActiveScene(QUESTARGS);
    RE::BSFixedString GetActiveStage(QUESTARGS);
    std::vector<RE::BSFixedString> GetPlayingScenes(QUESTARGS);
    std::vector<RE::Actor*> GetPositions(QUESTARGS);
    std::vector<RE::BSFixedString> AddContextExImpl(RE::TESQuest*, std::vector<RE::BSFixedString> a_oldcontext, std::string a_newcontext);

    void CreateInstance(QUESTARGS, std::vector<RE::Actor*> a_submissives, std::vector<RE::BSFixedString> a_scenesPrimary, std::vector<RE::BSFixedString> a_scenesLeadIn, std::vector<RE::BSFixedString> a_scenesCustom, int a_furniturepref);
    void DestroyInstance(RE::TESQuest* a_qst, bool a_preservePreparedActors);
    void CancelPendingAnimations(RE::TESQuest* a_qst);
    bool BeginActorRecovery(QUESTARGS);
    bool BeginPlayerDialogueWait(QUESTARGS);
    bool BeginPlayerSheatheWait(QUESTARGS);
    std::vector<RE::BSFixedString> GetLeadInScenes(QUESTARGS);
    std::vector<RE::BSFixedString> GetPrimaryScenes(QUESTARGS);
    std::vector<RE::BSFixedString> GetCustomScenes(QUESTARGS);
    std::vector<RE::BSFixedString> AdvanceScene(QUESTARGS, std::vector<RE::BSFixedString> a_history, RE::BSFixedString a_nextStage);
    int SelectNextStage(QUESTARGS, std::vector<RE::BSFixedString> a_tags);
    bool SetActiveScene(QUESTARGS, RE::BSFixedString a_sceneid);
    bool ReassignCenter(QUESTARGS, RE::TESObjectREFR* a_centeron);
    bool SetNextPermutation(QUESTARGS, RE::Actor* a_position);
    void UpdatePlacement(QUESTARGS, RE::Actor* a_position);

    void SetAnimationPlaybackSpeed(QUESTARGS, float a_playbackSpeed);
    bool RestartFixedLengthTimer(QUESTARGS);
    bool AdjustFixedLengthTimer(QUESTARGS, float a_delta);
    void SetFixedLengthTimerPaused(QUESTARGS, bool a_paused);
    bool ConsumeFixedLengthTimerExpiration(QUESTARGS);

    void AddExperience(QUESTARGS, std::vector<RE::Actor*> a_positions, RE::BSFixedString a_scene, std::vector<RE::BSFixedString> a_playedstages);
    void UpdateStatistics(QUESTARGS, RE::Actor* a_actor, std::vector<RE::Actor*> a_positions, RE::BSFixedString a_scene, std::vector<RE::BSFixedString> a_playedstages, float a_time);

    // INTERACTIONS
    bool IsCollisionRegistered(QUESTARGS);
    void UnregisterCollision(QUESTARGS);
    std::vector<bool> GetInteractionFlagsImpl(QUESTARGS, RE::Actor* a_actor, RE::Actor* a_partner);
    std::vector<int32_t> GetActiveInterTypesImpl(QUESTARGS, RE::Actor* a_actor, RE::Actor* a_partner);
    bool HasActiveInteractionImpl(QUESTARGS, RE::Actor* a_actor, RE::Actor* a_partner, int32_t a_interType);
    bool HasActiveInteractionAllImpl(QUESTARGS, RE::Actor* a_actor, std::vector<int32_t> a_types);
    bool HasActiveInteractionAnyImpl(QUESTARGS, RE::Actor* a_actor, std::vector<int32_t> a_types);
    RE::Actor* GetPartnerByInteractionTypeImpl(QUESTARGS, RE::Actor* a_actor, int32_t a_interType);
    std::vector<RE::Actor*> GetPartnersByInteractionTypeImpl(QUESTARGS, RE::Actor* a_actor, int32_t a_interType);
    float GetInteractionVelocityImpl(QUESTARGS, RE::Actor* a_actor, RE::Actor* a_partner, int32_t a_interType);
    RE::BSFixedString GetInteractionStringImpl(QUESTARGS, RE::Actor* a_actor);
    std::vector<RE::BSFixedString> GetInteractionStringArrayImpl(QUESTARGS, RE::Actor* a_actor);

    // SCENE HUD
    void InitSceneHUDImpl(QUESTARGS);
    void DestroySceneHUDImpl(QUESTARGS);
    void SetFocusSceneHUDImpl(QUESTARGS, bool a_focused);

    void UpdateMenuTimerDisplay(QUESTARGS, float a_duration, float a_time);
    void OnStageChangedUpdateHUD(QUESTARGS);
    void EnjBarsChangeHighlightedPartner(QUESTARGS, RE::Actor* a_actor);

    bool OpenStageSelectMenuImpl(QUESTARGS);
    void SetVisibilitySceneGraphImpl(QUESTARGS, bool a_open);

    inline bool Register(VM* a_vm)
    {
        REGISTERFUNC(GetActiveScene, "sslThreadModel", true);
        REGISTERFUNC(GetActiveStage, "sslThreadModel", true);
        REGISTERFUNC(GetPlayingScenes, "sslThreadModel", true);
        REGISTERFUNC(GetPositions, "sslThreadModel", true);
        REGISTERFUNC(AddContextExImpl, "sslThreadModel", true);

        REGISTERFUNC(CreateInstance, "sslThreadModel", true);
        REGISTERFUNC(DestroyInstance, "sslThreadModel", true);
        REGISTERFUNC(CancelPendingAnimations, "sslThreadModel", true);
        REGISTERFUNC(BeginActorRecovery, "sslThreadModel", false);
        REGISTERFUNC(BeginPlayerDialogueWait, "sslThreadModel", false);
        REGISTERFUNC(BeginPlayerSheatheWait, "sslThreadModel", false);
        REGISTERFUNC(GetLeadInScenes, "sslThreadModel", true);
        REGISTERFUNC(GetPrimaryScenes, "sslThreadModel", true);
        REGISTERFUNC(GetCustomScenes, "sslThreadModel", true);
        REGISTERFUNC(AdvanceScene, "sslThreadModel", false);
        REGISTERFUNC(SelectNextStage, "sslThreadModel", true);
        REGISTERFUNC(SetActiveScene, "sslThreadModel", false);
        REGISTERFUNC(ReassignCenter, "sslThreadModel", false);
        REGISTERFUNC(UpdatePlacement, "sslThreadModel", false);
        REGISTERFUNC(SetNextPermutation, "sslThreadModel", false);

        REGISTERFUNC(SetAnimationPlaybackSpeed, "sslThreadModel", false);
        REGISTERFUNC(RestartFixedLengthTimer, "sslThreadModel", false);
        REGISTERFUNC(AdjustFixedLengthTimer, "sslThreadModel", false);
        REGISTERFUNC(SetFixedLengthTimerPaused, "sslThreadModel", false);
        REGISTERFUNC(ConsumeFixedLengthTimerExpiration, "sslThreadModel", false);

        REGISTERFUNC(AddExperience, "sslThreadModel", true);
        REGISTERFUNC(UpdateStatistics, "sslThreadModel", true);

        REGISTERFUNC(IsCollisionRegistered, "sslThreadModel", true);
        REGISTERFUNC(UnregisterCollision, "sslThreadModel", true);
        REGISTERFUNC(GetInteractionFlagsImpl, "sslThreadModel", true);
        REGISTERFUNC(GetActiveInterTypesImpl, "sslThreadModel", true);
        REGISTERFUNC(HasActiveInteractionImpl, "sslThreadModel", true);
        REGISTERFUNC(HasActiveInteractionAllImpl, "sslThreadModel", true);
        REGISTERFUNC(HasActiveInteractionAnyImpl, "sslThreadModel", true);
        REGISTERFUNC(GetPartnerByInteractionTypeImpl, "sslThreadModel", true);
        REGISTERFUNC(GetPartnersByInteractionTypeImpl, "sslThreadModel", true);
        REGISTERFUNC(GetInteractionVelocityImpl, "sslThreadModel", true);
        REGISTERFUNC(GetInteractionStringImpl, "sslThreadModel", true);
        REGISTERFUNC(GetInteractionStringArrayImpl, "sslThreadModel", true);

        REGISTERFUNC(InitSceneHUDImpl, "sslThreadModel", true);
        REGISTERFUNC(DestroySceneHUDImpl, "sslThreadModel", true);
        REGISTERFUNC(SetFocusSceneHUDImpl, "sslThreadModel", true);

        REGISTERFUNC(UpdateMenuTimerDisplay, "sslThreadModel", true);
        REGISTERFUNC(OnStageChangedUpdateHUD, "sslThreadModel", true);
        REGISTERFUNC(EnjBarsChangeHighlightedPartner, "sslThreadModel", true);

        REGISTERFUNC(OpenStageSelectMenuImpl, "sslThreadModel", true);
        REGISTERFUNC(SetVisibilitySceneGraphImpl, "sslThreadModel", true);

        return ActorAlias::Register(a_vm);
    }

}  // namespace Papyrus::ThreadModel
