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
        Legs = 1 << 3,
        Arms = 1 << 4,
        Hips = 1 << 5,
        Torso = 1 << 6,
        Default = 1 << 7,

        All = static_cast<std::underlying_type_t<StripParts>>(-1),
    };

    struct Position
    {
        Position(std::ifstream& a_stream, uint8_t a_version);
        Position(const Legacy::Position& a_legacyPosition);
        ~Position() = default;

        void Save(YAML::Node& a_node) const;
        void Load(const YAML::Node& a_node);

        _NODISCARD const RE::BSFixedString& GetEvent() const { return event; }
        _NODISCARD bool IsClimax() const { return climax; }
        _NODISCARD const TagData& GetTags() const { return tags; }
        _NODISCARD const Transform& GetOffset() const { return offset; }
        _NODISCARD REX::EnumSet<StripParts> GetStrips() const { return strips; }

      private:
        RE::BSFixedString event{};

        bool climax{ false };
        TagData tags{};
        Transform offset{};
        REX::EnumSet<StripParts> strips{};
    };

    struct PositionMetaData
    {
        PositionMetaData(std::ifstream& a_stream, uint8_t a_version);
        PositionMetaData(const Legacy::PositionInfo& a_legacyPositionInfo);
        ~PositionMetaData() = default;

        _NODISCARD ActorFragment& get() { return data; }
        _NODISCARD const ActorFragment& get() const { return data; }

        _NODISCARD bool IsHuman() const { return data.IsHuman(); }
        _NODISCARD bool IsMale() const { return data.IsSex(Sex::Male); }
        _NODISCARD bool IsFemale() const { return data.IsSex(Sex::Female); }
        _NODISCARD bool IsFuta() const { return data.IsSex(Sex::Futa); }
        _NODISCARD PapyrusSex GetSexPapyrus() const;

        _NODISCARD bool IsSubmissive() const { return data.IsSubmissive(); }

        _NODISCARD bool CanFillPosition(RE::Actor* a_actor) const;
        _NODISCARD bool CanFillPosition(const PositionMetaData& a_other) const;
        _NODISCARD bool CanFillPosition(const ActorFragment& a_fragment) const;

      private:
        ActorFragment data{};
    };
}
