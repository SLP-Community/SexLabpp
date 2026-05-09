#include "FurnSelectionMenu.h"

namespace Thread::PrismaUI
{
    bool FurnSelectionMenu::Initialize()
    {
        if (!Thread::PrismaUI::IsAvailable()) {
            Thread::PrismaUI::Initialize();
            PrismaAPI = Thread::PrismaUI::GetAPI();
            if (!PrismaAPI) {
                logger::error("FurnSelectionMenu::Initialize >> FAILED - PrismaUI unavailable");
                return false;
            }
        }
        if (!PrismaAPI->IsValid(view)) {
            view = Thread::PrismaUI::CreateView(FILEPATH.data());
            if (!view) {
                logger::error("FurnSelectionMenu::Initialize >> FAILED - CreateView returned null");
                return false;
            }

            PrismaAPI->RegisterJSListener(view, "OnFurnItemSelected", [](const char* data) {
                HandleSelection(data ? data : "");
            });
            PrismaAPI->RegisterJSListener(view, "OnMenuClosed", []([[maybe_unused]] const char*) {
                HandleClose();
            });
            logger::info("FurnSelectionMenu >> Initialized successfully");
        }
        return true;
    }

    void FurnSelectionMenu::Open(RE::TESQuest* a_qst, const std::vector<Item>& a_items)
    {
        if (!PrismaAPI || !PrismaAPI->IsValid(view) || a_items.empty())
            return;
        s_linkedThread = a_qst;
        s_items = a_items;

        std::string json = "{\"items\":[";
        for (size_t i = 0; i < s_items.size(); ++i) {
            if (i > 0)
                json += ',';
            json += "{\"name\":\"";
            json += Thread::PrismaUI::JsonEscape(s_items[i].GetName());
            json += "\",\"type\":\"";
            json += Thread::PrismaUI::JsonEscape(s_items[i].GetValue());
            json += "\",\"index\":";
            json += std::to_string(i);
            json += '}';
        }
        json += "]}";

        if (PrismaAPI->IsValid(view)) {
            PrismaAPI->InteropCall(view, "populateItems", json.c_str());
            // Show
            PrismaAPI->Show(view);
            PrismaAPI->Focus(view, false);  // second arg -> pauseGame
        }
    }

    void FurnSelectionMenu::HandleSelection(const std::string& data)
    {
        PrismaAPI->Hide(view);
        if (!s_linkedThread) {
            logger::error("FurnSelectionMenu::HandleSelection >> s_linkedThread is null");
            return;
        }
        size_t index = 0;
        try {
            index = static_cast<size_t>(std::stoul(data));
            if (index >= s_items.size()) {
                logger::error("FurnSelectionMenu::HandleSelection >> index {} out of range (size {}), defaulting to 0", index, s_items.size());
                index = 0;
            }
        } catch (const std::exception& e) {
            logger::error("FurnSelectionMenu::HandleSelection >> failed to parse '{}': {}", data, e.what());
        }
        auto* qst = s_linkedThread;
        s_linkedThread = nullptr;
        s_items.clear();
        auto* instance = Instance::GetPendingInstance(qst);
        if (!instance) {
            logger::error("FurnSelectionMenu::HandleSelection >> instance not found");
            return;
        }
        instance->SetCenterRefSelected(index);
    }

    void FurnSelectionMenu::HandleClose()
    {
        // default to index 0 (actor fallback)
        PrismaAPI->Hide(view);
        if (!s_linkedThread)
            return;
        auto* qst = s_linkedThread;
        s_linkedThread = nullptr;
        s_items.clear();
        auto* instance = Instance::GetPendingInstance(qst);
        if (!instance)
            return;
        instance->SetCenterRefSelected(0);
    }

}  // namespace Thread::PrismaUI
