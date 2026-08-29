#include "DebugDraw.h"
#include "SKSEMenuFramework.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <numbers>

namespace Thread::Interface
{
    void DebugDraw::BeginFrame()
    {
        const std::scoped_lock guard(_lock);
        _pendingRings.clear();
        _pendingCapsules.clear();
    }

    void DebugDraw::AddRing(const RE::NiPoint3& a_center, const RE::NiPoint3& a_right, const RE::NiPoint3& a_up, float a_radius)
    {
        if (a_radius <= 0.0f) {
            return;
        }
        const std::scoped_lock guard(_lock);
        if (_pendingRings.size() + _pendingCapsules.size() < MAX_COMMANDS) {
            _pendingRings.push_back({ a_center, a_right, a_up, a_radius });
        }
    }

    void DebugDraw::AddTaperedCapsule(const RE::NiPoint3& a_start, const RE::NiPoint3& a_end, float a_startRadius, float a_endRadius)
    {
        if ((a_end - a_start).SqrLength() <= FLT_EPSILON || (a_startRadius <= 0.0f && a_endRadius <= 0.0f)) {
            return;
        }
        const std::scoped_lock guard(_lock);
        if (_pendingRings.size() + _pendingCapsules.size() < MAX_COMMANDS) {
            _pendingCapsules.push_back({ a_start, a_end, std::max(a_startRadius, 0.0f), std::max(a_endRadius, 0.0f) });
        }
    }

    void DebugDraw::Publish()
    {
        const std::scoped_lock guard(_lock);
        // Keep the latest complete collision frame stable until the producer publishes another one.
        _publishedRings.swap(_pendingRings);
        _publishedCapsules.swap(_pendingCapsules);
    }

    void DebugDraw::Render() const
    {
        auto* camera = RE::Main::WorldRootCamera();
        auto* drawList = ImGuiMCP::GetForegroundDrawList();
        const auto display = ImGuiMCP::GetIO()->DisplaySize;
        if (!camera || !drawList || display.x <= 0.0f || display.y <= 0.0f) {
            return;
        }

        constexpr std::size_t segments{ 8 };
        constexpr auto color = IM_COL32(0, 255, 210, 230);
        constexpr float thickness{ 1.5f };
        const auto project = [&](const RE::NiPoint3& a_point, ImGuiMCP::ImVec2& a_screen) {
            float x{}, y{}, z{};
            if (!camera->WorldPtToScreenPt3(a_point, x, y, z, 1.0e-5f)) {
                return false;
            }
            a_screen = { x * display.x, (1.0f - y) * display.y };
            return true;
        };

        const std::scoped_lock guard(_lock);
        for (const auto& ring : _publishedRings) {
            std::array<ImGuiMCP::ImVec2, segments> points{};
            std::array<bool, segments> visible{};
            for (std::size_t i = 0; i < segments; ++i) {
                const auto angle = static_cast<float>(i) * (2.0f * std::numbers::pi_v<float> / static_cast<float>(segments));
                visible[i] = project(ring.center + ring.right * (std::cos(angle) * ring.radius) + ring.up * (std::sin(angle) * ring.radius), points[i]);
            }
            for (std::size_t i = 0; i < segments; ++i) {
                const auto next = (i + 1) % segments;
                if (visible[i] && visible[next]) {
                    ImGuiMCP::ImDrawListManager::AddLine(drawList, points[i], points[next], color, thickness);
                }
            }
        }

        for (const auto& capsule : _publishedCapsules) {
            auto tangent = capsule.end - capsule.start;
            tangent.Unitize();
            // Build a stable perpendicular basis for the circular cross-sections.
            const auto reference = std::abs(tangent.z) < 0.9f ? RE::NiPoint3{ 0.0f, 0.0f, 1.0f } : RE::NiPoint3{ 1.0f, 0.0f, 0.0f };
            auto right = tangent.Cross(reference);
            right.Unitize();
            auto up = right.Cross(tangent);
            up.Unitize();

            std::array<ImGuiMCP::ImVec2, segments> startPoints{};
            std::array<ImGuiMCP::ImVec2, segments> endPoints{};
            std::array<bool, segments> startVisible{};
            std::array<bool, segments> endVisible{};
            for (std::size_t i = 0; i < segments; ++i) {
                const auto angle = static_cast<float>(i) * (2.0f * std::numbers::pi_v<float> / static_cast<float>(segments));
                const auto radial = right * std::cos(angle) + up * std::sin(angle);
                startVisible[i] = project(capsule.start + radial * capsule.startRadius, startPoints[i]);
                endVisible[i] = project(capsule.end + radial * capsule.endRadius, endPoints[i]);
            }
            for (std::size_t i = 0; i < segments; ++i) {
                const auto next = (i + 1) % segments;
                if (capsule.startRadius > 0.0f && startVisible[i] && startVisible[next]) {
                    ImGuiMCP::ImDrawListManager::AddLine(drawList, startPoints[i], startPoints[next], color, thickness);
                }
                if (capsule.endRadius > 0.0f && endVisible[i] && endVisible[next]) {
                    ImGuiMCP::ImDrawListManager::AddLine(drawList, endPoints[i], endPoints[next], color, thickness);
                }
                if (startVisible[i] && endVisible[i]) {
                    ImGuiMCP::ImDrawListManager::AddLine(drawList, startPoints[i], endPoints[i], color, thickness);
                }
            }
        }
    }

    void DebugDraw::Clear()
    {
        const std::scoped_lock guard(_lock);
        _pendingRings.clear();
        _pendingCapsules.clear();
        _publishedRings.clear();
        _publishedCapsules.clear();
    }
}
