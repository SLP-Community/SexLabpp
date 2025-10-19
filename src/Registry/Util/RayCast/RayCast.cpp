#include "../RayCast.h"
#include "Offsets.h"
#include "bhkLinearCastCollector.h"

/* From SmoothCam */
SkyrimSE::bhkLinearCastCollector* getCastCollector() noexcept
{
	static SkyrimSE::bhkLinearCastCollector collector = SkyrimSE::bhkLinearCastCollector();
	return &collector;
}

#ifdef SKYRIMVR
Raycast::RayResult Raycast::CastRay(glm::vec4, glm::vec4, float) noexcept
{
	logger::critical("No VR implementation for RayCast::CastRay");
	return {};
}
#else
Raycast::RayResult Raycast::CastRay(glm::vec4 start, glm::vec4 end, float traceHullSize) noexcept
{
	RayResult res;

	const auto ply = RE::PlayerCharacter::GetSingleton();
	const auto cam = RE::PlayerCamera::GetSingleton()->GetRuntimeData();
	if (!ply->parentCell || !cam.unk120)
		return res;

	auto physicsWorld = ply->parentCell->GetbhkWorld();
	if (physicsWorld) {
		typedef bool(__fastcall * RayCastFunType)(
			decltype(RE::PlayerCamera::GetSingleton()->GetRuntimeData().unk120) physics, RE::bhkWorld * world, glm::vec4 & rayStart,
			glm::vec4 & rayEnd, uint32_t * rayResultInfo, RE::NiAVObject * *hitActor, float traceHullSize);

		static auto cameraCaster = REL::Relocation<RayCastFunType>(Offsets::CameraCaster);
		res.hit = cameraCaster(
			cam.unk120, physicsWorld,
			start, end, static_cast<uint32_t*>(res.data), &res.hitObject,
			traceHullSize);
	}

	if (res.hit) {
		res.hitPos = end;
		res.rayLength = glm::length(static_cast<glm::vec3>(res.hitPos) - static_cast<glm::vec3>(start));
	}

	return res;
}
#endif


Raycast::RayResult Raycast::hkpCastRay(const glm::vec4& start, const glm::vec4& end) noexcept {
	return hkpCastRay(start, end, std::vector<RE::NiAVObject*>{});
}

Raycast::RayResult Raycast::hkpCastRay(const glm::vec4& start, const glm::vec4& end, std::initializer_list<const RE::TESObjectREFR*> a_filter) noexcept
{
	std::vector<RE::NiAVObject*> filter{};
	filter.reserve(a_filter.size());
	for (auto&& ref : a_filter) {
		auto niobj = ref->Get3D();
		if (niobj) {
			filter.push_back(niobj->AsNode());
		}
	}
	return hkpCastRay(start, end, filter);
}

Raycast::RayResult Raycast::hkpCastRay(const glm::vec4& start, const glm::vec4& end, const std::vector<RE::NiAVObject*>& a_filter) noexcept
{
	const auto hkpScale = RE::bhkWorld::GetWorldScale();
	const auto dif = end - start;

	SkyrimSE::bhkRayCastInfo info;
	info.start = start * hkpScale;
	info.end = dif * hkpScale;
	info.collector = getCastCollector();
	info.collector->reset();

	for (auto&& niobj : a_filter) {
		info.collector->addFilter(niobj);
	}

	const auto player = RE::PlayerCharacter::GetSingleton();
	if (!player->parentCell)
		return {};

	auto physicsWorld = player->parentCell->GetbhkWorld();
	if (physicsWorld) {
		typedef void (__thiscall RE::bhkWorld::*CastRay)(SkyrimSE::hkpRayCastInfo*) const;

		(physicsWorld->*reinterpret_cast<CastRay>(&RE::bhkWorld::PickObject))(&info);
		//physicsWorld->CastRay(&info); // <-- use this instead, fix param
	}

	SkyrimSE::bhkRayHitResult best = {};
	best.hitFraction = 1.0f;
	glm::vec4 bestPos = {};

	for (auto& hit : info.collector->results) {
		const auto pos = (dif * hit.hitFraction) + start;
		if (best.hit == nullptr) {
			best = hit;
			bestPos = pos;
			continue;
		}

		if (hit.hitFraction < best.hitFraction) {
			best = hit;
			bestPos = pos;
		}
	}

	RayResult result;
	result.hitPos = bestPos;
	result.rayLength = glm::length(bestPos - start);

	if (!best.hit)
		return result;

	typedef RE::NiAVObject* (*_GetUserData)(SkyrimSE::bhkShapeList*);
	auto getAVObject = REL::Relocation<_GetUserData>(Offsets::GetNiAVObject);
	auto av = getAVObject(best.hit);

	result.hit = av != nullptr;
	result.hitObject = av;

	return result;
}
