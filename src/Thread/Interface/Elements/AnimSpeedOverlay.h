#pragma once
#include "Thread/Interface/UI/Window.h"

namespace Thread::Interface
{
    class AnimSpeedOverlay final : public UI::WindowComponent
    {
      public:
        static AnimSpeedOverlay& GetSingleton();

        bool Register();
        void Init();
        void Destroy();
        void UpdateStageTimer(float a_duration, float a_timer);

      private:
        AnimSpeedOverlay() = default;

        static void __stdcall RenderCallback();
        void Render();
        void OnSpeedChange(float a_delta);

        float _speed{ 1.0f };
        float _stageDuration{ 0.0f };
        float _stageTimer{ 0.0f };
    };
}
