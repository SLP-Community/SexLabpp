#include "Settings.h"

#include <SimpleIni.h>

void Settings::Initialize()
{
    InitializeYAML();
    InitializeINI();
    InitializeData();
}

template <typename T>
struct is_vector : std::false_type
{};

template <typename T, typename Alloc>
struct is_vector<std::vector<T, Alloc>> : std::true_type
{};

void Settings::InitializeYAML()
{
    if (!fs::exists(YAMLPATH)) {
        logger::error("No Settings file (yaml) in {}", YAMLPATH);
        return;
    }

    try {
        const auto yaml = YAML::LoadFile(YAMLPATH);
        const auto ReadMCM = [&yaml]<typename T>(const char* a_key, T& a_out) {
            if (!yaml[a_key].IsDefined())
                return;
            const auto val = yaml[a_key].as<T>();
            if constexpr (is_vector<std::decay_t<T>>::value) {
                if (a_out.size() != val.size()) {
                    logger::error("Invalid array length for setting {}, expected {} but got {}", a_key, a_out.size(), val.size());
                    return;
                }
            }
            a_out = val;
        };
#define MCM_SETTING(STR, DEFAULT) ReadMCM(#STR, STR);
#include "mcm.def"
#undef MCM_SETTING

        logger::info("Finished loading yaml settings");
    } catch (const std::exception& e) {
        logger::error("Unable to laod settings, error: {}", e.what());
    }
}

void Settings::InitializeINI()
{
    if (!fs::exists(INIPATH)) {
        logger::error("No Settings file (ini) in {}", INIPATH);
        return;
    }
    CSimpleIniA inifile{};
    inifile.SetUnicode();
    const auto ec = inifile.LoadFile(INIPATH);
    if (ec < 0) {
        logger::error("Failed to read .ini Settings, Error: {}", ec);
        return;
    }
    const auto ReadIni = [&inifile]<typename T>(const char* a_section, const char* a_option, T& a_out) {
        if (!inifile.GetValue(a_section, a_option))
            return;
        if constexpr (std::is_integral_v<T>) {
            a_out = static_cast<T>(inifile.GetLongValue(a_section, a_option));
        } else if constexpr (std::is_floating_point_v<T>) {
            a_out = static_cast<T>(inifile.GetDoubleValue(a_section, a_option));
        } else {
            logger::error("Unknown Type for option {} in section {}", a_option, a_section);
        }
    };
#define INI_SETTING(STR, DEFAULT, CAT) ReadIni(CAT, #STR, STR);
#include "config.def"
#undef INI_SETTING

    if (fPercentageHetero + fPercentageHomo > 100) {
        logger::error("Sexuality Percentage Settings must be at most 100.0");
        const auto total = fPercentageHetero + fPercentageHomo;
        fPercentageHetero = (fPercentageHetero / total) * 100;
        fPercentageHomo = (fPercentageHomo / total) * 100;
        logger::info("Adjusted fPercentageHetero to {} and fPercentageHomo to {}", fPercentageHetero, fPercentageHomo);
    }
    logger::info("Finished loading .ini settings");
}

void Settings::InitializeData()
{
    const auto& handler = RE::TESDataHandler::GetSingleton();
    const auto root = YAML::LoadFile(SCHLONGPATH);
    for (auto&& i : root["Blacklist"]) {
        const auto esp = i["ESP"].as<std::string>();
        logger::info("Looking for non-schlongs in esp {}", esp);
        const auto node = i["ID"];
        if (node.IsSequence()) {
            for (auto&& id : node) {
                const auto formid = id.as<uint32_t>();
                const auto fac = handler->LookupFormID(formid, esp);
                if (fac) {
                    logger::info("Adding {} / {}", esp, formid);
                    SOS_ExcludeFactions.push_back(fac);
                }
            }
        } else {  // no sequence, simple entry
            const auto formid = node.as<uint32_t>();
            const auto fac = handler->LookupFormID(formid, esp);
            if (fac) {
                logger::info("Adding {} / {}", esp, formid);
                SOS_ExcludeFactions.push_back(fac);
            }
        }
    }
}

void Settings::Save()
{
    YAML::Node settings{};
#define MCM_SETTING(STR, DEFAULT) settings[#STR] = STR;
#include "mcm.def"
#undef MCM_SETTING

    std::ofstream fout{ YAMLPATH };
    fout << settings;
    logger::info("Finished saving user settings");
}

Settings::KeyType Settings::GetKeyType(uint32_t a_keyCode)
{
    const auto get = [](uint32_t key) {
        return key >= SKSE::InputMap::kMacro_GamepadOffset ? SKSE::InputMap::GamepadKeycodeToMask(key) : key;
    };
    if (a_keyCode == get(Settings::iToggleSceneHUD))      return KeyType::Menu;
    if (a_keyCode == get(Settings::iFocusSceneHUD))       return KeyType::Focus;
    if (a_keyCode == get(Settings::iKeyAdvance))          return KeyType::Advance;
    if (a_keyCode == get(Settings::iKeyEnd))              return KeyType::End;
    if (a_keyCode == get(Settings::iKeyMod))              return KeyType::Modifier;
    if (a_keyCode == get(Settings::iChangeAnimation))     return KeyType::Scene;
    if (a_keyCode == get(Settings::iMoveScene))           return KeyType::Move;
    if (a_keyCode == get(Settings::iChangePositions))     return KeyType::Position;
    if (a_keyCode == get(Settings::iToggleThreadControl)) return KeyType::Thread;
    if (a_keyCode == get(Settings::iToggleFreeCamera))    return KeyType::FreeCam;
    if (a_keyCode == get(Settings::iTargetActor))         return KeyType::Partner;
    if (a_keyCode == get(Settings::iGameRaiseEnjKey))     return KeyType::RaiseEnj;
    if (a_keyCode == get(Settings::iGameHoldbackKey))     return KeyType::Holdback;
    return KeyType::None;
}

uint32_t Settings::GetKeyCode(KeyType a_keyType)
{
    const auto get = [](uint32_t key) {
        return key >= SKSE::InputMap::kMacro_GamepadOffset ? SKSE::InputMap::GamepadKeycodeToMask(key) : key;
    };
    switch (a_keyType) {
    case KeyType::Menu:      return get(Settings::iToggleSceneHUD);
    case KeyType::Focus:     return get(Settings::iFocusSceneHUD);
    case KeyType::Advance:   return get(Settings::iKeyAdvance);
    case KeyType::End:       return get(Settings::iKeyEnd);
    case KeyType::Modifier:  return get(Settings::iKeyMod);
    case KeyType::Scene:     return get(Settings::iChangeAnimation);
    case KeyType::Move:      return get(Settings::iMoveScene);
    case KeyType::Position:  return get(Settings::iChangePositions);
    case KeyType::Thread:    return get(Settings::iToggleThreadControl);
    case KeyType::FreeCam:   return get(Settings::iToggleFreeCamera);
    case KeyType::Partner:   return get(Settings::iTargetActor);
    case KeyType::RaiseEnj:  return get(Settings::iGameRaiseEnjKey);
    case KeyType::Holdback:  return get(Settings::iGameHoldbackKey);
    default:
        logger::warn("GetKeyCode: Invalid KeyType {}", static_cast<int>(a_keyType));
        return 0;  // Return 0 for KeyType::None or invalid KeyType
    }
}
