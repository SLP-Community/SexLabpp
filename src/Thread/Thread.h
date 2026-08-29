#pragma once

#include "Registry/Library.h"
#include "Thread/Interaction/NiSurface/CollisionManager.h"

namespace Thread
{
    class Instance
    {
        constexpr static const char* CENTER_REF_NAME{ "CenterAlias" };

      public:
        enum class FurniturePreference
        {
            Disallow = 0,
            Allow = 1,
            Prefer = 2,
        };

        enum SceneType
        {
            Primary = 0,
            LeadIn,
            Custom,

            Total
        };

        using FurnitureMapping = std::vector<std::pair<RE::TESObjectREFR*, Registry::FurnitureOffset>>;
        using SceneMapping = std::array<std::vector<const Registry::Scene*>, SceneType::Total>;

      public:
        struct Position
        {
            Position(RE::BGSRefAlias* alias, RE::Actor* actor, bool submissive, bool dominant);

            RE::BGSRefAlias* alias;
            Registry::ActorFragment data;
            const Registry::Voice* voice{ nullptr };
            const Registry::Expression* expression{ nullptr };
            std::optional<float> ghostAlpha{ std::nullopt };
            uint8_t uniquePermutations{ 0 };
        };

        struct Center
        {
            Center(RE::BGSRefAlias* alias) :
              alias(alias) {}
            ~Center() = default;

            void SetReference(RE::TESObjectREFR* a_ref, Registry::FurnitureOffset a_offset);
            RE::TESObjectREFR* GetRef() { return alias->GetReference(); }

            RE::BGSRefAlias* alias;
            Registry::FurnitureOffset offset{};
            const Registry::FurnitureDetails* details{ nullptr };
        };

      public:
        Instance(RE::TESQuest* a_linkedQst, const std::vector<RE::Actor*>& a_submissives, const SceneMapping& a_scenes, FurniturePreference a_furniturePreference);
        ~Instance() = default;

        static void CreateInstance(RE::TESQuest* a_linkedQst, const std::vector<RE::Actor*> a_submissives, const SceneMapping& a_scenes, FurniturePreference a_furniturePreference);
        static void DestroyInstance(RE::TESQuest* a_linkedQst, bool a_preservePreparedActors = false);
        static void CancelPendingAnimations(RE::TESQuest* a_linkedQst);
        static Instance* GetInstance(RE::TESQuest* a_linkedQst);
        static Instance* GetPendingInstance(RE::TESQuest* a_linkedQst);
        static void FinalizeCenterRefSelection(RE::TESQuest* a_linkedQst);
        static void DispatchContinueSetup(RE::TESQuest* a_linkedQst, bool a_result);
        static void UpdateAnimations(float a_delta);

      public:
        bool HasInstanceNiSurface() const { return instanceNiSurface != nullptr; }
        Interaction::NiSurface::Scene* GetInstanceNiSurface() { return instanceNiSurface.get(); }
        void UnregisterInstanceNiSurface() {
          (Interaction::NiSurface::Manager::Unregister(linkedQst->GetFormID()),
          instanceNiSurface = nullptr);
        }

        void AdvanceScene(const Registry::Stage* a_nextStage);
        bool BeginActorRecovery();
        bool BeginPlayerDialogueWait();
        bool BeginPlayerSheatheWait();
        void RealignActors();
        bool SetActiveScene(const Registry::Scene* a_scene);
        const Registry::Scene* GetActiveScene() { return activeScene; }
        const Registry::Stage* GetActiveStage() { return activeStage; }
        std::vector<const Registry::Scene*> GetThreadScenes(SceneType a_type);
        std::vector<const Registry::Scene*> GetThreadScenes();

        const std::vector<RE::Actor*>& GetActors();
        Position* GetPosition(RE::Actor* a_actor);
        const Registry::PositionInfo* GetPositionInfo(RE::Actor* a_actor);
        void UpdatePlacement(RE::Actor* a_actor);

        RE::TESObjectREFR* GetCenterRef() { return center.GetRef(); }
        Registry::FurnitureType GetFurnitureType() { return center.offset.type; }
        bool ReplaceCenterRef(RE::TESObjectREFR* a_ref);
        void SetCenterRefSelected(size_t a_index);

        void SetAnimationPlaybackSpeed(float playbackSpeed);
        bool RestartFixedLengthTimer();
        bool AdjustFixedLengthTimer(float a_delta);
        void SetFixedLengthTimerPaused(bool a_paused);
        bool ConsumeFixedLengthTimerExpiration();
        void OffsetAdjustSet(uint32_t actorFormId, Registry::CoordinateType axis, float value);
        void OffsetAdjustReset(bool hasFurn);

        const Registry::Expression* GetExpression(RE::Actor* a_position);
        void SetExpression(RE::Actor* a_position, const Registry::Expression* a_expression);
        const Registry::Voice* GetVoice(RE::Actor* a_position);
        void SetVoice(RE::Actor* a_position, const Registry::Voice* a_voice);
        int32_t GetUniquePermutations(RE::Actor* a_position);
        int32_t GetCurrentPermutation(RE::Actor* a_position);
        bool SetNextPermutation(RE::Actor* a_position);

        template <typename T>
        T GetThreadProperty(const std::string& a_property);
        template <typename T>
        void SetThreadProperty(const std::string& a_property, T a_val);

