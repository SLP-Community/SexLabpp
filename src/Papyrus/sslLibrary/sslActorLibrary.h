#pragma once

namespace Papyrus::ActorLibrary
{
    int32_t ValidateActorImpl(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);

    void WriteStrip(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESForm* a_form, bool a_neverstrip);
    void EraseStrip(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESForm* a_form);
    void EraseStripAll(RE::StaticFunctionTag*);
    int32_t CheckStrip(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::TESForm* a_form);

    std::vector<RE::TESForm*> UnequipSlots(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_reference, uint32_t a_slotmask);

    bool HasVehicle(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor);

    RE::BSFixedString PickRandomFxSet(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, int32_t a_type);
    int32_t GetFxSetCount(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, int32_t a_type, RE::BSFixedString asSet);

    inline bool Register(VM* a_vm)
    {
        REGISTERFUNC(ValidateActorImpl, "sslActorLibrary", true);

        REGISTERFUNC(WriteStrip, "sslActorLibrary", true);
        REGISTERFUNC(EraseStrip, "sslActorLibrary", true);
        REGISTERFUNC(EraseStripAll, "sslActorLibrary", true);
        REGISTERFUNC(CheckStrip, "sslActorLibrary", true);

        REGISTERFUNC(UnequipSlots, "sslActorLibrary", false);

        REGISTERFUNC(HasVehicle, "sslActorLibrary", true);

        REGISTERFUNC(PickRandomFxSet, "sslActorLibrary", true);
        REGISTERFUNC(GetFxSetCount, "sslActorLibrary", true);

        return true;
    }

}  // namespace Papyrus::ActorLibrary
