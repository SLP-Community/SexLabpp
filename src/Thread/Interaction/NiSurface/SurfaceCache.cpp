#include "SurfaceCache.h"

namespace Thread::Interaction::NiSurface::Geometry
{
    namespace Detail
    {
        namespace
        {
            std::unordered_map<std::string, std::vector<OpeningTopology>> openingAssetTopologyCache;
            std::unordered_map<std::uint64_t, OpeningTopology> openingFingerprintTopologyCache;
            std::unordered_map<std::string, OpeningSelection> openingSelectionCache;
            std::unordered_map<std::string, std::vector<ShaftTopology>> shaftAssetTopologyCache;
            std::unordered_map<std::uint64_t, ShaftTopology> shaftFingerprintTopologyCache;
            std::unordered_map<std::string, std::string> shaftSelectionCache;
        }

        void HashValue(std::uint64_t& a_hash, std::uint64_t a_value)
        {
            a_hash ^= a_value + 0x9E3779B97F4A7C15ULL + (a_hash << 6) + (a_hash >> 2);
        }

        // Equipped-part pointer identity is a cheap signal that Skyrim has finished replacing actor geometry.
        std::uint64_t GetBipedSignature(RE::Actor* a_actor)
        {
            if (!a_actor) {
                return 0;
            }
            const auto& biped = a_actor->GetBiped();
            if (!biped) {
                return 0;
            }
            std::uint64_t result = 0xCBF29CE484222325ULL;
            HashValue(result, reinterpret_cast<std::uintptr_t>(biped.get()));
            for (const auto& object : biped->objects) {
                HashValue(result, reinterpret_cast<std::uintptr_t>(object.partClone.get()));
                HashValue(result, reinterpret_cast<std::uintptr_t>(object.part));
                HashValue(result, reinterpret_cast<std::uintptr_t>(object.addon));
            }
            return result;
        }

        // Asset paths are case-insensitive and may use either slash style.
        std::string NormalizeAssetPath(std::string_view a_path)
        {
            std::string result(a_path);
            for (auto& c : result) {
                if (c == '\\') {
                    c = '/';
                } else if (c >= 'A' && c <= 'Z') {
                    c += 'a' - 'A';
                }
            }
            return result;
        }

        std::string MakeAssetTopologyKey(std::string_view a_modelPath, std::string_view a_geometryName, std::string_view a_openingName)
        {
            if (a_modelPath.empty()) {
                return {};
            }
            auto result = NormalizeAssetPath(a_modelPath);
            result.reserve(result.size() + a_geometryName.size() + a_openingName.size() + 2);
            result.push_back('\n');
            result.append(a_geometryName);
            result.push_back('\n');
            result.append(a_openingName);
            return result;
        }

        std::string MakeShaftSelectionKey(std::string_view a_modelPath, std::string_view a_baseName)
        {
            if (a_modelPath.empty()) {
                return {};
            }
            auto result = NormalizeAssetPath(a_modelPath);
            result.reserve(result.size() + a_baseName.size() + 1);
            result.push_back('\n');
            result.append(a_baseName);
            return result;
        }

        std::optional<OpeningSelection> GetOpeningSelection(std::string_view a_modelPath)
        {
            const auto key = NormalizeAssetPath(a_modelPath);
            if (const auto cached = openingSelectionCache.find(key); cached != openingSelectionCache.end()) {
                return cached->second;
            }
            return std::nullopt;
        }

        void CacheOpeningSelection(std::string_view a_modelPath, std::string_view a_openingName, std::string_view a_geometryName)
        {
            if (a_modelPath.empty()) {
                return;
            }
            auto& selection = openingSelectionCache[NormalizeAssetPath(a_modelPath)];
            (a_openingName == "vaginal" ? selection.vaginal : selection.anal) = a_geometryName;
        }

        std::optional<std::string> GetShaftSelection(std::string_view a_modelPath, std::string_view a_baseName)
        {
            const auto key = MakeShaftSelectionKey(a_modelPath, a_baseName);
            if (key.empty()) {
                return std::nullopt;
            }
            if (const auto cached = shaftSelectionCache.find(key); cached != shaftSelectionCache.end()) {
                return cached->second;
            }
            return std::nullopt;
        }

        void CacheShaftSelection(std::string_view a_modelPath, std::string_view a_baseName, std::string_view a_geometryName)
        {
            const auto key = MakeShaftSelectionKey(a_modelPath, a_baseName);
            if (key.empty()) {
                return;
            }
            shaftSelectionCache[key] = a_geometryName;
        }

