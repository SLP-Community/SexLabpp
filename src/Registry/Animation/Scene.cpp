#include "Scene.h"

#include "Registry/Animation/Legacy/Animation.h"

namespace Registry::Animation {
    namespace
    {
        enum LegacySexTag : char
        {
            Male = 'M',
            Female = 'F',
            Futa = 'H',
            Creature = 'C',
        };

        void BuildLegacySexTags(const std::string& a_sceneID, const std::vector<PositionMetaData>& a_positions, TagData& a_tags)
        {
            std::vector<std::vector<LegacySexTag>> sexes{};
            sexes.reserve(a_positions.size());
            for (auto&& position : a_positions) {
                std::vector<LegacySexTag> vec{};
                if (position.data.IsHuman()) {
                    if (position.data.IsSex(Sex::Male)) {
                        vec.push_back(LegacySexTag::Male);
                    }
                    if (position.data.IsSex(Sex::Female)) {
                        vec.push_back(LegacySexTag::Female);
                    }
                    if (position.data.IsSex(Sex::Futa)) {
                        vec.push_back(LegacySexTag::Futa);
                    }
                } else {
                    vec.push_back(LegacySexTag::Creature);
                }
                if (vec.empty()) {
                    const auto err = std::format("Some position has no associated sex in scene: {}", a_sceneID);
                    throw std::runtime_error(err.c_str());
                }
                sexes.push_back(std::move(vec));
            }

            std::vector<LegacySexTag> combination{};
            combination.reserve(sexes.size());
            const std::function<void(size_t)> recurse = [&](size_t a_index) {
                if (a_index == sexes.size()) {
                    std::vector<char> genderTag{};
                    genderTag.reserve(combination.size());
                    for (auto&& it : combination) {
                        genderTag.push_back(static_cast<char>(it));
                    }

                    RE::BSFixedString gTag1{ std::string{ genderTag.begin(), genderTag.end() } };
                    RE::BSFixedString gTag2{ std::string{ genderTag.rbegin(), genderTag.rend() } };
                    a_tags.AddTag(gTag1);
                    if (gTag2 != gTag1) {
                        a_tags.AddTag(gTag2);
                    }
                    return;
                }

                for (auto&& sex : sexes[a_index]) {
                    combination.push_back(sex);
                    recurse(a_index + 1);
                    combination.pop_back();
                }
            };
            recurse(0);
        }
    }

    Scene::Scene(const Legacy::Scene& a_legacyScene, std::string_view a_hash) :
      id(a_legacyScene.id),
      name(a_legacyScene.name),
      start(nullptr),
      positions(),
      tags(),
      isEnabled(true),
      isPrivate(a_legacyScene.isPrivate),
      allowBed(a_legacyScene.allowBed),
      furnitureOffset(a_legacyScene.furnitureOffset),
      furnitureTypes(a_legacyScene.furnitureTypes)
    {
        positions.reserve(a_legacyScene.positions.size());
        for (auto&& legacyPositionInfo : a_legacyScene.positions) {
            positions.emplace_back(legacyPositionInfo);
        }
        BuildLegacySexTags(id, positions, tags);

        std::vector<std::shared_ptr<Stage>> stages{};
        stages.reserve(a_legacyScene.stages.size());
        std::map<std::string, std::shared_ptr<Stage>> stagesByID{};
        for (auto&& legacyStage : a_legacyScene.stages) {
            auto stage = std::make_shared<Stage>(*legacyStage, a_hash);
            tags.AddTag(legacyStage->tags);
            stagesByID.insert_or_assign(std::string{ stage->GetID() }, stage);
            stages.push_back(std::move(stage));
        }

        if (!a_legacyScene.startStageID.empty()) {
            if (const auto where = stagesByID.find(a_legacyScene.startStageID); where != stagesByID.end()) {
                start = where->second;
            }
        }
        if (!start && !stages.empty()) {
            start = stages[0];
        }
        if (!start && stages.empty()) {
            const auto err = std::format("Start animation {} is not found in scene {}", a_legacyScene.startStageID, id);
            throw std::runtime_error(err.c_str());
        }

        for (auto&& [legacyVertexID, legacyEdges] : a_legacyScene.graph) {
            const auto vertexWhere = stagesByID.find(legacyVertexID);
            if (vertexWhere == stagesByID.end()) {
                const auto err = std::format("Invalid vertex: {} in scene: {}", legacyVertexID, id);
                throw std::runtime_error(err.c_str());
            }

            for (auto&& legacyEdgeID : legacyEdges) {
                const auto edgeWhere = stagesByID.find(legacyEdgeID);
                if (edgeWhere == stagesByID.end()) {
                    const auto err = std::format("Invalid edge: {} for vertex: {} in scene: {}", legacyEdgeID, legacyVertexID, id);
                    throw std::runtime_error(err.c_str());
                }
                vertexWhere->second->AddOutgoingEdge(edgeWhere->second);
            }
        }
    }

