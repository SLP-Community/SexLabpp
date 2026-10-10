#include "Scene.h"

#include <unordered_map>
#include <unordered_set>

#include "Registry/Animation/Legacy/Animation.h"
#include "Registry/Library.h"

namespace Registry::Animation
{
    namespace
    {
        struct SexOptions
        {
            bool male{ false };
            bool female{ false };
        };

        bool LegacyMatchSex(const std::vector<SexOptions>& options, int32_t a_males, int32_t a_females, size_t index, int32_t males, int32_t females) {
            if (index >= options.size()) {
                const auto maleMatch = a_males == -1 || males == a_males;
                const auto femaleMatch = a_females == -1 || females == a_females;
                return maleMatch && femaleMatch;
            }

            const auto& current = options[index];
            if (current.male && (a_males == -1 || males + 1 <= a_males) &&
                LegacyMatchSex(options, a_males, a_females, index + 1, males + 1, females)) {
                return true;
            }
            if (current.female && (a_females == -1 || females + 1 <= a_females) &&
                LegacyMatchSex(options, a_males, a_females, index + 1, males, females + 1)) {
                return true;
            }
            return false;
        };
    }

    Scene::Scene(std::ifstream&, uint8_t)
    {
        throw std::runtime_error("Scene binary constructor is no longer implementable with the new shared-stage structure");
    }

    Scene::Scene(const Legacy::Scene& a_legacyScene, std::string_view a_hash, std::vector<std::shared_ptr<Stage>>* a_ownedStages) :
      id(a_legacyScene.id),
      name(a_legacyScene.name),
      tags(a_legacyScene.tags),
      isPrivate(a_legacyScene.isPrivate),
      allowBed(a_legacyScene.allowBed),
      furnitureOffset(a_legacyScene.furnitureOffset),
      furnitureTypes(a_legacyScene.furnitureTypes)
    {
        if (a_ownedStages == nullptr) {
            throw std::runtime_error(std::format("Missing stage ownership output for scene '{}'", id));
        }

        positions.reserve(a_legacyScene.positions.size());
        for (auto&& legacyPosition : a_legacyScene.positions) {
            positions.emplace_back(legacyPosition);
        }

        std::unordered_map<const Legacy::Stage*, std::shared_ptr<Stage>> legacyToStage{};
        legacyToStage.reserve(a_legacyScene.stages.size());
        for (auto&& legacyStage : a_legacyScene.stages) {
            auto stage = std::make_shared<Stage>(*legacyStage, a_hash);
            legacyToStage.emplace(legacyStage.get(), stage);
            a_ownedStages->push_back(stage);
        }

        if (a_legacyScene.startAnimation != nullptr) {
            const auto startIt = legacyToStage.find(a_legacyScene.startAnimation);
            if (startIt == legacyToStage.end()) {
                throw std::runtime_error(std::format("Failed to resolve start stage for scene '{}'", id));
            }
            start = startIt->second;
        }

        for (auto&& [fromLegacy, edgesLegacy] : a_legacyScene.graph) {
            const auto fromIt = legacyToStage.find(fromLegacy);
            if (fromIt == legacyToStage.end()) {
                throw std::runtime_error(std::format("Failed to resolve graph vertex for scene '{}'", id));
            }

            for (auto&& toLegacy : edgesLegacy) {
                const auto toIt = legacyToStage.find(toLegacy);
                if (toIt == legacyToStage.end()) {
                    throw std::runtime_error(std::format("Failed to resolve graph edge for scene '{}'", id));
                }
                fromIt->second->AddOutgoingEdge(toIt->second);
            }
        }
    }

    void Scene::Save(YAML::Node& a_node) const
    {
        a_node["enabled"] = isEnabled;
        auto annotationsNode = a_node["annotations"];
        tags.Save(annotationsNode);
        ForEachStage(start.lock().get(), [&](const Stage* stage) {
            auto node = a_node[stage->GetId().data()];
            stage->Save(node);
            return false;
        });
    }

