#include "Shaft.h"
#include "SurfaceCache.h"

namespace Thread::Interaction::NiSurface::Geometry
{
    using namespace Detail;

    namespace
    {
        constexpr float MIN_SHAFT_LENGTH{ 13.0f };
    }

    Shaft::Shaft(RE::Actor* a_actor, RE::NiPointer<RE::NiNode> a_baseNode, const glm::mat3& a_rotation) :
      ownerActor(a_actor),
      skeletonNodes({ a_baseNode }),
      rotation({ a_rotation[0].x, a_rotation[0].y, a_rotation[0].z }, { a_rotation[1].x, a_rotation[1].y, a_rotation[1].z }, { a_rotation[2].x, a_rotation[2].y, a_rotation[2].z }),
      equipmentSignature(GetBipedSignature(a_actor))
    {
        assert(a_baseNode);
        DiscoverSurface(a_actor);
        if (surface) {
            return;
        }

        // Mesh discovery failed, so preserve the previous skeleton-only chain as the compatibility fallback.
        do {
            auto parent = skeletonNodes.back();
            auto& children = parent->GetChildren();
            switch (children.size()) {
            case 0:
                break;
            case 1:
                {
                    auto& child = children.front();
                    auto* childNode = child ? child->AsNode() : nullptr;
                    if (childNode) {
                        skeletonNodes.emplace_back(childNode);
                    }
                }
                break;
            default:
                {
                    const auto direction = skeletonNodes.size() < 2 ? parent->world.rotate.GetVectorY() : parent->world.translate - a_baseNode->world.translate;
                    if (direction.SqrLength() > FLT_EPSILON) {
                        for (const auto& child : children) {
                            if (!child) {
                                continue;
                            }
                            auto* childNode = child->AsNode();
                            if (!childNode) {
                                continue;
                            }
                            const auto childDirection = childNode->world.translate - a_baseNode->world.translate;
                            if (GeometryMath::GetAngleDegree(direction, childDirection) <= 90.0f) {
                                skeletonNodes.emplace_back(childNode);
                                break;
                            }
                        }
                    } else {
                        auto* user = a_baseNode->GetUserData();
                        logger::error("NiSurface Interaction: Ambiguous skeleton structure for user {:X} at node depth {}", user ? user->GetFormID() : 0, skeletonNodes.size());
                    }
                }
                break;
            }
            if (skeletonNodes.back() == parent) {
                break;
            }
        } while (true);
    }

