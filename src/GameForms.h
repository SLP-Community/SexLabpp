#pragma once

namespace GameForms
{
#define LOOKUPMOD(form, formid, mod)                                                                               \
    form = RE::TESDataHandler::GetSingleton()->LookupForm<std::remove_pointer<decltype(form)>::type>(formid, mod); \
    if (!form) {                                                                                                   \
        logger::critical("Failed to lookup form {} in {}: 0x{:X}", #form, mod, formid);                            \
        return false;                                                                                              \
    }

#define LOOKUPGAME(form, formid)                                                       \
    form = RE::TESForm::LookupByID<std::remove_pointer<decltype(form)>::type>(formid); \
    if (!form) {                                                                       \
        logger::critical("Failed to lookup form {}: 0x{:X}", #form, formid);           \
        return false;                                                                  \
    }

    // Vanilla Forms
    inline RE::BGSKeyword* FurnitureBedRoll;
    inline RE::BGSKeyword* DLC2RieklingMountedKeyword;

    // SexLab Forms
    inline const RE::TESFaction* GenderFaction;
    inline const RE::TESFaction* AnimatingFaction;
    inline const RE::TESFaction* ForbiddenFaction;

    inline const RE::TESQuest* ConfigQuest;

    inline bool LoadData()
    {
        LOOKUPGAME(FurnitureBedRoll, 0xE4AD6);
        LOOKUPMOD(DLC2RieklingMountedKeyword, 0x03A159, "Dragonborn.esm");

        LOOKUPMOD(GenderFaction, 0x043A43, "SexLab.esm");
        LOOKUPMOD(AnimatingFaction, 0xE50F, "SexLab.esm");
        LOOKUPMOD(ForbiddenFaction, 0x049068, "SexLab.esm");
        LOOKUPMOD(ConfigQuest, 0xD62, "SexLab.esm");

        return true;
    }
}  // namespace SLPP
