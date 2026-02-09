#include "sslSystemConfig.h"

#include "Registry/Library.h"
#include "UserData/StripData.h"

namespace Papyrus::SystemConfig
{
	template <class T, size_t ArgC = -1>
	inline T GetSetting(VM* a_vm, StackID a_stackID, const std::string& a_setting)
	{
		auto w = Settings::table.find(a_setting);
		if (w == Settings::table.end()) {
			const auto err = std::format("Unrecognized setting: {}", a_setting);
			a_vm->TraceStack(err.c_str(), a_stackID);
			return 0;
		} else if (!std::holds_alternative<T>(w->second)) {
			const auto err = std::format("Setting {} is of invalid type, expected {}", a_setting, typeid(T).name());
			a_vm->TraceStack(err.c_str(), a_stackID);
			return 0;
		}
		return std::get<T>(w->second);
	}

	int GetSettingInt(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting)
	{
		auto ret = GetSetting<int*>(a_vm, a_stackID, a_setting);
		return ret ? *ret : 0;
	}

	float GetSettingFlt(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting)
	{
		auto ret = GetSetting<float*>(a_vm, a_stackID, a_setting);
		return ret ? *ret : 0;
	}

	bool GetSettingBool(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting)
	{
		auto ret = GetSetting<bool*>(a_vm, a_stackID, a_setting);
		return ret ? *ret : 0;
	}

	std::string GetSettingStr(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting)
	{
		auto ret = GetSetting<std::string*>(a_vm, a_stackID, a_setting);
		return ret ? *ret : ""s;
	}

	int GetSettingIntA(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting, size_t n)
	{
		auto ret = GetSetting<std::vector<int>*>(a_vm, a_stackID, a_setting);
		if (!ret)
			return 0;
		if (n < 0 || n >= ret->size()) {
			const auto err = std::format("Index out of range: {}/{}", n, ret->size());
			a_vm->TraceStack(err.c_str(), a_stackID);
			return 0;
		}
		return ret->at(n);
	}

	float GetSettingFltA(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting, size_t n)
	{
		auto ret = GetSetting<std::vector<float>*>(a_vm, a_stackID, a_setting);
		if (!ret)
			return 0;
		if (n < 0 || n >= ret->size()) {
			const auto err = std::format("Index out of range: {}/{}", n, ret->size());
			a_vm->TraceStack(err.c_str(), a_stackID);
			return 0;
		}
		return ret->at(n);
	}

	void SetSettingInt(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting, int a_value)
	{
		auto s = GetSetting<int*>(a_vm, a_stackID, a_setting);
		if (!s)
			return;

		*s = a_value;
	}

	void SetSettingFlt(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting, float a_value)
	{
		auto s = GetSetting<float*>(a_vm, a_stackID, a_setting);
		if (!s)
			return;

		*s = a_value;
	}

	void SetSettingBool(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting, bool a_value)
	{
		auto s = GetSetting<bool*>(a_vm, a_stackID, a_setting);
		if (!s)
			return;

		*s = a_value;
	}

	void SetSettingStr(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting, std::string a_value)
	{
		auto s = GetSetting<std::string*>(a_vm, a_stackID, a_setting);
		if (!s)
			return;

		*s = a_value;
	}

	void SetSettingIntA(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting, int a_value, int n)
	{
		auto s = GetSetting<std::vector<int>*>(a_vm, a_stackID, a_setting);
		if (!s)
			return;
		if (n < 0 || n >= s->size()) {
			const auto err = std::format("Index out of range: {}/{}", n, s->size());
			a_vm->TraceStack(err.c_str(), a_stackID);
			return;
		}

		(*s)[n] = a_value;
	}

	void SetSettingFltA(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, std::string a_setting, float a_value, int n)
	{
		auto s = GetSetting<std::vector<float>*>(a_vm, a_stackID, a_setting);
		if (!s)
			return;
		if (n < 0 || n >= s->size()) {
			const auto err = std::format("Index out of range: {}/{}", n, s->size());
			a_vm->TraceStack(err.c_str(), a_stackID);
			return;
		}

		(*s)[n] = a_value;
	}

	int GetAnimationCount(RE::StaticFunctionTag*)
	{
		return static_cast<int32_t>(Registry::Library::GetSingleton()->GetSceneCount());
	}

	static const std::vector<float*> EnjoymentFactorsList = {
		&Settings::f_pStimulation,
		&Settings::f_aAnimObjFace,
		&Settings::f_pAnimObjFace,
		&Settings::f_pSuckingToes,
		&Settings::f_pGrinding,
		&Settings::f_pSkullfuck,
		&Settings::f_aHandJob,
		&Settings::f_aFootJob,
		&Settings::f_aBoobJob,
		&Settings::f_bKissing,
		&Settings::f_aSuckingToes,
		&Settings::f_pFacial,
		&Settings::f_aOral,
		&Settings::f_aLickingShaft,
		&Settings::f_aDeepthroat,
		&Settings::f_pVaginal,
		&Settings::f_pAnal,
		&Settings::f_aFacial,
		&Settings::f_aGrinding,
		&Settings::f_pHandJob,
		&Settings::f_pFootJob,
		&Settings::f_pBoobJob,
		&Settings::f_pLickingShaft,
		&Settings::f_pOral,
		&Settings::f_pDeepthroat,
		&Settings::f_aSkullfuck,
		&Settings::f_aVaginal,
		&Settings::f_aAnal
	};

	std::vector<float> GetEnjoymentFactors(RE::StaticFunctionTag*)
	{
		std::vector<float> values;
		values.reserve(EnjoymentFactorsList.size());
		for (float* ptr : EnjoymentFactorsList) {
				values.push_back(ptr ? *ptr : 0.0f);
		}
		return values;
	}

	float GetEnjoymentFactor(RE::StaticFunctionTag*, int32_t idx)
	{
		if (idx < 0 || static_cast<size_t>(idx) >= EnjoymentFactorsList.size()) {
			return 0.0f; 
		}
		float* ptr = EnjoymentFactorsList[idx];
		if (ptr) {
			return *ptr;
		}
		return 0.0f;
	}

	std::vector<RE::TESForm*> GetStrippableItems(VM* a_vm, StackID a_stackID, RE::StaticFunctionTag*, RE::Actor* a_target, bool a_wornonly)
	{
		if (!a_target) {
			a_vm->TraceStack("Cannot retrieve hdt spell from a none reference", a_stackID);
			return {};
		}
		std::vector<RE::TESForm*> ret{};
		const auto cstrip = UserData::StripData::GetSingleton();
		const auto& inventory = a_target->GetInventory();
		for (const auto& [form, data] : inventory) {
			if (!data.second->IsWorn()) {
				if (a_wornonly)
					continue;
				if (!form->IsArmor() && !form->IsWeapon() && !form->IsAmmo())
					continue;
			}
			if (cstrip->CheckKeywords(form) != UserData::Strip::None)
				continue;

			ret.push_back(form);
		}
		return ret;
	}

}	 // namespace Papyrus
