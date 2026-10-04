#pragma once
#include <memory>
#include <vector>

#include "Position.h"

namespace Registry::Animation
{
    namespace Legacy
    {
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

        _NODISCARD std::string_view GetID() const { return id; }
        void AddOutgoingEdge(const std::shared_ptr<Stage>& a_stage) { outgoingEdges.push_back(a_stage); }

      private:
        std::string id{""};
        std::string_view eventHash{""};
        std::vector<Position> positions{};
        std::vector<std::shared_ptr<Stage>> outgoingEdges{};

        TagData tags{};
        float fixedDuration{0.0f};
        std::string navigationText{""};
    };
}
