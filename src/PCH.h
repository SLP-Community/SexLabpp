#pragma once

#pragma warning(push)
#pragma warning(disable : 4200)
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#pragma warning(pop)

#include <atomic>
#include <glm/glm.hpp>
#include <magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <ranges>
#include <unordered_map>
#include <yaml-cpp/yaml.h>
static_assert(magic_enum::is_magic_enum_supported);

#pragma warning(push)
#ifdef NDEBUG
#include <spdlog/sinks/basic_file_sink.h>
#else
#include <spdlog/sinks/msvc_sink.h>
#endif
#pragma warning(pop)

namespace logger = SKSE::log;
namespace fs = std::filesystem;
using namespace std::literals;

#include "GameForms.h"
#include "UserData/Settings.h"
#include "Util/FormLookup.h"
#include "Util/Misc.h"
#include "Util/Random.h"
#include "Util/Singleton.h"

#define ESPNAME "SexLab.esm"
constexpr auto YAMLPATH{ "Data\\SKSE\\SexLab\\Settings.yaml" };
constexpr auto INIPATH{ "Data\\SKSE\\Plugins\\SexLab.ini" };
constexpr auto MODELPATH{ "Data\\SKSE\\SexLab\\LinearModel.ini" };
constexpr auto MODELDATAPATH{ "Data\\SKSE\\SexLab\\ModelData" };

#define CONFIGPATH(path) "Data\\SKSE\\SexLab\\" path
#define USER_CONFIGS(path) CONFIGPATH("UserData\\") path
#define SCHLONGPATH CONFIGPATH("SchlongsOfSkyrim.yaml")
#define STRIP_PATH USER_CONFIGS("Stripping.yaml")
#define CUM_FX_PATH "Data/Textures/SexLab/CumFx/"

#ifdef SKYRIM_SUPPORT_AE
#define OFFSET(SE, AE) AE
#else
#define OFFSET(SE, AE) SE
#endif

namespace stl
{
	using namespace SKSE::stl;

	constexpr std::uint32_t version_pack(REL::Version a_version) noexcept
	{
		return static_cast<std::uint32_t>(
			(a_version[0] & 0x0FF) << 24u |
			(a_version[1] & 0x0FF) << 16u |
			(a_version[2] & 0xFFF) << 4u |
			(a_version[3] & 0x00F) << 0u);
	}

	template <class T>
	void write_thunk_call(std::uintptr_t a_src)
	{
		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(14);

		T::func = trampoline.write_call<5>(a_src, T::thunk);
	}

	template <class F, class T>
	void write_vfunc()
	{
		REL::Relocation<std::uintptr_t> vtbl{ F::VTABLE[0] };
		T::func = vtbl.write_vfunc(T::size, T::thunk);
	}

	inline bool read_string(SKSE::SerializationInterface* a_intfc, std::string& a_str)
	{
		std::size_t size = 0;
		if (!a_intfc->ReadRecordData(size)) {
			return false;
		}
		a_str.resize(size);
		if (!a_intfc->ReadRecordData(a_str.data(), static_cast<std::uint32_t>(size))) {
			return false;
		}
		a_str.erase(std::find(a_str.cbegin(), a_str.cend(), '\0'), a_str.cend());
		return true;
	}

	template <class S>
	inline bool write_string(SKSE::SerializationInterface* a_intfc, const S& a_str)
	{
		std::size_t size = a_str.length() + 1;
		return a_intfc->WriteRecordData(size) && a_intfc->WriteRecordData(a_str.data(), static_cast<std::uint32_t>(size));
	}
}

namespace Papyrus
{
#define REGISTERFUNC(func, class, dont_delay) a_vm->RegisterFunction(#func##sv, class, func, dont_delay)
#define STATICARGS [[maybe_unused]] VM *a_vm, [[maybe_unused]] RE::VMStackID a_stackID, RE::StaticFunctionTag *
#define QUESTARGS [[maybe_unused]] VM *a_vm, [[maybe_unused]] StackID a_stackID, [[maybe_unused]] RE::TESQuest *a_qst
#define ALIASARGS [[maybe_unused]] VM *a_vm, [[maybe_unused]] StackID a_stackID, [[maybe_unused]] RE::BGSRefAlias *a_alias
#define TRACESTACK(err) a_vm->TraceStack(err, a_stackID)

	using VM = RE::BSScript::IVirtualMachine;
	using StackID = RE::VMStackID;
}

namespace Registry
{
	struct FixedStringCompare
	{
		bool operator()(const RE::BSFixedString& lhs, const RE::BSFixedString& rhs) const
		{
			return strcmp(lhs.data(), rhs.data()) < 0;
		}
	};

	template <typename E>
	constexpr std::vector<E> FlagToComponents(E a_enum)
	{
		using underlying = typename std::underlying_type<E>::type;
		constexpr auto iterations = sizeof(underlying) * 8;
		auto number = static_cast<underlying>(a_enum);
		std::vector<E> ret{};
		for (size_t n = 0; n < iterations; n++) {
			size_t i = 1ULL << n;
			if (number & i) {
				ret.push_back(E(i));
			}
		}
		return ret;
	}

	template <typename E>
	constexpr size_t FlagIndex(E a_enum)
	{
		using underlying = typename std::underlying_type<E>::type;
		constexpr auto iterations = sizeof(underlying) * 8;
		auto number = static_cast<underlying>(a_enum);
		for (size_t n = 0; n < iterations; n++) {
			size_t i = 1ULL << n;
			if (number & i) {
				return n;
			}
		}
		return 0;
	}

	template <typename E>
	constexpr size_t CountFlagSize()
	{
		auto max = static_cast<size_t>(E::Total) - 1, ret = 0;
		while ((1 << ret++) < max) {}
		return ret;
	}
}

template <>
struct std::formatter<RE::BSFixedString> : std::formatter<const char*>
{
	template <typename FormatContext>
	auto format(const RE::BSFixedString& myStr, FormatContext& ctx) const
	{
		return std::formatter<const char*>::format(myStr.data(), ctx);
	}
};

template <>
struct std::formatter<YAML::Mark> : std::formatter<std::string>
{
	template <typename FormatContext>
	auto format(const YAML::Mark& mark, FormatContext& ctx) const
	{
		auto str = std::format("[Ln {}, Col {}]", mark.line + 1, mark.column + 1);
		return std::formatter<std::string>::format(str, ctx);
	}
};

template <typename T>
struct is_std_array : std::false_type {};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename T>
struct is_std_vector : std::false_type {};

template <typename T>
struct is_std_vector<std::vector<T>> : std::true_type {};

template <typename T>
	requires (is_std_vector<T>::value || is_std_array<T>::value)
struct std::formatter<T> : std::formatter<std::string>
{
	template <typename FormatContext>
	auto format(const T& container, FormatContext& ctx) const
	{
		std::string result = "[";
		bool first = true;
		for (const auto& elem : container) {
			if (!first) result += ", ";
			result += std::format("{}", elem);
			first = false;
		}
		result += "]";
		return std::formatter<std::string>::format(result, ctx);
	}
};

#define DLLEXPORT __declspec(dllexport)
