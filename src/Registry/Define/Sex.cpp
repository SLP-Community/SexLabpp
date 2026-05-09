#include "Sex.h"

#include "Util/StringUtil.h"

namespace Registry
{
    Sex GetSex(RE::Actor* a_actor, bool a_skipfactions)
    {
        Sex ret = Sex::None;
        if (!a_skipfactions) {
            a_actor->VisitFactions([&](auto a_faction, auto a_rank) {
                if (a_faction == GameForms::GenderFaction) {
                    switch (a_rank) {
                    case 0:
                        ret = Sex::Male;
                        break;
                    case 1:
                        ret = Sex::Female;
                        break;
                    case 2:
                        ret = Sex::Futa;
                        break;
                    default:
                        logger::info("Actor {} has invalid gender faction rank ({})", a_actor->GetFormID(), a_rank);
                        break;
                    }
                    return true;
                }
                return false;
            });
            if (ret != Sex::None) {
                return ret;
            }
        }

        const auto base = a_actor->GetActorBase();
        if (!base) {
            logger::error("Unable to retrieve actor base for actor {:X}", a_actor->formID);
            return Sex::None;
        }
        switch (base->GetSex()) {
        default:
        case RE::SEXES::kMale:
            return Sex::Male;
        case RE::SEXES::kFemale:
            if (!a_actor->IsHumanoid()) {
                return Settings::bCreatureGender ? Sex::Female : Sex::Male;
            }
            return IsFuta(a_actor) ? Sex::Futa : Sex::Female;
        }
    }

    bool IsFuta(RE::Actor* a_actor)
    {
        static const auto tngkeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("TNG_SkinWithPenis");
        if (tngkeyword) {
            if (auto skin = a_actor->GetSkin(); skin && skin->HasKeyword(tngkeyword)) {
                return true;
            }
        }

        static const auto sosfaction = RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESFaction>(0x00AFF8, "Schlongs of Skyrim.esp");
        if (sosfaction) {
            bool ret = false;
            a_actor->VisitFactions([&ret](RE::TESFaction* a_faction, int8_t a_rank) -> bool {
                if (!a_faction || a_rank < 0)
                    return false;

                if (a_faction == sosfaction) {
                    ret = true;
                    return false;
                } else if (std::ranges::contains(Settings::SOS_ExcludeFactions, a_faction->formID)) {
                    ret = false;
                    return true;
                } else if (std::string name{ a_faction->GetFullName() }; !name.empty()) {
                    Util::ToLower(name);
                    if (name.find("pubic") != std::string::npos) {
                        ret = false;
                        return true;
                    }
                }
                return false;
            });
            return ret;
        }
        return false;
    }

}  // namespace Registry