    void Scene::Save(YAML::Node& a_node) const
    {
        a_node["enabled"] = this->enabled;
        for (auto&& annotation : tags.GetAnnotations()) {
            // annotations of a stage are saved with that stage and merged back in on load
            const auto fromStage = std::ranges::any_of(stages, [&](auto&& stage) { return stage->tags.HasAnnotation(annotation); });
            if (!fromStage) {
                a_node["annotations"].push_back(annotation.data());
            }
        }
        for (auto&& stage : stages) {
            auto node = a_node[stage->id];
            stage->Save(node);
        }
    }

    void Scene::Load(const YAML::Node& a_node)
    {
        if (const auto enable = a_node["enabled"]; enable.IsDefined())
            this->enabled = enable.as<bool>();

        if (const auto annotations = a_node["annotations"]; annotations.IsDefined()) {
            for (auto&& annotation : annotations) {
                tags.AddAnnotation(annotation.as<std::string>());
            }
        }

        for (auto&& stage : stages) {
            if (auto node = a_node[stage->id]; node.IsDefined()) {
                stage->Load(node);
                for (auto&& annotation : stage->tags.GetAnnotations()) {
                    tags.AddAnnotation(annotation);
                }
            }
        }
    }


    REX::EnumSet<FurnitureType::Value> Scene::GetFurnitureTypes() const
    {
        auto ret = furnitureTypes;
        if (allowBed) {
            ret.set(FurnitureType::BedDouble, FurnitureType::BedSingle, FurnitureType::BedRoll);
        }
        return ret;
    }

    Stage* Scene::GetStageByID(const RE::BSFixedString& a_key)
    {
        if (a_key.empty()) {
            return start_animation;
        }
        const auto where = std::find_if(stages.begin(), stages.end(), [&](const std::unique_ptr<Stage>& it) { return a_key == it->id.data(); });
        return where == stages.end() ? nullptr : where->get();
    }

    const Stage* Scene::GetStageByID(const RE::BSFixedString& a_key) const
    {
        if (a_key.empty()) {
            return start_animation;
        }
        const auto where = std::find_if(stages.begin(), stages.end(), [&](const std::unique_ptr<Stage>& it) { return a_key == it->id.data(); });
        return where == stages.end() ? nullptr : where->get();
    }

    bool Scene::HasCreatures() const
    {
        return std::ranges::any_of(positions, [](auto&& info) { return !info.IsHuman(); });
    }

    uint32_t Scene::CountSubmissives() const
    {
        return static_cast<uint32_t>(std::ranges::count_if(positions, [](auto&& info) { return info.IsSubmissive(); }));
    }

    const PositionInfo* Scene::GetNthPosition(size_t n) const
    {
        return &positions.at(n);
    }

    uint32_t Scene::CountPositions() const
    {
        return static_cast<uint32_t>(positions.size());
    }

    bool Scene::IsEnabled() const
    {
        return enabled;
    }

    bool Scene::IsPrivate() const
    {
        return isPrivate;
    }

    bool Scene::IsCompatibleTags(const TagData& a_tags) const
    {
        return this->tags.HasTags(a_tags, true);
    }
    bool Scene::IsCompatibleTags(const TagDetails& a_details) const
    {
        return a_details.MatchTags(tags);
    }

    bool Scene::RequiresFurniture() const
    {
        return furnitureTypes != FurnitureType::None;
    }

    RE::BSFixedString Scene::GetPackageHash() const
    {
        return hash;
    }

    bool Scene::IsCompatibleFurniture(const RE::TESObjectREFR* a_reference) const
    {
        const auto details = Library::GetSingleton()->GetFurnitureDetails(a_reference);
        return IsCompatibleFurniture(details);
    }

    bool Scene::IsCompatibleFurniture(const FurnitureDetails* a_details) const
    {
        if (!a_details)
            return !RequiresFurniture();
        return IsCompatibleFurniture(a_details->GetTypes().get());
    }

    bool Scene::IsCompatibleFurniture(FurnitureType a_furniture) const
    {
        if (a_furniture.IsNone()) {
            return !RequiresFurniture();
        } else if (a_furniture.IsBed() && allowBed) {
            return true;
        }
        return GetFurnitureTypes().any(a_furniture.value);
    }

