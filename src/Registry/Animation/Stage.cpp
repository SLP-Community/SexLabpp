#include "Stage.h"

#include "Registry/Animation/Legacy/Animation.h"
#include "Registry/Util/Decode.h"

namespace Registry::Animation
{
    Stage::Stage(std::ifstream& a_stream, std::string_view a_eventHash, uint8_t a_version) :
      id(std::string(Decode::ID_SIZE, '\0')),
      eventHash(a_eventHash)
    {
        a_stream.read(id.data(), Decode::ID_SIZE);

        const auto positionCount = Decode::Read<uint64_t>(a_stream);
        positions.reserve(positionCount);
        for (uint64_t i = 0; i < positionCount; i++) {
            positions.emplace_back(a_stream, a_version);
        }

        Decode::Read(a_stream, fixedDuration);
        Decode::Read(a_stream, navigationText);
        tags = TagData{ a_stream };
    }

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
        const auto hasChanges = std::ranges::find_if(positions, [](const auto& position) { return position.GetOffset().HasChanges(); });
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

    std::vector<RE::BSFixedString> Stage::GetAnimationEvents() const
    {
        std::vector<RE::BSFixedString> events;
        events.reserve(positions.size());
        for (auto&& position : positions) {
            const auto event = std::format("{}{}", eventHash, position.GetEvent().data());
            events.emplace_back(event);
        }
        return events;
    }

    std::vector<const Stage*> Stage::GetLongestPath() const
    {
        struct Frame
        {
            const Stage* stage;
            std::size_t nextEdge;
        };

        std::vector<Frame> stack{};
        std::vector<const Stage*> currentPath{};
        std::vector<const Stage*> longestPath{};
        std::unordered_set<const Stage*> visited{};

        stack.push_back({ this, 0 });
        currentPath.push_back(this);
        visited.insert(this);

        while (!stack.empty()) {
            auto& frame = stack.back();
            const auto& edges = frame.stage->GetOutgoingEdges();
            const auto hasAnyLiveEdge = std::ranges::any_of(edges, [](const auto& edge) { return !edge.expired(); });
            bool pushedNext = false;
            while (frame.nextEdge < edges.size()) {
                const auto next = edges[frame.nextEdge++].lock();
                if (!next) {
                    continue;
                }
                if (visited.contains(next.get())) {
                    continue;
                }
                visited.insert(next.get());
                currentPath.push_back(next.get());
                stack.push_back({ next.get(), 0 });
                pushedNext = true;
                break;
            }

            if (pushedNext) {
                continue;
            }

            if (!hasAnyLiveEdge) {  // Exit node (or all targets expired).
                if (currentPath.size() > longestPath.size()) {
                    longestPath = currentPath;
                }
            }

            visited.erase(frame.stage);
            currentPath.pop_back();
            stack.pop_back();
        }
        return longestPath;
    }

    std::vector<const Stage*> Stage::GetShortestPath() const
    {
        std::queue<const Stage*> queue{};
        std::unordered_set<const Stage*> visited{};
        std::unordered_map<const Stage*, const Stage*> parent{};

        queue.push(this);
        visited.insert(this);

        while (!queue.empty()) {
            const auto stage = queue.front();
            queue.pop();

            const auto& edges = stage->GetOutgoingEdges();
            if (edges.empty()) {  // We reached an exit.
                std::vector<const Stage*> path{};

                for (auto current = stage; current != nullptr;) {
                    path.push_back(current);
                    if (current == this) {
                        break;
                    }
                    current = parent[current];
                }

                std::reverse(path.begin(), path.end());
                return path;
            }

            bool enqueuedAny = false;
            for (auto&& edge : edges) {
                const auto next = edge.lock();
                if (!next) {
                    continue;
                }
                enqueuedAny = true;
                if (visited.insert(next.get()).second) {
                    parent[next.get()] = stage;
                    queue.push(next.get());
                }
            }

            if (!enqueuedAny) {
                std::vector<const Stage*> path{};
                for (auto current = stage; current != nullptr;) {
                    path.push_back(current);
                    if (current == this) {
                        break;
                    }
                    current = parent[current];
                }
                std::reverse(path.begin(), path.end());
                return path;
            }
        }
        return {};
    }
}