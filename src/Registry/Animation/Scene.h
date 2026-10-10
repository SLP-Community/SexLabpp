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
        Scene(std::ifstream& a_stream, uint8_t a_version);
        Scene(const Legacy::Scene& a_legacyScene, std::string_view a_hash, std::vector<std::shared_ptr<Stage>>* a_ownedStages);
        ~Scene() = default;

        _NODISCARD bool IsEnabled() const;
        _NODISCARD bool IsPrivate() const;
        _NODISCARD bool HasCreatures() const;
        _NODISCARD bool RequiresFurniture() const;

        _NODISCARD bool IsCompatibleTags(const TagData& a_tags) const;
        _NODISCARD bool IsCompatibleTags(const TagDetails& a_details) const;
        _NODISCARD bool IsCompatibleFurniture(FurnitureType a_furniture) const;
        _NODISCARD bool IsCompatibleFurniture(const FurnitureDetails* a_details) const;
        _NODISCARD bool IsCompatibleFurniture(const RE::TESObjectREFR* a_reference) const;

        _NODISCARD uint32_t GetNumPositions() const;
        _NODISCARD uint32_t GetNumSubmissives() const;
        _NODISCARD std::vector<PositionMetaData>& GetPositions() { return positions; }
        _NODISCARD const PositionMetaData& GetNthPosition(size_t n) const;
        _NODISCARD const std::vector<PositionMetaData>& GetPositions() const { return positions; }
        void SetEnabled(bool a_enabled) { isEnabled = a_enabled; }

        _NODISCARD REX::EnumSet<FurnitureType::Value> GetFurnitureTypes() const;
        _NODISCARD std::vector<std::vector<RE::Actor*>> FindAssignments(const std::vector<ActorFragment>& a_fragments) const;

        _NODISCARD size_t GetNumStages() const;
        _NODISCARD std::vector<const Stage*> GetAllStages() const;
        _NODISCARD Stage* GetStageById(const RE::BSFixedString& a_stage);
        _NODISCARD const Stage* GetStageById(const RE::BSFixedString& a_stage) const;

        _NODISCARD std::vector<const Stage*> GetEndingStages() const;
        _NODISCARD std::vector<const Stage*> GetClimaxStages() const;
        _NODISCARD std::vector<const Stage*> GetFixedLengthStages() const;
        void ForEachStage(std::function<bool(Stage*)> a_visitor);
        void ForEachStage(Stage* a_start, std::function<bool(Stage*)> a_visitor);
        void ForEachStage(std::function<bool(const Stage*)> a_visitor) const;
        void ForEachStage(const Stage* a_start, std::function<bool(const Stage*)> a_visitor) const;

        void Save(YAML::Node& a_node) const;
        void Load(const YAML::Node& a_node);

      public:
        _NODISCARD bool Legacy_IsCompatibleSexCount(int32_t a_males, int32_t a_females) const;
        _NODISCARD bool Legacy_IsCompatibleSexCountCrt(int32_t a_males, int32_t a_females) const;
        _NODISCARD std::string_view GetId() const { return id; }
        _NODISCARD std::string_view GetName() const { return name; }
        _NODISCARD TagData& GetTags() { return tags; }
        _NODISCARD const TagData& GetTags() const { return tags; }
        _NODISCARD Transform& GetFurnitureOffset() { return furnitureOffset; }
        _NODISCARD const Transform& GetFurnitureOffset() const { return furnitureOffset; }

      private:
        std::string id{};
        std::string name{ "???" };

        std::weak_ptr<Stage> start{};
        std::vector<PositionMetaData> positions{};

        TagData tags{};
        bool isEnabled{ true };
        bool isPrivate{ false };

      private:
        bool allowBed{ false };
        Transform furnitureOffset{};
        REX::EnumSet<FurnitureType::Value> furnitureTypes{ FurnitureType::None };
    };
}
