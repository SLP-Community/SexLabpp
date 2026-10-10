#include "Thread.h"

#include "GameForms.h"
#include "Registry/Util/Scale.h"
#include "Thread/Collision/CollisionHandler.h"
#include "Thread/Hooks.h"
#include "Util/Script.h"

#include <bit>

namespace Thread
{
    namespace
    {
        enum ActorStatus : int32_t
        {
            Unconscious = -5,
            Dying = -10,
        };

        struct ActorPreparation
        {
            RE::TESQuest* owner;
            RE::Actor* actor;
            RE::ACTOR_LIFE_STATE lifeState;
            int32_t isNPC{ 0 };
            bool humanoidFootIKDisabled{ false };
            bool hasIsNPC{ false };
            bool hasHumanoidFootIKDisabled{ false };
        };

        std::mutex actorPreparationLock;
        std::unordered_map<RE::FormID, ActorPreparation> actorPreparations;

        bool IsPlayerDialogueActive()
        {
            const auto ui = RE::UI::GetSingleton();
            return ui && ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME);
        }

        bool PrepareActorForAnimation(RE::TESQuest* a_owner, RE::Actor* a_actor, bool a_human)
        {
            std::scoped_lock lock{ actorPreparationLock };
            const auto actorID = a_actor->GetFormID();
            const auto existing = actorPreparations.find(actorID);
            if (existing != actorPreparations.end() && existing->second.owner != a_owner) {
                logger::error("Actor {:X} is already prepared by another thread.", actorID);
                return false;
            }

            const auto [entry, inserted] = actorPreparations.try_emplace(actorID, ActorPreparation{ a_owner, a_actor, a_actor->AsActorState()->GetLifeState() });
            auto& preparation = entry->second;
            if (inserted && a_human) {
                preparation.hasIsNPC = a_actor->GetGraphVariableInt("IsNPC", preparation.isNPC);
                preparation.hasHumanoidFootIKDisabled = a_actor->GetGraphVariableBool("bHumanoidFootIKDisable", preparation.humanoidFootIKDisabled);
            }

            a_actor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kParalysis, 0.0f);
            a_actor->AddToFaction(const_cast<RE::TESFaction*>(GameForms::AnimatingFaction), 1);
            a_actor->EvaluatePackage();
            if (a_actor->IsPlayerRef()) {
                a_actor->AsActorState()->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kAlive;
            } else {
                if (inserted) {
                    switch (preparation.lifeState) {
                    case RE::ACTOR_LIFE_STATE::kUnconcious:
                        a_actor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable05, ActorStatus::Unconscious);
                        break;
                    case RE::ACTOR_LIFE_STATE::kDying:
                    case RE::ACTOR_LIFE_STATE::kDead:
                        a_actor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable05, ActorStatus::Dying);
                        a_actor->Resurrect(false, true);
                        break;
                    default:
                        break;
                    }
                }
                a_actor->AsActorState()->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kRestrained;
            }
            if (!a_actor->IsPlayerRef() && a_actor->AsActorState()->IsWeaponDrawn()) {
                a_actor->DrawWeaponMagicHands(false);
                if (a_actor->IsSneaking()) {
                    a_actor->AsActorState()->actorState2.forceSneak = false;
                }
            }
            if (a_human) {
                a_actor->SetGraphVariableInt("IsNPC", 0);
                a_actor->SetGraphVariableBool("bHumanoidFootIKDisable", true);
            }
            return true;
        }

        void RestorePreparedActorState(RE::TESQuest* a_owner)
        {
            std::vector<ActorPreparation> restore;
            bool restorePlayer = false;
            {
                std::scoped_lock lock{ actorPreparationLock };
                for (auto actor = actorPreparations.begin(); actor != actorPreparations.end();) {
                    if (actor->second.owner != a_owner) {
                        ++actor;
                        continue;
                    }
                    restore.push_back(actor->second);
                    actor = actorPreparations.erase(actor);
                }
            }
            for (const auto& preparation : restore) {
                const auto actor = preparation.actor;
                if (!actor) {
                    continue;
                }
                restorePlayer |= actor->IsPlayerRef();
                Registry::Scale::GetSingleton()->RemoveScale(actor);
                if (preparation.hasIsNPC) {
                    actor->SetGraphVariableInt("IsNPC", preparation.isNPC);
                }
                if (preparation.hasHumanoidFootIKDisabled) {
                    actor->SetGraphVariableBool("bHumanoidFootIKDisable", preparation.humanoidFootIKDisabled);
                }
                actor->AsActorState()->actorState1.lifeState = preparation.lifeState == RE::ACTOR_LIFE_STATE::kUnconcious ? RE::ACTOR_LIFE_STATE::kUnconcious : RE::ACTOR_LIFE_STATE::kAlive;
                if (!actor->IsPlayerRef()) {
                    actor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable05, 0.0f);
                }
            }
            if (restorePlayer) {
                Hooks::SetWeaponDrawBlocked(false);
            }
            if (!restore.empty()) {
                logger::info("Restored {} natively prepared actor(s) for thread {:X}.", restore.size(), a_owner->GetFormID());
            }
        }
    }

    void Instance::RestorePreparedActors(RE::TESQuest* a_linkedQst)
    {
        RestorePreparedActorState(a_linkedQst);
    }

    void Instance::CancelPendingAnimations(RE::TESQuest* a_linkedQst)
    {
        {
            std::unique_lock lock{ _mInstances };
            for (auto&& instance : instances) {
                if (instance->linkedQst != a_linkedQst) {
                    continue;
                }
                instance->CancelFixedLengthTimer();
                instance->ReleaseAnimations();
                if (!instance->pendingAnimations.empty()) {
                    logger::info("Cancelled {} pending animation synchronization(s).", instance->pendingAnimations.size());
                    instance->pendingAnimations.clear();
                }
                instance->pendingRecoveries.clear();
                instance->actorRecoveryPreparationBarrier = false;
                instance->playerDialoguePending = false;
                instance->playerSheathePending = false;
                instance->playerSheatheElapsed = 0.0f;
                instance->playerSheathePreviousState = RE::WEAPON_STATE::kSheathed;
                instance->playerSheatheActionSubmitted = false;
                if (instance->GetPosition(RE::PlayerCharacter::GetSingleton())) {
                    Hooks::SetWeaponDrawBlocked(false);
                }
                break;
            }
        }
        RestorePreparedActorState(a_linkedQst);
    }

    bool Instance::BeginActorRecovery()
    {
        return !QueueActorRecoveries(true);
    }

    // Player dialogue MUST be waited on. We can race with the engine's dialogue closure handler which
    // leads to the player permanently being stuck since the dialogue handler fails to cleanup properly
    bool Instance::BeginPlayerDialogueWait()
    {
        if (playerDialoguePending) {
            return false;
        }
        const auto player = RE::PlayerCharacter::GetSingleton();
        if (!player || !GetPosition(player) || !IsPlayerDialogueActive()) {
            return true;
        }
        playerDialoguePending = true;
        logger::info("Waiting for player dialogue to finish before preparing actors.");
        return false;
    }

    bool Instance::QueueActorRecoveries(bool a_preparationBarrier)
    {
        if (!pendingRecoveries.empty()) {
            actorRecoveryPreparationBarrier |= a_preparationBarrier;
            return true;
        }
        actorRecoveryPreparationBarrier = a_preparationBarrier;
        for (const auto actor : GetActors()) {
            if (!actor) {
                continue;
            }
            const auto actorState = actor->AsActorState();
            if (actorState->GetKnockState() == RE::KNOCK_STATE_ENUM::kNormal && !actor->IsInRagdollState()) {
                continue;
            }
            pendingRecoveries.emplace_back(actor);
        }
        if (pendingRecoveries.empty()) {
            actorRecoveryPreparationBarrier = false;
            return false;
        }
        logger::info("Queued native recovery for {} actor(s) before {}.", pendingRecoveries.size(), a_preparationBarrier ? "Papyrus preparation" : "animation dispatch");
        return true;
    }

    void Instance::UpdatePendingRecoveries(float a_delta)
    {
        constexpr float recoveryTimeout = 15.0f;
        bool recoveryFailed = false;
        for (auto recovery = pendingRecoveries.begin(); recovery != pendingRecoveries.end();) {
            recovery->elapsed += a_delta;
            const auto actor = recovery->actor;
            if (!actor || !actor->Is3DLoaded() || actor->IsOnMount()) {
                if (actor) {
                    logger::error("Actor {:X} recovery prerequisites failed: loaded={}, mounted={}.", actor->GetFormID(), actor->Is3DLoaded(), actor->IsOnMount());
                } else {
                    logger::error("Actor recovery failed because the actor is unavailable.");
                }
                recoveryFailed = true;
                break;
            }
            if (actor->IsDead()) {
                const auto position = GetPosition(actor);
                if (!position || !PrepareActorForAnimation(linkedQst, actor, position->data.IsHuman()) || actor->IsDead()) {
                    logger::error("Actor {:X} could not be temporarily resurrected for native recovery.", actor->GetFormID());
                    recoveryFailed = true;
                    break;
                }
                logger::info("Temporarily resurrected actor {:X} for native recovery.", actor->GetFormID());
            }
            const auto actorState = actor->AsActorState();
            const auto knockState = actorState->GetKnockState();
            if (knockState == RE::KNOCK_STATE_ENUM::kNormal && !actor->IsInRagdollState()) {
                logger::info("Completed native recovery for actor {:X} after {:.3f}s.", actor->GetFormID(), recovery->elapsed);
                recovery = pendingRecoveries.erase(recovery);
                continue;
            }
            if (recovery->elapsed >= recoveryTimeout) {
                logger::error("Actor {:X} did not complete native recovery after {:.3f}s.", actor->GetFormID(), recovery->elapsed);
                recoveryFailed = true;
                break;
            }
            if (!recovery->getUpEndQueued && knockState == RE::KNOCK_STATE_ENUM::kGetUp) {
                const auto process = actor->GetActorRuntimeData().currentProcess;
                if (!process || !process->InHighProcess() || !actor->GetActorRuntimeData().movementController) {
                    logger::error("Actor {:X} cannot queue native get-up completion without high process and a movement controller.", actor->GetFormID());
                    recoveryFailed = true;
                    break;
                }
                using GetUpEndHandler = bool (*)(void*, RE::Actor*);
                static REL::Relocation<GetUpEndHandler> getUpEndHandler{ REL::VariantID(41799, 42880, 0x722DC0) };
                static_cast<void>(getUpEndHandler(nullptr, actor));
                recovery->getUpRequested = true;
                recovery->getUpEndQueued = true;
                logger::info("Queued native get-up completion for actor {:X}.", actor->GetFormID());
            } else if (!recovery->getUpRequested && actor->IsInRagdollState()) {
                const auto process = actor->GetActorRuntimeData().currentProcess;
                if (!process || !process->InHighProcess() || !actor->GetActorRuntimeData().movementController) {
                    logger::error("Actor {:X} cannot begin native get-up without high process and a movement controller.", actor->GetFormID());
                    recoveryFailed = true;
                    break;
                }
                if (RE::SourceActionMap::DoAction(actor, RE::DEFAULT_OBJECT::kActionGetUp)) {
                    recovery->getUpRequested = true;
                    logger::info("Requested get-up for ragdolled actor {:X}.", actor->GetFormID());
                }
            }
            ++recovery;
        }

        if (!recoveryFailed && !pendingRecoveries.empty()) {
            return;
        }
        const bool preparationBarrier = actorRecoveryPreparationBarrier;
        pendingRecoveries.clear();
        actorRecoveryPreparationBarrier = false;
        if (preparationBarrier) {
            const auto scriptObject = Script::GetScriptObject(linkedQst, "sslThreadModel");
            Script::CallbackPtr callbackPtr{};
            if (!scriptObject || !Script::DispatchMethodCall(scriptObject, "OnNativeActorRecoveryComplete", callbackPtr, bool{ !recoveryFailed })) {
                logger::error("Failed to notify Papyrus of native actor recovery completion for thread {:X}.", linkedQst->GetFormID());
            }
        } else if (recoveryFailed) {
            ReleaseAnimations();
            pendingAnimations.clear();
            const auto scriptObject = Script::GetScriptObject(linkedQst, "sslThreadModel");
            Script::CallbackPtr callbackPtr{};
            if (!scriptObject || !Script::DispatchMethodCall(scriptObject, "OnAnimationSyncFailed", callbackPtr)) {
                logger::error("Failed to notify Papyrus of actor recovery failure for thread {:X}.", linkedQst->GetFormID());
            }
        }
    }

    // The player's weapon MUST be sheathed before we unequip the weapon, and proceed with the scene.
    // Otherwise you'll run into the annoying sword sheathe skyrim bug
    bool Instance::BeginPlayerSheatheWait()
    {
        if (playerSheathePending) {
            return false;
        }
        const auto player = RE::PlayerCharacter::GetSingleton();
        if (!player || !GetPosition(player)) {
            return true;
        }
        Hooks::SetWeaponDrawBlocked(true);

        const auto weaponState = player->AsActorState()->GetWeaponState();
        if (weaponState == RE::WEAPON_STATE::kSheathed) {
            return true;
        }
        playerSheatheElapsed = 0.0f;
        playerSheathePreviousState = weaponState;
        playerSheatheActionSubmitted = false;
        playerSheathePending = true;
        logger::info("Waiting for the player weapon sheath animation to finish before stripping.");
        return false;
    }

    bool Instance::StartFixedLengthTimer()
    {
        fixedLengthTimer.state = FixedLengthTimer::State::Stopped;
        if (!activeStage || activeStage->GetFixedDuration() == 0.0f) {
            return false;
        }
        const auto duration = activeStage->GetFixedDuration() / 1000.0f;
        fixedLengthTimer.duration = duration;
        fixedLengthTimer.remaining = duration;
        fixedLengthTimer.state = FixedLengthTimer::State::Running;
        UpdateMenuTimerDisplay(duration, duration);
        return true;
    }

    void Instance::CancelFixedLengthTimer()
    {
        fixedLengthTimer.state = FixedLengthTimer::State::Stopped;
    }

    bool Instance::RestartFixedLengthTimer()
    {
        return StartFixedLengthTimer();
    }

    bool Instance::AdjustFixedLengthTimer(float a_delta)
    {
        if (fixedLengthTimer.state == FixedLengthTimer::State::Stopped) {
            return false;
        }
        fixedLengthTimer.remaining += a_delta;
        fixedLengthTimer.state = FixedLengthTimer::State::Running;
        UpdateMenuTimerDisplay(fixedLengthTimer.duration, std::max(0.0f, fixedLengthTimer.remaining));
        return true;
    }

    void Instance::SetFixedLengthTimerPaused(bool a_paused)
    {
        fixedLengthTimer.paused = a_paused;
    }

    bool Instance::ConsumeFixedLengthTimerExpiration()
    {
        if (fixedLengthTimer.state != FixedLengthTimer::State::Expired) {
            return false;
        }
        fixedLengthTimer.state = FixedLengthTimer::State::Stopped;
        return true;
    }

    void Instance::UpdateFixedLengthTimer(float a_delta)
    {
        if (fixedLengthTimer.state != FixedLengthTimer::State::Running || fixedLengthTimer.paused || a_delta <= 0.0f) {
            return;
        }
        fixedLengthTimer.remaining = std::max(0.0f, fixedLengthTimer.remaining - a_delta);
        const auto expired = fixedLengthTimer.remaining <= 0.0f;
        if (expired) {
            fixedLengthTimer.state = FixedLengthTimer::State::Expired;
        }
        UpdateMenuTimerDisplay(fixedLengthTimer.duration, fixedLengthTimer.remaining);
        if (!expired) {
            return;
        }
        const auto scriptObject = Script::GetScriptObject(linkedQst, "sslThreadModel");
        Script::CallbackPtr callbackPtr{};
        if (!scriptObject || !Script::DispatchMethodCall(scriptObject, "OnFixedLengthStageComplete", callbackPtr)) {
            logger::error("Failed to notify Papyrus of fixed-length timer completion for thread {:X}.", linkedQst->GetFormID());
        }
    }

    void Instance::UpdateAnimations(float a_delta)
    {
        const auto timeoutDelta = Util::IsGamePausedOrFrozen() ? 0.0f : a_delta;
        std::shared_lock lock{ _mInstances };
        for (auto&& instance : instances) {
            instance->UpdateFixedLengthTimer(timeoutDelta);
            instance->UpdatePendingAnimations(timeoutDelta);
        }
    }

    bool Instance::GetActiveClips(RE::Actor* a_actor, std::vector<ActiveClip>& a_clips) const
    {
        RE::BSAnimationGraphManagerPtr graphManager;
        if (!a_actor->GetAnimationGraphManager(graphManager) || !graphManager) {
            return false;
        }

        auto& runtime = graphManager->GetRuntimeData();
        RE::BSSpinLockGuard lock{ runtime.updateLock };
        if (runtime.activeGraph >= graphManager->graphs.size()) {
            return false;
        }

        const auto animationGraph = graphManager->graphs[runtime.activeGraph].get();
        if (!animationGraph || !animationGraph->behaviorGraph || !animationGraph->behaviorGraph->isActive) {
            return false;
        }

        const auto activeNodes = *reinterpret_cast<RE::hkArray<RE::hkbNodeInfo>**>(&animationGraph->behaviorGraph->activeNodes);
        if (!activeNodes) {
            return true;
        }

        const auto addClip = [&](RE::hkbClipGenerator* a_clip) {
            if (!a_clip || std::ranges::contains(a_clips, a_clip, &ActiveClip::generator)) {
                return;
            }
            const auto animationNameAddress = std::bit_cast<std::uintptr_t>(a_clip->animationName) & ~std::uintptr_t{ RE::hkStringPtr::kManaged };
            const auto animationName = reinterpret_cast<const char*>(animationNameAddress);
            a_clips.emplace_back(
                a_clip,
                animationName ? std::string{ animationName } : std::string{},
                a_clip->localTime,
                a_clip->animationControl ? a_clip->animationControl->weight : 1.0f);
        };

        for (const auto& activeNode : *activeNodes) {
            if (!activeNode.nodeClone) {
                continue;
            }
            if (const auto clip = skyrim_cast<RE::hkbClipGenerator*>(activeNode.nodeClone)) {
                addClip(clip);
            } else if (const auto synchronizedClip = skyrim_cast<RE::BSSynchronizedClipGenerator*>(activeNode.nodeClone)) {
                addClip(synchronizedClip->clipGenerator);
            }
        }
        return true;
    }

    void Instance::TryStartAnimations()
    {
        std::vector<std::vector<ActiveClip>> activeClips(pendingAnimations.size());

        for (auto& pending : pendingAnimations) {
            if (!pending.transitionAcknowledged && pending.retryDelay <= 0.0f) {
                pending.readinessChecks++;
            }
        }
        if (QueueActorRecoveries(false)) {
            return;
        }

        // Exit furniture and other active states once before waiting for stability
        if (!actorPreparationApplied) {
            for (auto& pending : pendingAnimations) {
                if (pending.transitionAcknowledged || pending.retryDelay > 0.0f) {
                    continue;
                }
                pending.actor->StopCombat();
                pending.actor->EndDialogue();
                pending.actor->InterruptCast(false);
                pending.actor->StopInteractingQuick(true);
                pending.actor->NotifyAnimationGraph("IdleFurnitureExit");
                pending.actor->NotifyAnimationGraph("AnimObjectUnequip");
                pending.actor->NotifyAnimationGraph("IdleStop");
                pending.actor->NotifyAnimationGraph("IdleForceDefaultState");
                if (const auto position = GetPosition(pending.actor); position && !position->data.IsHuman()) {
                    pending.actor->NotifyAnimationGraph("ReturnDefaultState");
                    pending.actor->NotifyAnimationGraph("ReturnToDefault");
                    pending.actor->NotifyAnimationGraph("FNISDefault");
                    pending.actor->NotifyAnimationGraph("IdleReturnToDefault");
                    pending.actor->NotifyAnimationGraph("ForceFurnExit");
                    pending.actor->NotifyAnimationGraph("Reset");
                }
                if (const auto process = pending.actor->GetActorRuntimeData().currentProcess) {
                    process->ClearMuzzleFlashes();
                }
                const auto position = GetPosition(pending.actor);
                if (!position || !PrepareActorForAnimation(linkedQst, pending.actor, position->data.IsHuman())) {
                    return;
                }
            }
            actorPreparationApplied = true;
            const auto scriptObject = Script::GetScriptObject(linkedQst, "sslThreadModel");
            Script::CallbackPtr callbackPtr{};
            if (!scriptObject || !Script::DispatchMethodCall(scriptObject, "OnNativeActorsPrepared", callbackPtr)) {
                logger::warn("Failed to notify Papyrus of native actor preparation for thread {:X}.", linkedQst->GetFormID());
            }
            return;
        }

        // Wait for requested activity exits to reach stable actor states
        for (size_t i = 0; i < pendingAnimations.size(); i++) {
            const auto& pending = pendingAnimations[i];
            if (pending.transitionAcknowledged || pending.retryDelay > 0.0f) {
                continue;
            }
            const auto actorState = pending.actor->AsActorState();
            if (actorState->GetSitSleepState() != RE::SIT_SLEEP_STATE::kNormal || actorState->GetKnockState() != RE::KNOCK_STATE_ENUM::kNormal) {
                return;
            }
        }

        // Snapshot active clips so later updates can verify each transition
        for (size_t i = 0; i < pendingAnimations.size(); i++) {
            const auto& pending = pendingAnimations[i];
            if (pending.transitionAcknowledged || pending.retryDelay > 0.0f) {
                continue;
            }
            if (!GetActiveClips(pending.actor, activeClips[i])) {
                return;
            }
        }

        // Apply native locks and placement before dispatching each animation event
        for (size_t i = 0; i < pendingAnimations.size(); i++) {
            auto& pending = pendingAnimations[i];
            if (pending.transitionAcknowledged || pending.retryDelay > 0.0f) {
                continue;
            }
            const bool firstDispatch = pending.dispatchAttempts == 0;
            if (firstDispatch) {
                const auto& actorData = activeScene->GetNthPosition(pending.position).get();
                Collision::CollisionHandler::AddActor(pending.actor->GetFormID());
                Registry::Scale::GetSingleton()->SetScale(pending.actor, actorData.GetRace(), actorData.GetScale());
            }
            ReassertPlacement(pending.position, firstDispatch);
            pending.dispatchAttempts++;
            if (!pending.actor->NotifyAnimationGraph(pending.event)) {
                pending.retryDelay = 0.1f;
                continue;
            }
            pending.previousClips = std::move(activeClips[i]);
            pending.transitionAcknowledged = true;
            const auto readinessElapsed = pending.elapsed;
            pending.elapsed = 0.0f;
            logger::info("Actor {:X} accepted animation event {} after {} dispatch attempt(s), {} readiness check(s), and {:.3f}s.", pending.actor->GetFormID(), pending.event, pending.dispatchAttempts, pending.readinessChecks, readinessElapsed);
        }
    }

    bool Instance::HoldAnimation(PendingAnimation& a_pending)
    {
        RE::BSAnimationGraphManagerPtr graphManager;
        if (!a_pending.actor->GetAnimationGraphManager(graphManager) || !graphManager) {
            return false;
        }

        auto& runtime = graphManager->GetRuntimeData();
        RE::BSSpinLockGuard lock{ runtime.updateLock };
        if (runtime.activeGraph >= graphManager->graphs.size()) {
            return false;
        }

        const auto animationGraph = graphManager->graphs[runtime.activeGraph].get();
        if (!animationGraph || !animationGraph->behaviorGraph || !animationGraph->behaviorGraph->activeNodes) {
            return false;
        }

        for (const auto& activeNode : *animationGraph->behaviorGraph->activeNodes) {
            auto clip = skyrim_cast<RE::hkbClipGenerator*>(activeNode.nodeClone);
            if (!clip) {
                if (const auto synchronizedClip = skyrim_cast<RE::BSSynchronizedClipGenerator*>(activeNode.nodeClone)) {
                    clip = synchronizedClip->clipGenerator;
                }
            }
            if (clip != a_pending.observedGenerator) {
                continue;
            }
            clip->playbackSpeed = 0.0f;
            clip->localTime = clip->startTime;
            clip->time = clip->startTime;
            if (clip->animationControl) {
                clip->animationControl->playbackSpeed = 0.0f;
                clip->animationControl->localTime = clip->startTime;
            }
            a_pending.playbackHeld = true;
            return true;
        }
        return false;
    }

    void Instance::ReleaseAnimations()
    {
        std::vector<RE::BSAnimationGraphManagerPtr> graphManagers;
        std::vector<std::unique_ptr<RE::BSSpinLockGuard>> locks;
        graphManagers.reserve(pendingAnimations.size());
        locks.reserve(pendingAnimations.size());

        for (const auto& pending : pendingAnimations) {
            if (!pending.playbackHeld) {
                graphManagers.emplace_back();
                continue;
            }
            RE::BSAnimationGraphManagerPtr graphManager;
            if (!pending.actor->GetAnimationGraphManager(graphManager) || !graphManager) {
                graphManagers.emplace_back();
                continue;
            }
            graphManagers.emplace_back(graphManager);
            locks.emplace_back(std::make_unique<RE::BSSpinLockGuard>(graphManager->GetRuntimeData().updateLock));
        }

        for (size_t i = 0; i < pendingAnimations.size(); i++) {
            const auto& pending = pendingAnimations[i];
            const auto& graphManager = graphManagers[i];
            if (!pending.playbackHeld || !graphManager) {
                continue;
            }
            const auto& runtime = graphManager->GetRuntimeData();
            if (runtime.activeGraph >= graphManager->graphs.size()) {
                continue;
            }
            const auto animationGraph = graphManager->graphs[runtime.activeGraph].get();
            if (!animationGraph || !animationGraph->behaviorGraph || !animationGraph->behaviorGraph->activeNodes) {
                continue;
            }
            for (const auto& activeNode : *animationGraph->behaviorGraph->activeNodes) {
                auto clip = skyrim_cast<RE::hkbClipGenerator*>(activeNode.nodeClone);
                if (!clip) {
                    if (const auto synchronizedClip = skyrim_cast<RE::BSSynchronizedClipGenerator*>(activeNode.nodeClone)) {
                        clip = synchronizedClip->clipGenerator;
                    }
                }
                if (clip != pending.observedGenerator) {
                    continue;
                }
                clip->localTime = clip->startTime;
                clip->time = clip->startTime;
                clip->playbackSpeed = animationPlaybackSpeed;
                if (clip->animationControl) {
                    clip->animationControl->localTime = clip->startTime;
                    clip->animationControl->playbackSpeed = animationPlaybackSpeed;
                }
                break;
            }
        }
    }

    void Instance::UpdatePendingAnimations(float a_delta)
    {
        constexpr float playerSheatheTimeout = 15.0f;
        constexpr float retryTimeout = 15.0f;
        constexpr float playbackTimeout = 2.0f;
        constexpr float minimumClipWeight = 0.01f;
        constexpr float minimumTimeChange = 0.0001f;

        if (playerDialoguePending) {
            if (IsPlayerDialogueActive()) {
                return;
            }
            playerDialoguePending = false;
            logger::info("Player dialogue finished; continuing actor preparation.");
            const auto scriptObject = Script::GetScriptObject(linkedQst, "sslThreadModel");
            Script::CallbackPtr callbackPtr{};
            if (!scriptObject || !Script::DispatchMethodCall(scriptObject, "OnPlayerDialogueComplete", callbackPtr)) {
                logger::error("Failed to notify Papyrus of player dialogue completion for thread {:X}.", linkedQst->GetFormID());
            }
            return;
        }

        const auto player = RE::PlayerCharacter::GetSingleton();
        if (!pendingRecoveries.empty()) {
            UpdatePendingRecoveries(a_delta);
            return;
        }

        if (playerSheathePending) {
            playerSheatheElapsed += a_delta;
            const auto weaponState = player ? player->AsActorState()->GetWeaponState() : RE::WEAPON_STATE::kSheathed;
            const bool sheathed = player && weaponState == RE::WEAPON_STATE::kSheathed;
            const bool timedOut = playerSheatheElapsed >= playerSheatheTimeout;
            if (!sheathed && !timedOut && player) {
                switch (weaponState) {
                case RE::WEAPON_STATE::kDrawn:
                    if (!playerSheatheActionSubmitted || playerSheathePreviousState != RE::WEAPON_STATE::kDrawn) {
                        playerSheatheActionSubmitted = RE::SourceActionMap::DoAction(player, RE::DEFAULT_OBJECT::kActionSheath);
                        if (playerSheatheActionSubmitted) {
                            logger::info("Requested native weapon sheathing for the player.");
                        }
                    }
                    break;
                case RE::WEAPON_STATE::kWantToSheathe:
                case RE::WEAPON_STATE::kSheathing:
                    playerSheatheActionSubmitted = true;
                    break;
                case RE::WEAPON_STATE::kWantToDraw:
                case RE::WEAPON_STATE::kDrawing:
                    playerSheatheActionSubmitted = false;
                    break;
                default:
                    break;
                }
                playerSheathePreviousState = weaponState;
            }
            if (sheathed || timedOut) {
                playerSheathePending = false;
                playerSheatheActionSubmitted = false;
                playerSheathePreviousState = RE::WEAPON_STATE::kSheathed;
                if (!sheathed && timedOut) {
                    logger::error("Player weapon did not finish sheathing after {:.3f}s; cancelling animation startup.", playerSheatheElapsed);
                }
                const auto scriptObject = Script::GetScriptObject(linkedQst, "sslThreadModel");
                Script::CallbackPtr callbackPtr{};
                if (!scriptObject || !Script::DispatchMethodCall(scriptObject, "OnPlayerSheatheComplete", callbackPtr, bool{ sheathed })) {
                    logger::error("Failed to notify Papyrus of player weapon sheath completion for thread {:X}.", linkedQst->GetFormID());
                }
            }
        }

        bool retryStart = false;
        for (auto& pending : pendingAnimations) {
            pending.elapsed += a_delta;
            if (!pending.transitionAcknowledged) {
                pending.retryDelay -= a_delta;
                retryStart |= pending.elapsed < retryTimeout && pending.retryDelay <= 0.0f;
            }
        }
        if (retryStart) {
            TryStartAnimations();
        }

        bool synchronizationFailed = false;
        for (auto& pending : pendingAnimations) {
            if (!pending.transitionAcknowledged) {
                if (pending.elapsed >= retryTimeout) {
                    logger::error("Actor {:X} did not accept animation event {} after {} dispatch attempt(s), {} readiness check(s), and {:.3f}s.", pending.actor->GetFormID(), pending.event, pending.dispatchAttempts, pending.readinessChecks, pending.elapsed);
                    synchronizationFailed = true;
                }
                continue;
            }
            if (pending.playbackHeld) {
                continue;
            }

            std::vector<ActiveClip> activeClips;
            if (GetActiveClips(pending.actor, activeClips)) {
                for (const auto& clip : activeClips) {
                    if (clip.animationName.empty() || clip.weight <= minimumClipWeight) {
                        continue;
                    }
                    const auto previous = std::ranges::find(pending.previousClips, clip.generator, &ActiveClip::generator);
                    const bool changed = previous == pending.previousClips.end() || previous->animationName != clip.animationName || clip.localTime + minimumTimeChange < previous->localTime;
                    if (!changed) {
                        continue;
                    }
                    if (pending.observedGenerator == clip.generator && pending.observedAnimation == clip.animationName && std::abs(clip.localTime - pending.observedLocalTime) > minimumTimeChange) {
                        if (HoldAnimation(pending)) {
                            logger::info("Actor {:X} held animation clip {} for synchronization.", pending.actor->GetFormID(), clip.animationName);
                        }
                        break;
                    }
                    pending.observedGenerator = clip.generator;
                    pending.observedAnimation = clip.animationName;
                    pending.observedLocalTime = clip.localTime;
                }
            }

            if (!pending.playbackHeld && pending.elapsed >= playbackTimeout) {
                logger::warn("Actor {:X} acknowledged animation event {}, but clip playback could not be verified.", pending.actor->GetFormID(), pending.event);
                synchronizationFailed = true;
            }
        }

        if (synchronizationFailed) {
            ReleaseAnimations();
            pendingAnimations.clear();
            const auto scriptObject = Script::GetScriptObject(linkedQst, "sslThreadModel");
            Script::CallbackPtr callbackPtr{};
            if (!scriptObject || !Script::DispatchMethodCall(scriptObject, "OnAnimationSyncFailed", callbackPtr)) {
                logger::error("Failed to notify Papyrus of animation synchronization failure for thread {:X}.", linkedQst->GetFormID());
            }
            return;
        }
        if (!pendingAnimations.empty() && std::ranges::all_of(pendingAnimations, &PendingAnimation::playbackHeld)) {
            for (const auto& pending : pendingAnimations) {
                ReassertPlacement(pending.position, false);
            }
            ReleaseAnimations();
            logger::info("Released {} synchronized animation clips.", pendingAnimations.size());
            pendingAnimations.clear();
            StartFixedLengthTimer();
            const auto scriptObject = Script::GetScriptObject(linkedQst, "sslThreadModel");
            Script::CallbackPtr callbackPtr{};
            if (!scriptObject || !Script::DispatchMethodCall(scriptObject, "OnAnimationSynchronized", callbackPtr)) {
                logger::error("Failed to notify Papyrus of animation synchronization completion for thread {:X}.", linkedQst->GetFormID());
            }
        }
    }

    void Instance::SetAnimationPlaybackSpeed(float playbackSpeed)
    {
        if (animationPlaybackSpeed == playbackSpeed) {
            return;
        }
        animationPlaybackSpeed = playbackSpeed;
        std::vector<std::pair<RE::BSAnimationGraphManagerPtr, std::unique_ptr<RE::BSSpinLockGuard>>> lockedGraphs;

        for (auto& position : positions) {
            const auto* actor = position.data.GetActor();
            if (!actor) {
                continue;
            }

            RE::BSAnimationGraphManagerPtr graphMgr;
            if (!actor->GetAnimationGraphManager(graphMgr) || !graphMgr) {
                continue;
            }

            auto& runtime = graphMgr->GetRuntimeData();
            lockedGraphs.emplace_back(
                graphMgr,
                std::make_unique<RE::BSSpinLockGuard>(runtime.updateLock));
        }

        for (auto& [graphMgr, lock] : lockedGraphs) {
            auto& runtime = graphMgr->GetRuntimeData();
            auto activeGraph = runtime.activeGraph;

            RE::BShkbAnimationGraph* animationGraph = graphMgr->graphs[activeGraph].get();
            if (!animationGraph || !animationGraph->behaviorGraph) {
                continue;
            }

            auto* activeNodes = *reinterpret_cast<RE::hkArray<RE::hkbNodeInfo>**>(
                &animationGraph->behaviorGraph->activeNodes);
            if (!activeNodes) {
                continue;
            }

            for (const RE::hkbNodeInfo& activeNode : *activeNodes) {
                if (!activeNode.nodeClone) {
                    continue;
                }
                if (auto* clip = skyrim_cast<RE::hkbClipGenerator*>(activeNode.nodeClone)) {
                    const bool held = std::ranges::any_of(pendingAnimations, [&](const auto& pending) {
                        return pending.playbackHeld && pending.observedGenerator == clip;
                    });
                    clip->playbackSpeed = held ? 0.0f : playbackSpeed;
                    if (clip->animationControl) {
                        clip->animationControl->playbackSpeed = held ? 0.0f : playbackSpeed;
                    }
                }
            }
        }
    }
}  // namespace Thread