    void Shaft::DiscoverSurface(RE::Actor* a_actor)
    {
        if (!a_actor || skeletonNodes.empty()) {
            return;
        }
        auto* base = skeletonNodes.front().get();
        const auto baseName = std::string_view(base->name.c_str());
        const auto& biped = a_actor->GetBiped();
        if (biped) {
            for (const auto& object : biped->objects) {
                if (!object.part || !object.partClone) {
                    continue;
                }
                const auto* path = object.part->GetModel();
                const auto modelPath = path ? std::string_view(path) : std::string_view{};
                const auto selectedGeometry = GetShaftSelection(modelPath, baseName);
                if (!selectedGeometry) {
                    continue;
                }
                RE::BSVisit::TraverseScenegraphGeometries(object.partClone.get(), [&](RE::BSGeometry* a_geometry) {
                    if (std::string_view(a_geometry->name.c_str()) == *selectedGeometry && BindSurface(a_geometry, modelPath)) {
                        return RE::BSVisit::BSVisitControl::kStop;
                    }
                    return RE::BSVisit::BSVisitControl::kContinue;
                });
                if (surface) {
                    return;
                }
            }
        }

        RE::NiPointer<RE::BSGeometry> bestGeometry;
        ShaftWeights bestWeights{};
        std::size_t geometryCount = 0;
        std::size_t skinnedGeometryCount = 0;
        std::size_t longestWeightedChain = 0;

        RE::BSVisit::TraverseScenegraphGeometries(a_actor->Get3D(), [&](RE::BSGeometry* a_geometry) {
            ++geometryCount;
            const auto& runtime = a_geometry->GetGeometryRuntimeData();
            if (runtime.skinInstance && runtime.skinInstance->skinData && runtime.skinInstance->skinPartition && runtime.skinInstance->bones) {
                ++skinnedGeometryCount;
            }
            const auto weights = GetShaftWeights(a_geometry, base);
            if (!weights) {
                return RE::BSVisit::BSVisitControl::kContinue;
            }
            longestWeightedChain = std::max(longestWeightedChain, weights->chainBones.size());
            if (weights->chainBones.size() < 2) {
                return RE::BSVisit::BSVisitControl::kContinue;
            }
            bool preferModelPath = false;
            if (bestGeometry && weights->depth == bestWeights.depth && weights->chainBones.size() == bestWeights.chainBones.size() && weights->coverage == bestWeights.coverage) {
                preferModelPath = !GetModelPath(a_actor, a_geometry).empty() && GetModelPath(a_actor, bestGeometry.get()).empty();
            }
            if (!bestGeometry || weights->depth > bestWeights.depth ||
                (weights->depth == bestWeights.depth && weights->chainBones.size() > bestWeights.chainBones.size()) ||
                (weights->depth == bestWeights.depth && weights->chainBones.size() == bestWeights.chainBones.size() && weights->coverage > bestWeights.coverage) || preferModelPath) {
                bestGeometry.reset(a_geometry);
                bestWeights = *weights;
            }
            return RE::BSVisit::BSVisitControl::kContinue;
        });
        if (!bestGeometry) {
            logger::info("NiSurface Interaction: Shaft surface not found for actor {:X}: base='{}', geometries={}, skinned={}, longest weighted descendant chain={}", a_actor->GetFormID(),
                baseName, geometryCount, skinnedGeometryCount, longestWeightedChain);
            return;
        }

        const auto modelPath = GetModelPath(a_actor, bestGeometry.get());
        logger::info("NiSurface Interaction: Shaft surface selected '{}' in '{}': base='{}', skinned chain bones={}", bestGeometry->name, modelPath, baseName, bestWeights.chainBones.size());
        if (BindSurface(bestGeometry.get(), modelPath, std::addressof(bestWeights.chainBones))) {
            CacheShaftSelection(modelPath, baseName, std::string_view(bestGeometry->name.c_str()));
        } else {
            logger::info("NiSurface Interaction: Shaft surface bind failed for '{}' in '{}': base='{}'", bestGeometry->name, modelPath, baseName);
        }
    }

