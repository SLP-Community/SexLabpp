#pragma once
#include "PrismaUtil.h"

namespace Thread::PrismaUI
{
	class FurnSelectionMenu
	{
	public:
		struct Item
		{
		private:
			std::string name;
			std::string value;
		public:
			Item(const std::string& a_name, const std::string& a_value) : name(a_name), value(a_value) {}
			const std::string& GetName() const { return name; }
			const std::string& GetValue() const { return value; }
		};

	public:
		static bool Initialize();
		static void Open(RE::TESQuest* a_qst, const std::vector<Item>& a_items);
	
	private:
		static inline constexpr std::string_view FILEPATH{ "SexLab\\FurnSelectionMenu.html" };
		static inline PrismaView view{ 0 };
		static inline std::vector<Item> s_items{};
		static inline RE::TESQuest* s_linkedThread{ nullptr };

		static void HandleSelection(const std::string& data);
		static void HandleClose();
	};

}	// namespace Thread::PrismaUI
