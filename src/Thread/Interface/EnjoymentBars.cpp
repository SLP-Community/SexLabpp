#include "EnjoymentBars.h"

namespace Thread::PrismaUI
{
    bool EnjoymentBars::Initialize()
    {
        if (!Thread::PrismaUI::IsAvailable()) {
            Thread::PrismaUI::Initialize();
            PrismaAPI = Thread::PrismaUI::GetAPI();
            if (!PrismaAPI) {
                logger::error("EnjoymentBars::Initialize >> FAILED - PrismaUI unavailable");
                return false;
            }
        }
        if (!PrismaAPI->IsValid(view)) {
            view = Thread::PrismaUI::CreateView(FILEPATH.data());
            if (!view) {
                logger::error("EnjoymentBars::Initialize >> FAILED - CreateView returned null");
                return false;
            }

            PrismaAPI->RegisterJSListener(view, "EnjBars_OnTimedAttempt", []([[maybe_unused]] const char*) {
                OnRaiseEnjAttemptResult(true);
            });
            PrismaAPI->RegisterJSListener(view, "EnjBars_OnMissedAttempt", []([[maybe_unused]] const char*) {
                OnRaiseEnjAttemptResult(false);
            });
            logger::info("EnjoymentBars >> Initialized successfully");
        }
        return true;
    }

    void EnjoymentBars::InitAndShow(Script::ObjectPtr a_scriptObj, const std::vector<RE::Actor*>& a_positions)
    {
        if (!PrismaAPI || !PrismaAPI->IsValid(view))
            return;
        s_threadScript = a_scriptObj;

        std::string json = "{\"actorsInfo\":[";
        for (size_t i = 0; i < a_positions.size(); ++i) {
            if (i > 0)
                json += ',';
            json += "{\"id\":";
            json += std::to_string(a_positions[i]->GetFormID());
            json += ",\"name\":\"";
            json += Thread::PrismaUI::JsonEscape(a_positions[i]->GetName());
            json += "\"}";
        }
        json += "]}";

        if (PrismaAPI->IsValid(view)) {
            PrismaAPI->InteropCall(view, "EnjBars_InitActors", json.c_str());
            Thread::PrismaUI::OverlaySuppressor::Register(view);
            PrismaAPI->Show(view);
        }
    }

    void EnjoymentBars::HideAndClear()
    {
        if (!PrismaAPI || !PrismaAPI->IsValid(view))
            return;
        PrismaAPI->InteropCall(view, "EnjBars_ClearActors", "");
        Thread::PrismaUI::OverlaySuppressor::Unregister(view);
        PrismaAPI->Hide(view);
        s_threadScript = nullptr;
    }

    void EnjoymentBars::ToggleEnjoymentBars()
    {
        if (!PrismaAPI || !PrismaAPI->IsValid(view))
            return;
        if (IsVisible()) {
            PrismaAPI->Hide(view);
        } else {
            PrismaAPI->Show(view);
        }
    }

    bool EnjoymentBars::IsVisible()
    {
        return PrismaAPI && PrismaAPI->IsValid(view) && !PrismaAPI->IsHidden(view);
    }

    void EnjoymentBars::UpdateSlider(RE::Actor* a_actor, float a_enjoyment, RE::BSFixedString a_interactions)
    {
        if (!IsVisible())
            return;
        std::string payload = std::to_string(a_actor->GetFormID()) + "^" + std::to_string(a_enjoyment) + "^" + a_interactions.c_str();
        PrismaAPI->InteropCall(view, "EnjBars_UpdateSlider", payload.c_str());
    }

    void EnjoymentBars::UpdateHighlightedPartner(RE::Actor* a_partner)
    {
        if (!IsVisible())
            return;
        std::string partnerID = std::to_string(a_partner->GetFormID());
        PrismaAPI->InteropCall(view, "EnjBars_UpdateHighlightedPartner", std::to_string(a_partner->GetFormID()).c_str());
    }

    void EnjoymentBars::RegisterRaiseEnjAttempt(RE::Actor* a_actor, float a_nextTimeCycle)
    {
        if (!PrismaAPI || !PrismaAPI->IsValid(view))
            return;
        std::string payload = std::to_string(a_actor->GetFormID()) + "^" + std::to_string(a_nextTimeCycle);
        PrismaAPI->InteropCall(view, "EnjBars_RaiseEnjAttempt", payload.c_str());
    }

    void EnjoymentBars::OnRaiseEnjAttemptResult(bool a_success)
    {
        if (!s_threadScript) {
            logger::info("EnjoymentBars >> OnRaiseEnjAttemptResult: s_threadScript is null!");
            return;
        }
        Script::CallbackPtr callbackPtr{};
        Script::DispatchMethodCall(s_threadScript, "OnRaiseEnjAttemptResult", callbackPtr, a_success ? 1 : 0);
    }
}
