#pragma once
namespace Thread::Interface
{
    class SceneHUD;

    class AnimSpeedOverlay final
    {
      public:
        void Render(SceneHUD& a_hud);
        void UpdateStageTimer(float a_duration, float a_timer);

      private:
        void OnSpeedChange(SceneHUD& a_hud, float a_delta);

        float _speed{ 1.0f };
        float _stageDuration{ 0.0f };
        float _stageTimer{ 0.0f };
    };
}
