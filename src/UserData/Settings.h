#pragma once

struct StringCmp
{
	bool operator()(const std::string& a_lhs, const std::string& a_rhs) const { return _strcmpi(a_lhs.c_str(), a_rhs.c_str()) < 0; }
};

struct Settings
{
	static void Initialize();
	static void Save();

	// --- Key Codes
	enum class KeyType
	{
		None,
		Up,
		Down,
		Left,
		Right,
		Select,
		End,
		Extra2,
		Modes,
		Reset,
	};
	static KeyType GetKeyType(uint32_t a_keyCode);
	static uint32_t GetKeyCode(KeyType a_keyType);

	// --- MCM
	enum class FurnitureSlection
	{
		Never = 0,
		Sometimes = 1,
		Always = 2,
		AskAlways = 3,
		IfNotSubmissive = 4,
	};

#define MCM_SETTING(STR, DEFAULT) static inline decltype(DEFAULT) STR{ DEFAULT };
#include "mcm.def"
#undef MCM_SETTING

	using SettingsVariants = std::variant<float*, std::string*, bool*, int*, std::vector<float>*, std::vector<int>*>;
	static inline std::map<std::string, SettingsVariants, StringCmp> table{
#define MCM_SETTING(STR, DEFAULT) { #STR##s, &STR },
#include "mcm.def"
#undef MCM_SETTING
	};

	// --- INI
#define INI_SETTING(STR, DEFAULT, CAT) static inline decltype(DEFAULT) STR{ DEFAULT };
#include "config.def"
#undef INI_SETTING

	// --- Misc
	static inline std::vector<RE::FormID> SOS_ExcludeFactions{};

private:
	static void InitializeYAML();
	static void InitializeINI();
	static void InitializeData();
};
