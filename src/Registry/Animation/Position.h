#pragma once

namespace Registry::Animation
{
    namespace Legacy
    {
        struct Position;
        struct PositionInfo;
    }

    enum class StripParts : uint8_t
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

        All = static_cast<std::underlying_type_t<StripParts>>(-1),
    };

    struct Position
    {
      public:
        Position(std::ifstream& a_stream, uint8_t a_version);
        Position(const Legacy::Position& a_legacyPosition);
        ~Position() = default;

        void Save(YAML::Node& a_node) const;
        void Load(const YAML::Node& a_node);

      public:
        RE::BSFixedString event;

        bool climax;
        TagData tags;
        Transform offset;
        REX::EnumSet<StripParts> strips;
    };

    struct PositionMetaData
    {
        PositionMetaData(std::ifstream& a_stream, uint8_t a_version);
        PositionMetaData(const Legacy::PositionInfo& a_legacyPositionInfo);
        ~PositionMetaData() = default;

        _NODISCARD bool IsHuman() const { return data.IsHuman(); }
        _NODISCARD bool IsMale() const { return data.IsSex(Sex::Male); }
        _NODISCARD bool IsFemale() const { return data.IsSex(Sex::Female); }
        _NODISCARD bool IsFuta() const { return data.IsSex(Sex::Futa); }
        _NODISCARD PapyrusSex GetSexPapyrus() const;

        _NODISCARD bool IsSubmissive() const { return data.IsSubmissive(); }

        _NODISCARD bool CanFillPosition(RE::Actor* a_actor) const;
        _NODISCARD bool CanFillPosition(const PositionMetaData& a_other) const;
        _NODISCARD bool CanFillPosition(const ActorFragment& a_fragment) const;

      public:
        ActorFragment data;
        std::vector<RE::BSFixedString> annotations{};
    };
}
