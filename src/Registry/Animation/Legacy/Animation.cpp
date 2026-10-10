#include "Animation.h"

#include "Registry/Util/Decode.h"
#include "Util/Combinatorics.h"

namespace Registry::Animation::Legacy
{
    AnimPackage::AnimPackage(std::ifstream& stream, uint8_t a_version)
    {
        Decode::Read(stream, name);
        Decode::Read(stream, author);
        hash.resize(Decode::HASH_SIZE);
        stream.read(hash.data(), Decode::HASH_SIZE);

        uint64_t scene_count;
        Decode::Read(stream, scene_count);
        scenes.reserve(scene_count);
        for (size_t i = 0; i < scene_count; i++) {
            scenes.push_back(std::make_unique<Scene>(stream, a_version));
        }
    }

    Scene::Scene(std::ifstream& a_stream, uint8_t a_version)
    {
        // initialize startAnimation to avoid crash
        startAnimation = nullptr;

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
                startAnimation = stage.get();
            }
        }
        if (!startAnimation && stages.size() == 0) {
            const auto err = std::format("Start animation {} is not found in scene {}", startstage, id);
            throw std::runtime_error(err.c_str());
        }
        if (!startAnimation) {
            startAnimation = stages[0].get();
        }
        // --- Graph
        uint64_t graph_vertices;
        Decode::Read(a_stream, graph_vertices);
        if (graph_vertices != stage_count) {
            const auto err = std::format("Invalid graph vertex count; expected {} but got {}", stage_count, graph_vertices);
            throw std::runtime_error(err.c_str());
        }
        const auto findStageById = [&](std::string_view a_stageId) -> const Stage* {
            for (auto&& stage : stages) {
                if (stage && stage->id == a_stageId) {
                    return stage.get();
                }
            }
            return nullptr;
        };

        std::string vertexid(Decode::ID_SIZE, 'X');
        for (size_t i = 0; i < graph_vertices; i++) {
            a_stream.read(vertexid.data(), Decode::ID_SIZE);
            const auto vertex = findStageById(vertexid);
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
                const auto edge = findStageById(edgeid);
                if (!edge) {
                    const auto err = std::format("Invalid edge: {} for vertex: {} in scene: {}", edgeid, vertexid, id);
                    throw std::runtime_error(err.c_str());
                }
                edges.push_back(edge);
            }
            graph.emplace(vertex, std::move(edges));
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
        if (a_version == 3)
            Decode::Read<int8_t>(a_stream);
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

}  // namespace Registry::Animation::Legacy
