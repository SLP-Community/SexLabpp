#include "SLToolsMenu.h"

namespace Thread::PrismaUI
{
	bool SLToolsMenu::Initialize()
	{
		if (!Thread::PrismaUI::IsAvailable()) {
			Thread::PrismaUI::Initialize();
			PrismaAPI = Thread::PrismaUI::GetAPI();
			if (!PrismaAPI) {
				logger::error("SLToolsMenu::Initialize >> FAILED - PrismaUI unavailable");
				return false;
			}
		}
		if (!PrismaAPI->IsValid(view)) {
			view = Thread::PrismaUI::CreateView(FILEPATH.data());
			if (!view) {
				logger::error("SLToolsMenu::Initialize >> FAILED - CreateView returned null");
				return false;
			}

			PrismaAPI->RegisterJSListener(view, "OnSceneSelected", [](const char* data) {
				s_result[0] = "OnSceneSelected";
				HandleSelection(data ? data : "");
			});
			PrismaAPI->RegisterJSListener(view, "OnOffsetModeSelected", [](const char* data) {
				s_result[0] = "OnOffsetModeSelected";
				HandleSelection(data ? data : "");
			});
			PrismaAPI->RegisterJSListener(view, "OnSceneResetBySearch", [](const char* data) {
				s_result[0] = "OnSceneResetBySearch";
				HandleSelection(data ? data : "");
			});
			PrismaAPI->RegisterJSListener(view, "OnMenuClosed", []([[maybe_unused]] const char*) {
				logger::info("SLToolsMenu >> OnMenuClosed");
				HandleSelection("");
			});
			logger::info("SLToolsMenu >> Initialized successfully");
		}
		return true;
	}

	void SLToolsMenu::Open(Script::ObjectPtr a_scriptObj, RE::BSFixedString activeSceneName, const std::vector<RE::BSFixedString>& playingScenesNames)
	{
		if (!PrismaAPI || !PrismaAPI->IsValid(view)) return;
		s_threadScript = a_scriptObj;
	
		std::string json = "{\"activeScene\":\"" + Thread::PrismaUI::JsonEscape(activeSceneName.c_str()) + "\",\"scenes\":[";
		for (size_t i = 0; i < playingScenesNames.size(); i++) {
			if (i > 0) json += ',';
			json += '"';
			json += Thread::PrismaUI::JsonEscape(playingScenesNames[i].c_str());
			json += '"';
		}
		json += "]}";

		if (PrismaAPI->IsValid(view)) {
			PrismaAPI->Invoke(view, ("populateScenes('" + json + "')").c_str());
			// Reset
			s_result[0].clear();
			s_result[1].clear();
			// Show
			PrismaAPI->Show(view);
			PrismaAPI->Focus(view, false); // second arg -> pauseGame
		}
	}

	void SLToolsMenu::HandleSelection(std::string s_select)
	{
		if (!PrismaAPI->IsValid(view)) return;
		s_result[1] = std::move(s_select);
		PrismaAPI->Hide(view);

		if (s_result[1] == "") return;
		if (!s_threadScript) {
			logger::info("SLToolsMenu >> HandleSelection: s_threadScript is null!");
			return;
		}
		logger::info("SLToolsMenu >> Invoking OnPrismaMenuEvent: {} {}", s_result[0], s_result[1]);
		Script::CallbackPtr callbackPtr{};
		Script::DispatchMethodCall(s_threadScript, "OnPrismaMenuEvent", callbackPtr,
			RE::BSFixedString{ s_result[0] }, RE::BSFixedString{ s_result[1] } );
		s_threadScript = nullptr;
	}
}