        std::optional<ShaftTopology> FindShaftAssetTopology(std::string_view a_key, RE::NiSkinInstance* a_skin, RE::NiSkinData* a_skinData, RE::NiSkinPartition* a_partition, RE::NiAVObject* a_base)
        {
            if (const auto cached = shaftAssetTopologyCache.find(std::string(a_key)); cached != shaftAssetTopologyCache.end()) {
                if (const auto match = std::ranges::find_if(cached->second, [&](const ShaftTopology& a_topology) {
                        return MatchesShaftTopology(a_topology, a_skin, a_skinData, a_partition, a_base);
                    }); match != cached->second.end()) {
                    return *match;
                }
            }
            return std::nullopt;
        }

        std::optional<ShaftTopology> FindShaftFingerprintTopology(std::uint64_t a_fingerprint, RE::NiSkinInstance* a_skin, RE::NiSkinData* a_skinData, RE::NiSkinPartition* a_partition, RE::NiAVObject* a_base)
        {
            if (const auto cached = shaftFingerprintTopologyCache.find(a_fingerprint);
                cached != shaftFingerprintTopologyCache.end() && MatchesShaftTopology(cached->second, a_skin, a_skinData, a_partition, a_base)) {
                return cached->second;
            }
            return std::nullopt;
        }

        void CacheShaftTopology(std::string_view a_assetKey, const ShaftTopology& a_topology)
        {
            shaftFingerprintTopologyCache.try_emplace(a_topology.fingerprint, a_topology);
            if (a_assetKey.empty()) {
                return;
            }
            auto& variants = shaftAssetTopologyCache[std::string(a_assetKey)];
            if (std::ranges::none_of(variants, [&](const ShaftTopology& a_variant) { return a_variant.fingerprint == a_topology.fingerprint; })) {
                variants.push_back(a_topology);
            }
        }

        bool ShouldUseGeometry(RE::BSGeometry* a_geometry)
        {
            const auto& runtime = a_geometry->GetGeometryRuntimeData();
            if (!runtime.skinInstance || !runtime.shaderProperty) {
                return false;
            }

            using Flag = RE::BSShaderProperty::EShaderPropertyFlag;
            const auto& flags = runtime.shaderProperty->flags;
            return !flags.any(Flag::kHairTint, Flag::kEyeReflect, Flag::kEffectLighting, Flag::kSoftEffect, Flag::kDecal, Flag::kDynamicDecal);
        }

        std::optional<OpeningWeights> GetOpeningWeights(RE::BSGeometry* a_geometry, const std::array<RE::NiAVObject*, 2>& a_targets)
        {
            if (!ShouldUseGeometry(a_geometry)) {
                return std::nullopt;
            }

            auto* skin = a_geometry->GetGeometryRuntimeData().skinInstance.get();
            auto* skinData = skin ? skin->skinData.get() : nullptr;
            auto* skinPartition = skin ? skin->skinPartition.get() : nullptr;
            if (!skinData || !skinPartition || !skin->bones || skinPartition->numPartitions == 0 || skinPartition->vertexCount == 0) {
                return std::nullopt;
            }

            OpeningWeights result{};
            const auto boneCount = std::min(skin->numMatrices, skinData->GetBoneCount());
            for (std::size_t landmark = 0; landmark < a_targets.size(); ++landmark) {
                const auto boneIndex = [&]() -> std::optional<std::uint16_t> {
                    for (std::uint32_t i = 0; i < boneCount; ++i) {
                        if (skin->bones[i] == a_targets[landmark]) {
                            return static_cast<std::uint16_t>(i);
                        }
                    }
                    return std::nullopt;
                }();
                if (!boneIndex) {
                    return std::nullopt;
                }

                result.bones[landmark] = *boneIndex;
                const auto* weightedVertices = skinData->GetBoneDataBoneVertData(*boneIndex);
                result.counts[landmark] = skinData->GetBoneDataVerts(*boneIndex);
                if (!weightedVertices) {
                    return std::nullopt;
                }
                for (std::uint16_t i = 0; i < result.counts[landmark]; ++i) {
                    const auto& vertex = weightedVertices[i];
                    if (vertex.vert < skinPartition->vertexCount) {
                        result.maximum[landmark] = std::max(result.maximum[landmark], vertex.weight);
                    }
                }
                if (result.maximum[landmark] <= FLT_EPSILON) {
                    return std::nullopt;
                }
            }
            return result;
        }

