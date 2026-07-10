#pragma once
#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interface
{
    class PseudoPanelStack
    {
      public:
        static void Init();
        static void Destroy();
        static void __stdcall Render();

      private:
        inline static bool isVisible{ false };
    };
}