    void Scene::Load(const YAML::Node& a_node)
    {
        if (const auto enable = a_node["enabled"]; enable.IsDefined()) {
            isEnabled = enable.as<bool>();
        }
        if (auto annotations = a_node["annotations"]; annotations.IsDefined()) {
            tags.Load(annotations);
        }
        ForEachStage(start.lock().get(), [&](Stage* stage) {
            if (auto node = a_node[stage->GetId().data()]; node.IsDefined()) {
                stage->Load(node);
            }
            return false;
        });
    }

    REX::EnumSet<FurnitureType::Value> Scene::GetFurnitureTypes() const
    {
        auto ret = furnitureTypes;
        if (allowBed) {
            ret.set(FurnitureType::BedDouble, FurnitureType::BedSingle, FurnitureType::BedRoll);
        }
        return ret;
    }

    bool Scene::HasCreatures() const
    {
        return std::ranges::any_of(positions, [](auto&& info) { return !info.IsHuman(); });
    }

    uint32_t Scene::GetNumPositions() const
    {
        return static_cast<uint32_t>(positions.size());
    }


    uint32_t Scene::GetNumSubmissives() const
    {
        return static_cast<uint32_t>(std::ranges::count_if(positions, [](auto&& info) { return info.IsSubmissive(); }));
    }

    const PositionMetaData& Scene::GetNthPosition(size_t n) const
    {
        if (n >= positions.size()) {
            throw std::out_of_range(std::format("Position index {} out of range for scene '{}'", n, id));
        }
        return positions[n];
    }

    bool Scene::IsEnabled() const
    {
        return isEnabled;
    }

    bool Scene::IsPrivate() const
    {
        return isPrivate;
    }

    bool Scene::IsCompatibleTags(const TagData& a_tags) const
    {
        return tags.HasTags(a_tags, true);
    }

    bool Scene::IsCompatibleTags(const TagDetails& a_details) const
    {
        return a_details.MatchTags(tags);
    }

    bool Scene::RequiresFurniture() const
    {
        return furnitureTypes != FurnitureType::None;
    }

    bool Scene::IsCompatibleFurniture(const RE::TESObjectREFR* a_reference) const
    {
        const auto details = Library::GetSingleton()->GetFurnitureDetails(a_reference);
        return IsCompatibleFurniture(details);
    }

