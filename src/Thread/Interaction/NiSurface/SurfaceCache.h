#pragma once

#include "CollisionShapes.h"
#include "GeometryMath.h"

namespace Thread::Interaction::NiSurface::Geometry
{
    struct TrackedOpening
    {
        bool Bind(RE::BSGeometry* a_geometry, std::string_view a_modelPath, std::string_view a_shapeName, const std::array<RE::NiAVObject*, 2>& a_targets, RE::NiAVObject* a_deep);
        std::optional<OpeningShape> Update();

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

        struct Landmark
        {
            std::vector<Sample> samples;
        };

        struct Bone
        {
            std::uint16_t skinIndex;
            RE::NiTransform transform;
        };

        RE::NiPointer<RE::BSGeometry> geometry;
        // Careful with this borrowed pointer; validate the geometry attachment and current skin instance before trusting it.
        RE::NiSkinInstance* skinInstance{ nullptr };
        RE::NiPointer<RE::NiAVObject> deep;
        std::array<Landmark, 2> landmarks;
        std::vector<Bone> bones;
    };
}

namespace Thread::Interaction::NiSurface::Geometry::Detail
{
    inline constexpr std::size_t SAMPLES_PER_LANDMARK{ 4 };
    inline constexpr std::size_t MAX_CANDIDATES_PER_LANDMARK{ 32 };
    inline constexpr std::size_t SHAFT_SECTION_COUNT{ 4 };
    inline constexpr std::size_t SHAFT_RING_SAMPLES{ 6 };
    inline constexpr std::size_t SHAFT_TIP_SAMPLES{ 4 };
    inline constexpr std::size_t MAX_SHAFT_RING_CANDIDATES{ 32 };
    inline constexpr std::uint8_t SHAFT_EQUIPMENT_STABLE_FRAMES{ 2 };

    struct Candidate
    {
        std::uint16_t vertex;
        float weight;
        RE::NiPoint3 local;
    };

    struct CachedInfluence
    {
        std::uint16_t skinIndex;
        float weight;
    };

    struct CachedSample
    {
        std::uint16_t vertex;
        std::vector<CachedInfluence> influences;
    };

    struct OpeningTopology
    {
        std::array<std::vector<CachedSample>, 2> landmarks;
        std::uint32_t vertexCount;
        std::uint32_t boneCount;
        std::uint32_t stride;
        std::uint16_t vertexFlags;
        std::array<std::uint16_t, 2> targetBones;
        std::array<std::uint16_t, 2> targetVertexCounts;
        std::uint64_t fingerprint;
    };

    struct ShaftRingTopology
    {
        std::vector<CachedSample> samples;
    };

    struct ShaftTopology
    {
        std::vector<ShaftRingTopology> rings;
        std::vector<CachedSample> tip;
        std::vector<std::uint16_t> chainBones;
        std::vector<std::uint16_t> chainVertexCounts;
        std::uint32_t vertexCount;
        std::uint32_t boneCount;
        std::uint32_t stride;
        std::uint16_t vertexFlags;
        std::uint64_t fingerprint;
    };

    struct ShaftWeights
    {
        std::vector<std::uint16_t> chainBones;
        std::size_t depth;
        float coverage;
    };

    struct ShaftCandidate
    {
        std::uint16_t vertex;
        float chainWeight;
        RE::NiPoint3 local;
        RE::NiPoint3 world;
        std::vector<CachedInfluence> influences;
    };

    struct OpeningWeights
    {
        std::array<std::uint16_t, 2> bones;
        std::array<std::uint16_t, 2> counts;
        std::array<float, 2> maximum;
    };

    struct OpeningSelection
    {
        std::string vaginal;
        std::string anal;
    };

    void HashValue(std::uint64_t& a_hash, std::uint64_t a_value);
    std::uint64_t GetBipedSignature(RE::Actor* a_actor);
    std::string NormalizeAssetPath(std::string_view a_path);
    std::string MakeAssetTopologyKey(std::string_view a_modelPath, std::string_view a_geometryName, std::string_view a_shapeName);
    std::string MakeShaftSelectionKey(std::string_view a_modelPath, std::string_view a_baseName);
    std::optional<OpeningSelection> GetOpeningSelection(std::string_view a_modelPath);
    void CacheOpeningSelection(std::string_view a_modelPath, std::string_view a_shapeName, std::string_view a_geometryName);
    std::optional<std::string> GetShaftSelection(std::string_view a_modelPath, std::string_view a_baseName);
    void CacheShaftSelection(std::string_view a_modelPath, std::string_view a_baseName, std::string_view a_geometryName);
    std::optional<ShaftTopology> FindShaftAssetTopology(std::string_view a_key, RE::NiSkinInstance* a_skin, RE::NiSkinData* a_skinData, RE::NiSkinPartition* a_partition, RE::NiAVObject* a_base);
    std::optional<ShaftTopology> FindShaftFingerprintTopology(std::uint64_t a_fingerprint, RE::NiSkinInstance* a_skin, RE::NiSkinData* a_skinData, RE::NiSkinPartition* a_partition, RE::NiAVObject* a_base);
    void CacheShaftTopology(std::string_view a_assetKey, const ShaftTopology& a_topology);
    bool ShouldUseGeometry(RE::BSGeometry* a_geometry);
    std::optional<OpeningWeights> GetOpeningWeights(RE::BSGeometry* a_geometry, const std::array<RE::NiAVObject*, 2>& a_targets);
    std::optional<ShaftWeights> GetShaftWeights(RE::BSGeometry* a_geometry, RE::NiAVObject* a_base);
    std::string_view GetModelPath(RE::Actor* a_actor, RE::BSGeometry* a_geometry);
    bool ReadCandidateLocalPositions(RE::BSGeometry* a_geometry, RE::NiSkinPartition* a_partition, std::array<std::vector<Candidate>, 2>& a_candidates);
    bool ReadShaftCandidateLocalPositions(RE::BSGeometry* a_geometry, RE::NiSkinPartition* a_partition, std::vector<ShaftCandidate>& a_candidates);
    std::vector<CachedSample> SelectShaftRingSamples(const std::vector<ShaftCandidate>& a_candidates, const RE::NiPoint3& a_center, RE::NiPoint3 a_tangent);
    std::vector<CachedSample> SelectShaftTipSamples(const std::vector<ShaftCandidate>& a_candidates, const RE::NiPoint3& a_center, RE::NiPoint3 a_tangent);
    std::vector<std::uint16_t> SelectSamples(const std::vector<Candidate>& a_candidates);
    bool MatchesOpeningTopology(const OpeningTopology& a_topology, RE::NiSkinInstance* a_skin, RE::NiSkinData* a_skinData, RE::NiSkinPartition* a_partition, const std::array<RE::NiAVObject*, 2>& a_targets);
    bool MatchesShaftTopology(const ShaftTopology& a_topology, RE::NiSkinInstance* a_skin, RE::NiSkinData* a_skinData, RE::NiSkinPartition* a_partition, RE::NiAVObject* a_base);
    std::optional<OpeningShape> MakeNodeOpening(const GeometryMath::Segment& a_segment, const RE::NiPoint3& a_left, const RE::NiPoint3& a_right);
}
