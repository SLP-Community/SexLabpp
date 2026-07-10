#pragma once
#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interface
{
    class AnimSpeedOverlay
    {
      public:
        static void Init();
        static void Destroy();
        static void __stdcall Render();

        static void SetAnimSpeedDisplay(float value);
        static void UpdateStageTimerDisplay(float duration, float timer);

      private:
        static void OnSpeedChange(float delta);

        inline static bool isVisible{ false };
        inline static std::atomic<float> speed{ 1.0f };
        inline static std::atomic<float> stageDuration{ 0.0f };
        inline static std::atomic<float> stageTimer{ 0.0f };
    };
}
