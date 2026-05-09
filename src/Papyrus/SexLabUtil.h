#pragma once

namespace Papyrus::SexLabUtil
{
    bool HasKeywordSub(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESForm* a_form, std::string_view a_substring)
    {
        if (!a_form) {
            a_vm->TraceStack("Form is none", a_stackID);
            return false;
        }
        const auto& kwd = a_form->As<RE::BGSKeywordForm>();
        return kwd && kwd->ContainsKeywordString(a_substring);
    }
    RE::BSFixedString RemoveSubString(RE::StaticFunctionTag*, std::string a_str, std::string a_substring)
    {
        const auto where = a_str.find(a_substring);
        if (where == std::string::npos)
            return a_str;
        a_str.erase(where, a_substring.length());
        return a_str;
    }
    void PrintConsole(RE::StaticFunctionTag*, std::string a_str)
    {
        Util::PrintConsole(a_str);
    }

    int IntMinMaxIndex(RE::StaticFunctionTag*, std::vector<int> arr, bool findHighestValue)
    {
        if (arr.empty()) {
            return 0;
        }
        size_t w = 0;
        int m = 0;
        for (size_t i = 0; i < arr.size(); i++) {
            if (findHighestValue && arr[i] > m || !findHighestValue && arr[i] < m) {
                w = i;
                m = arr[i];
            }
        }
        return static_cast<int>(w);
    }
    int IntMinMaxValue(RE::StaticFunctionTag*, std::vector<int> arr, bool findHighestValue)
    {
        if (arr.empty()) {
            return 0;
        }
        return arr[IntMinMaxIndex(nullptr, arr, findHighestValue)];
    }
    int FloatMinMaxIndex(RE::StaticFunctionTag*, std::vector<float> arr, bool findHighestValue)
    {
        if (arr.empty()) {
            return 0;
        }
        size_t w = 0;
        float m = 0;
        for (size_t i = 0; i < arr.size(); i++) {
            if (findHighestValue && arr[i] > m || !findHighestValue && arr[i] < m) {
                w = i;
                m = arr[i];
            }
        }
        return static_cast<int>(w);
    }
    float FloatMinMaxValue(RE::StaticFunctionTag*, std::vector<float> arr, bool findHighestValue)
    {
        if (arr.empty()) {
            return 0;
        }
        return arr[FloatMinMaxIndex(nullptr, arr, findHighestValue)];
    }
    std::vector<RE::Actor*> MakeActorArray(RE::StaticFunctionTag*, RE::Actor* a1, RE::Actor* a2, RE::Actor* a3, RE::Actor* a4, RE::Actor* a5)
    {
        std::vector<RE::Actor*> ret{};
        ret.reserve(5);
        if (a1)
            ret.push_back(a1);
        if (a2)
            ret.push_back(a2);
        if (a3)
            ret.push_back(a3);
        if (a4)
            ret.push_back(a4);
        if (a5)
            ret.push_back(a5);
        return ret;
    }

    float GetCurrentGameRealTime(RE::StaticFunctionTag*)
    {
        constexpr auto seconds_per_day = 86400.0f;
        const auto calendar = RE::Calendar::GetSingleton();
        const auto timescale = std::max<float>(1, calendar->GetTimescale());
        return (calendar->GetCurrentGameTime() / timescale) * seconds_per_day;
    }

    std::string GetTranslation(RE::StaticFunctionTag*, std::string a_str)
    {
        std::string ret;
        if (SKSE::Translation::Translate(a_str, ret)) {
            if (ret.empty() || ret == a_str) {
                return "";
            }
            return ret;
        }
        return "";
    }

    bool IsGodModeEnabled(RE::StaticFunctionTag*)
    {
        const auto player = RE::PlayerCharacter::GetSingleton();
        return player->IsGodMode();
    }

    std::vector<RE::BSFixedString> ShuffleStringArray(RE::StaticFunctionTag*, std::vector<RE::BSFixedString> arr, RE::BSFixedString asFirst, int aiMaxLen)
    {
        if (arr.empty())
            return arr;
        bool bFirstSet = false;
        if (!asFirst.empty()) {
            for (int i = 0; i < static_cast<int>(arr.size()); i++) {
                if (arr[i] == asFirst) {
                    std::swap(arr[0], arr[i]);
                    bFirstSet = true;
                    break;
                }
            }
        }
        const int total = static_cast<int>(arr.size());
        int pickLen = aiMaxLen;
        if (pickLen < 1)
            pickLen = 1;
        if (pickLen > total)
            pickLen = total;
        thread_local std::mt19937 rng{ std::random_device{}() };
        const int shuffleStart = bFirstSet ? 1 : 0;
        for (int j = shuffleStart; j < pickLen; j++) {
            std::uniform_int_distribution<int> dist(j, total - 1);
            int k = dist(rng);
            std::swap(arr[j], arr[k]);
        }
        arr.resize(pickLen);
        return arr;
    }

    static constexpr std::array<std::string_view, 21> HUD_ELEMENTS = {
        "HUD Menu", "LootMenu", "TrueHUD", "BTPS Menu", "BTPS Ovelay Menu", "oxygenMeter2", "CastingBar", "MiniMapMenu",
        "Floating Damage V2", "Durability Menu", "KNNWidgetMeter", "KNNWidgetMeterOp", "lvlWidget", "goldWidget",
        "gametimeWidget", "shoutWidget", "resistWidget", "playtimeWidget", "weightWidget", "equipWidget_STB", "STBActiveEffects"
    };

    void HideElementsGameHUD(RE::StaticFunctionTag*, bool a_hide)
    {
        SKSE::GetTaskInterface()->AddUITask([a_hide]() {
            RE::GFxValue alphaValue{ a_hide ? 0.0 : 100.0 };
            auto ui = RE::UI::GetSingleton();
            for (const auto& name : HUD_ELEMENTS) {
                if (auto uiMovie = ui->GetMovieView(name)) {
                    uiMovie->SetVariable("_root._alpha", alphaValue);
                }
            }
        });
    }

    inline bool Register(VM* a_vm)
    {
        REGISTERFUNC(HasKeywordSub, "SexLabUtil", true);
        REGISTERFUNC(RemoveSubString, "SexLabUtil", true);
        REGISTERFUNC(PrintConsole, "SexLabUtil", true);
        REGISTERFUNC(IntMinMaxIndex, "SexLabUtil", true);
        REGISTERFUNC(IntMinMaxValue, "SexLabUtil", true);
        REGISTERFUNC(FloatMinMaxIndex, "SexLabUtil", true);
        REGISTERFUNC(FloatMinMaxValue, "SexLabUtil", true);
        REGISTERFUNC(MakeActorArray, "SexLabUtil", true);
        REGISTERFUNC(GetCurrentGameRealTime, "SexLabUtil", true);
        REGISTERFUNC(GetTranslation, "SexLabUtil", true);
        REGISTERFUNC(IsGodModeEnabled, "SexLabUtil", true);
        REGISTERFUNC(ShuffleStringArray, "SexLabUtil", true);
        REGISTERFUNC(HideElementsGameHUD, "SexLabUtil", true);

        return true;
    }

}  // namespace Papyrus::SexLabUtil
