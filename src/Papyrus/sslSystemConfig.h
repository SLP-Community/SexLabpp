#pragma once

namespace Papyrus::SystemConfig
{
	float GetMinSetupTime(RE::StaticFunctionTag*);

	int GetAnimationCount(RE::StaticFunctionTag*);
	std::vector<float> GetEnjoymentFactors(RE::StaticFunctionTag*);
	float GetEnjoymentFactor(RE::StaticFunctionTag*, int32_t idx);
	std::vector<RE::TESForm*> GetStrippableItems(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_target, bool a_wornonly);

	int GetSettingInt(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting);
	float GetSettingFlt(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting);
	bool GetSettingBool(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting);
	std::string GetSettingStr(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting);
	int GetSettingIntA(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting, size_t n);
	float GetSettingFltA(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting, size_t n);

	void SetSettingInt(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting, int a_value);
	void SetSettingFlt(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting, float a_value);
	void SetSettingBool(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting, bool a_value);
	void SetSettingStr(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting, std::string a_value);
	void SetSettingIntA(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting, int a_value, int n);
	void SetSettingFltA(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting, float a_value, int n);

	inline bool Register(VM* a_vm)
	{
		REGISTERFUNC(GetAnimationCount, "sslSystemConfig", true);
		REGISTERFUNC(GetEnjoymentFactors, "sslSystemConfig", true);
		REGISTERFUNC(GetEnjoymentFactor, "sslSystemConfig", true);
		REGISTERFUNC(GetStrippableItems, "sslSystemConfig", true);

		REGISTERFUNC(GetSettingInt, "sslSystemConfig", true);
		REGISTERFUNC(GetSettingFlt, "sslSystemConfig", true);
		REGISTERFUNC(GetSettingBool, "sslSystemConfig", true);
		REGISTERFUNC(GetSettingStr, "sslSystemConfig", true);
		REGISTERFUNC(GetSettingIntA, "sslSystemConfig", true);
		REGISTERFUNC(GetSettingFltA, "sslSystemConfig", true);

		REGISTERFUNC(SetSettingInt, "sslSystemConfig", true);
		REGISTERFUNC(SetSettingFlt, "sslSystemConfig", true);
		REGISTERFUNC(SetSettingBool, "sslSystemConfig", true);
		REGISTERFUNC(SetSettingStr, "sslSystemConfig", true);
		REGISTERFUNC(SetSettingIntA, "sslSystemConfig", true);
		REGISTERFUNC(SetSettingFltA, "sslSystemConfig", true);

		return true;
	}

} // namespace Papyrus
