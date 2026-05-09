#pragma once

#include <shared_mutex>

#include "Animation.h"
#include "RaceKey.h"

namespace Registry
{
    enum class LegacyVoice
    {
        Hot,
        Mild,
        Medium
    };

    enum class VoiceAnnotation
    {
        None = 0,
        Submissive = 1 << 0,
        Dominant = 1 << 1,
        Muffled = 1 << 7,
    };

    enum class Pitch
    {
        Unknown,
        Normal,
        High,
        Low,
    };

    struct VoiceSet
    {
        VoiceSet(const YAML::Node& a_node);
        VoiceSet(bool a_aslegacyextra);
        ~VoiceSet() = default;

        bool IsValid(REX::EnumSet<VoiceAnnotation> a_annotations) const { return annotations == a_annotations; }
        RE::TESSound* GetOrgasm() const { return orgasm; }
        RE::TESSound* Get(uint32_t a_excitement) const;
        RE::TESSound* Get(LegacyVoice a_setting) const;

      public:
        void SetSound(bool front, RE::TESSound* a_sound);
        YAML::Node AsYaml() const;

      private:
        RE::TESSound* orgasm{ nullptr };
        std::vector<std::pair<RE::TESSound*, uint8_t>> data{};
        REX::EnumSet<VoiceAnnotation> annotations{ VoiceAnnotation::None };
    };

    class Voice
    {
      public:
        Voice(const YAML::Node& a_node);
        Voice(RE::BSFixedString a_name) :
          name(a_name), defaultset(false), extrasets({ { true } }) {}
        ~Voice() = default;

        RE::BSFixedString GetId() const { return name; }
        RE::BSFixedString GetDisplayName() const { return displayName.empty() ? name : displayName; }
        bool HasRace(RaceKey a_race) const
        {
            return std::ranges::find_if(races, [&](const auto& rk) { return a_race.IsCompatibleWith(rk); }) != races.end();
        }
        const VoiceSet& GetApplicableSet(REX::EnumSet<VoiceAnnotation> a_annotation) const;

        RE::TESSound* PickSound(LegacyVoice a_legacysetting) const;
        RE::TESSound* PickSound(uint32_t a_excitement, REX::EnumSet<VoiceAnnotation> a_annotation) const;
        RE::TESSound* PickOrgasmSound(REX::EnumSet<VoiceAnnotation> a_annotation) const;

      public:
        void SaveToFile(std::string_view a_fileLocation) const;
        void Save(YAML::Node& a_node) const;
        void Load(const YAML::Node& a_node);

      public:
        bool operator==(const Voice& a_rhs) const noexcept { return name == a_rhs.name; }
        bool operator!=(const Voice& a_rhs) const noexcept { return name != a_rhs.name; }
        bool operator<(const Voice& a_rhs) const noexcept { return strcmp(name.data(), a_rhs.name.data()) < 0; }

      public:
        RE::BSFixedString name;
        RE::BSFixedString displayName;
        bool enabled{ true };

        TagData tags{};
        RE::SEXES::SEX sex{ RE::SEXES::SEX::kNone };
        std::vector<RaceKey> races{};
        Pitch pitch{ Pitch::Unknown };

        VoiceSet defaultset;
        std::vector<VoiceSet> extrasets{};
    };

    struct VoicePitch
    {
        VoicePitch(const Voice* a_voice) :
          voice(a_voice) {}
        VoicePitch(Pitch a_pitch) :
          pitch(a_pitch) {}
        ~VoicePitch() = default;

        Pitch pitch{ Pitch::Unknown };
        const Voice* voice{ nullptr };
    };

}  // namespace Registry::Voice
