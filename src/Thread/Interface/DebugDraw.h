#pragma once

#include <cstddef>
#include <mutex>
#include <vector>

namespace Thread::Interface
{
    class DebugDraw final
    {
      public:
        void BeginFrame();
        void AddRing(const RE::NiPoint3& a_center, const RE::NiPoint3& a_right, const RE::NiPoint3& a_up, float a_radius);
        void AddTaperedCapsule(const RE::NiPoint3& a_start, const RE::NiPoint3& a_end, float a_startRadius, float a_endRadius);
        void Publish();
        void Render() const;
        void Clear();

      private:
        struct Ring
        {
            RE::NiPoint3 center;
            RE::NiPoint3 right;
            RE::NiPoint3 up;
            float radius;
        };

        struct TaperedCapsule
        {
            RE::NiPoint3 start;
            RE::NiPoint3 end;
            float startRadius;
            float endRadius;
        };

        static constexpr std::size_t MAX_COMMANDS{ 256 };

        std::vector<Ring> _pendingRings;
        std::vector<TaperedCapsule> _pendingCapsules;
        std::vector<Ring> _publishedRings;
        std::vector<TaperedCapsule> _publishedCapsules;
        mutable std::mutex _lock;
    };
}
