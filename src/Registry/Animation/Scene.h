#pragma once

#include "Stage.h"

namespace Registry::Animation
{
    namespace Legacy
    {
        struct Scene;
    }

    struct Scene
    {
        Scene(std::ifstream& a_stream, std::string_view a_hash, uint8_t a_version);
        Scene(const Legacy::Scene& a_legacyScene, std::string_view a_hash);
        ~Scene() = default;

        _NODISCARD bool IsEnabled() const;
        _NODISCARD bool IsPrivate() const;
        _NODISCARD bool HasCreatures() const;
        _NODISCARD bool RequiresFurniture() const;
        _NODISCARD RE::BSFixedString GetPackageHash() const;

        _NODISCARD bool IsCompatibleTags(const TagData& a_tags) const;
        _NODISCARD bool IsCompatibleTags(const TagDetails& a_details) const;
        _NODISCARD bool IsCompatibleFurniture(FurnitureType a_furniture) const;
        _NODISCARD bool IsCompatibleFurniture(const FurnitureDetails* a_details) const;
        _NODISCARD bool IsCompatibleFurniture(const RE::TESObjectREFR* a_reference) const;

        _NODISCARD uint32_t CountPositions() const;
        _NODISCARD uint32_t CountSubmissives() const;
        _NODISCARD const PositionInfo* GetNthPosition(size_t n) const;

        _NODISCARD REX::EnumSet<FurnitureType::Value> GetFurnitureTypes() const;
        _NODISCARD std::vector<std::vector<RE::Actor*>> FindAssignments(const std::vector<ActorFragment>& a_fragments) const;

        _NODISCARD size_t GetNumStages() const;
        _NODISCARD const std::vector<const Stage*> GetAllStages() const;
        _NODISCARD Stage* GetStageByID(const RE::BSFixedString& a_stage);
        _NODISCARD const Stage* GetStageByID(const RE::BSFixedString& a_stage) const;
        _NODISCARD std::vector<const Stage*> GetLongestPath(const Stage* a_src) const;
        _NODISCARD std::vector<const Stage*> GetShortestPath(const Stage* a_src) const;
        void ForEachStage(std::function<bool(Stage*)> a_visitor);

        _NODISCARD NodeType GetStageNodeType(const Stage* a_stage) const;
        _NODISCARD std::vector<const Stage*> GetEndingStages() const;
        _NODISCARD std::vector<const Stage*> GetClimaxStages() const;
        _NODISCARD std::vector<const Stage*> GetFixedLengthStages() const;
        _NODISCARD size_t GetNumAdjacentStages(const Stage* a_stage) const;
        _NODISCARD const Stage* GetNthAdjacentStage(const Stage* a_stage, size_t n) const;
        _NODISCARD const std::vector<const Stage*>* GetAdjacentStages(const Stage* a_stage) const;
        _NODISCARD RE::BSFixedString GetNthAnimationEvent(const Stage* a_stage, size_t n) const;
        _NODISCARD std::vector<RE::BSFixedString> GetAnimationEvents(const Stage* a_stage) const;

        void Save(YAML::Node& a_node) const;
        void Load(const YAML::Node& a_node);

      public:
        // If the animation only includes humans, with specified amount of males and females
        _NODISCARD bool Legacy_IsCompatibleSexCount(int32_t a_males, int32_t a_females) const;
        _NODISCARD bool Legacy_IsCompatibleSexCountCrt(int32_t a_males, int32_t a_females) const;

      private:
        std::string id;
        std::string name;

        std::shared_ptr<Stage> start;
        std::vector<PositionMetaData> positions;

        TagData tags;
        bool isEnabled;
        bool isPrivate;

      private:
        bool allowBed;
        Transform furnitureOffset;
        REX::EnumSet<FurnitureType::Value> furnitureTypes{ FurnitureType::None };
    };
}
