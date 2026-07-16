#pragma once
namespace Thread::Interface
{
    class SceneHUD;

    class ElementCtrlPanel final
    {
      public:
        void Open(SceneHUD& a_hud);
        void Close();
        void Render(SceneHUD& a_hud);

      private:
        void OnScaleChange(SceneHUD& a_hud, float a_value);
        void OnTextScaleChange(SceneHUD& a_hud, float a_value);

        float _scaleAdjustment{ 1.5f };
        float _textScaleAdjustment{ 1.0f };
        bool _elementSectionOpen{ true };
    };
}
