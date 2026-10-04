#include "AnimPack.h"

#include "Registry/Animation/Legacy/Animation.h"
#include "Registry/Util/Decode.h"

namespace Registry::Animation
{
    AnimPack::AnimPack(const fs::path a_file)
    {
        std::ifstream stream(a_file, std::ios::binary);
        stream.unsetf(std::ios::skipws);
        stream.exceptions(std::fstream::eofbit);
        stream.exceptions(std::fstream::badbit);
        stream.exceptions(std::fstream::failbit);

        uint8_t version;
        stream.read(reinterpret_cast<char*>(&version), 1);
        if (version != kLegacyVersion && (version < kMinVersion || version > kCurrentVersion)) {
            const auto err = std::format("Invalid version: {}", version);
            throw std::runtime_error(err.c_str());
        } else if (version == kLegacyVersion) {
            Legacy::AnimPackage legacyPack{ stream, version };
            name = legacyPack.name;
            author = legacyPack.author;
            hash = legacyPack.hash;

            scenes.reserve(legacyPack.scenes.size());
            for (auto&& legacyScene : legacyPack.scenes) {
                auto scene = std::make_unique<Scene>(*legacyScene, hash);
                scene->ForEachStage([&](Stage* a_stage) {
                    stages.push_back(std::make_shared<Stage>(*a_stage));
                    return false;
                });
                scenes.push_back(std::move(scene));
            }
        } else {
            Decode::Read(stream, name);
            Decode::Read(stream, author);
            hash.resize(Decode::HASH_SIZE);
            stream.read(hash.data(), Decode::HASH_SIZE);

            const auto stageCount = Decode::Read<uint64_t>(stream);
            stages.reserve(stageCount);
            for (uint64_t i = 0; i < stageCount; i++) {
                stages.push_back(std::make_shared<Stage>(stream, hash, version));
            }

            const auto sceneCount = Decode::Read<uint64_t>(stream);
            scenes.reserve(sceneCount);
            for (uint64_t i = 0; i < sceneCount; i++) {
                scenes.push_back(std::make_unique<Scene>(stream, hash, version));
            }
        }
    }
}
