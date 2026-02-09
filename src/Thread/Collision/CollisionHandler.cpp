#include "CollisionHandler.h"

namespace Thread::Collision
{
	namespace
	{
		[[nodiscard]] auto GetTESObjectREFR(const RE::hkpCollidable* a_collidable) -> RE::TESObjectREFR*
		{
			if (!a_collidable || a_collidable->ownerOffset >= 0) {
				return nullptr;
			}

			using enum RE::hkpWorldObject::BroadPhaseType;
			switch (static_cast<RE::hkpWorldObject::BroadPhaseType>(a_collidable->broadPhaseHandle.type)) {
			case kEntity:
				if (auto* body = a_collidable->GetOwner<RE::hkpRigidBody>()) {
					return body->GetUserData();
				}
				break;
			case kPhantom:
				if (auto* phantom = a_collidable->GetOwner<RE::hkpPhantom>()) {
					return phantom->GetUserData();
				}
				break;
			default:
				break;
			}

			return nullptr;
		}

		[[nodiscard]] auto GetCollisionLayer(const RE::hkpCollidable* a_collidable) -> RE::COL_LAYER
		{
			if (!a_collidable) {
				return RE::COL_LAYER::kUnidentified;
			}
			auto info = *reinterpret_cast<const std::uint32_t*>(&a_collidable->broadPhaseHandle.collisionFilterInfo);
			return static_cast<RE::COL_LAYER>(info & 0x7F);
		}

		[[nodiscard]] constexpr bool IsBipedCollisionLayer(RE::COL_LAYER a_layer) noexcept
		{
			using enum RE::COL_LAYER;
			switch (a_layer) {
			case kBiped:
			case kCharController:
			case kDeadBip:
			case kBipedNoCC:
				return true;
			default:
				return false;
			}
		}

		constexpr void ZeroVector4(RE::hkVector4& a_vec)
		{
			a_vec.quad.m128_f32[0] = 0.0f;
			a_vec.quad.m128_f32[1] = 0.0f;
			a_vec.quad.m128_f32[2] = 0.0f;
			a_vec.quad.m128_f32[3] = 0.0f;
		}

		constexpr void SetUnitZVector4(RE::hkVector4& a_vec)
		{
			a_vec.quad.m128_f32[0] = 0.0f;
			a_vec.quad.m128_f32[1] = 0.0f;
			a_vec.quad.m128_f32[2] = 1.0f;
			a_vec.quad.m128_f32[3] = 0.0f;
		}

		struct hkbFootIkDriver : RE::hkReferencedObject
		{
			std::byte pad10[0x20];
			RE::hkQuaternion alignedGroundRotation;
			std::uint64_t unk40;
			std::uint16_t unk48;
			bool disableFootIk;
			std::byte pad4B[5];
		};
		static_assert(offsetof(hkbFootIkDriver, alignedGroundRotation) == 0x30);
		static_assert(offsetof(hkbFootIkDriver, disableFootIk) == 0x4A);

		struct ShadowhkbCharacter : RE::hkReferencedObject
		{
			std::byte pad10[0x30];
			RE::hkRefPtr<hkbFootIkDriver> footIkDriver;
		};
		static_assert(offsetof(ShadowhkbCharacter, footIkDriver) == 0x40);
	}

	void CollisionHandler::Install()
	{
		auto& trampoline = SKSE::GetTrampoline();

		REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_bhkCollisionFilter[1] };
		_IsCollisionEnabled = vtbl.write_vfunc(0x1, IsCollisionEnabled);

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

	void CollisionHandler::ConfigureControllerForNoCollision(RE::bhkCharacterController* a_controller)
	{
		a_controller->pitchAngle = 0.0f;
		a_controller->rollAngle = 0.0f;
		a_controller->calculatePitchTimer = 55.0f;

		RE::hkVector4 zeroVec{};
		ZeroVector4(zeroVec);
		a_controller->SetLinearVelocityImpl(zeroVec);

		ZeroVector4(a_controller->outVelocity);
		ZeroVector4(a_controller->initialVelocity);
		ZeroVector4(a_controller->velocityMod);
		ZeroVector4(a_controller->pushDelta);
		ZeroVector4(a_controller->fakeSupportStart);
		SetUnitZVector4(a_controller->supportNorm);

		a_controller->flags.set(
		  RE::CHARACTER_FLAGS::kNoGravityOnGround,
		  RE::CHARACTER_FLAGS::kNoSim,
		  RE::CHARACTER_FLAGS::kSupport);

		a_controller->flags.reset(
		  RE::CHARACTER_FLAGS::kCheckSupport,
		  RE::CHARACTER_FLAGS::kHasPotentialSupportManifold,
		  RE::CHARACTER_FLAGS::kStuckQuad,
		  RE::CHARACTER_FLAGS::kOnStairs,
		  RE::CHARACTER_FLAGS::kTryStep);

		a_controller->context.currentState = RE::hkpCharacterStateType::kSwimming;
	}

