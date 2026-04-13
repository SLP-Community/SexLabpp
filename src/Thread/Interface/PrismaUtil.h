#pragma once
#include "PrismaUI_API.h"
#include "Thread/Thread.h"
#include "Util/Script.h"

namespace Thread::PrismaUI
{
	inline PRISMA_UI_API::IVPrismaUI2* PrismaAPI{ nullptr };
	inline PRISMA_UI_API::IVPrismaUI2* GetAPI() { return PrismaAPI; }
	inline bool IsAvailable() { return PrismaAPI != nullptr; }

	inline bool Initialize()
	{
		PrismaAPI = static_cast<PRISMA_UI_API::IVPrismaUI2*>(PRISMA_UI_API::RequestPluginAPI(PRISMA_UI_API::InterfaceVersion::V2));
		return (PrismaAPI ? true : false);
	}

	inline PrismaView CreateView(const char* htmlPath)
	{
		if (!IsAvailable()) return 0;
		const auto view = PrismaAPI->CreateView(htmlPath);
		if (view) PrismaAPI->Hide(view);
		return view;
	}

	inline std::string JsonEscape(const std::string& s)
	{
		std::string out;
		out.reserve(s.size());
		for (char c : s) {
			if (c == '"') out += "\\\"";
			else if (c == '\\') out += "\\\\";
			else out += c;
		}
		return out;
	}

	class OverlaySuppressor : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
	{
	public:
		static inline constexpr std::array SUPPRESSING_MENUS{
			// menus with pause effects on game
			RE::Console::MENU_NAME, RE::JournalMenu::MENU_NAME, RE::MainMenu::MENU_NAME, RE::TweenMenu::MENU_NAME,
			RE::MapMenu::MENU_NAME, RE::LoadingMenu::MENU_NAME, RE::MessageBoxMenu::MENU_NAME,
			RE::SleepWaitMenu::MENU_NAME, RE::RaceSexMenu::MENU_NAME,
			// menus that might get activated by user interactions
			RE::DialogueMenu::MENU_NAME, RE::BarterMenu::MENU_NAME, RE::ContainerMenu::MENU_NAME, 
			RE::InventoryMenu::MENU_NAME, RE::MagicMenu::MENU_NAME, RE::FavoritesMenu::MENU_NAME,
			RE::GiftMenu::MENU_NAME, RE::FaderMenu::MENU_NAME,
			// other nicities
			RE::LevelUpMenu::MENU_NAME, RE::StatsMenu::MENU_NAME
		};

		static void Register(PrismaView a_view)
		{
			GetInstance().AddView(a_view);
		}

		static void Unregister(PrismaView a_view)
		{
			GetInstance().RemoveView(a_view);
		}

	private:
		struct Entry
		{
			PrismaView view;
			bool suppressedByMenu{ false };
		};

		std::vector<Entry> m_entries;
		std::mutex m_mutex;

		static OverlaySuppressor& GetInstance()
		{
			static OverlaySuppressor instance;
			return instance;
		}

		OverlaySuppressor()
		{
			RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(this);
		}

		void AddView(PrismaView a_view)
		{
			std::lock_guard lock(m_mutex);
			for (auto& e : m_entries)
				if (e.view == a_view) return;
			m_entries.push_back({ a_view });
		}

		void RemoveView(PrismaView a_view)
		{
			std::lock_guard lock(m_mutex);
			std::erase_if(m_entries, [&](const Entry& e) { return e.view == a_view; });
		}

		RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
		{
			if (!a_event || !PrismaAPI)
				return RE::BSEventNotifyControl::kContinue;

			const bool isSuppressing = std::ranges::any_of(SUPPRESSING_MENUS,
				[&](const auto& name) { return a_event->menuName == name; });

			if (!isSuppressing)
				return RE::BSEventNotifyControl::kContinue;

			std::lock_guard lock(m_mutex);
			for (auto& e : m_entries) {
				if (!PrismaAPI->IsValid(e.view)) continue;
				if (a_event->opening) {
					if (!PrismaAPI->IsHidden(e.view)) {
						e.suppressedByMenu = true;
						PrismaAPI->Hide(e.view);
					}
				} else {
					if (e.suppressedByMenu) {
						e.suppressedByMenu = false;
						PrismaAPI->Show(e.view);
					}
				}
			}
			return RE::BSEventNotifyControl::kContinue;
		}
	};
}