    bool Shaft::BindSurface(RE::BSGeometry* a_geometry, std::string_view a_modelPath, const std::vector<std::uint16_t>* a_knownChain)
    {
        if (skeletonNodes.empty() || !ShouldUseGeometry(a_geometry)) {
            return false;
        }
        auto* skin = a_geometry->GetGeometryRuntimeData().skinInstance.get();
        auto* skinData = skin ? skin->skinData.get() : nullptr;
        auto* skinPartition = skin ? skin->skinPartition.get() : nullptr;
        if (!skinData || !skinPartition || !skin->bones || skinPartition->numPartitions == 0 || skinPartition->vertexCount == 0) {
            return false;
        }

        auto* base = skeletonNodes.front().get();
        const auto baseName = std::string_view(base->name.c_str());
        const auto geometryName = std::string_view(a_geometry->name.c_str());
        auto cacheName = std::string("shaft:");
        cacheName.append(baseName);
        const auto assetKey = MakeAssetTopologyKey(a_modelPath, geometryName, cacheName);
        ShaftTopology topology{};
        bool cacheHit = false;
        if (!assetKey.empty()) {
            // The asset recipe skips geometry scoring and bone-weight scans for later actors using the same NIF.
            if (auto cached = FindShaftAssetTopology(assetKey, skin, skinData, skinPartition, base)) {
                topology = std::move(*cached);
                cacheHit = true;
            }
        }

        if (!cacheHit) {
            std::vector<std::uint16_t> chainBones;
            if (a_knownChain) {
                chainBones = *a_knownChain;
            } else if (const auto weights = GetShaftWeights(a_geometry, base)) {
                chainBones = weights->chainBones;
            }
            if (chainBones.size() < 2) {
                return false;
            }

            const auto boneCount = std::min(skin->numMatrices, skinData->GetBoneCount());
            std::uint64_t fingerprint = 0xCBF29CE484222325ULL;
            HashValue(fingerprint, skinPartition->vertexCount);
            HashValue(fingerprint, skinPartition->numPartitions);
            HashValue(fingerprint, skinPartition->partitions[0].vertexDesc.GetSize());
            HashValue(fingerprint, static_cast<std::uint16_t>(skinPartition->partitions[0].vertexDesc.GetFlags()));
            for (const auto value : { geometryName, a_modelPath, baseName }) {
                HashValue(fingerprint, value.size());
                for (const auto c : value) {
                    HashValue(fingerprint, static_cast<std::uint8_t>(c));
                }
            }

            std::vector<ShaftCandidate> candidates;
            std::vector<std::int32_t> candidateByVertex(skinPartition->vertexCount, -1);
            for (const auto boneIndex : chainBones) {
                if (boneIndex >= boneCount) {
                    return false;
                }
                const auto* weightedVertices = skinData->GetBoneDataBoneVertData(boneIndex);
                const auto weightedVertexCount = skinData->GetBoneDataVerts(boneIndex);
                if (!weightedVertices || weightedVertexCount == 0) {
                    return false;
                }
                HashValue(fingerprint, boneIndex);
                HashValue(fingerprint, weightedVertexCount);
                for (std::uint16_t i = 0; i < weightedVertexCount; ++i) {
                    const auto& vertex = weightedVertices[i];
                    if (vertex.vert >= skinPartition->vertexCount || vertex.weight <= FLT_EPSILON) {
                        continue;
                    }
                    HashValue(fingerprint, vertex.vert);
                    HashValue(fingerprint, std::bit_cast<std::uint32_t>(vertex.weight));
                    auto& slot = candidateByVertex[vertex.vert];
                    if (slot < 0) {
                        slot = static_cast<std::int32_t>(candidates.size());
                        candidates.push_back({ vertex.vert, vertex.weight, {}, {}, {} });
                    } else {
                        candidates[slot].chainWeight += vertex.weight;
                    }
                }
            }
            if (candidates.empty()) {
                return false;
            }

            // Pathless meshes can still reuse an identical structure discovered earlier in this process.
            if (auto cachedTopology = FindShaftFingerprintTopology(fingerprint, skin, skinData, skinPartition, base)) {
                topology = std::move(*cachedTopology);
                cacheHit = true;
            }

            if (!cacheHit) {
                if (!ReadShaftCandidateLocalPositions(a_geometry, skinPartition, candidates)) {
                    return false;
                }
                for (std::uint32_t boneIndex = 0; boneIndex < boneCount; ++boneIndex) {
                    const auto* weightedVertices = skinData->GetBoneDataBoneVertData(boneIndex);
                    if (!weightedVertices) {
                        continue;
                    }
                    for (std::uint16_t i = 0; i < skinData->GetBoneDataVerts(boneIndex); ++i) {
                        const auto vertex = weightedVertices[i].vert;
                        if (vertex < candidateByVertex.size() && candidateByVertex[vertex] >= 0) {
                            candidates[candidateByVertex[vertex]].influences.push_back({ static_cast<std::uint16_t>(boneIndex), weightedVertices[i].weight });
                        }
                    }
                }

                std::vector<RE::NiTransform> transforms(boneCount);
                std::vector<bool> validTransforms(boneCount, false);
                for (std::uint32_t boneIndex = 0; boneIndex < boneCount; ++boneIndex) {
                    if (skin->bones[boneIndex]) {
                        transforms[boneIndex] = skin->bones[boneIndex]->world * skinData->GetBoneDataSkinToBone(boneIndex);
                        validTransforms[boneIndex] = true;
                    }
                }
                for (auto& candidate : candidates) {
                    float totalWeight = 0.0f;
                    for (const auto& influence : candidate.influences) {
                        if (validTransforms[influence.skinIndex]) {
                            candidate.world += (transforms[influence.skinIndex] * candidate.local) * influence.weight;
                            totalWeight += influence.weight;
                        }
                    }
                    if (totalWeight <= FLT_EPSILON) {
                        return false;
                    }
                    candidate.world /= totalWeight;
                }

                topology.vertexCount = skinPartition->vertexCount;
                topology.boneCount = boneCount;
                topology.stride = skinPartition->partitions[0].vertexDesc.GetSize();
                topology.vertexFlags = static_cast<std::uint16_t>(skinPartition->partitions[0].vertexDesc.GetFlags());
                topology.chainBones = chainBones;
                topology.fingerprint = fingerprint;
                topology.chainVertexCounts.reserve(chainBones.size());
                for (const auto boneIndex : chainBones) {
                    topology.chainVertexCounts.push_back(skinData->GetBoneDataVerts(boneIndex));
                }

                // Skip a multi-bone chain's root because it commonly sits inside the pelvis or also weights the scrotum.
                const std::size_t firstChainIndex = chainBones.size() > 2 ? 1 : 0;
                const auto sectionCount = std::min(SHAFT_SECTION_COUNT, chainBones.size() - firstChainIndex);
                topology.rings.reserve(sectionCount);
                for (std::size_t section = 0; section < sectionCount; ++section) {
                    const auto chainIndex = firstChainIndex + section * (chainBones.size() - firstChainIndex - 1) / (sectionCount - 1);
                    const auto previous = chainIndex == 0 ? chainIndex : chainIndex - 1;
                    const auto next = chainIndex + 1 < chainBones.size() ? chainIndex + 1 : chainIndex;
                    const auto center = skin->bones[chainBones[chainIndex]]->world.translate;
                    const auto tangent = skin->bones[chainBones[next]]->world.translate - skin->bones[chainBones[previous]]->world.translate;
                    topology.rings.push_back({ SelectShaftRingSamples(candidates, center, tangent) });
                }
                const auto last = chainBones.size() - 1;
                const auto tipCenter = skin->bones[chainBones[last]]->world.translate;
                const auto tipTangent = tipCenter - skin->bones[chainBones[last - 1]]->world.translate;
                topology.tip = SelectShaftTipSamples(candidates, tipCenter, tipTangent);

                if (!MatchesShaftTopology(topology, skin, skinData, skinPartition, base)) {
                    return false;
                }
            }
            CacheShaftTopology(assetKey, topology);
        }

        SkinnedSurface result;
        result.geometry.reset(a_geometry);
        result.skinInstance = skin;
        const auto makeSample = [&](const CachedSample& a_cachedSample) {
            Sample sample{ a_cachedSample.vertex, {}, {} };
            sample.influences.reserve(a_cachedSample.influences.size());
            for (const auto& influence : a_cachedSample.influences) {
                auto bone = std::ranges::find(result.bones, influence.skinIndex, &Bone::skinIndex);
                std::uint16_t slot;
                if (bone == result.bones.end()) {
                    slot = static_cast<std::uint16_t>(result.bones.size());
                    result.bones.push_back({ influence.skinIndex, {} });
                } else {
                    slot = static_cast<std::uint16_t>(std::distance(result.bones.begin(), bone));
                }
                sample.influences.push_back({ slot, influence.weight });
            }
            return sample;
        };
        result.rings.reserve(topology.rings.size());
        for (const auto& cachedRing : topology.rings) {
            SampleRing ring;
            ring.samples.reserve(cachedRing.samples.size());
            for (const auto& cachedSample : cachedRing.samples) {
                ring.samples.push_back(makeSample(cachedSample));
            }
            result.rings.push_back(std::move(ring));
        }
        result.tip.samples.reserve(topology.tip.size());
        for (const auto& cachedSample : topology.tip) {
            result.tip.samples.push_back(makeSample(cachedSample));
        }

        const auto setLocalPositions = [&](const auto& a_getPosition) {
            for (auto& ring : result.rings) {
                for (auto& sample : ring.samples) {
                    sample.local = a_getPosition(sample.vertex);
                }
            }
            for (auto& sample : result.tip.samples) {
                sample.local = a_getPosition(sample.vertex);
            }
        };
        // Dynamic shapes can change local vertex data at runtime, so refresh only the cached samples.
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

        std::vector<RE::NiPointer<RE::NiNode>> skinnedNodes{ skeletonNodes.front() };
        for (const auto boneIndex : topology.chainBones) {
            auto* node = skin->bones[boneIndex] ? skin->bones[boneIndex]->AsNode() : nullptr;
            if (node && node != skinnedNodes.back().get()) {
                skinnedNodes.emplace_back(node);
            }
        }
        if (skinnedNodes.size() > 1) {
            skeletonNodes = std::move(skinnedNodes);
        }
        surface = std::move(result);
        logger::info("NiSurface Interaction: Shaft surface topology {} for '{}' in '{}' ({:016X}, {} rings)", cacheHit ? "cache hit" : "cached", a_geometry->name, a_modelPath,
            topology.fingerprint, topology.rings.size());
        return true;
    }

