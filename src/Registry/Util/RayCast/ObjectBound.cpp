#include "ObjectBound.h"
#include "Math.h"
#include "bhkRigidBodyT.h"

RE::NiPointer<RE::NiCollisionObject> ObjectBound::GetCollisionNodeRecurse(RE::NiNode* a_node, size_t a_recurse)
{
	static constexpr size_t MAX_RECURSE = 10;

	if (a_node->collisionObject) {
		return a_node->collisionObject;
	}
	if (a_recurse > MAX_RECURSE) {
		return nullptr;
	}
	for (auto child : a_node->GetChildren()) {
		if (!child)
			continue;
		if (child->collisionObject)
			return child->collisionObject;
		if (auto node = child->AsNode()) {
			if (auto rec = GetCollisionNodeRecurse(node, a_recurse + 1)) {
				return rec;
			}
		}
	}
	return nullptr;
}

std::optional<ObjectBound> ObjectBound::MakeBoundingBox(RE::NiNode* a_niobj)
{
	const auto collision = GetCollisionNodeRecurse(a_niobj, 0);
	if (!collision) {
		return std::nullopt;
	}
	const auto bhkcollision = collision->AsBhkNiCollisionObject();
	if (!bhkcollision || !bhkcollision->body.get()) {
		return std::nullopt;
	}
	const auto rigidbody = bhkcollision->body->AsBhkRigidBody();
	if (!rigidbody || !rigidbody->referencedObject.get()) {
		return std::nullopt;
	}
	auto havokRigidBody = static_cast<RE::hkpRigidBody*>(rigidbody->referencedObject.get());
	if (!havokRigidBody) {
		return std::nullopt;
	}
	uintptr_t hkpBoxShape = (uintptr_t)havokRigidBody + 0x20;
	if (!hkpBoxShape)
		return std::nullopt;
	RE::hkpShape* hkpShape = *(RE::hkpShape**)hkpBoxShape;
	if (!hkpShape)
		return std::nullopt;

	ObjectBound bound{};

	// havok uses different coordinate system. Multiply by this to convert havok units to Skyrim units
	float scaleInverse = RE::bhkWorld::GetWorldScaleInverse();

	RE::hkVector4 bhkBodyPosition;
	rigidbody->GetPosition(bhkBodyPosition);
	// just gets the data out of the register Havok uses into a more usable format
	float bodyPosition[4];
	_mm_store_ps(bodyPosition, bhkBodyPosition.quad);

	RE::hkTransform shapeTransform;
	// use identity matrix for the BB of the unrotated object
	shapeTransform.rotation.col0 = { 1.0f, 0.0f, 0.0f, 0.0f };
	shapeTransform.rotation.col1 = { 0.0f, 1.0f, 0.0f, 0.0f };
	shapeTransform.rotation.col2 = { 0.0f, 0.0f, 1.0f, 0.0f };

	shapeTransform.translation.quad = _mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f);
	RE::hkAabb boundingBoxLocal;
	hkpShape->GetAabbImpl(shapeTransform, 0.0f, boundingBoxLocal);

	float boundMinLocal[4];
	_mm_store_ps(boundMinLocal, boundingBoxLocal.min.quad);
	float boundMaxLocal[4];
	_mm_store_ps(boundMaxLocal, boundingBoxLocal.max.quad);

	RE::hkVector4 rigidBodyLocalTranslation;
	glm::vec3 rigidBodyLocalRotation(0.0f);

	auto rigidBodyT = skyrim_cast<bhkRigidBodyT*>(rigidbody);

	// some objects have a local translation on their collisionObject. In that
	// case, the node is of type bhkRigidBodyT The BB takes that into
	// account, so I have to as well. Can be seen in nifscope
	if (rigidBodyT) {
		rigidBodyLocalTranslation = rigidBodyT->translation;

		// don't forget that bhkRigidBodyT also has a local rotation!
		auto hkLocalRot = rigidBodyT->rotation;
		float localRotArr[4];
		_mm_store_ps(localRotArr, hkLocalRot.vec.quad);

		rigidBodyLocalRotation = Misc::Math::QuatToEuler(glm::quat(
			Misc::Math::MakeValid(localRotArr[3], 0.0f),
			Misc::Math::MakeValid(localRotArr[0], 0.0f),
			Misc::Math::MakeValid(localRotArr[1], 0.0f),
			Misc::Math::MakeValid(localRotArr[2], 0.0f)));
	}

	float localTranslation[4];
	_mm_store_ps(localTranslation, rigidBodyLocalTranslation.quad);

	// for some reason still requires adding local rotation offset. Rotation is accurate for most
	// objects, but some are weird: eg. books, wooden handle of mills (somehow on the mill object,
	// local rotation offset shouldn't be added?)
	if (collision->sceneObject) {
		auto clObjectParentRot = Misc::Math::RotMatrixToEuler(collision->sceneObject->world.rotate);
		bound.rotation = clObjectParentRot + rigidBodyLocalRotation;
	}

	// put local positions into output
	bound.boundMin = glm::vec3(boundMinLocal[0], boundMinLocal[1], boundMinLocal[2]) * scaleInverse;
	bound.boundMax = glm::vec3(boundMaxLocal[0], boundMaxLocal[1], boundMaxLocal[2]) * scaleInverse;

	auto glmLocalT = Misc::Math::MakeValid(glm::vec3(localTranslation[0], localTranslation[1], localTranslation[2]) * scaleInverse, 0.0f);
	// apply the local rotation of bhkRigidBodyT to its translation
	auto glmLocalTRotated = Misc::Math::RotateVector(rigidBodyLocalRotation, glmLocalT);

	auto glmBodyPosition = glm::vec3(bodyPosition[0], bodyPosition[1], bodyPosition[2]) * scaleInverse;

	// take what would be the local origin location of the BB and rotate it with the body
	auto boundMinRotated_origin = Misc::Math::RotateVector(bound.rotation, bound.boundMin + glmLocalTRotated);
	auto boundMaxRotated_origin = Misc::Math::RotateVector(bound.rotation, bound.boundMax + glmLocalTRotated);

	// global origin location
	bound.worldBoundMin = glmBodyPosition + boundMinRotated_origin;
	bound.worldBoundMax = glmBodyPosition + boundMaxRotated_origin;

	return bound;
}

glm::vec3 ObjectBound::GetCenterWorld() const
{
	return worldBoundMin + (worldBoundMax - worldBoundMin) / 2.0f;
}

bool ObjectBound::IsPointInside(float a_x, float a_y, float a_z) const
{
	return (a_x >= worldBoundMin.x && a_x <= worldBoundMax.x &&
					a_y >= worldBoundMin.y && a_y <= worldBoundMax.y &&
					a_z >= worldBoundMin.z && a_z <= worldBoundMax.z);
}

bool ObjectBound::IsPointInside(const glm::vec3& a_point) const
{
	return IsPointInside(a_point.x, a_point.y, a_point.z);
}

bool ObjectBound::IsPointInside(const RE::NiPoint3& a_point) const
{
	return IsPointInside(a_point.x, a_point.y, a_point.z);
}

bool ObjectBound::IsValid() const
{
	return (boundMin.x < boundMax.x &&
					boundMin.y < boundMax.y &&
					boundMin.z < boundMax.z);
}
