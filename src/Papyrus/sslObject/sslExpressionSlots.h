#pragma once

namespace Papyrus::ExpressionSlots
{
    enum ExpressionStatus
    {
        None = 0,
        Submissive = 1,
        Dominant = 2
    };

    namespace BaseExpression
    {
        float GetModifier(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* ActorRef, uint32_t a_id);
        float GetPhoneme(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* ActorRef, uint32_t a_id);
        float GetExpression(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* ActorRef, bool a_getid);

        int GetVersion(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id);
        std::vector<RE::BSFixedString> GetExpressionTags(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id);
        void SetExpressionTags(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id, std::vector<RE::BSFixedString> a_newtags);
        bool GetEnabled(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id);
        void SetEnabled(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id, bool a_enabled);
        int GetExpressionScaleMode(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id);
        void SetExpressionScaleMode(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id, int a_idx);
        std::vector<int32_t> GetLevelCounts(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id);
        std::vector<float> GetValues(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id, bool a_female, float a_strength);
        std::vector<float> GetNthValues(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id, bool a_female, int n);
        void SetValues(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id, bool a_female, int a_level, std::vector<float> a_values);

        bool CreateEmptyProfile(RE::StaticFunctionTag*, RE::BSFixedString a_id);
        void SaveExpression(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::BSFixedString a_id);

        inline bool Register(VM* a_vm)
        {
            REGISTERFUNC(GetModifier, "sslBaseExpression", true);
            REGISTERFUNC(GetPhoneme, "sslBaseExpression", true);
            REGISTERFUNC(GetExpression, "sslBaseExpression", true);

            REGISTERFUNC(GetVersion, "sslBaseExpression", true);
            REGISTERFUNC(GetExpressionTags, "sslBaseExpression", true);
            REGISTERFUNC(SetExpressionTags, "sslBaseExpression", true);
            REGISTERFUNC(GetEnabled, "sslBaseExpression", true);
            REGISTERFUNC(SetEnabled, "sslBaseExpression", true);
            REGISTERFUNC(GetExpressionScaleMode, "sslBaseExpression", true);
            REGISTERFUNC(SetExpressionScaleMode, "sslBaseExpression", true);
            REGISTERFUNC(GetLevelCounts, "sslBaseExpression", true);
            REGISTERFUNC(GetValues, "sslBaseExpression", true);
            REGISTERFUNC(GetNthValues, "sslBaseExpression", true);
            REGISTERFUNC(SetValues, "sslBaseExpression", true);

            REGISTERFUNC(CreateEmptyProfile, "sslBaseExpression", true);
            REGISTERFUNC(SaveExpression, "sslBaseExpression", true);

            return true;
        }
    }  // namespace BaseExpression

    std::vector<RE::BSFixedString> GetAllProfileIDs(RE::StaticFunctionTag*);
    std::vector<RE::BSFixedString> GetExpressionsByStatus(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, int a_status);
    std::vector<RE::BSFixedString> GetExpressionsByTags(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_actor, std::string_view a_tags);

    inline bool Register(VM* a_vm)
    {
        REGISTERFUNC(GetAllProfileIDs, "sslExpressionSlots", true);
        REGISTERFUNC(GetExpressionsByStatus, "sslExpressionSlots", true);
        REGISTERFUNC(GetExpressionsByTags, "sslExpressionSlots", true);

        return BaseExpression::Register(a_vm);
    }
}  // namespace Papyrus::ExpressionSlots