    void Shaft::UpdateCollisionShape()
    {
        if (surface && (!surface->geometry || !surface->geometry->parent || surface->geometry->GetGeometryRuntimeData().skinInstance.get() != surface->skinInstance)) {
            // Re-arm discovery when an equipped shaft is replaced or removed during a scene.
            surface.reset();
            collisionShape.reset();
            equipmentSignature = GetBipedSignature(ownerActor);
            stableEquipmentFrames = 0;
            surfaceSearchPending = true;
        }

        if (!surface) {
            const auto currentSignature = GetBipedSignature(ownerActor);
            if (currentSignature != equipmentSignature) {
                equipmentSignature = currentSignature;
                stableEquipmentFrames = 0;
                surfaceSearchPending = true;
            }

            // SOS can equip its mesh after scene setup. Wait for the biped state to settle, then scan exactly once.
            if (surfaceSearchPending && ++stableEquipmentFrames >= SHAFT_EQUIPMENT_STABLE_FRAMES) {
                surfaceSearchPending = false;
                stableEquipmentFrames = 0;
                const auto start = std::chrono::high_resolution_clock::now();
                DiscoverSurface(ownerActor);
                const auto elapsed = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - start);
                logger::info("NiSurface Interaction: Shaft delayed surface initialization: {:.2f}ms ({})", elapsed.count(), surface ? "bound" : "not found");
            }
            if (!surface) {
                collisionShape.reset();
                return;
            }
        }
        auto* skinData = surface->skinInstance->skinData.get();
        if (!skinData || !surface->skinInstance->bones) {
            collisionShape.reset();
            return;
        }

