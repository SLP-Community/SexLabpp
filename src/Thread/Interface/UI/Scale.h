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

        void SetTextMultiplier(float a_multiplier) { _textMultiplier = a_multiplier; }

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

        [[nodiscard]] float TextPx(float a_units)
        {
            return Px(a_units) * _textMultiplier;
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
            const float shortestSide = std::min(_displayWidth, _displayHeight);
            float resolutionScale;
            if (shortestSide <= 1080.0f) {
                resolutionScale = 5.0f / 3.0f;
            } else if (shortestSide <= 2160.0f) {
                resolutionScale = (5.0f / 3.0f) + ((shortestSide - 1080.0f) / 1080.0f) * (1.0f / 3.0f);
            } else {
                resolutionScale = shortestSide / 1080.0f;
            }
            _factor = resolutionScale * _multiplier;
            _valid = true;
        }

        float _multiplier{ 1.5f };
        float _textMultiplier{ 1.0f };
        float _factor{ 0.0f };
        float _displayWidth{ -1.0f };
        float _displayHeight{ -1.0f };
        bool _valid{ false };
    };
}
