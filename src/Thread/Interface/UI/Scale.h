#pragma once

#include "SKSEMenuFramework.h"

#include <algorithm>

namespace Thread::Interface::UI
{
    class Scale final
    {
      public:
        void SetMultiplier(float a_multiplier)
        {
            _multiplier = a_multiplier;
            Invalidate();
        }

        void Invalidate() { _valid = false; }

        [[nodiscard]] float Factor()
        {
            Refresh();
            return _factor;
        }

        [[nodiscard]] float Px(float a_units)
        {
            return a_units * Factor();
        }

        [[nodiscard]] float Clamp(float a_minUnits, float a_percent, float a_maxUnits, float a_axisSize)
        {
            return std::clamp(a_axisSize * a_percent * 0.01f, Px(a_minUnits), Px(a_maxUnits));
        }

      private:
        void Refresh()
        {
            const auto* io = ImGuiMCP::GetIO();
            if (_valid && io->DisplaySize.x == _displayWidth && io->DisplaySize.y == _displayHeight)
                return;

            _displayWidth = io->DisplaySize.x;
            _displayHeight = io->DisplaySize.y;
            _factor = (std::min(_displayWidth, _displayHeight) * 0.01f / 10.8f) * _multiplier;
            _valid = true;
        }

        float _multiplier{ 1.5f };
        float _factor{ 0.0f };
        float _displayWidth{ -1.0f };
        float _displayHeight{ -1.0f };
        bool _valid{ false };
    };
}
