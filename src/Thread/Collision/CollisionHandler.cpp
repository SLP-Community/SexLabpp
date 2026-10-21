#include "CollisionHandler.h"

namespace Thread::Collision
{
    template <class T>
    struct CollisionHandler::MoveHavokHook
    {
        static void Install()
        {
            REL::Relocation<std::uintptr_t> vtable{ T::VTABLE[0] };
            const auto slot = REL::Module::IsVR() ? 0x86 : 0x85;
            original = vtable.write_vfunc(slot, Thunk);
        }

        static void Thunk(T* a_actor, bool a_forceRec)
        {
            // MoveHavok also aligns the controller and binds it to the current skeleton. We need to preserve that synchronization
            original(a_actor, a_forceRec);
            if (HasActor(a_actor->GetFormID())) {
                // The original has just changed Havok state in this context, so detach
                // here too instead of leaving the controller attached until a queued task.
                ApplyActorLock(a_actor, true);
            }
        }

        static inline REL::Relocation<decltype(Thunk)> original;
    };

    void CollisionHandler::Install()
    {
        auto& trampoline = SKSE::GetTrampoline();

        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(36359, 37350) };
        _originalApplyMovementDelta = trampoline.write_call<5>(target.address() + OFFSET(0xF0, 0xFB), Hook_ApplyMovementDelta);

        MoveHavokHook<RE::Actor>::Install();
        MoveHavokHook<RE::Character>::Install();
        MoveHavokHook<RE::PlayerCharacter>::Install();

        logger::info("CollisionHandler hooks installed.");
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

    void CollisionHandler::SetActorLocked(RE::Actor* a_actor, bool a_locked)
    {
        auto applyLock = [handle = a_actor->GetHandle(), a_locked]() {
            const auto actor = handle.get();
            if (!actor || HasActor(actor->GetFormID()) != a_locked) {
                return;
            }

            ApplyActorLock(actor.get(), a_locked);
        };

        // RagdollStartHandler and GetUpStartHandler queue world changes when required so we follow that pattern for safety (Shouldn't be needed)
        if (RE::TaskQueueInterface::ShouldUseTaskQueue()) {
            SKSE::GetTaskInterface()->AddTask(std::move(applyLock));
        } else {
            applyLock();
        }
    }

    void CollisionHandler::ApplyActorLock(RE::Actor* a_actor, bool a_locked)
    {
        if (auto* controller = a_actor->GetCharController()) {
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
            // Retains the controller and invalidates its foot-IK raycast cache.
            // RagdollStartHandler uses the same underlying controller/world disconnection.
            a_actor->DetachCharController();
        } else {
            // Align and reconnect the controller, as on get-up/resurrection.
            a_actor->MoveHavok(false);
        }
        SetFootIKEnabled(a_actor, !a_locked);
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
