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
            Refresh();
            return a_units * _factor * _textResolutionScale * _textMultiplier;
        }

        [[nodiscard]] float Clamp(float a_minUnits, float a_percent, float a_maxUnits, float a_axisSize)
        {
            return std::clamp(a_axisSize * a_percent * 0.01f, Px(a_minUnits), Px(a_maxUnits));
        }

      private:
        // Calibrated so multiplier=1.0 matches former manual tweaks:
        //   900p: UI 0.8 / Text 1.7 under the old algo
        //   4K:   UI 1.5 / Text 1.0 under the old algo
        static constexpr float kAnchorLow = 900.0f;
        static constexpr float kAnchorHigh = 2160.0f;
        static constexpr float kUiScaleLow = 4.0f / 3.0f;
        static constexpr float kUiScaleHigh = 3.0f;
        static constexpr float kTextScaleLow = 1.7f;
        static constexpr float kTextScaleHigh = 1.0f;

        [[nodiscard]] static float UiResolutionScale(float a_shortest)
        {
            if (a_shortest <= kAnchorLow)
                return kUiScaleLow * (a_shortest / kAnchorLow);
            if (a_shortest >= kAnchorHigh)
                return kUiScaleHigh * (a_shortest / kAnchorHigh);
            const float t = (a_shortest - kAnchorLow) / (kAnchorHigh - kAnchorLow);
            return kUiScaleLow + t * (kUiScaleHigh - kUiScaleLow);
        }

        [[nodiscard]] static float TextResolutionScale(float a_shortest)
        {
            if (a_shortest <= kAnchorLow)
                return kTextScaleLow;
            if (a_shortest >= kAnchorHigh)
                return kTextScaleHigh;
            const float t = (a_shortest - kAnchorLow) / (kAnchorHigh - kAnchorLow);
            return kTextScaleLow + t * (kTextScaleHigh - kTextScaleLow);
        }

        void Refresh()
        {
            const auto* io = ImGuiMCP::GetIO();
            if (_valid && io->DisplaySize.x == _displayWidth && io->DisplaySize.y == _displayHeight)
                return;

            _displayWidth = io->DisplaySize.x;
            _displayHeight = io->DisplaySize.y;
            const float shortestSide = std::min(_displayWidth, _displayHeight);
            _factor = UiResolutionScale(shortestSide) * _multiplier;
            _textResolutionScale = TextResolutionScale(shortestSide);
            _valid = true;
        }

        float _multiplier{ 1.0f };
        float _textMultiplier{ 1.0f };
        float _factor{ 0.0f };
        float _textResolutionScale{ 1.0f };
        float _displayWidth{ -1.0f };
        float _displayHeight{ -1.0f };
        bool _valid{ false };
    };
}
