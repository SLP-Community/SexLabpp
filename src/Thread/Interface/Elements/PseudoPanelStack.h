#pragma once
#include "Thread/Interface/UI/Window.h"

namespace Thread::Interface
{
    class PseudoPanelStack final : public UI::WindowComponent
    {
      public:
        static PseudoPanelStack& GetSingleton();

        bool Register();
        void Open();
        void Close();
        void SetInputBlocking(bool a_blocking) { SetBlocksInput(a_blocking); }

      private:
        PseudoPanelStack() = default;

        static void __stdcall RenderCallback();
        void Render();
    };
}
