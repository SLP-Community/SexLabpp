#include "Animation.h"

#include "Registry/Define/RaceKey.h"
#include "Registry/Library.h"
#include "Registry/Util/Decode.h"
#include "Util/Combinatorics.h"
#include "Util/StringUtil.h"

namespace Registry
{
	AnimPackage::AnimPackage(const fs::path a_file)
	{
		std::ifstream stream(a_file, std::ios::binary);
		stream.unsetf(std::ios::skipws);
		stream.exceptions(std::fstream::eofbit);
		stream.exceptions(std::fstream::badbit);
		stream.exceptions(std::fstream::failbit);

		uint8_t version;
		constexpr uint8_t MIN_VERSION = 1;
		constexpr uint8_t MAX_VERSION = 4;
		stream.read(reinterpret_cast<char*>(&version), 1);
		if (version < MIN_VERSION || version > MAX_VERSION) {
			const auto err = std::format("Invalid version: {}", version);
			throw std::runtime_error(err.c_str());
		}
		Decode::Read(stream, name);
		Decode::Read(stream, author);
		hash.resize(Decode::HASH_SIZE);
		stream.read(hash.data(), Decode::HASH_SIZE);

		uint64_t scene_count;
		Decode::Read(stream, scene_count);
		scenes.reserve(scene_count);
		for (size_t i = 0; i < scene_count; i++) {
			scenes.push_back(std::make_unique<Scene>(stream, hash, version));
		}
	}

	Scene::Scene(std::ifstream& a_stream, std::string_view a_hash, uint8_t a_version) :
		hash(a_hash), enabled(true)
	{
		
		// initialize start_animation to avoid crash
		start_animation=nullptr;

		id.resize(Decode::ID_SIZE);
		a_stream.read(id.data(), Decode::ID_SIZE);
		Decode::Read(a_stream, name);
		// --- Position Infos
		uint64_t info_count;
		Decode::Read(a_stream, info_count);
		positions.reserve(info_count);
		for (size_t i = 0; i < info_count; i++) {
			positions.emplace_back(a_stream, a_version);
		}

		enum legacySex : char
		{
			Male = 'M',
			Female = 'F',
			Futa = 'H',
			Creature = 'C',
		};
		std::vector<std::vector<legacySex>> sexes{};
		sexes.reserve(positions.size());
		for (auto&& position : positions) {
			std::vector<legacySex> vec{};
			if (position.data.IsHuman()) {
				if (position.data.IsSex(Sex::Male))
					vec.push_back(legacySex::Male);
				if (position.data.IsSex(Sex::Female))
					vec.push_back(legacySex::Female);
				if (position.data.IsSex(Sex::Futa))
					vec.push_back(legacySex::Futa);
			} else {
				vec.push_back(legacySex::Creature);
			}
			if (vec.empty()) {
				const auto err = std::format("Some position has no associated sex in scene: {}", id);
				throw std::runtime_error(err.c_str());
			}
			sexes.push_back(vec);
		}
		Combinatorics::ForEachCombination(sexes, [&](auto& it) {
			std::vector<char> gender_tag{};
			for (auto&& sex : it) {
				gender_tag.push_back(*sex);
			}
			RE::BSFixedString gTag1{ std::string{ gender_tag.begin(), gender_tag.end() } };
			RE::BSFixedString gTag2{ std::string{ gender_tag.rbegin(), gender_tag.rend() } };
			tags.AddTag(gTag1);
			if (gTag2 != gTag1) {
				tags.AddTag(gTag2);
			}
			return Combinatorics::CResult::Next;
		});
		// --- Stages
		std::string startstage(Decode::ID_SIZE, 'X');
		if (a_version < 4) {
			a_stream.read(startstage.data(), Decode::ID_SIZE);
 		}
		uint64_t stage_count;
		Decode::Read(a_stream, stage_count);
		stages.reserve(stage_count);
		for (size_t i = 0; i < stage_count; i++) {
			const auto& stage = stages.emplace_back(
				std::make_unique<Stage>(a_stream, a_version));

			tags.AddTag(stage->tags);
			if (stage->id == startstage) {
				start_animation = stage.get();
			}
		}
		if (!start_animation && stages.size() == 0) {
			const auto err = std::format("Start animation {} is not found in scene {}", startstage, id);
			throw std::runtime_error(err.c_str());
		}
		if (!start_animation) {
			start_animation = stages[0].get();
		}
		// --- Graph
		uint64_t graph_vertices;
		Decode::Read(a_stream, graph_vertices);
		if (graph_vertices != stage_count) {
			const auto err = std::format("Invalid graph vertex count; expected {} but got {}", stage_count, graph_vertices);
			throw std::runtime_error(err.c_str());
		}
		std::string vertexid(Decode::ID_SIZE, 'X');
		for (size_t i = 0; i < graph_vertices; i++) {
			a_stream.read(vertexid.data(), Decode::ID_SIZE);
			const auto vertex = GetStageByID(vertexid.data());
			if (!vertex) {
				const auto err = std::format("Invalid vertex: {} in scene: {}", vertexid, id);
				throw std::runtime_error(err.c_str());
			}
			std::vector<const Stage*> edges{};
			uint64_t edge_count;
			Decode::Read(a_stream, edge_count);
			std::string edgeid(Decode::ID_SIZE, 'X');
			for (size_t n = 0; n < edge_count; n++) {
				a_stream.read(edgeid.data(), Decode::ID_SIZE);
				const auto edge = GetStageByID(edgeid.data());
				if (!edge) {
					const auto err = std::format("Invalid edge: {} for vertex: {} in scene: {}", edgeid, vertexid, id);
					throw std::runtime_error(err.c_str());
				}
				edges.push_back(edge);
			}
			graph.insert(std::make_pair(vertex, edges));
		}
		// --- Misc
		Decode::Read(a_stream, *reinterpret_cast<uint32_t*>(&furnitureTypes));
		a_stream.read(reinterpret_cast<char*>(&allowBed), 1);
		furnitureOffset = Coordinate(a_stream);
		a_stream.read(reinterpret_cast<char*>(&isPrivate), 1);
	}

