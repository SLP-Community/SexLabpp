#include "RaceKey.h"

#include "Registry/Util/Scale.h"
#include "Util/StringUtil.h"

namespace Registry
{
    static inline const std::map<RaceKey::Value, RE::BSFixedString> LegacyRaceKeys = {
        { RaceKey::Human, "Humans" },
        { RaceKey::AshHopper, "AshHoppers" },
        { RaceKey::Bear, "Bears" },
        { RaceKey::BoarAny, "BoarsAny" },
        { RaceKey::BoarMounted, "BoarsMounted" },
        { RaceKey::BoarSingle, "Boars" },
        { RaceKey::Canine, "Canines" },
        { RaceKey::Chaurus, "Chaurus" },
        { RaceKey::ChaurusHunter, "ChaurusHunters" },
        { RaceKey::ChaurusReaper, "ChaurusReapers" },
        { RaceKey::Chicken, "Chickens" },
        { RaceKey::Cow, "Cows" },
        { RaceKey::Deer, "Deers" },
        { RaceKey::Dog, "Dogs" },
        { RaceKey::Dragon, "Dragons" },
        { RaceKey::DragonPriest, "DragonPriests" },
        { RaceKey::Draugr, "Draugrs" },
        { RaceKey::DwarvenBallista, "DwarvenBallistas" },
        { RaceKey::DwarvenCenturion, "DwarvenCenturions" },
        { RaceKey::DwarvenSphere, "DwarvenSpheres" },
        { RaceKey::DwarvenSpider, "DwarvenSpiders" },
        { RaceKey::Falmer, "Falmers" },
        { RaceKey::FlameAtronach, "FlameAtronach" },
        { RaceKey::Fox, "Foxes" },
        { RaceKey::FrostAtronach, "FrostAtronach" },
        { RaceKey::Gargoyle, "Gargoyles" },
        { RaceKey::Giant, "Giants" },
        { RaceKey::GiantSpider, "GiantSpiders" },
        { RaceKey::Goat, "Goats" },
        { RaceKey::Hagraven, "Hagravens" },
        { RaceKey::Hare, "Rabbits" },
        { RaceKey::Horker, "Horkers" },
        { RaceKey::Horse, "Horses" },
        { RaceKey::IceWraith, "IceWraiths" },
        { RaceKey::LargeSpider, "LargeSpiders" },
        { RaceKey::Lurker, "Lurkers" },
        { RaceKey::Mammoth, "Mammoths" },
        { RaceKey::Mudcrab, "Mudcrabs" },
        { RaceKey::Netch, "Netches" },
        { RaceKey::Riekling, "Rieklings" },
        { RaceKey::Sabrecat, "Sabrecats" },
        { RaceKey::Seeker, "Seekers" },
        { RaceKey::Skeever, "Skeevers" },
        { RaceKey::Slaughterfish, "Slaughterfishes" },
        { RaceKey::Spider, "Spiders" },
        { RaceKey::Spriggan, "Spriggans" },
        { RaceKey::StormAtronach, "StormAtronach" },
        { RaceKey::Troll, "Trolls" },
        { RaceKey::VampireLord, "VampireLords" },
        { RaceKey::Werewolf, "Werewolves" },
        { RaceKey::Wisp, "Wisps" },
        { RaceKey::Wispmother, "Wispmothers" },
        { RaceKey::Wolf, "Wolves" },
    };

    RaceKey::RaceKey(RE::Actor* a_actor) :
      RaceKey(a_actor->GetRace(), [&]() { auto base = a_actor->GetActorBase(); return base ? base->GetSex() : RE::SEXES::kMale; }()) {}

    RaceKey::RaceKey(const RE::BSFixedString& a_raceStr) :
      value(magic_enum::enum_cast<Value>(a_raceStr, magic_enum::case_insensitive).value_or(Value::None))
    {
        if (value != Value::None)
            return;
        const auto where = std::ranges::find_if(LegacyRaceKeys, [&](const auto& pair) { return pair.second == a_raceStr; });
        value = (where == LegacyRaceKeys.end()) ? Value::None : where->first;
    }

