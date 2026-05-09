#include "Voice.h"

#include "Util/StringUtil.h"

namespace Registry
{
    Voice::Voice(const YAML::Node& a_node) :
      name(a_node["Name"].as<std::string>()),
      displayName(a_node["DisplayName"].as<std::string>(""s)),
      enabled(true),
      sex([&]() {
          auto node = a_node["Actor"]["Sex"];
          if (!node.IsDefined())
              return RE::SEXES::kNone;
          auto str = node.as<std::string>();
          Util::ToLower(str);
          return str == "female" ? RE::SEXES::kFemale :
                 str == "male"   ? RE::SEXES::kMale :
                                   RE::SEXES::kNone;
      }()),
      races([&]() -> decltype(races) {
          const auto& node = a_node["Actor"]["Race"];
          if (node.IsScalar())
              return { RaceKey{ node.as<std::string>() } };
          return std::ranges::fold_left(node, decltype(races){}, [](auto acc, auto&& it) {
              acc.emplace_back(it.as<std::string>());
              return acc;
          });
      }()),
      pitch([&]() {
          const auto& node = a_node["Actor"]["Pitch"];
          if (!node.IsDefined())
              return Pitch::Unknown;
          return magic_enum::enum_cast<Pitch>(node.as<std::string>(), magic_enum::case_insensitive).value_or(Pitch::Unknown);
      }()),
      tags([&]() -> decltype(tags) {
          const auto& node = a_node["Tags"];
          return { node.IsScalar() ? std::vector{ node.as<std::string>() } : node.as<std::vector<std::string>>() };
      }()),
      defaultset(a_node),
      extrasets([&]() {
          decltype(extrasets) ret{};
          ret.emplace_back(a_node);
          auto extra = a_node["Extra"];
          if (extra.IsDefined())
              for (auto&& it : extra) {
                  ret.emplace_back(it);
              }
          return ret;
      }())
    {}

    const VoiceSet& Voice::GetApplicableSet(REX::EnumSet<VoiceAnnotation> a_annotation) const
    {
        while (a_annotation != VoiceAnnotation::None) {
            for (auto&& vset : extrasets) {
                if (vset.IsValid(a_annotation)) {
                    return vset;
                }
            }
            auto u = a_annotation.underlying();
            a_annotation = VoiceAnnotation(u & (u - 1));
        }
        return defaultset;
    }

    RE::TESSound* Voice::PickSound(LegacyVoice a_legacysetting) const
    {
        bool maybe_victim = a_legacysetting == LegacyVoice::Medium;
        return maybe_victim && !extrasets.empty() ? extrasets.front().Get(a_legacysetting) : defaultset.Get(a_legacysetting);
    }

    RE::TESSound* Voice::PickSound(uint32_t a_excitement, REX::EnumSet<VoiceAnnotation> a_annotation) const
    {
        return GetApplicableSet(a_annotation).Get(a_excitement);
    }

    RE::TESSound* Voice::PickOrgasmSound(REX::EnumSet<VoiceAnnotation> a_annotation) const
    {
        const auto s = GetApplicableSet(a_annotation);
        return s.GetOrgasm() ? s.GetOrgasm() : s.Get(100);
    }

    void Voice::SaveToFile(std::string_view a_fileLocation) const
    {
        const auto path = std::format("{}\\{}.yaml", a_fileLocation, GetId());
        if (fs::exists(path)) {
            return;
        }
        YAML::Node root{};
        root["Name"] = GetId().data();
        switch (sex) {
        case RE::SEXES::kFemale:
            root["Actor"]["Sex"] = "Female";
            break;
        case RE::SEXES::kMale:
            root["Actor"]["Sex"] = "Male";
            break;
        default:
            root["Actor"]["Sex"] = "Any";
            break;
        }
        if (races.empty()) {
            root["Actor"]["Race"].push_back("Human");
        } else {
            for (auto&& r : races) {
                auto str = r.AsString();
                root["Actor"]["Race"].push_back(str.data());
            }
        }
        for (auto&& t : tags.AsVector()) {
            root["Tags"].push_back(t.data());
        }
        const auto dSet = defaultset.AsYaml();
        root["Voices"] = dSet["Voices"];
        if (dSet["Orgasm"].IsDefined()) {
            root["Orgasm"] = dSet["Orgasm"];
        }
        for (auto&& e : extrasets) {
            root["Extra"].push_back(e.AsYaml());
        }
        std::ofstream fout{ path };
        fout << root;
    }

