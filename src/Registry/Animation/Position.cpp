#include "Position.h"

#include "Registry/Animation/Legacy/Animation.h"
#include "Registry/Define/RaceKey.h"
#include "Registry/Util/Decode.h"

namespace Registry::Animation
{
    Position::Position(std::ifstream& a_stream, uint8_t a_version) :
      event(Decode::Read<RE::BSFixedString>(a_stream)),
      climax(Decode::Read<uint8_t>(a_stream) > 0),
      tags(),
      offset(Transform(a_stream)),
      strips(static_cast<StripParts>(Decode::Read<uint8_t>(a_stream)))
    {
        if (a_version == 3) {
            Decode::Read<int8_t>(a_stream);
        }
        if (a_version >= 4) {
            const auto extraCustom = Decode::Read<uint64_t>(a_stream);
            for (uint64_t i = 0; i < extraCustom; i++) {
                RE::BSFixedString tag;
                Decode::Read(a_stream, tag);
                tags.AddTag(tag);
            }
        }
    }

    Position::Position(const Legacy::Position& a_legacyPosition) :
      event(a_legacyPosition.event),
      climax(a_legacyPosition.climax),
      tags(a_legacyPosition.tags),
      offset(a_legacyPosition.offset),
      strips(static_cast<StripParts>(a_legacyPosition.strips.underlying()))
    {}

    PositionMetaData::PositionMetaData(std::ifstream& a_stream, uint8_t a_version)
    {
        enum Extra : uint8_t
        {
            Submissive = 1 << 0,
            Vampire = 1 << 1,
            Unconscious = 1 << 2
        };

        RaceKey race;
        REX::EnumSet<Sex> sex;
        float scale;
        REX::EnumSet<Extra> extra;

        a_stream.read(reinterpret_cast<char*>(&race), 1);
        a_stream.read(reinterpret_cast<char*>(&sex), 1);
        Decode::Read(a_stream, scale);
        a_stream.read(reinterpret_cast<char*>(&extra), 1);

        data = ActorFragment(
            sex,
            race,
            scale,
            extra.all(Extra::Vampire),
            extra.all(Extra::Submissive),
            extra.all(Extra::Unconscious));

        if (a_version > 1 && a_version < 4) {
            const auto extraCustom = Decode::Read<uint64_t>(a_stream);
            for (uint64_t i = 0; i < extraCustom; i++) {
                Decode::Read<RE::BSFixedString>(a_stream);
            }
        }
    }

    PositionMetaData::PositionMetaData(const Legacy::PositionInfo& a_legacyPositionInfo) :
      data(a_legacyPositionInfo.data)
    {}

    void Position::Save(YAML::Node& a_node) const
    {
        auto annotationsNode = a_node["annotations"];
        auto transformNode = a_node["transform"];
        offset.Save(transformNode);
        tags.Save(annotationsNode);
    }

    void Position::Load(const YAML::Node& a_node)
    {
        if (auto transform = a_node["transform"]; transform.IsDefined()) {
            offset.Load(transform);
        }
        if (auto annotations = a_node["annotations"]; annotations.IsDefined()) {
            tags.Load(annotations);
        }
    }

    bool PositionMetaData::CanFillPosition(RE::Actor* a_actor) const
    {
        auto fragment = ActorFragment(a_actor, false);
        return CanFillPosition(fragment);
    }

    bool PositionMetaData::CanFillPosition(const ActorFragment& a_fragment) const
    {
        return data.GetCompatibilityScore(a_fragment) != 0;
    }

    bool PositionMetaData::CanFillPosition(const PositionMetaData& a_other) const
    {
        return CanFillPosition(a_other.data);
    }

    PapyrusSex PositionMetaData::GetSexPapyrus() const
    {
        auto sex = data.GetSex();
        REX::EnumSet<PapyrusSex> ret{ PapyrusSex::None };
        if (data.IsHuman()) {
#define SET_SEX(s)       \
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
}