        std::optional<ShaftWeights> GetShaftWeights(RE::BSGeometry* a_geometry, RE::NiAVObject* a_base)
        {
            if (!ShouldUseGeometry(a_geometry) || !a_base) {
                return std::nullopt;
            }
            auto* skin = a_geometry->GetGeometryRuntimeData().skinInstance.get();
            auto* skinData = skin ? skin->skinData.get() : nullptr;
            auto* partition = skin ? skin->skinPartition.get() : nullptr;
            if (!skinData || !partition || !skin->bones || partition->vertexCount == 0) {
                return std::nullopt;
            }

            struct Descendant
            {
                std::uint16_t skinIndex;
                std::size_t depth;
                float distanceSq;
            };
            std::vector<Descendant> descendants;
            const auto boneCount = std::min(skin->numMatrices, skinData->GetBoneCount());
            for (std::uint32_t boneIndex = 0; boneIndex < boneCount; ++boneIndex) {
                auto* bone = skin->bones[boneIndex];
                if (!bone || skinData->GetBoneDataVerts(boneIndex) == 0 || !skinData->GetBoneDataBoneVertData(boneIndex)) {
                    continue;
                }
                std::size_t depth = 0;
                for (auto* current = bone; current; current = current->parent, ++depth) {
                    if (current == a_base) {
                        descendants.push_back({ static_cast<std::uint16_t>(boneIndex), depth, (bone->world.translate - a_base->world.translate).SqrLength() });
                        break;
                    }
                }
            }
            if (descendants.empty()) {
                return std::nullopt;
            }

            // A skin can contain side branches. The deepest, then farthest, descendant identifies the shaft branch.
            const Descendant* distal = std::addressof(descendants.front());
            for (const auto& descendant : descendants) {
                if (descendant.depth > distal->depth || (descendant.depth == distal->depth && descendant.distanceSq > distal->distanceSq)) {
                    distal = std::addressof(descendant);
                }
            }
            std::vector<std::uint16_t> chain;
            for (auto* current = skin->bones[distal->skinIndex]; current; current = current->parent) {
                if (const auto match = std::ranges::find_if(descendants, [&](const Descendant& a_descendant) { return skin->bones[a_descendant.skinIndex] == current; }); match != descendants.end()) {
                    chain.push_back(match->skinIndex);
                }
                if (current == a_base) {
                    break;
                }
            }
            std::ranges::reverse(chain);
            if (chain.empty()) {
                return std::nullopt;
            }

            std::size_t weightedVertices = 0;
            for (const auto boneIndex : chain) {
                weightedVertices += skinData->GetBoneDataVerts(boneIndex);
            }
            return ShaftWeights{ std::move(chain), distal->depth, static_cast<float>(weightedVertices) / static_cast<float>(partition->vertexCount) };
        }

        std::string_view GetModelPath(RE::Actor* a_actor, RE::BSGeometry* a_geometry)
        {
            const auto& biped = a_actor->GetBiped();
            if (!biped) {
                return {};
            }
            for (const auto& object : biped->objects) {
                if (!object.part || !object.partClone) {
                    continue;
                }
                for (auto* current = static_cast<RE::NiAVObject*>(a_geometry); current; current = current->parent) {
                    if (current == object.partClone.get()) {
                        const auto* path = object.part->GetModel();
                        return path ? std::string_view(path) : std::string_view{};
                    }
                }
            }
            return {};
        }

        bool ReadCandidateLocalPositions(RE::BSGeometry* a_geometry, RE::NiSkinPartition* a_partition, std::array<std::vector<Candidate>, 2>& a_candidates)
        {
            if (auto* dynamicShape = netimmerse_cast<RE::BSDynamicTriShape*>(a_geometry)) {
                auto& dynamicData = dynamicShape->GetDynamicTrishapeRuntimeData();
                if (dynamicData.dynamicData) {
                    RE::BSSpinLockGuard lock(dynamicData.lock);
                    const auto* vertices = static_cast<const DirectX::XMVECTOR*>(dynamicData.dynamicData);
                    for (auto& candidates : a_candidates) {
                        for (auto& candidate : candidates) {
                            DirectX::XMFLOAT3 position;
                            DirectX::XMStoreFloat3(&position, vertices[candidate.vertex]);
                            candidate.local = { position.x, position.y, position.z };
                        }
                    }
                    return true;
                }
            }

            auto& partition = a_partition->partitions[0];
            if (!partition.vertexDesc.HasFlag(RE::BSGraphics::Vertex::Flags::VF_VERTEX) || !partition.buffData || !partition.buffData->rawVertexData) {
                return false;
            }

            const auto stride = partition.vertexDesc.GetSize();
            const auto* vertexBuffer = reinterpret_cast<const std::uint8_t*>(partition.buffData->rawVertexData);
            for (auto& candidates : a_candidates) {
                for (auto& candidate : candidates) {
                    const auto* vertex = vertexBuffer + candidate.vertex * stride;
                    candidate.local = {
                        *reinterpret_cast<const float*>(vertex),
                        *reinterpret_cast<const float*>(vertex + 4),
                        *reinterpret_cast<const float*>(vertex + 8)
                    };
                }
            }
            return true;
        }