    RaceKey::RaceKey(const RE::TESRace* a_race, RE::SEXES::SEX a_sex)
    {
        const std::string_view rootTMP{ a_race->rootBehaviorGraphNames[a_sex].data() };
        const auto root{ rootTMP.substr(rootTMP.rfind('\\') + 1) };
        static const std::map<std::string_view, RaceKey> behaviorfiles{
            { "0_Master.hkx", Human },
            { "WolfBehavior.hkx", Wolf },
            { "DogBehavior.hkx", Dog },
            { "ChickenBehavior.hkx", Chicken },
            { "HareBehavior.hkx", Hare },
            { "AtronachFlameBehavior.hkx", FlameAtronach },
            { "AtronachFrostBehavior.hkx", FrostAtronach },
            { "AtronachStormBehavior.hkx", StormAtronach },
            { "BearBehavior.hkx", Bear },
            { "ChaurusBehavior.hkx", Chaurus },
            { "H-CowBehavior.hkx", Cow },
            { "DeerBehavior.hkx", Deer },
            { "CHaurusFlyerBehavior.hkx", ChaurusHunter },
            { "VampireBruteBehavior.hkx", Gargoyle },
            { "BenthicLurkerBehavior.hkx", Lurker },
            { "BoarBehavior.hkx", BoarAny },
            { "BCBehavior.hkx", DwarvenBallista },
            { "HMDaedra.hkx", Seeker },
            { "NetchBehavior.hkx", Netch },
            { "RieklingBehavior.hkx", Riekling },
            { "ScribBehavior.hkx", AshHopper },
            { "DragonBehavior.hkx", Dragon },
            { "Dragon_Priest.hkx", DragonPriest },
            { "DraugrBehavior.hkx", Draugr },
            { "SCBehavior.hkx", DwarvenSphere },
            { "DwarvenSpiderBehavior.hkx", DwarvenSpider },
            { "SteamBehavior.hkx", DwarvenCenturion },
            { "FalmerBehavior.hkx", Falmer },
            { "FrostbiteSpiderBehavior.hkx", Spider },
            { "GiantBehavior.hkx", Giant },
            { "GoatBehavior.hkx", Goat },
            { "HavgravenBehavior.hkx", Hagraven },
            { "HorkerBehavior.hkx", Horker },
            { "HorseBehavior.hkx", Horse },
            { "IceWraithBehavior.hkx", IceWraith },
            { "MammothBehavior.hkx", Mammoth },
            { "MudcrabBehavior.hkx", Mudcrab },
            { "SabreCatBehavior.hkx", Sabrecat },
            { "SkeeverBehavior.hkx", Skeever },
            { "SlaughterfishBehavior.hkx", Slaughterfish },
            { "SprigganBehavior.hkx", Spriggan },
            { "TrollBehavior.hkx", Troll },
            { "VampireLord.hkx", VampireLord },
            { "WerewolfBehavior.hkx", Werewolf },
            { "WispBehavior.hkx", Wispmother },
            { "WitchlightBehavior.hkx", Wisp },
        };
        const auto where = behaviorfiles.find(root);
        if (where == behaviorfiles.end()) {
            logger::error("Unrecognized Behavior: {} (Used by Race {:X})", root, a_race->GetFormID());
            value = Value::None;
            return;
        }
        const auto editorId = Util::CastLower(std::string{ a_race->formEditorID });
        switch (where->second) {
        case Value::BoarAny:
            if (a_race->HasKeyword(GameForms::DLC2RieklingMountedKeyword)) {
                value = Value::BoarMounted;
            } else {
                value = Value::BoarSingle;
            }
            break;
        case Value::Chaurus:
            if (editorId.find("reaper") != std::string::npos) {
                value = Value::ChaurusReaper;
            } else {
                value = Value::Chaurus;
            }
            break;
        case Value::Spider:
            if (editorId.find("giant") != std::string::npos) {
                value = Value::GiantSpider;
            } else if (editorId.find("large") != std::string::npos) {
                value = Value::LargeSpider;
            } else {
                value = Value::Spider;
            }
            break;
        case Value::Wolf:
            if (editorId.find("fox") != std::string::npos) {
                value = Value::Fox;
            } else {
                value = Value::Wolf;
            }
            break;
        default:
            value = where->second;
            break;
        }
        logger::info("Race: {} {:X}, Havok Behavior Id: {} => {}", a_race->formEditorID, a_race->formID, std::to_underlying(where->second.value), std::to_underlying(value));
    }

    RE::BSFixedString RaceKey::AsString() const
    {
        const auto where = LegacyRaceKeys.find(value);
        return where != LegacyRaceKeys.end() ? where->second : "";
    }

    bool RaceKey::IsCompatibleWith(RaceKey a_other) const
    {
        switch (value) {
        case Value::Canine:
            return a_other.IsAnyOf(Canine, Dog, Wolf);
        case Value::BoarAny:
            return a_other.IsAnyOf(BoarAny, BoarSingle, BoarMounted);
        case Value::Dog:
        case Value::Wolf:
            return a_other.IsAnyOf(Canine, value);
        case Value::BoarSingle:
        case Value::BoarMounted:
            return a_other.IsAnyOf(BoarAny, value);
        default:
            return a_other.value == value;
        }
    }

    RaceKey RaceKey::GetMetaRace() const
    {
        switch (value) {
        case Value::Wolf:
        case Value::Dog:
            return Value::Canine;
        case Value::BoarMounted:
        case Value::BoarSingle:
            return Value::BoarAny;
        default:
            return Value::None;
        }
    }

    std::vector<RE::BSFixedString> RaceKey::GetAllRaceKeys(bool a_ignoreAmbiguous)
    {
        std::vector<RE::BSFixedString> ret;
        for (auto [enumVal, name] : LegacyRaceKeys) {
            if (a_ignoreAmbiguous && (enumVal == Value::BoarAny || enumVal == Value::Canine)) {
                continue;
            }
            ret.push_back(name);
        }
        return ret;
    }

}  // namespace SexLab::RaceKey