    void Voice::Save(YAML::Node& a_node) const
    {
        a_node = enabled;
    }

    void Voice::Load(const YAML::Node& a_node)
    {
        if (a_node.IsDefined()) {
            enabled = a_node.as<bool>();
        }
    }


    VoiceSet::VoiceSet(const YAML::Node& a_node)
    {
        auto v = a_node["Voices"];
        if (v.IsMap()) {
            for (auto&& it : v) {
                auto sound = Util::FormFromString<RE::TESSound*>(it.first.as<std::string>());
                data.emplace_back(sound, static_cast<uint8_t>(it.second.as<uint32_t>()));
            }
        } else {
            const auto max = static_cast<float>(v.size());
            for (float i = 0; i < max; i++) {
                auto sound = Util::FormFromString<RE::TESSound*>(v[i].as<std::string>());
                if (!sound)
                    continue;
                data.emplace_back(sound, static_cast<uint8_t>((i / max) * 100.0f));
            }
        }
        if (data.empty()) {
            throw std::exception("Need at least 1 Voice per Set");
        }
        std::sort(data.begin(), data.end(), [](auto& a, auto& b) {
            return a.second < b.second;
        });
        if (auto o = a_node["Orgasm"]; o.IsDefined()) {
            orgasm = Util::FormFromString<RE::TESSound*>(o.as<std::string>());
        }
        auto convec = a_node["Conditions"];
        if (!convec.IsDefined())
            return;
        for (auto&& con : convec) {
            auto key = con.first.as<std::string>();
            auto value = con.second.as<bool>();
            if (!value)
                continue;
            auto annotation = magic_enum::enum_cast<VoiceAnnotation>(key, magic_enum::case_insensitive);
            if (annotation.has_value()) {
                annotations |= annotation.value();
            }
        }
    }

    VoiceSet::VoiceSet(bool a_aslegacyextra) :
      data({ { nullptr, uint8_t(0) }, { nullptr, uint8_t(75) } })
    {
        if (a_aslegacyextra) {
            annotations |= VoiceAnnotation::Submissive;
        }
    }

    RE::TESSound* VoiceSet::Get(uint32_t a_priority) const
    {
        auto count = std::min<size_t>(data.size() - 1, std::numeric_limits<int>::max());
        for (int i = static_cast<int>(count); i >= 0; i--) {
            auto& [voice, value] = data[i];
            if (value <= a_priority)
                return voice;
        }
        return nullptr;
    }

    RE::TESSound* VoiceSet::Get(LegacyVoice a_setting) const
    {
        switch (a_setting) {
        case LegacyVoice::Hot:
            return data.back().first;
        default:
            return data.front().first;
        }
    }

    void VoiceSet::SetSound(bool front, RE::TESSound* a_sound)
    {
        auto& obj = front ? data.front() : data.back();
        obj.first = a_sound;
    }

    YAML::Node VoiceSet::AsYaml() const
    {
        YAML::Node ret{};
        for (auto&& [v, prio] : data) {
            if (!v)
                continue;
            auto key = Util::FormToString(v);
            if (ret["Voices"][key].IsDefined())
                continue;
            ret["Voices"][key] = static_cast<int32_t>(prio);
        }
        if (orgasm) {
            ret["Orgasm"] = Util::FormToString(orgasm);
        }
        const auto components = FlagToComponents(annotations.get());
        for (auto&& c : components) {
            const auto name = magic_enum::enum_name(c);
            ret["Conditions"][name.data()] = true;
        }
        return ret;
    }

}  // namespace Registry
