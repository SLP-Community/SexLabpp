#pragma once

#include "RE/B/BSFlattenedBoneTree.h"

namespace Thread::Interaction::NiSurface::Geometry::Detail
{
    // Skyrim represents a skin bone as either a node or a flattened-tree entry.
    // Retain the owner, not a borrowed pointer into its transform array.
    struct SkinBone
    {
        RE::NiPointer<RE::NiAVObject> owner;
        std::int32_t index{ -1 };

        RE::NiAVObject* GetNode() const;
        const RE::NiTransform* GetWorld() const;
        SkinBone GetParent() const;
        std::optional<std::size_t> DepthBelow(RE::NiAVObject* a_base) const;
        bool SameBone(const SkinBone& a_other) const;
        bool Matches(RE::NiSkinInstance* a_skin, std::uint16_t a_skinIndex) const;

      private:
        const RE::BSFlattenedBoneTree::BoneEntry* GetEntry() const;
    };

    SkinBone ResolveSkinBone(RE::NiSkinInstance* a_skin, std::uint16_t a_skinIndex);
}
