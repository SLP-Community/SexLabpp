#pragma once
#include <memory>
#include <vector>

#include "Position.h"

namespace Registry::Animation
{
    namespace Legacy
    {
        struct Scene;
        struct Stage;
    }

    struct Stage
    {
      public:
        Stage(std::ifstream& a_stream, std::string_view a_eventHash, uint8_t a_version);
        Stage(const Legacy::Stage& a_legacyStage, std::string_view a_eventHash);
        ~Stage() = default;

        void Save(YAML::Node& a_node) const;
        void Load(const YAML::Node& a_node);

        _NODISCARD std::string_view GetId() const { return id; }

        _NODISCARD RE::BSFixedString GetAnimationEvent(size_t n) const;
        _NODISCARD std::vector<RE::BSFixedString> GetAnimationEvents() const;
        _NODISCARD std::vector<Position>& GetPositions() { return positions; }
        _NODISCARD const std::vector<Position>& GetPositions() const { return positions; }
        _NODISCARD const std::vector<Stage*> GetOutgoingEdges() const;

        _NODISCARD TagData& GetTags() { return tags; }
        _NODISCARD const TagData& GetTags() const { return tags; }
        _NODISCARD float GetFixedDuration() const { return fixedDuration; }
        _NODISCARD std::string_view GetNavigationText() const { return navigationText; }

        _NODISCARD std::vector<const Stage*> GetLongestPath() const;
        _NODISCARD std::vector<const Stage*> GetShortestPath() const;

        void AddOutgoingEdge(const std::shared_ptr<Stage>& a_stage) { outgoingEdges.push_back(a_stage); }

      private:
        std::string id{ "" };
        std::string_view eventHash{ "" };
        std::vector<Position> positions{};
        std::vector<std::weak_ptr<Stage>> outgoingEdges{};

        TagData tags{};
        float fixedDuration{ 0.0f };
        std::string navigationText{ "" };
    };
}