	void CollisionHandler::DisableRigidBodyPhysics(RE::Actor* a_actor)
	{
		auto* process = a_actor->GetMiddleHighProcess();
		if (!process) return;

		auto* controller = process->charController.get();
		if (!controller) return;

		controller->flags.set(RE::CHARACTER_FLAGS::kNotPushablePermanent);
		controller->flags.reset(RE::CHARACTER_FLAGS::kPossiblePathObstacle);
		ZeroVector4(controller->surfaceInfo.surfaceVelocity);

		auto* rigidBodyController = skyrim_cast<RE::bhkCharRigidBodyController*>(controller);
		if (!rigidBodyController) return;

		auto* hkCharRB = *reinterpret_cast<RE::hkpCharacterRigidBody**>(
		  reinterpret_cast<std::uintptr_t>(&rigidBodyController->charRigidBody) + 0x10);

		if (hkCharRB && hkCharRB->character && hkCharRB->character->GetCollidableRW()) {
			hkCharRB->character->motion.SetMassInv(0.0f);
			hkCharRB->character->motion.gravityFactor = 0.0f;
		}
	}

	void CollisionHandler::RestoreRigidBodyPhysics(RE::Actor* a_actor)
	{
		auto* process = a_actor->GetMiddleHighProcess();
		if (!process) return;

		auto* controller = process->charController.get();
		if (!controller) return;

		controller->flags.reset(
		  RE::CHARACTER_FLAGS::kNoGravityOnGround,
		  RE::CHARACTER_FLAGS::kNoSim,
		  RE::CHARACTER_FLAGS::kNotPushablePermanent);

		controller->flags.set(
		  RE::CHARACTER_FLAGS::kSupport,
		  RE::CHARACTER_FLAGS::kCheckSupport);

		controller->context.currentState = RE::hkpCharacterStateType::kOnGround;

		auto* rigidBodyController = skyrim_cast<RE::bhkCharRigidBodyController*>(controller);
		if (!rigidBodyController) return;

		auto* hkCharRB = *reinterpret_cast<RE::hkpCharacterRigidBody**>(
		  reinterpret_cast<std::uintptr_t>(&rigidBodyController->charRigidBody) + 0x10);

		if (hkCharRB && hkCharRB->character && hkCharRB->character->GetCollidableRW()) {
			hkCharRB->character->motion.SetMassInv(1.0f);
			hkCharRB->character->motion.gravityFactor = 1.0f;
		}
	}

	void CollisionHandler::SetFootIKEnabled(RE::Actor* a_actor, bool a_enabled)
	{
		if (!a_actor) return;

		RE::BSAnimationGraphManagerPtr graphMgr;
		if (!a_actor->GetAnimationGraphManager(graphMgr) || !graphMgr) return;

		RE::BSSpinLockGuard locker(graphMgr->GetRuntimeData().updateLock);
		for (auto& graph : graphMgr->graphs) {
			if (!graph) continue;

			auto* shadow = reinterpret_cast<ShadowhkbCharacter*>(&graph->characterInstance);
			if (auto* driver = shadow->footIkDriver.get()) {
				driver->disableFootIk = !a_enabled;
				driver->alignedGroundRotation.vec = { 0.0f, 0.0f, 0.0f, 1.0f };
			}
		}
	}

	void CollisionHandler::Hook_ApplyMovementDelta(RE::Actor* a_actor, float a_delta)
	{
		if (a_actor) {
			std::shared_lock lock{ _mutex };
			if (std::ranges::contains(_cache, a_actor->GetFormID())) {
				if (auto* process = a_actor->GetMiddleHighProcess()) {
					if (auto* controller = process->charController.get()) {
						ConfigureControllerForNoCollision(controller);
						return;
					}
				}
			}
		}
		_originalApplyMovementDelta(a_actor, a_delta);
	}

	bool* CollisionHandler::IsCollisionEnabled(RE::hkpCollidableCollidableFilter* a_this, bool* a_result, const RE::hkpCollidable* a_collidableA, const RE::hkpCollidable* a_collidableB)
	{
		a_result = _IsCollisionEnabled(a_this, a_result, a_collidableA, a_collidableB);

		if (!*a_result) {
			return a_result;
		}

		if (!IsBipedCollisionLayer(GetCollisionLayer(a_collidableA)) ||
			!IsBipedCollisionLayer(GetCollisionLayer(a_collidableB))) {
			return a_result;
		}

		auto* refA = GetTESObjectREFR(a_collidableA);
		auto* refB = GetTESObjectREFR(a_collidableB);

		if (!refA || !refB || refA == refB) {
			return a_result;
		}

		auto* actorA = refA->As<RE::Actor>();
		auto* actorB = refB->As<RE::Actor>();

		if (actorA && actorB) {
			auto idA = actorA->GetFormID();
			auto idB = actorB->GetFormID();
			const std::shared_lock lock{ _mutex };
			if (std::ranges::any_of(_cache, [=](RE::FormID id) { return id == idA || id == idB; })) {
				*a_result = false;
			}
		}

		return a_result;
	}

	void CollisionHandler::AddActor(RE::FormID a_actor)
	{
		const std::unique_lock lock{ _mutex };

		if (std::ranges::contains(_cache, a_actor)) return;

		_cache.push_back(a_actor);

		auto* actor = RE::TESForm::LookupByID<RE::Actor>(a_actor);
		if (!actor) return;

		if (auto* process = actor->GetMiddleHighProcess()) {
			if (auto* controller = process->charController.get()) {
				ConfigureControllerForNoCollision(controller);
			}
		}

		DisableRigidBodyPhysics(actor);
		SetFootIKEnabled(actor, false);
	}

	void CollisionHandler::RemoveActor(RE::FormID a_actor)
	{
		const std::unique_lock lock{ _mutex };
		std::erase(_cache, a_actor);

		auto* actor = RE::TESForm::LookupByID<RE::Actor>(a_actor);
		if (!actor) return;

		SetFootIKEnabled(actor, true);
		RestoreRigidBodyPhysics(actor);
	}
}
