// Skyrim can keep loaded bone transforms/hierarchy in BSFlattenedBoneTree while leaving skin->bones slots null. individual NiAVObject nodes are not
// required. GetObjectByName can lazily create those nodes and their parents, changing the skeleton without repairing existing skin slots. Resolve the
// skin's current world-transform address to its node/tree owner, as Skyrim does, so discovery and skinning use existing bone data without forcing node
// creation. A null node alone does not mean the bone is missing or unloaded.

#include "SkinBone.h"

namespace Thread::Interaction::NiSurface::Geometry::Detail
{
    namespace
    {
        SkinBone FindWorldTransform(RE::NiAVObject* a_object, RE::NiAVObject* a_exclude, const RE::NiTransform* a_world)
        {
            if (!a_object || a_object == a_exclude) {
                return {};
            }
            if (a_world == std::addressof(a_object->world)) {
                return { RE::NiPointer{ a_object } };
            }
            if (auto* tree = netimmerse_cast<RE::BSFlattenedBoneTree*>(a_object)) {
                const auto& data = tree->GetRuntimeData();
                const auto address = reinterpret_cast<std::uintptr_t>(a_world);
                const auto entries = reinterpret_cast<std::uintptr_t>(data.boneEntries);
                if (data.boneEntries && address >= entries) {
                    const auto offset = address - entries;
                    // Match only entry.world, never entry.local or an arbitrary non-null transform.
                    if (offset % sizeof(RE::BSFlattenedBoneTree::BoneEntry) == offsetof(RE::BSFlattenedBoneTree::BoneEntry, world) &&
                        offset / sizeof(RE::BSFlattenedBoneTree::BoneEntry) < data.numBones) {
                        return { RE::NiPointer<RE::NiAVObject>{ tree }, static_cast<std::int32_t>(offset / sizeof(RE::BSFlattenedBoneTree::BoneEntry)) };
                    }
                }
            }
            if (auto* node = a_object->AsNode()) {
                for (const auto& child : node->GetChildren()) {
                    if (auto bone = FindWorldTransform(child.get(), a_exclude, a_world); bone.owner) {
                        return bone;
                    }
                }
            }
            return {};
        }
    }

    const RE::BSFlattenedBoneTree::BoneEntry* SkinBone::GetEntry() const
    {
        auto* tree = owner && index >= 0 ? netimmerse_cast<RE::BSFlattenedBoneTree*>(owner.get()) : nullptr;
        if (!tree) {
            return nullptr;
        }
        const auto& data = tree->GetRuntimeData();
        return data.boneEntries && static_cast<std::uint32_t>(index) < data.numBones ? std::addressof(data.boneEntries[index]) : nullptr;
    }

    RE::NiAVObject* SkinBone::GetNode() const
    {
        if (index < 0) {
            return owner.get();
        }
        const auto* entry = GetEntry();
        return entry ? entry->node : nullptr;
    }

    const RE::NiTransform* SkinBone::GetWorld() const
    {
        if (index < 0) {
            return owner ? std::addressof(owner->world) : nullptr;
        }
        const auto* entry = GetEntry();
        return entry ? std::addressof(entry->world) : nullptr;
    }

    SkinBone SkinBone::GetParent() const
    {
        if (index < 0) {
            return owner ? SkinBone{ RE::NiPointer<RE::NiAVObject>{ owner->parent } } : SkinBone{};
        }
        const auto* entry = GetEntry();
        if (!entry) {
            return {};
        }
        if (entry->parentIndex < 0) {
            return { owner };
        }
        const auto& data = static_cast<RE::BSFlattenedBoneTree*>(owner.get())->GetRuntimeData();
        return entry->parentIndex != index && static_cast<std::uint32_t>(entry->parentIndex) < data.numBones ? SkinBone{ owner, entry->parentIndex } : SkinBone{};
    }

    std::optional<std::size_t> SkinBone::DepthBelow(RE::NiAVObject* a_base) const
    {
        if (!a_base) {
            return std::nullopt;
        }
        std::size_t depth = 0;
        std::size_t flattenedSteps = 0;
        for (auto current = *this; current.owner; current = current.GetParent(), ++depth) {
            if (current.GetNode() == a_base) {
                return depth;
            }
            if (current.index >= 0) {
                auto* tree = netimmerse_cast<RE::BSFlattenedBoneTree*>(current.owner.get());
                if (!tree || ++flattenedSteps > tree->GetRuntimeData().numBones) {
                    return std::nullopt;
                }
            }
        }
        return std::nullopt;
    }

    bool SkinBone::SameBone(const SkinBone& a_other) const
    {
        if (owner && owner == a_other.owner && index == a_other.index) {
            return true;
        }
        auto* node = GetNode();
        return node && node == a_other.GetNode();
    }

    bool SkinBone::Matches(RE::NiSkinInstance* a_skin, std::uint16_t a_skinIndex) const
    {
        if (!a_skin || !a_skin->skinData || a_skinIndex >= a_skin->numMatrices || a_skinIndex >= a_skin->skinData->GetBoneCount()) {
            return false;
        }
        const auto* world = GetWorld();
        if (!world) {
            return false;
        }
        auto* node = a_skin->bones ? a_skin->bones[a_skinIndex] : nullptr;
        if (node) {
            return node == GetNode();
        }
        return a_skin->boneWorldTransforms && a_skin->boneWorldTransforms[a_skinIndex] == world;
    }

    SkinBone ResolveSkinBone(RE::NiSkinInstance* a_skin, std::uint16_t a_skinIndex)
    {
        if (!a_skin || !a_skin->skinData || a_skinIndex >= a_skin->numMatrices || a_skinIndex >= a_skin->skinData->GetBoneCount()) {
            return {};
        }
        if (auto* node = a_skin->bones ? a_skin->bones[a_skinIndex] : nullptr) {
            return { RE::NiPointer{ node } };
        }
        const auto* world = a_skin->boneWorldTransforms ? a_skin->boneWorldTransforms[a_skinIndex] : nullptr;
        if (!world) {
            return {};
        }
        // Mirror NiSkinInstance's bone lookup (SE 0x140C7E030 / 0x140C7EC00): identify the world-transform owner
        // from rootParent outward, excluding subtrees already visited. Name lookups
        // would materialize nodes and still leave the original skin slots null.
        RE::NiAVObject* exclude = nullptr;
        for (auto* root = a_skin->rootParent; root; exclude = root, root = root->parent) {
            if (auto bone = FindWorldTransform(root, exclude, world); bone.owner) {
                return bone;
            }
        }
        return {};
    }
}
