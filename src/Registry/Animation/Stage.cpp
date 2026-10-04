#pragma once

#include "Stage.h"

#include "Registry/Animation/Legacy/Animation.h"

namespace Registry::Animation {
    Stage::Stage(const Legacy::Stage& a_legacyStage, std::string_view a_eventHash) :
      id(a_legacyStage.id),
      eventHash(a_eventHash),
      tags(a_legacyStage.tags),
      fixedDuration(a_legacyStage.fixedlength),
      navigationText(a_legacyStage.navtext)
    {
        positions.reserve(a_legacyStage.positions.size());
        for (auto&& legacyPosition : a_legacyStage.positions) {
            positions.emplace_back(*legacyPosition);
        }
    }

    void Stage::Save(YAML::Node& a_node) const
    {
        for (auto&& annotation : tags.GetAnnotations()) {
            a_node["annotations"].push_back(annotation.data());
        }
        const auto hasChanges = std::ranges::find_if(positions, [](auto& position) { return position.offset.HasChanges(); });
        if (hasChanges != positions.end()) {
            for (size_t i = 0; i < positions.size(); i++) {
                auto node = a_node[i];
                positions[i].Save(node);
            }
        }
    }

    void Stage::Load(const YAML::Node& a_node)
    {
        if (auto annotations = a_node["annotations"]; annotations.IsDefined()) {
            for (auto&& annotation : annotations) {
                tags.AddAnnotation(annotation.as<std::string>());
            }
        }
        for (size_t i = 0; i < positions.size(); i++) {
            if (auto node = a_node[i]; node.IsDefined()) {
                positions[i].Load(node);
            }
        }
    }
}