        bool ReadShaftCandidateLocalPositions(RE::BSGeometry* a_geometry, RE::NiSkinPartition* a_partition, std::vector<ShaftCandidate>& a_candidates)
        {
            if (auto* dynamicShape = netimmerse_cast<RE::BSDynamicTriShape*>(a_geometry)) {
                auto& dynamicData = dynamicShape->GetDynamicTrishapeRuntimeData();
                if (dynamicData.dynamicData) {
                    RE::BSSpinLockGuard lock(dynamicData.lock);
                    const auto* vertices = static_cast<const DirectX::XMVECTOR*>(dynamicData.dynamicData);
                    for (auto& candidate : a_candidates) {
                        DirectX::XMFLOAT3 position;
                        DirectX::XMStoreFloat3(&position, vertices[candidate.vertex]);
                        candidate.local = { position.x, position.y, position.z };
                    }
                    return true;
                }
            }

            auto& partition = a_partition->partitions[0];
            if (!partition.vertexDesc.HasFlag(RE::BSGraphics::Vertex::Flags::VF_VERTEX) || !partition.buffData || !partition.buffData->rawVertexData) {
                return false;
            }
            const auto stride = partition.vertexDesc.GetSize();
            const auto* vertexBuffer = reinterpret_cast<const std::uint8_t*>(partition.buffData->rawVertexData);
            for (auto& candidate : a_candidates) {
                const auto* vertex = vertexBuffer + candidate.vertex * stride;
                candidate.local = {
                    *reinterpret_cast<const float*>(vertex),
                    *reinterpret_cast<const float*>(vertex + 4),
                    *reinterpret_cast<const float*>(vertex + 8)
                };
            }
            return true;
        }

        std::vector<CachedSample> SelectShaftRingSamples(const std::vector<ShaftCandidate>& a_candidates, const RE::NiPoint3& a_center, RE::NiPoint3 a_tangent)
        {
            if (a_candidates.empty() || a_tangent.SqrLength() <= FLT_EPSILON) {
                return {};
            }
            a_tangent.Unitize();
            std::vector<const ShaftCandidate*> pool;
            pool.reserve(a_candidates.size());
            for (const auto& candidate : a_candidates) {
                pool.push_back(std::addressof(candidate));
            }
            std::ranges::sort(pool, [&](const ShaftCandidate* a_lhs, const ShaftCandidate* a_rhs) {
                const auto score = [&](const ShaftCandidate* a_candidate) {
                    return std::abs((a_candidate->world - a_center).Dot(a_tangent)) / std::max(a_candidate->chainWeight, 0.05f);
                };
                return score(a_lhs) < score(a_rhs);
            });
            if (pool.size() > MAX_SHAFT_RING_CANDIDATES) {
                pool.resize(MAX_SHAFT_RING_CANDIDATES);
            }

            // Start near the bone plane, then spread samples around the circumference.
            std::vector<const ShaftCandidate*> selected{ pool.front() };
            while (selected.size() < std::min(SHAFT_RING_SAMPLES, pool.size())) {
                const ShaftCandidate* best = nullptr;
                float bestScore = -1.0f;
                for (const auto* candidate : pool) {
                    if (std::ranges::contains(selected, candidate)) {
                        continue;
                    }
                    const auto candidateOffset = candidate->world - a_center;
                    const auto candidateRadial = candidateOffset - a_tangent * candidateOffset.Dot(a_tangent);
                    float distanceSq = std::numeric_limits<float>::max();
                    for (const auto* sample : selected) {
                        const auto sampleOffset = sample->world - a_center;
                        const auto sampleRadial = sampleOffset - a_tangent * sampleOffset.Dot(a_tangent);
                        distanceSq = std::min(distanceSq, (candidateRadial - sampleRadial).SqrLength());
                    }
                    const auto score = distanceSq * candidate->chainWeight;
                    if (score > bestScore) {
                        best = candidate;
                        bestScore = score;
                    }
                }
                if (!best) {
                    break;
                }
                selected.push_back(best);
            }

            std::vector<CachedSample> result;
            result.reserve(selected.size());
            for (const auto* sample : selected) {
                result.push_back({ sample->vertex, sample->influences });
            }
            return result;
        }