    bool Scene::Legacy_IsCompatibleSexCount(int32_t a_males, int32_t a_females) const
    {
        if (a_males < 0 && a_females < 0) {
            return true;
        }

        bool ret = false;
        tags.ForEachExtra([&](const std::string_view a_tag) {
            if (a_tag.find_first_not_of("MFC") != std::string_view::npos) {
                return false;
            }
            if (a_males == -1 || std::count(a_tag.begin(), a_tag.end(), 'M') == a_males) {
                if (a_females == -1 || std::count(a_tag.begin(), a_tag.end(), 'F') == a_females) {
                    ret = true;
                    return true;
                }
            }
            return false;
        });
        return ret;
    }

    bool Scene::Legacy_IsCompatibleSexCountCrt(int32_t a_males, int32_t a_females) const
    {
        enum
        {
            Male = 0,
            Female = 1,
            Either = 2,
        };

        bool ret = false;
        tags.ForEachExtra([&](const std::string_view a_tag) {
            if (a_tag.find_first_not_of("MFC") != std::string_view::npos) {
                return false;
            }
            const auto crt_total = std::count(a_tag.begin(), a_tag.end(), 'C');
            if (crt_total != a_males + a_females) {
                return true;
            }

            int count[3];
            for (auto&& position : positions) {
                if (position.data.IsHuman())
                    continue;
                if (position.data.IsNotSex(Sex::Female)) {
                    count[Male]++;
                } else if (position.data.IsNotSex(Sex::Male)) {
                    count[Female]++;
                } else {
                    count[Either]++;
                }
            }
            if (count[Male] <= a_males && count[Male] + count[Either] >= a_males) {
                count[Either] -= a_males - count[Male];
                ret = count[Female] + count[Either] == a_females;
            }
            return true;
        });
        return ret;
    }


    std::vector<std::vector<RE::Actor*>> Scene::FindAssignments(const std::vector<ActorFragment>& a_fragments) const
    {
        if (a_fragments.size() != positions.size())
            return {};

        const auto N = a_fragments.size();
        std::vector fragmentGraph(N, std::vector<std::pair<size_t, int32_t>>{});  // fragment[i] = { { positionIdx, score }, ... }
        for (size_t i = 0; i < N; i++) {
            const auto& fragment = a_fragments[i];
            for (size_t j = 0; j < N; j++) {
                const auto& position = positions[j];
                const auto score = position.data.GetCompatibilityScore(fragment);
                if (score != 0) {
                    fragmentGraph[i].emplace_back(j, score);
                }
            }
        }

        using Assignment = std::vector<std::pair<ActorFragment, size_t>>;
        struct ScoredAssignment
        {
            Assignment assignment{};
            int32_t score{ 0 };

            bool operator<(const ScoredAssignment& other) const { return score > other.score; }
        };
        std::vector<ScoredAssignment> assignments{};
        std::vector<bool> used(N, false);
        Assignment current;
        const std::function<void(size_t, int32_t)> helper = [&](size_t fragmentIdx, int32_t accScore) {
            if (fragmentIdx == N) {
                assignments.emplace_back(current, accScore);
                return;
            }
            for (auto&& [positionIdx, score] : fragmentGraph[fragmentIdx]) {
                if (used[positionIdx]) {
                    continue;
                }
                used[positionIdx] = true;
                current.emplace_back(a_fragments[fragmentIdx], positionIdx);
                helper(fragmentIdx + 1, accScore + score);
                current.pop_back();
                used[positionIdx] = false;
            }
        };
        helper(0, 0);
        if (assignments.empty()) {
            return {};
        }
        std::sort(assignments.begin(), assignments.end());

#ifdef DEBUG
        logger::info("Scene: {} | Found {} assignments", id, assignments.size());
        for (auto&& assignment : assignments) {
            std::string str{};
            str.reserve(assignment.assignment.size() * 2);
            for (auto&& [fragment, positionIdx] : assignment.assignment) {
                str += std::format("{} ", positionIdx);
            }
            logger::info("Assignment: {} | Score: {}", str, assignment.score);
        }
#endif

        std::vector<std::vector<RE::Actor*>> ret{};
        ret.reserve(assignments.size());
        for (auto&& assignment : assignments) {
            std::vector<RE::Actor*> actors(N, nullptr);
            for (auto&& [fragment, positionIdx] : assignment.assignment) {
                actors[positionIdx] = fragment.GetActor();
            }
            ret.push_back(actors);
        }
        return ret;
    }

    size_t Scene::GetNumAdjacentStages(const Stage* a_stage) const
    {
        const auto where = graph.find(a_stage);
        if (where == graph.end())
            return 0;

        return where->second.size();
    }

    const Stage* Scene::GetNthAdjacentStage(const Stage* a_stage, size_t n) const
    {
        const auto where = graph.find(a_stage);
        if (where == graph.end())
            return 0;

        if (n < 0 || n >= where->second.size())
            return 0;

        return where->second[n];
    }

