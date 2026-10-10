#pragma once

#include "Scene.h"
#include "Stage.h"

namespace Registry::Animation
{
    struct AnimPack
    {
        constexpr static inline size_t kMinVersion = 1;
        constexpr static inline size_t kFinalLegacyVersion = 4;
        constexpr static inline size_t kCurrentVersion = 5;

      public:
        AnimPack(const fs::path a_file);
        ~AnimPack() = default;

        _NODISCARD std::string_view GetHash() const { return hash; }
        _NODISCARD RE::BSFixedString GetName() const { return name; }
        _NODISCARD RE::BSFixedString GetAuthor() const { return author; }

        _NODISCARD const std::vector<std::unique_ptr<Scene>>& GetScenes() const { return scenes; }
        _NODISCARD std::vector<const Stage*> GetStages() const;

      private:
        std::string hash{};
        RE::BSFixedString name{ "???" };
        RE::BSFixedString author{};

        std::vector<std::unique_ptr<Scene>> scenes{};
        std::vector<std::shared_ptr<Stage>> stages{};
    };
}