	PositionInfo::PositionInfo(std::ifstream& a_stream, uint8_t a_version)
	{
		enum Extra : uint8_t
		{
			Submissive = 1 << 0,
			Vampire = 1 << 1,
			Unconscious = 1 << 2
		};
		float scale;
		RaceKey race;
		REX::EnumSet<Sex> sex;
		REX::EnumSet<Extra> extra;
		a_stream.read(reinterpret_cast<char*>(&race), 1);
		a_stream.read(reinterpret_cast<char*>(&sex), 1);
		Decode::Read(a_stream, scale);
		a_stream.read(reinterpret_cast<char*>(&extra), 1);

		data = ActorFragment(sex, race, scale, extra.all(Extra::Vampire), extra.all(Extra::Submissive), extra.all(Extra::Unconscious));

		if (a_version > 1 && a_version < 4) {
			uint64_t extra_custom;
			Decode::Read(a_stream, extra_custom);
			annotations.reserve(extra_custom);
			for (size_t j = 0; j < extra_custom; j++) {
				RE::BSFixedString tag;
				Decode::Read(a_stream, tag);
				annotations.push_back(tag);
			}
		} else {
			annotations = {};
		}
	}

	Stage::Stage(std::ifstream& a_stream, uint8_t a_version)
	{
		id.resize(Decode::ID_SIZE);
		a_stream.read(id.data(), Decode::ID_SIZE);

		uint64_t position_count;
		Decode::Read(a_stream, position_count);
		positions.reserve(position_count);
		for (size_t i = 0; i < position_count; i++) {
			positions.emplace_back(a_stream, a_version);
		}
		Decode::Read(a_stream, fixedlength);
		Decode::Read(a_stream, navtext);
		tags = TagData{ a_stream };
	}

	Position::Position(std::ifstream& a_stream, uint8_t a_version) :
		event(Decode::Read<decltype(event)>(a_stream)),
		climax(Decode::Read<uint8_t>(a_stream) > 0),
		offset(Transform(a_stream)),
		strips(decltype(strips)::enum_type(Decode::Read<uint8_t>(a_stream))),
		tags({})
	{
		if (a_version == 3) Decode::Read<int8_t>(a_stream);
		if (a_version >= 4) {
			uint64_t extra_custom;
			Decode::Read(a_stream, extra_custom);
			tags.reserve(extra_custom);
			for (size_t j = 0; j < extra_custom; j++) {
				RE::BSFixedString tag;
				Decode::Read(a_stream, tag);
				tags.push_back(tag);
			}
		}
	}

	void Position::Save(YAML::Node& a_node) const
	{
		auto transform = a_node["transform"];
		offset.Save(transform);
	}

	void Position::Load(const YAML::Node& a_node)
	{
		if (auto transform = a_node["transform"]; transform.IsDefined()) {
			offset.Load(transform);
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

	void Scene::Save(YAML::Node& a_node) const
	{
		a_node["enabled"] = this->enabled;
		for (auto&& stage : stages) {
			auto node = a_node[stage->id];
			stage->Save(node);
		}
	}

	void Scene::Load(const YAML::Node& a_node)
	{
		if (const auto enable = a_node["enabled"]; enable.IsDefined())
			this->enabled = enable.as<bool>();

		for (auto&& stage : stages) {
			if (auto node = a_node[stage->id]; node.IsDefined()) {
				stage->Load(node);
			}
		}
	}


	bool PositionInfo::CanFillPosition(RE::Actor* a_actor) const
	{
		auto fragment = ActorFragment(a_actor, false);
		return CanFillPosition(fragment);
	}

	bool PositionInfo::CanFillPosition(const ActorFragment& a_fragment) const
	{
		return data.GetCompatibilityScore(a_fragment) != 0;
	}

	bool PositionInfo::CanFillPosition(const PositionInfo& a_other) const
	{
		return CanFillPosition(a_other.data);
	}

	bool PositionInfo::HasExtraCstm(const RE::BSFixedString& a_extra) const
	{
		return std::ranges::find(annotations, a_extra) != annotations.end();
	}

	std::string PositionInfo::ConcatExtraCstm() const
	{
		return Util::StringJoin(annotations, ", ");
	}

	PapyrusSex PositionInfo::GetSexPapyrus() const
	{
		auto sex = data.GetSex();
		REX::EnumSet<PapyrusSex> ret{ PapyrusSex::None };
		if (data.IsHuman()) {
#define SET_SEX(s)     \
	if (sex.all(Sex::s)) \
		ret.set(PapyrusSex::s);
			SET_SEX(Male);
			SET_SEX(Female);
			SET_SEX(Futa);
#undef SET_SEX
		} else {
			const auto crtSex = sex.any(Sex::Female) ? PapyrusSex::CrtFemale : PapyrusSex::CrtMale;
			ret.set(crtSex);
		}
		return ret.get();
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
		if (!a_details) return !RequiresFurniture();
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
		std::vector fragmentGraph(N, std::vector<std::pair<size_t, int32_t>>{});	// fragment[i] = { { positionIdx, score }, ... }
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

#ifndef NDEBUG
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

}	 // namespace Registry
