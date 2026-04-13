#pragma once
#include "PrismaUtil.h"

namespace Thread::PrismaUI
{
	class EnjoymentBars
	{
	public:
		static bool Initialize();

		static void InitAndShow(Script::ObjectPtr a_scriptObj, const std::vector<RE::Actor*>& a_positions);
		static void HideAndClear();
		static void ToggleEnjoymentBars();

		static void UpdateSlider(RE::Actor* a_actor, float a_enjoyment, RE::BSFixedString a_interactions);
		static void UpdateHighlightedPartner(RE::Actor* a_partner);

		static void RegisterRaiseEnjAttempt(RE::Actor* a_actor, float a_nextTimeCycle);

	private:
		static inline constexpr std::string_view FILEPATH{ "SexLab\\EnjoymentBars.html" };
		static inline PrismaView view{ 0 };
		static inline std::array<std::string, 2> s_result{};
		static inline Script::ObjectPtr s_threadScript{ nullptr };

		static bool IsVisible();
		static void OnRaiseEnjAttemptResult(bool a_success);
	};
}
