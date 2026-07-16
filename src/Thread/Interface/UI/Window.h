#pragma once

#include "SKSEMenuFramework.h"

namespace Thread::Interface::UI
{
    class FrameworkWindow final
    {
      public:
        FrameworkWindow() = default;
        FrameworkWindow(const FrameworkWindow&) = delete;
        FrameworkWindow(FrameworkWindow&&) = delete;
        FrameworkWindow& operator=(const FrameworkWindow&) = delete;
        FrameworkWindow& operator=(FrameworkWindow&&) = delete;

        [[nodiscard]] bool Register(SKSEMenuFramework::Model::RenderFunction a_renderer, bool a_blocksInput)
        {
            if (_handle)
                return true;
            _handle = SKSEMenuFramework::AddWindow(a_renderer, a_blocksInput);
            if (!_handle)
                return false;

            _handle->IsOpen = false;
            return true;
        }

        void Open()
        {
            if (_handle)
                _handle->IsOpen = true;
        }

        void Close()
        {
            if (_handle)
                _handle->IsOpen = false;
        }

        void SetOpen(bool a_open)
        {
            a_open ? Open() : Close();
        }

        void SetBlocksInput(bool a_blocksInput)
        {
            if (_handle)
                _handle->BlockUserInput = a_blocksInput;
        }

        [[nodiscard]] bool IsRegistered() const { return _handle != nullptr; }
        [[nodiscard]] bool IsOpen() const { return _handle && _handle->IsOpen.load(); }

      private:
        SKSEMenuFramework::Model::WindowInterface* _handle{};
    };

    class WindowComponent
    {
      public:
        [[nodiscard]] bool RegisterWindow(SKSEMenuFramework::Model::RenderFunction a_renderer, bool a_blocksInput = false)
        {
            return _window.Register(a_renderer, a_blocksInput);
        }

        [[nodiscard]] bool IsVisible() const { return _window.IsOpen(); }

      protected:
        void Show() { _window.Open(); }
        void Hide() { _window.Close(); }
        void SetVisible(bool a_visible) { _window.SetOpen(a_visible); }
        void SetBlocksInput(bool a_blocksInput) { _window.SetBlocksInput(a_blocksInput); }

      private:
        FrameworkWindow _window;
    };

}