    bool Scene::IsCompatibleFurniture(const FurnitureDetails* a_details) const
    {
        if (!a_details) {
            return !RequiresFurniture();
        }
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

    std::vector<std::vector<RE::Actor*>> Scene::FindAssignments(const std::vector<ActorFragment>& a_fragments) const
    {
        if (a_fragments.size() != positions.size()) {
            return {};
        }

        const auto N = a_fragments.size();
        std::vector fragmentGraph(N, std::vector<std::pair<size_t, int32_t>>{});
        for (size_t i = 0; i < N; i++) {
            const auto& fragment = a_fragments[i];
            for (size_t j = 0; j < N; j++) {
                const auto& position = positions[j];
                const auto score = position.get().GetCompatibilityScore(fragment);
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
        Assignment current{};
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
            ret.push_back(std::move(actors));
        }
        return ret;
    }

    size_t Scene::GetNumStages() const
    {
        return GetAllStages().size();
    }

    std::vector<const Stage*> Scene::GetAllStages() const
    {
        std::vector<const Stage*> ret{};
        ForEachStage(start.lock().get(), [&](const Stage* stage) {
            ret.push_back(stage);
            return false;
        });
        return ret;
    }

    Stage* Scene::GetStageById(const RE::BSFixedString& a_key)
    {
        if (a_key.empty()) {
            return start.lock().get();
        }
        Stage* ret = nullptr;
        ForEachStage(start.lock().get(), [&](Stage* a_stage) {
            if (a_key == a_stage->GetId().data()) {
                ret = a_stage;
                return true;
            }
            return false;
        });
        return ret;
    }

    const Stage* Scene::GetStageById(const RE::BSFixedString& a_key) const
    {
        if (a_key.empty()) {
            return start.lock().get();
        }
        const Stage* ret = nullptr;
        ForEachStage(start.lock().get(), [&](const Stage* a_stage) {
            if (a_key == a_stage->GetId().data()) {
                ret = a_stage;
                return true;
            }
            return false;
        });
        return ret;
    }

    std::vector<const Stage*> Scene::GetEndingStages() const
    {
        std::vector<const Stage*> ret{};
        ForEachStage(start.lock().get(), [&](const Stage* stage) {
            if (stage->GetOutgoingEdges().empty()) {
                ret.push_back(stage);
            }
            return false;
        });
        return ret;
    }

    std::vector<const Stage*> Scene::GetClimaxStages() const
    {
        std::vector<const Stage*> ret{};
        ForEachStage(start.lock().get(), [&](const Stage* stage) {
            if (std::ranges::any_of(stage->GetPositions(), [](const auto& position) { return position.IsClimax(); })) {
                ret.push_back(stage);
            }
            return false;
        });
        return ret;
    }

    std::vector<const Stage*> Scene::GetFixedLengthStages() const
    {
        std::vector<const Stage*> ret{};
        ForEachStage(start.lock().get(), [&](const Stage* stage) {
            if (stage->GetFixedDuration() > 0.0f) {
                ret.push_back(stage);
            }
            return false;
        });
        return ret;
    }

    void Scene::ForEachStage(std::function<bool(const Stage*)> a_visitor) const
    {
        ForEachStage(start.lock().get(), a_visitor);
    }

    void Scene::ForEachStage(std::function<bool(Stage*)> a_visitor)
    {
        ForEachStage(start.lock().get(), a_visitor);
    }

    void Scene::ForEachStage(Stage* a_start, std::function<bool(Stage*)> a_visitor)
    {
        if (!a_start) {
            return;
        }

        std::stack<Stage*> stack{};
        std::unordered_set<const Stage*> visited{};

        stack.push(a_start);

        while (!stack.empty()) {
            auto* stage = stack.top();
            stack.pop();

            if (!visited.insert(stage).second) {
                continue;
            }

            if (a_visitor(stage)) {
                return;
            }

            for (auto&& edge : stage->GetOutgoingEdges()) {
                stack.push(edge);
            }
        }
    }

    void Scene::ForEachStage(const Stage* a_start, std::function<bool(const Stage*)> a_visitor) const
    {
        if (!a_start) {
            return;
        }

        std::stack<const Stage*> stack{};
        std::unordered_set<const Stage*> visited{};

        stack.push(a_start);

        while (!stack.empty()) {
            const auto stage = stack.top();
            stack.pop();

            if (!visited.insert(stage).second) {
                continue;
            }

            if (a_visitor(stage)) {
                return;
            }

            for (auto&& edge : stage->GetOutgoingEdges()) {
                stack.push(edge);
            }
        }
    }

    bool Scene::Legacy_IsCompatibleSexCount(int32_t a_males, int32_t a_females) const
    {
        if (a_males < 0 && a_females < 0) {
            return true;
        }

        std::vector<SexOptions> options{};
        options.reserve(positions.size());
        for (auto&& position : positions) {
            if (!position.IsHuman()) {
                continue;
            }
            options.push_back({
                .male = position.IsMale(),
                .female = position.IsFemale(),
            });
        }

        return LegacyMatchSex(options, a_males, a_females, 0, 0, 0);
    }

    bool Scene::Legacy_IsCompatibleSexCountCrt(int32_t a_males, int32_t a_females) const
    {
        if (a_males < 0 && a_females < 0) {
            return true;
        }

        std::vector<SexOptions> options{};
        options.reserve(positions.size());
        for (auto&& position : positions) {
            if (position.IsHuman()) {
                continue;
            }

            const auto canMale = position.IsMale();
            const auto canFemale = position.IsFemale();
            if (!canMale && !canFemale) {
                return false;
            }
            options.push_back({
                .male = canMale,
                .female = canFemale,
            });
        }

        if (a_males != -1 && a_females != -1 &&
            static_cast<size_t>(a_males + a_females) != options.size()) {
            return false;
        }

        return LegacyMatchSex(options, a_males, a_females, 0, 0, 0);
    }
}
