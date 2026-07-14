#pragma once
#include "Thread/Interface/UI/Window.h"

namespace Thread::Interface
{
    class ElementCtrlPanel final : public UI::WindowComponent, public UI::Panel
    {
      public:
        static ElementCtrlPanel& GetSingleton();

        bool Register();
        void Open() override;
        void Close() override;

      private:
        ElementCtrlPanel() = default;

        static void __stdcall RenderCallback();
        void Render();
        void OnScaleChange(float a_value);

        bool _elementSectionOpen{ true };
    };
}
