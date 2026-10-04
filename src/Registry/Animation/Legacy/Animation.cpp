#include "Animation.h"

#include "Registry/Util/Decode.h"

namespace Registry::Animation::Legacy
{
    AnimPackage::AnimPackage(std::ifstream& stream, uint8_t a_version)
    {
        Decode::Read(stream, name);
        Decode::Read(stream, author);
        hash.resize(Decode::HASH_SIZE);
        stream.read(hash.data(), Decode::HASH_SIZE);

        const auto sceneCount = Decode::Read<uint64_t>(stream);
        scenes.reserve(sceneCount);
        for (size_t i = 0; i < sceneCount; i++) {
            scenes.push_back(std::make_unique<Scene>(stream, hash, a_version));
        }
    }

    Scene::Scene(std::ifstream& a_stream, std::string_view a_hash, uint8_t a_version)
    {
        (void)a_hash;

        id.resize(Decode::ID_SIZE);
        a_stream.read(id.data(), Decode::ID_SIZE);
        Decode::Read(a_stream, name);

        uint64_t info_count;
        Decode::Read(a_stream, info_count);
        positions.reserve(info_count);
        for (size_t i = 0; i < info_count; i++) {
            positions.emplace_back(a_stream, a_version);
        }

        startStageID.clear();
        if (a_version < 4) {
            startStageID.resize(Decode::ID_SIZE);
            a_stream.read(startStageID.data(), Decode::ID_SIZE);
        }

        uint64_t stage_count;
        Decode::Read(a_stream, stage_count);
        stages.reserve(stage_count);
        for (size_t i = 0; i < stage_count; i++) {
            stages.emplace_back(std::make_unique<Stage>(a_stream, a_version));
        }

        uint64_t graph_vertices;
        Decode::Read(a_stream, graph_vertices);
        if (graph_vertices != stage_count) {
            const auto err = std::format("Invalid graph vertex count; expected {} but got {}", stage_count, graph_vertices);
            throw std::runtime_error(err.c_str());
        }

        std::string vertexid(Decode::ID_SIZE, 'X');
        for (size_t i = 0; i < graph_vertices; i++) {
            a_stream.read(vertexid.data(), Decode::ID_SIZE);

            std::vector<std::string> edges{};
            uint64_t edge_count;
            Decode::Read(a_stream, edge_count);
            edges.reserve(edge_count);
            std::string edgeid(Decode::ID_SIZE, 'X');
            for (size_t n = 0; n < edge_count; n++) {
                a_stream.read(edgeid.data(), Decode::ID_SIZE);
                edges.push_back(edgeid);
            }
            graph.insert(std::make_pair(vertexid, edges));
        }

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

        a_stream.read(reinterpret_cast<char*>(&race), 1);
        a_stream.read(reinterpret_cast<char*>(&sex), 1);
        Decode::Read(a_stream, scale);
        a_stream.read(reinterpret_cast<char*>(&extra), 1);

        const auto flags = REX::EnumSet<Extra>{ static_cast<Extra>(extra) };
        data = ActorFragment(
            sex,
            race,
            scale,
            flags.all(Extra::Vampire),
            flags.all(Extra::Submissive),
            flags.all(Extra::Unconscious));

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
            annotations.clear();
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