        if (auto* dynamicShape = netimmerse_cast<RE::BSDynamicTriShape*>(surface->geometry.get())) {
            auto& dynamicData = dynamicShape->GetDynamicTrishapeRuntimeData();
            if (dynamicData.dynamicData) {
                RE::BSSpinLockGuard lock(dynamicData.lock);
                const auto* vertices = static_cast<const DirectX::XMVECTOR*>(dynamicData.dynamicData);
                const auto updateSamples = [&](SampleRing& a_ring) {
                    for (auto& sample : a_ring.samples) {
                        DirectX::XMFLOAT3 position;
                        DirectX::XMStoreFloat3(&position, vertices[sample.vertex]);
                        sample.local = { position.x, position.y, position.z };
                    }
                };
                for (auto& ring : surface->rings) {
                    updateSamples(ring);
                }
                updateSamples(surface->tip);
            }
        }

        for (auto& bone : surface->bones) {
            if (bone.skinIndex >= surface->skinInstance->numMatrices || !surface->skinInstance->bones[bone.skinIndex]) {
                collisionShape.reset();
                return;
            }
            bone.transform = surface->skinInstance->bones[bone.skinIndex]->world * skinData->GetBoneDataSkinToBone(bone.skinIndex);
        }
        const auto skinSamples = [&](SampleRing& a_ring) {
            a_ring.worldPositions.resize(a_ring.samples.size());
            for (std::size_t i = 0; i < a_ring.samples.size(); ++i) {
                const auto& sample = a_ring.samples[i];
                RE::NiPoint3 position{};
                float totalWeight = 0.0f;
                for (const auto& influence : sample.influences) {
                    position += (surface->bones[influence.bone].transform * sample.local) * influence.weight;
                    totalWeight += influence.weight;
                }
                if (totalWeight <= FLT_EPSILON) {
                    return false;
                }
                a_ring.worldPositions[i] = position / totalWeight;
            }
            return true;
        };