        std::vector<CachedSample> SelectShaftTipSamples(const std::vector<ShaftCandidate>& a_candidates, const RE::NiPoint3& a_center, RE::NiPoint3 a_tangent)
        {
            if (a_candidates.empty() || a_tangent.SqrLength() <= FLT_EPSILON) {
                return {};
            }
            a_tangent.Unitize();
            std::vector<const ShaftCandidate*> sorted;
            sorted.reserve(a_candidates.size());
            for (const auto& candidate : a_candidates) {
                sorted.push_back(std::addressof(candidate));
            }
            std::ranges::sort(sorted, [&](const ShaftCandidate* a_lhs, const ShaftCandidate* a_rhs) {
                return (a_lhs->world - a_center).Dot(a_tangent) > (a_rhs->world - a_center).Dot(a_tangent);
            });
            std::vector<CachedSample> result;
            result.reserve(std::min(SHAFT_TIP_SAMPLES, sorted.size()));
            for (std::size_t i = 0; i < std::min(SHAFT_TIP_SAMPLES, sorted.size()); ++i) {
                result.push_back({ sorted[i]->vertex, sorted[i]->influences });
            }
            return result;
        }

        std::vector<std::uint16_t> SelectSamples(const std::vector<Candidate>& a_candidates)
        {
            std::vector<const Candidate*> selected;
            selected.reserve(std::min(SAMPLES_PER_LANDMARK, a_candidates.size()));
            selected.push_back(std::addressof(a_candidates.front()));

            while (selected.size() < std::min(SAMPLES_PER_LANDMARK, a_candidates.size())) {
                const Candidate* best = nullptr;
                float bestScore = -1.0f;
                for (const auto& candidate : a_candidates) {
                    if (std::ranges::any_of(selected, [&](const Candidate* a_selected) { return a_selected->vertex == candidate.vertex; })) {
                        continue;
                    }

                    float distanceSq = std::numeric_limits<float>::max();
                    for (const auto* sample : selected) {
                        distanceSq = std::min(distanceSq, (candidate.local - sample->local).SqrLength());
                    }
                    const auto score = distanceSq * candidate.weight;
                    if (score > bestScore) {
                        best = std::addressof(candidate);
                        bestScore = score;
                    }
                }
                if (!best) {
                    break;
                }
                selected.push_back(best);
            }
            return selected | std::views::transform([](const Candidate* a_candidate) { return a_candidate->vertex; }) | std::ranges::to<std::vector>();
        }

        bool MatchesOpeningTopology(const OpeningTopology& a_topology, RE::NiSkinInstance* a_skin, RE::NiSkinData* a_skinData, RE::NiSkinPartition* a_partition, const std::array<RE::NiAVObject*, 2>& a_targets)
        {
            const auto boneCount = std::min(a_skin->numMatrices, a_skinData->GetBoneCount());
            auto& partition = a_partition->partitions[0];
            if (a_topology.vertexCount != a_partition->vertexCount || a_topology.boneCount != boneCount || a_topology.stride != partition.vertexDesc.GetSize() ||
                a_topology.vertexFlags != static_cast<std::uint16_t>(partition.vertexDesc.GetFlags())) {
                return false;
            }
            for (std::size_t landmark = 0; landmark < a_targets.size(); ++landmark) {
                const auto boneIndex = a_topology.targetBones[landmark];
                if (a_topology.landmarks[landmark].empty() || boneIndex >= boneCount || a_skin->bones[boneIndex] != a_targets[landmark] ||
                    a_topology.targetVertexCounts[landmark] != a_skinData->GetBoneDataVerts(boneIndex)) {
                    return false;
                }
                for (const auto& sample : a_topology.landmarks[landmark]) {
                    if (sample.vertex >= a_partition->vertexCount || sample.influences.empty() ||
                        std::ranges::any_of(sample.influences, [&](const CachedInfluence& a_influence) { return a_influence.skinIndex >= boneCount; })) {
                        return false;
                    }
                }
            }
            return true;
        }

        bool MatchesShaftTopology(const ShaftTopology& a_topology, RE::NiSkinInstance* a_skin, RE::NiSkinData* a_skinData, RE::NiSkinPartition* a_partition, RE::NiAVObject* a_base)
        {
            const auto boneCount = std::min(a_skin->numMatrices, a_skinData->GetBoneCount());
            auto& partition = a_partition->partitions[0];
            if (!a_base || a_topology.rings.size() < 2 || a_topology.tip.empty() || a_topology.chainBones.size() != a_topology.chainVertexCounts.size() ||
                a_topology.vertexCount != a_partition->vertexCount || a_topology.boneCount != boneCount || a_topology.stride != partition.vertexDesc.GetSize() ||
                a_topology.vertexFlags != static_cast<std::uint16_t>(partition.vertexDesc.GetFlags())) {
                return false;
            }
            for (std::size_t i = 0; i < a_topology.chainBones.size(); ++i) {
                const auto boneIndex = a_topology.chainBones[i];
                if (boneIndex >= boneCount || !a_skin->bones[boneIndex] || a_topology.chainVertexCounts[i] != a_skinData->GetBoneDataVerts(boneIndex)) {
                    return false;
                }
                auto* current = a_skin->bones[boneIndex];
                while (current && current != a_base) {
                    current = current->parent;
                }
                if (current != a_base) {
                    return false;
                }
            }
            const auto validSamples = [&](const auto& a_samples) {
                return !a_samples.empty() && std::ranges::all_of(a_samples, [&](const CachedSample& a_sample) {
                    return a_sample.vertex < a_partition->vertexCount && !a_sample.influences.empty() &&
                           std::ranges::all_of(a_sample.influences, [&](const CachedInfluence& a_influence) { return a_influence.skinIndex < boneCount; });
                });
            };
            return std::ranges::all_of(a_topology.rings, [&](const ShaftRingTopology& a_ring) { return validSamples(a_ring.samples); }) && validSamples(a_topology.tip);
        }

