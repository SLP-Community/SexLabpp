#pragma once

#include "Registry/Define/Fragment.h"
#include "Registry/Define/Furniture.h"
#include "Registry/Define/RaceKey.h"
#include "Registry/Define/Sex.h"
#include "Registry/Define/Tags.h"
#include "Registry/Define/Transform.h"

namespace Registry::Animation::Legacy
{
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

        RE::BSFixedString event;

        bool climax;
        Transform offset;
        stl::enumeration<StripData> strips;
        std::vector<RE::BSFixedString> tags;
    };

    struct PositionInfo
    {
        PositionInfo(std::ifstream& a_stream, uint8_t a_version);
        ~PositionInfo() = default;

        ActorFragment data;
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

    struct Scene
    {
        Scene(std::ifstream& a_stream, uint8_t a_version);
        ~Scene() = default;

        std::string id;
        std::string name;

        std::vector<PositionInfo> positions;
        Transform furnitureOffset;
        TagData tags;

        REX::EnumSet<FurnitureType::Value> furnitureTypes{ FurnitureType::None };
        bool allowBed;
        bool isPrivate;

        std::vector<std::unique_ptr<Stage>> stages;
        std::map<const Stage*, std::vector<const Stage*>> graph;
        Stage* startAnimation;
    };

    struct AnimPackage
    {
        AnimPackage(std::ifstream& stream, uint8_t a_version);
        ~AnimPackage() = default;

        std::vector<std::unique_ptr<Scene>> scenes;
        RE::BSFixedString name;
        RE::BSFixedString author;
        std::string hash;
    };

}  // namespace Registry::Animation::Legacy