        if (!collisionShape) {
            collisionShape.emplace();
        }
        auto& shape = *collisionShape;
        shape.sections.clear();
        shape.tip = {};
        shape.sections.reserve(surface->rings.size());
        for (auto& ring : surface->rings) {
            if (!skinSamples(ring) || ring.worldPositions.empty()) {
                collisionShape.reset();
                return;
            }
            RE::NiPoint3 center{};
            for (const auto& point : ring.worldPositions) {
                center += point;
            }
            center /= static_cast<float>(ring.worldPositions.size());
            shape.sections.push_back({ center, 0.0f });
        }
        if (shape.sections.size() < 2) {
            collisionShape.reset();
            return;
        }

        // Radius is measured perpendicular to the local centerline, so bent poses do not inflate it.
        for (std::size_t i = 0; i < shape.sections.size(); ++i) {
            const auto previous = i == 0 ? i : i - 1;
            const auto next = i + 1 < shape.sections.size() ? i + 1 : i;
            auto tangent = shape.sections[next].center - shape.sections[previous].center;
            if (tangent.SqrLength() <= FLT_EPSILON) {
                collisionShape.reset();
                return;
            }
            tangent.Unitize();
            for (const auto& point : surface->rings[i].worldPositions) {
                const auto offset = point - shape.sections[i].center;
                shape.sections[i].radius += (offset - tangent * offset.Dot(tangent)).Length();
            }
            shape.sections[i].radius /= static_cast<float>(surface->rings[i].worldPositions.size());
        }

        if (!skinSamples(surface->tip) || surface->tip.worldPositions.empty()) {
            collisionShape.reset();
            return;
        }
        for (const auto& point : surface->tip.worldPositions) {
            shape.tip += point;
        }
        shape.tip /= static_cast<float>(surface->tip.worldPositions.size());
    }

    GeometryMath::Segment Shaft::GetReferenceSegment() const
    {
        if (collisionShape && collisionShape->sections.size() >= 2) {
            return { collisionShape->sections.front().center, collisionShape->tip };
        }
        switch (skeletonNodes.size()) {
        case 0:
            assert(false);
            throw std::invalid_argument("Shaft Data without any Nodes?");
        case 1:
            {
                auto translate = rotation * skeletonNodes.front()->world.rotate;
                auto vforward = translate.GetVectorY() * MIN_SHAFT_LENGTH;
                vforward.Unitize();
                auto s1 = skeletonNodes.front()->world.translate;
                auto s2 = (vforward * MIN_SHAFT_LENGTH) + s1;
                return GeometryMath::Segment(s1, s2);
            }
        default:
            {
                std::vector<Eigen::Vector3f> argV{};
                argV.reserve(skeletonNodes.size());
                for (auto&& node : skeletonNodes) {
                    if (!node)
                        continue;
                    auto argT = GeometryMath::ToEigen(node->world.translate);
                    argV.push_back(argT);
                }
                return GeometryMath::LeastSquares(argV, MIN_SHAFT_LENGTH);
            }
        }
    }

    RE::NiPointer<RE::NiNode> Shaft::GetBaseReferenceNode() const
    {
        switch (skeletonNodes.size()) {
        case 0:
            assert(false);
            throw std::invalid_argument("Shaft Data without any Nodes?");
        default:
            return skeletonNodes.front();
        }
    }
}