        std::optional<OpeningShape> MakeNodeOpening(const GeometryMath::Segment& a_segment, const RE::NiPoint3& a_left, const RE::NiPoint3& a_right)
        {
            auto axis = a_segment.Vector();
            if (axis.SqrLength() <= FLT_EPSILON) {
                return std::nullopt;
            }
            axis.Unitize();

            auto right = a_right - a_left;
            right -= axis * right.Dot(axis);
            const auto diameter = right.Length();
            if (diameter <= FLT_EPSILON) {
                return std::nullopt;
            }
            right /= diameter;

            auto up = right.Cross(axis);
            if (up.SqrLength() <= FLT_EPSILON) {
                return std::nullopt;
            }
            up.Unitize();
            return OpeningShape{ a_segment.start, a_segment.end, axis, right, up, diameter * 0.5f };
        }
    }

    bool TrackedOpening::Bind(RE::BSGeometry* a_geometry, std::string_view a_modelPath, std::string_view a_shapeName, const std::array<RE::NiAVObject*, 2>& a_targets, RE::NiAVObject* a_deep)
    {
        using namespace Detail;
        if (!a_deep || !ShouldUseGeometry(a_geometry)) {
            return false;
        }

        auto* skin = a_geometry->GetGeometryRuntimeData().skinInstance.get();
        auto* skinData = skin ? skin->skinData.get() : nullptr;
        auto* skinPartition = skin ? skin->skinPartition.get() : nullptr;
        if (!skinData || !skinPartition || !skin->bones || skinPartition->numPartitions == 0 || skinPartition->vertexCount == 0) {
            return false;
        }

        const auto boneCount = std::min(skin->numMatrices, skinData->GetBoneCount());
        const auto geometryName = std::string_view(a_geometry->name.c_str());
        const auto assetKey = MakeAssetTopologyKey(a_modelPath, geometryName, a_shapeName);
        OpeningTopology topology{};
        bool cacheHit = false;
        if (!assetKey.empty()) {
            if (const auto cached = openingAssetTopologyCache.find(assetKey); cached != openingAssetTopologyCache.end()) {
                for (const auto& variant : cached->second) {
                    if (MatchesOpeningTopology(variant, skin, skinData, skinPartition, a_targets)) {
                        topology = variant;
                        cacheHit = true;
                        break;
                    }
                }
            }
        }

        if (!cacheHit) {
            const auto openingWeights = GetOpeningWeights(a_geometry, a_targets);
            if (!openingWeights) {
                return false;
            }

            std::array<std::vector<Candidate>, 2> candidates;
            std::uint64_t fingerprint = 0xCBF29CE484222325ULL;
            HashValue(fingerprint, skinPartition->vertexCount);
            HashValue(fingerprint, skinPartition->numPartitions);
            HashValue(fingerprint, skinPartition->partitions[0].vertexDesc.GetSize());
            HashValue(fingerprint, static_cast<std::uint16_t>(skinPartition->partitions[0].vertexDesc.GetFlags()));
            for (const auto value : { geometryName, a_modelPath, a_shapeName }) {
                HashValue(fingerprint, value.size());
                for (const auto c : value) {
                    HashValue(fingerprint, static_cast<std::uint8_t>(c));
                }
            }

            for (std::size_t landmark = 0; landmark < a_targets.size(); ++landmark) {
                const auto boneIndex = openingWeights->bones[landmark];
                const auto* weightedVertices = skinData->GetBoneDataBoneVertData(boneIndex);
                const auto weightedVertexCount = skinData->GetBoneDataVerts(boneIndex);
                if (!weightedVertices || weightedVertexCount == 0) {
                    return false;
                }

                for (std::uint16_t i = 0; i < weightedVertexCount; ++i) {
                    const auto& vertex = weightedVertices[i];
                    if (vertex.vert < skinPartition->vertexCount && vertex.weight > FLT_EPSILON) {
                        candidates[landmark].push_back({ vertex.vert, vertex.weight, {} });
                    }
                }
                if (candidates[landmark].empty()) {
                    return false;
                }
                const auto keep = std::min(candidates[landmark].size(), MAX_CANDIDATES_PER_LANDMARK);
                std::partial_sort(candidates[landmark].begin(), candidates[landmark].begin() + keep, candidates[landmark].end(),
                    [](const Candidate& a_lhs, const Candidate& a_rhs) { return a_lhs.weight > a_rhs.weight; });
                candidates[landmark].resize(keep);

                HashValue(fingerprint, boneIndex);
                for (const auto& vertex : candidates[landmark]) {
                    HashValue(fingerprint, vertex.vertex);
                    HashValue(fingerprint, std::bit_cast<std::uint32_t>(vertex.weight));
                }
            }

            if (const auto cached = openingFingerprintTopologyCache.find(fingerprint);
                cached != openingFingerprintTopologyCache.end() && MatchesOpeningTopology(cached->second, skin, skinData, skinPartition, a_targets)) {
                topology = cached->second;
                cacheHit = true;
            }

            if (!cacheHit) {
                if (!ReadCandidateLocalPositions(a_geometry, skinPartition, candidates)) {
                    return false;
                }
                topology.vertexCount = skinPartition->vertexCount;
                topology.boneCount = boneCount;
                topology.stride = skinPartition->partitions[0].vertexDesc.GetSize();
                topology.vertexFlags = static_cast<std::uint16_t>(skinPartition->partitions[0].vertexDesc.GetFlags());
                topology.targetBones = openingWeights->bones;
                topology.targetVertexCounts = openingWeights->counts;
                topology.fingerprint = fingerprint;
                for (std::size_t landmark = 0; landmark < candidates.size(); ++landmark) {
                    for (const auto vertex : SelectSamples(candidates[landmark])) {
                        topology.landmarks[landmark].push_back({ vertex, {} });
                    }
                }

                for (std::uint32_t boneIndex = 0; boneIndex < boneCount; ++boneIndex) {
                    const auto* weightedVertices = skinData->GetBoneDataBoneVertData(boneIndex);
                    if (!weightedVertices) {
                        continue;
                    }
                    for (std::uint16_t i = 0; i < skinData->GetBoneDataVerts(boneIndex); ++i) {
                        for (auto& landmark : topology.landmarks) {
                            for (auto& sample : landmark) {
                                if (sample.vertex == weightedVertices[i].vert) {
                                    sample.influences.push_back({ static_cast<std::uint16_t>(boneIndex), weightedVertices[i].weight });
                                }
                            }
                        }
                    }
                }

                if (!MatchesOpeningTopology(topology, skin, skinData, skinPartition, a_targets)) {
                    return false;
                }
                openingFingerprintTopologyCache.try_emplace(fingerprint, topology);
            }

            if (!assetKey.empty()) {
                auto& variants = openingAssetTopologyCache[assetKey];
                if (std::ranges::none_of(variants, [&](const OpeningTopology& a_variant) { return a_variant.fingerprint == topology.fingerprint; })) {
                    variants.push_back(topology);
                }
            }
        }

        landmarks = {};
        bones.clear();
        for (std::size_t landmark = 0; landmark < landmarks.size(); ++landmark) {
            landmarks[landmark].samples.reserve(topology.landmarks[landmark].size());
            for (const auto& cachedSample : topology.landmarks[landmark]) {
                Sample sample{ cachedSample.vertex, {}, {} };
                sample.influences.reserve(cachedSample.influences.size());
                for (const auto& influence : cachedSample.influences) {
                    auto bone = std::ranges::find(bones, influence.skinIndex, &Bone::skinIndex);
                    if (bone == bones.end()) {
                        bones.push_back({ influence.skinIndex, {} });
                        bone = std::prev(bones.end());
                    }
                    sample.influences.push_back({ static_cast<std::uint16_t>(std::distance(bones.begin(), bone)), influence.weight });
                }
                landmarks[landmark].samples.push_back(std::move(sample));
            }
        }

        const auto setLocalPositions = [&](const auto& a_getPosition) {
            for (auto& landmark : landmarks) {
                for (auto& sample : landmark.samples) {
                    sample.local = a_getPosition(sample.vertex);
                }
            }
        };
        if (auto* dynamicShape = netimmerse_cast<RE::BSDynamicTriShape*>(a_geometry); dynamicShape && dynamicShape->GetDynamicTrishapeRuntimeData().dynamicData) {
            auto& dynamicData = dynamicShape->GetDynamicTrishapeRuntimeData();
            RE::BSSpinLockGuard lock(dynamicData.lock);
            const auto* vertices = static_cast<const DirectX::XMVECTOR*>(dynamicData.dynamicData);
            setLocalPositions([&](std::uint16_t a_vertex) {
                DirectX::XMFLOAT3 position;
                DirectX::XMStoreFloat3(&position, vertices[a_vertex]);
                return RE::NiPoint3{ position.x, position.y, position.z };
            });
        } else {
            auto& partition = skinPartition->partitions[0];
            if (!partition.vertexDesc.HasFlag(RE::BSGraphics::Vertex::Flags::VF_VERTEX) || !partition.buffData || !partition.buffData->rawVertexData) {
                return false;
            }
            const auto stride = partition.vertexDesc.GetSize();
            const auto* vertexBuffer = reinterpret_cast<const std::uint8_t*>(partition.buffData->rawVertexData);
            setLocalPositions([&](std::uint16_t a_vertex) {
                const auto* vertex = vertexBuffer + a_vertex * stride;
                return RE::NiPoint3{
                    *reinterpret_cast<const float*>(vertex),
                    *reinterpret_cast<const float*>(vertex + 4),
                    *reinterpret_cast<const float*>(vertex + 8)
                };
            });
        }

        if (landmarks[0].samples.empty() || landmarks[1].samples.empty()) {
            return false;
        }

        geometry.reset(a_geometry);
        skinInstance = skin;
        deep.reset(a_deep);
        logger::info("NiSurface Interaction: {} surface topology {} for '{}' in '{}' ({:016X})", a_shapeName, cacheHit ? "cache hit" : "cached", a_geometry->name, a_modelPath, topology.fingerprint);
        return true;
    }