    const std::vector<const Stage*>* Scene::GetAdjacentStages(const Stage* a_stage) const
    {
        const auto where = graph.find(a_stage);
        return where != graph.end() ? &where->second : nullptr;
    }

    RE::BSFixedString Scene::GetNthAnimationEvent(const Stage* a_stage, size_t n) const
    {
        std::string ret{ hash };
        return ret + a_stage->positions[n].event.data();
    }

    std::vector<RE::BSFixedString> Scene::GetAnimationEvents(const Stage* a_stage) const
    {
        return std::ranges::fold_left(a_stage->positions, std::vector<RE::BSFixedString>{}, [this](auto&& acc, auto&& it) {
            return (acc.push_back(std::format("{}{}", hash, it.event)), acc);
        });
    }

    size_t Scene::GetNumStages() const
    {
        return stages.size();
    }

    const std::vector<const Stage*> Scene::GetAllStages() const
    {
        std::vector<const Stage*> ret{};
        ret.reserve(stages.size());
        for (auto&& stage : stages) {
            ret.push_back(stage.get());
        }
        return ret;
    }

    Scene::NodeType Scene::GetStageNodeType(const Stage* a_stage) const
    {
        if (a_stage == start_animation)
            return NodeType::Root;

        const auto where = graph.find(a_stage);
        if (where == graph.end())
            return NodeType::None;

        return where->second.size() == 0 ? NodeType::Sink : NodeType::Default;
    }

    std::vector<const Stage*> Scene::GetLongestPath(const Stage* a_src) const
    {
        if (GetStageNodeType(a_src) == NodeType::Sink)
            return { a_src };

        std::set<const Stage*> visited{};
        std::function<std::vector<const Stage*>(const Stage*)> DFS = [&](const Stage* src) -> std::vector<const Stage*> {
            if (visited.contains(src))
                return {};
            visited.insert(src);

            std::vector<const Stage*> longest_path{ src };
            const auto& neighbours = this->graph.find(src);
            assert(neighbours != this->graph.end());
            for (auto&& n : neighbours->second) {
                const auto cmp = DFS(n);
                if (cmp.size() + 1 > longest_path.size()) {
                    longest_path.assign(cmp.begin(), cmp.end());
                    longest_path.insert(longest_path.begin(), src);
                }
            }
            return longest_path;
        };
        return DFS(a_src);
    }

    std::vector<const Stage*> Scene::GetShortestPath(const Stage* a_src) const
    {
        if (GetStageNodeType(a_src) == NodeType::Sink)
            return { a_src };

        std::function<std::vector<const Stage*>(const Stage*)> BFS = [&](const Stage* src) -> std::vector<const Stage*> {
            std::set<const Stage*> visited{ src };
            std::map<const Stage*, const Stage*> pred{ { src, nullptr } };
            std::queue<const Stage*> queue{ { src } };
            while (!queue.empty()) {
                const auto it = queue.front();
                const auto neighbours = graph.find(it);
                assert(neighbours != this->graph.end());
                for (auto&& n : neighbours->second) {
                    if (visited.contains(n))
                        continue;
                    if (GetStageNodeType(n) == NodeType::Sink) {
                        std::vector<const Stage*> ret{};
                        auto p = pred.at(it);
                        while (p != nullptr) {
                            ret.push_back(p);
                            p = pred.at(p);
                        }
                        return { ret.rbegin(), ret.rend() };
                    }
                    pred.emplace(n, it);
                    visited.insert(n);
                    queue.push(n);
                }
                queue.pop();
            }
            return { src };
        };
        return BFS(a_src);
    }

    void Scene::ForEachStage(std::function<bool(Stage*)> a_visitor)
    {
        for (auto&& stage : stages) {
            if (a_visitor(stage.get())) {
                return;
            }
        }
    }

    std::vector<const Stage*> Scene::GetEndingStages() const
    {
        std::vector<const Stage*> ret{};
        for (auto&& [vert, edges] : graph) {
            if (edges.empty()) {
                ret.push_back(vert);
            }
        }
        return ret;
    }

    std::vector<const Stage*> Scene::GetClimaxStages() const
    {
        std::vector<const Stage*> ret{};
        for (auto&& stage : stages) {
            for (auto&& position : stage->positions) {
                if (position.climax) {
                    ret.push_back(stage.get());
                    break;
                }
            }
        }
        return ret;
    }

    std::vector<const Stage*> Scene::GetFixedLengthStages() const
    {
        std::vector<const Stage*> ret{};
        for (auto&& stage : stages) {
            if (stage->fixedlength)
                ret.push_back(stage.get());
        }
        return ret;
    }
}