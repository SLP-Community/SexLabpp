#pragma once

#include "Registry/Define/Sex.h"

namespace Papyrus
{
    enum LegacySex
    {
        Male = 0,
        Female,
        CrtMale,
        CrtFemale,

        None = -1,
    };

    inline LegacySex GetLegacySex(RE::Actor* a_actor)
    {
        auto creature = !a_actor->IsHumanoid();
        auto sex = Registry::GetSex(a_actor);
        switch (sex) {
        case Registry::Sex::None:
        case Registry::Sex::Male:
            return creature ? CrtMale : Male;
        case Registry::Sex::Female:
        case Registry::Sex::Futa:
            return creature ? (Settings::bCreatureGender ? CrtFemale : CrtMale) : Female;
        }
        return LegacySex::Male;
    }

    inline std::array<int32_t, 4> GetLegacySex(std::vector<RE::Actor*> a_positions)
    {
        std::array<int32_t, 4> ret{ 0, 0, 0, 0 };
        for (auto&& actor : a_positions) {
            auto sex = GetLegacySex(actor);
            ret[sex]++;
        }
        return ret;
    }

}  // namespace Papyrus