    std::optional<OpeningShape> TrackedOpening::Update()
    {
        if (!geometry || !geometry->parent || !deep || geometry->GetGeometryRuntimeData().skinInstance.get() != skinInstance) {
            return std::nullopt;
        }

        auto* skinData = skinInstance->skinData.get();
        auto* skinPartition = skinInstance->skinPartition.get();
        if (!skinData || !skinPartition || !skinInstance->bones) {
            return std::nullopt;
        }

        if (auto* dynamicShape = netimmerse_cast<RE::BSDynamicTriShape*>(geometry.get())) {
            auto& dynamicData = dynamicShape->GetDynamicTrishapeRuntimeData();
            if (dynamicData.dynamicData) {
                RE::BSSpinLockGuard lock(dynamicData.lock);
                const auto* vertices = static_cast<const DirectX::XMVECTOR*>(dynamicData.dynamicData);
                for (auto& landmark : landmarks) {
                    for (auto& sample : landmark.samples) {
                        DirectX::XMFLOAT3 position;
                        DirectX::XMStoreFloat3(&position, vertices[sample.vertex]);
                        sample.local = { position.x, position.y, position.z };
                    }
                }
            }
        }

        for (auto& bone : bones) {
            if (bone.skinIndex >= skinInstance->numMatrices || !skinInstance->bones[bone.skinIndex]) {
                return std::nullopt;
            }
            bone.transform = skinInstance->bones[bone.skinIndex]->world * skinData->GetBoneDataSkinToBone(bone.skinIndex);
        }

        std::array<RE::NiPoint3, 2> points;
        for (std::size_t landmarkIndex = 0; landmarkIndex < landmarks.size(); ++landmarkIndex) {
            for (const auto& sample : landmarks[landmarkIndex].samples) {
                RE::NiPoint3 position{};
                float totalWeight = 0.0f;
                for (const auto& influence : sample.influences) {
                    position += (bones[influence.bone].transform * sample.local) * influence.weight;
                    totalWeight += influence.weight;
                }
                if (totalWeight > FLT_EPSILON) {
                    points[landmarkIndex] += position / totalWeight;
                }
            }
            points[landmarkIndex] /= static_cast<float>(landmarks[landmarkIndex].samples.size());
        }

        const auto center = (points[0] + points[1]) * 0.5f;
        auto axis = deep->world.translate - center;
        if (axis.SqrLength() <= FLT_EPSILON) {
            return std::nullopt;
        }
        axis.Unitize();

        auto right = points[1] - points[0];
        right -= axis * right.Dot(axis);
        const auto diameter = right.Length();
        if (diameter <= FLT_EPSILON) {
            return std::nullopt;
        }
        right /= diameter;

        auto up = right.Cross(axis);
        if (up.SqrLength() <= FLT_EPSILON) {
            return std::nullopt;
        }
        up.Unitize();
        return OpeningShape{ center, deep->world.translate, axis, right, up, diameter * 0.5f };
    }
}
