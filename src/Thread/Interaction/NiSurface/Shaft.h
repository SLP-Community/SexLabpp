#pragma once

#include "CollisionShapes.h"
#include "GeometryMath.h"

namespace Thread::Interaction::NiSurface::Geometry
{
    class Shaft
    {
      public:
        Shaft(RE::Actor* a_actor, RE::NiPointer<RE::NiNode> a_baseNode, const glm::mat3& a_rotation);

        GeometryMath::Segment GetReferenceSegment() const;
        RE::NiPointer<RE::NiNode> GetBaseReferenceNode() const;
        void UpdateCollisionShape();
        const ShaftShape* GetCollisionShape() const { return collisionShape ? std::addressof(*collisionShape) : nullptr; }

      private:
        struct Influence
        {
            std::uint16_t bone;
            float weight;
        };

        struct Sample
        {
            std::uint16_t vertex;
            RE::NiPoint3 local;
            std::vector<Influence> influences;
        };

        struct SampleRing
        {
            std::vector<Sample> samples;
            std::vector<RE::NiPoint3> worldPositions;
        };

        struct Bone
        {
            std::uint16_t skinIndex;
            RE::NiTransform transform;
        };

        struct SkinnedSurface
        {
            RE::NiPointer<RE::BSGeometry> geometry;
            RE::NiSkinInstance* skinInstance{ nullptr };
            std::vector<SampleRing> rings;
            SampleRing tip;
            std::vector<Bone> bones;
        };

        // ActorState keeps the owner actor alive longer than this object; retain it for delayed equipment discovery.
        RE::Actor* ownerActor{ nullptr };
        std::vector<RE::NiPointer<RE::NiNode>> skeletonNodes;
        RE::NiMatrix3 rotation;
        std::optional<SkinnedSurface> surface;
        std::optional<ShaftShape> collisionShape;
        std::uint64_t equipmentSignature{ 0 };
        std::uint8_t stableEquipmentFrames{ 0 };
        bool surfaceSearchPending{ false };

        void DiscoverSurface(RE::Actor* a_actor);
        bool BindSurface(RE::BSGeometry* a_geometry, std::string_view a_modelPath, const std::vector<std::uint16_t>* a_knownChain = nullptr);
    };
}
