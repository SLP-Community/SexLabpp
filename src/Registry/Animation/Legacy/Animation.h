#pragma once

#include "Registry/Define/Fragment.h"
#include "Registry/Define/Furniture.h"
#include "Registry/Define/RaceKey.h"
#include "Registry/Define/Sex.h"
#include "Registry/Define/Tags.h"
#include "Registry/Define/Transform.h"

namespace Registry::Animation::Legacy
{
    struct AnimPackage
    {
        AnimPackage(std::ifstream& stream, uint8_t a_version);
        ~AnimPackage() = default;

        std::vector<std::unique_ptr<Scene>> scenes;
        RE::BSFixedString name;
        RE::BSFixedString author;
        std::string hash;
    };

    struct Scene
    {
        Scene(std::ifstream& a_stream, std::string_view a_hash, uint8_t a_version);
        ~Scene() = default;

        std::string id;
        std::string name;
        std::vector<PositionInfo> positions;
        bool isPrivate;

        bool allowBed;
        Transform furnitureOffset;
        REX::EnumSet<FurnitureType::Value> furnitureTypes{ FurnitureType::None };

        std::string startStageID;
        std::vector<std::unique_ptr<Stage>> stages;
        std::map<std::string, std::vector<std::string>> graph;
    };

    struct PositionInfo
    {
        PositionInfo(std::ifstream& a_stream, uint8_t a_version);
        ~PositionInfo() = default;

        ActorFragment data;
        RaceKey race;
        REX::EnumSet<Sex> sex;
        float scale;
        uint8_t extra;
        std::vector<RE::BSFixedString> annotations;
    };

    struct Stage
    {
        Stage(std::ifstream& a_stream, uint8_t a_version);
        ~Stage() = default;

        std::string id;
        std::vector<Position> positions;

        float fixedlength;
        std::string navtext;
        TagData tags;
    };

    struct Position
    {
        enum class StripData : uint8_t
        {
            None = 0,
            Helmet = 1 << 0,
            Gloves = 1 << 1,
            Boots = 1 << 2,
            // Unused = 1 << 3,
            // Unused = 1 << 4,
            // Unused = 1 << 5,
            // Unused = 1 << 6,
            Default = 1 << 7,

            All = static_cast<std::underlying_type_t<StripData>>(-1),
        };

      public:
        Position(std::ifstream& a_stream, uint8_t a_version);
        ~Position() = default;

      public:
        RE::BSFixedString event;

        bool climax;
        Transform offset;
        stl::enumeration<StripData> strips;
        std::vector<RE::BSFixedString> tags;
    };
}  // namespace Registry
