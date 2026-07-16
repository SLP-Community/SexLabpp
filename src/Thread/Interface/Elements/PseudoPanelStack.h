#pragma once
namespace Thread::Interface
{
    class SceneHUD;

    class PseudoPanelStack final
    {
      public:
        void Render(SceneHUD& a_hud);
    };
}
