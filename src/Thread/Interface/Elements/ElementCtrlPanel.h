#pragma once
#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interface
{
    class ElementCtrlPanel
    {
      public:
        static void Init();
        static void Destroy();
        static void __stdcall Render();

      private:
        static void OnScaleChange(float val);

        inline static bool isVisible{ false };
        inline static bool s_elementSectionOpen{ true };
    };
}