        // SceneHUD
        void InitSceneHUDImpl();
        void DestroySceneHUDImpl();
        void SetFocusSceneHUDImpl(bool a_focused);

        void UpdateMenuTimerDisplay(float a_duration, float a_timer);
        void EnjBarsChangeHighlightedPartner(RE::Actor* a_target);
        void EnjBarsUpdateSlider(RE::Actor* a_position, float a_enjoyment, RE::BSFixedString a_interactions);
        void RegisterRaiseEnjAttempt(RE::Actor* a_position, float a_nextTimeCycle);
        void OnStageChangedUpdateHUD();

        bool OpenStageSelectMenuImpl();
        void SetVisibilitySceneGraphImpl(bool a_open);

      private:
        struct ActiveClip
        {
            const RE::hkbClipGenerator* generator;
            std::string animationName;
            float localTime;
            float weight;
        };

        struct PendingAnimation
        {
            RE::Actor* actor;
            RE::BSFixedString event;
            std::vector<ActiveClip> previousClips;
            const RE::hkbClipGenerator* observedGenerator{ nullptr };
            std::string observedAnimation{};
            float observedLocalTime{ 0.0f };
            float elapsed{ 0.0f };
            float retryDelay{ 0.0f };
            size_t position;
            uint32_t readinessChecks{ 0 };
            uint8_t dispatchAttempts{ 0 };
            bool transitionAcknowledged{ false };
            bool playbackHeld{ false };
        };

        struct PendingRecovery
        {
            RE::Actor* actor;
            float elapsed{ 0.0f };
            bool getUpRequested{ false };
            bool getUpEndQueued{ false };
        };

        // Used by animation stages that have percise animation play time, to avoid the Papyrus 0.5
        // second poll delay
        struct FixedLengthTimer
        {
            enum class State : uint8_t
            {
                Stopped,
                Running,
                Expired,
            };

            float duration{ 0.0f };
            float remaining{ 0.0f };
            State state{ State::Stopped };
            bool paused{ false };
        };

        RE::TESQuest* linkedQst;
        std::shared_ptr<Interaction::NiSurface::Scene> instanceNiSurface{ nullptr };

        Center center;
        std::vector<Position> positions;
        Registry::Coordinate baseCoordinates{};
        std::vector<std::vector<RE::Actor*>> assignments{};
        std::vector<std::vector<RE::Actor*>>::iterator activeAssignment{ assignments.end() };
        const Registry::Scene* activeScene{ nullptr };
        const Registry::Stage* activeStage{ nullptr };
        SceneMapping scenes{};
        std::vector<PendingAnimation> pendingAnimations{};
        std::vector<PendingRecovery> pendingRecoveries{};
        float playerSheatheElapsed{ 0.0f };
        RE::WEAPON_STATE playerSheathePreviousState{ RE::WEAPON_STATE::kSheathed };
        bool actorPreparationApplied{ false };
        bool actorRecoveryPreparationBarrier{ false };
        bool playerDialoguePending{ false };
        bool playerSheatheActionSubmitted{ false };
        bool playerSheathePending{ false };
        float animationPlaybackSpeed{ 1.0f };
        FixedLengthTimer fixedLengthTimer{};

        // used during center selection through menu
        RE::TESQuest* pendingQst{ nullptr };
        FurnitureMapping pendingFurnitureMap{};
        RE::Actor* pendingCenterAct{ nullptr };

      private:
        enum class CenterSelection
        {
            Actor,
            Furniture,
            SelectionMenu,
        };

        void FinalizeInstanceMake();
        RE::Actor* InitializeReferences(const std::vector<RE::Actor*>& a_submissives);
        std::vector<Registry::ActorFragment> InitializeScenes(const SceneMapping& a_scenes, FurniturePreference a_furniturePreference);
        std::vector<const Registry::Scene*>& InitializeCenter(RE::Actor* centerAct, FurniturePreference furniturePreference);
        bool InitializeFixedCenter(RE::Actor* centerAct, std::vector<const Registry::Scene*>& prioScenes, REX::EnumSet<Registry::FurnitureType::Value> sceneTypes);
        CenterSelection GetSelectionMethod(FurniturePreference furniturePreference);
        void InitializeCenterRefMenu(const FurnitureMapping& a_furnitures, RE::Actor* a_tmpCenter);
        FurnitureMapping GetUniqueFurnituesOfTypeInBound(RE::Actor* a_centerAct, REX::EnumSet<Registry::FurnitureType::Value> a_furnitureTypes);
        bool GetActiveClips(RE::Actor* a_actor, std::vector<ActiveClip>& a_clips) const;
        bool QueueActorRecoveries(bool a_preparationBarrier);
        void TryStartAnimations();
        bool HoldAnimation(PendingAnimation& a_pending);
        void ReleaseAnimations();
        bool StartFixedLengthTimer();
        void CancelFixedLengthTimer();
        void UpdateFixedLengthTimer(float a_delta);
        void UpdatePendingAnimations(float a_delta);
        void UpdatePendingRecoveries(float a_delta);
        void ReassertPlacement(size_t a_position, bool a_force);
        static void RestorePreparedActors(RE::TESQuest* a_linkedQst);

      private:
        static inline std::shared_mutex _mInstances{};
        static inline std::vector<std::unique_ptr<Instance>> instances{};
        static inline std::vector<std::unique_ptr<Instance>> pendingInstances{};
    };

}  // namespace Thread
