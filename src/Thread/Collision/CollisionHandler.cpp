#include "CollisionHandler.h"

namespace Thread::Collision
{
    void CollisionHandler::Install()
    {
        auto& trampoline = SKSE::GetTrampoline();

        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(36359, 37350) };
        _originalApplyMovementDelta = trampoline.write_call<5>(target.address() + OFFSET(0xF0, 0xFB), Hook_ApplyMovementDelta);

        logger::info("CollisionHandler hook installed.");
    }

    void CollisionHandler::Clear()
    {
        const std::unique_lock lock{ _mutex };
        _cache.clear();
    }

    bool CollisionHandler::HasActor(RE::FormID a_actor)
    {
        const std::shared_lock lock{ _mutex };
        return std::ranges::contains(_cache, a_actor);
    }

    // Note: Technically a mod or skyrim it's self could re-attach the controller. Just something to keep in mind
    // if this seems to "not work" for certain mod lists and what not...
    void CollisionHandler::SetActorLocked(RE::Actor* a_actor, bool a_locked)
    {
        auto applyLock = [handle = a_actor->GetHandle(), a_locked]() {
            const auto actor = handle.get();
            if (!actor || HasActor(actor->GetFormID()) != a_locked) {
                return;
            }

            if (auto* controller = actor->GetCharController()) {
                const RE::hkVector4 zeroVec{};
                controller->SetLinearVelocityImpl(zeroVec);
                controller->outVelocity = zeroVec;
                controller->initialVelocity = zeroVec;
                controller->velocityMod = zeroVec;
                controller->pushDelta = zeroVec;
                controller->surfaceInfo.surfaceVelocity = zeroVec;
                if (a_locked) {
                    controller->pitchAngle = 0.0f;
                    controller->rollAngle = 0.0f;
                }
            }

            if (a_locked) {
                // This function retains the controller and invalidates its foot-IK raycast cache.
                // RagdollStartHandler (at 140722B30) uses the same underlying controller/world disconnection
                actor->DetachCharController();
            } else {
                // Actor::MoveHavok aligns the retained controller and
                // reconnects it to the current cell's world, as on get-up/resurrection
                actor->MoveHavok(false);
            }
            SetFootIKEnabled(actor.get(), !a_locked);
        };

        // RagdollStartHandler and GetUpStartHandler queue world changes when required so we follow that pattern for safety (Shouldn't be needed)
        if (RE::TaskQueueInterface::ShouldUseTaskQueue()) {
            SKSE::GetTaskInterface()->AddTask(std::move(applyLock));
        } else {
            applyLock();
        }
    }

    void CollisionHandler::SetFootIKEnabled(RE::Actor* a_actor, bool a_enabled)
    {
        if (!a_actor)
            return;

        RE::BSAnimationGraphManagerPtr graphMgr;
        if (!a_actor->GetAnimationGraphManager(graphMgr) || !graphMgr)
            return;

        RE::BSSpinLockGuard locker(graphMgr->GetRuntimeData().updateLock);
        for (auto& graph : graphMgr->graphs) {
            if (!graph)
                continue;

            if (auto* driver = graph->characterInstance.footIkDriver.get()) {
                driver->disableFootIk = !a_enabled;
                driver->alignWithGroundRotation.vec = { 0.0f, 0.0f, 0.0f, 1.0f };
            }
        }
    }

    void CollisionHandler::Hook_ApplyMovementDelta(RE::Actor* a_actor, float a_delta)
    {
        // Movement application (at 1405D87F0) consumes movementcontroller output and
        // cached animation deltas, including rotation. Detachment skips ordinary controller
        // simulation when its world is null, but explicit position/rotation paths remain...
        // So this hook preserves a strict movement lock while skeletal animation continues.
        if (a_actor && HasActor(a_actor->GetFormID())) {
            return;
        }
        _originalApplyMovementDelta(a_actor, a_delta);
    }

    void CollisionHandler::AddActor(RE::FormID a_actor)
    {
        {
            const std::unique_lock lock{ _mutex };
            if (std::ranges::contains(_cache, a_actor))
                return;

            _cache.push_back(a_actor);
        }

        auto* actor = RE::TESForm::LookupByID<RE::Actor>(a_actor);
        if (!actor)
            return;

        SetActorLocked(actor, true);
    }

    void CollisionHandler::RemoveActor(RE::FormID a_actor)
    {
        {
            const std::unique_lock lock{ _mutex };
            if (std::erase(_cache, a_actor) == 0) {
                return;
            }
        }

        auto* actor = RE::TESForm::LookupByID<RE::Actor>(a_actor);
        if (!actor)
            return;

        SetActorLocked(actor, false);
    }
}
