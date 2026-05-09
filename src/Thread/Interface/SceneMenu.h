#pragma once

#include "Registry/Define/Animation.h"
#include "Thread/Thread.h"

namespace Thread::Interface
{
    class SceneMenu :
      public RE::IMenu,
      public RE::BSTEventSink<RE::InputEvent*>
    {
        using GRefCountBaseStatImpl::operator new;
        using GRefCountBaseStatImpl::operator delete;

      public:
        static constexpr std::string_view MENU_NAME{ "SLSceneMenu" };
        static constexpr std::string_view FILEPATH{ "SexLab\\SceneMenu" };
        static constexpr int8_t DEPTH_PRIORITY{ 4 };

        SceneMenu();
        ~SceneMenu() = default;
        static void Register() { (RE::UI::GetSingleton()->Register(MENU_NAME, Create), logger::info("Registered Menu: {}", MENU_NAME)); }
        static RE::IMenu* Create() { return new SceneMenu(); }

      public:
        static void Show(Instance* instance) { (threadInstance = instance, RE::UIMessageQueue::GetSingleton()->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr)); }
        static void Hide() { RE::UIMessageQueue::GetSingleton()->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr); }
        static bool IsOpen() { return RE::UI::GetSingleton()->IsMenuOpen(MENU_NAME); }

        static bool IsInstance(Instance* a_instance) { return threadInstance == a_instance; }
        static void UpdateSlider(RE::FormID a_actorId, float a_enjoyment);
        static void SetSliderTo(RE::FormID a_actorId, float a_enjoyment);
        static void UpdatePositions();
        static void UpdateStageInfo();
        static void UpdateActiveScene();
        static void UpdateTimer(float a_time);
        static void DisableTimer();

      protected:
        // IMenu
        RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message) override;

        // Events
        RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) override;

      private:
        static inline Instance* threadInstance{ nullptr };

      private:
        void AttachSexLabAPIFunctions(RE::GPtr<RE::GFxMovieView> a_view);

        struct GFxFunctionHandlerWrapper : public RE::GFxFunctionHandler
        {
          protected:
            RE::Actor* GetActorByReferenceId(Params& a_args, size_t argIdx);
        };

        // clang-format off
		struct SLAPI_GetHotkeyCombination : public RE::GFxFunctionHandler { void Call(Params& a_args) override; };
		struct SLAPI_GetActiveFurnitureName : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_GetOffset : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_ResetOffsets : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_GetOffsetStepSize : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_AdjustOffsetStepSize : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_GetAdjustStageOnly : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_SetAdjustStageOnly : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_GetAlternateScenes : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_ToggleAutoPlay : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_IsAutoPlay : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_SetHideHUD : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_GetHideHUD : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_GetPermutationData : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_SelectNextPermutation : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_GetGhostMode : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_SetGhostMode : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_GetExpressionName : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_SetExpression : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_GetExpressions : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_GetVoiceName : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_SetVoice : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
		struct SLAPI_GetVoices : public GFxFunctionHandlerWrapper { void Call(Params& a_args) override; };
        // clang-format on

      private:
        struct HUDMenu_ShowMessageEx : public RE::GFxFunctionHandler
        {
            void Call(Params& a_args) override;
        };
    };
}
