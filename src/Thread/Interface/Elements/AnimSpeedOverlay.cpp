#include "AnimSpeedOverlay.h"
#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interface
{
    using UI::DrawTextShadowed;
    using UI::SetWindowFontSize;

    AnimSpeedOverlay& AnimSpeedOverlay::GetSingleton()
    {
        static AnimSpeedOverlay singleton;
        return singleton;
    }

    bool AnimSpeedOverlay::Register()
    {
        return RegisterWindow(RenderCallback);
    }

    void __stdcall AnimSpeedOverlay::RenderCallback()
    {
        GetSingleton().Render();
    }

    void AnimSpeedOverlay::Init()
    {
        auto& hud = SceneHUD::GetSingleton();
        auto* inst = hud.GetThreadInstance();
        if (!inst)
            return;
        if (!inst->GetThreadProperty<bool>("ElementUI_AnimSpeed"))
            return;
        Show();
    }

    void AnimSpeedOverlay::Destroy()
    {
        Hide();
        _speed = 1.0f;
        _stageDuration = 0.0f;
        _stageTimer = 0.0f;
    }

    void AnimSpeedOverlay::UpdateStageTimer(float a_duration, float a_timer)
    {
        _stageDuration = a_duration;
        _stageTimer = a_timer;
    }

    void AnimSpeedOverlay::OnSpeedChange(float a_delta)
    {
        auto& hud = SceneHUD::GetSingleton();
        auto* inst = hud.GetThreadInstance();
        if (!inst)
            return;
        const float next = std::clamp(_speed + a_delta, 0.25f, 3.0f);
        _speed = next;
        inst->SetAnimationPlaybackSpeed(next);
        Script::DispatchMethodCall(
            Script::GetScriptObject(hud.GetLinkedThread(), "sslThreadModel"),
            "UpdateBaseSpeed", hud.GetCallback(), static_cast<float>(_speed));
    }

    void AnimSpeedOverlay::Render()
    {
        auto& hud = SceneHUD::GetSingleton();
        if (!IsVisible() || !hud.IsActive())
            return;
        auto& scale = hud.GetScale();

        auto* io = ImGuiMCP::GetIO();
        const float dw = io->DisplaySize.x;
        const float dh = io->DisplaySize.y;

        const float edgeH = scale.Clamp(14.0f, 2.5f, 48.0f, dw);
        const float edgeV = scale.Clamp(16.0f, 1.8f, 32.0f, dh);
        const float zoneW = scale.Px(100.0f);
        const float zoneH = scale.Px(60.0f);
        const float timerH = scale.Px(UI::Theme::Spacing::xs);
        const float gap = scale.Px(UI::Theme::Spacing::sm);

        // Pinned to the bottom-right corner.
        const float winX = dw - zoneW - edgeH;
        const float winY = dh - zoneH - edgeV;

        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
            ImGuiMCP::ImGuiWindowFlags_NoMove | ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
            ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize | ImGuiMCP::ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiMCP::ImGuiWindowFlags_NoNav | ImGuiMCP::ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiMCP::ImGuiWindowFlags_NoBackground;

        ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{ winX, winY }, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{ zoneW, zoneH }, ImGuiMCP::ImGuiCond_Always);

        if (!ImGuiMCP::Begin("##slpp_AnimSpeed", nullptr, kFlags)) {
            ImGuiMCP::End();
            return;
        }

        SetWindowFontSize(scale.TextPx(UI::Theme::FontSize::overlay));

        const float spd = _speed;
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%.2fx", spd);

        // [slower]  value  [faster]
        const float btnW = scale.Px(20.0f);
        const float rowH = std::max(scale.Px(UI::Theme::Spacing::xl),
            scale.TextPx(UI::Theme::FontSize::overlay) + scale.Px(UI::Theme::Spacing::xxs));
        const ImGuiMCP::ImVec2 valSz = ImGuiMCP::CalcTextSize(buf);
        auto* dl = ImGuiMCP::GetForegroundDrawList();
        const ImGuiMCP::ImVec2 rowScreenPos = ImGuiMCP::GetCursorScreenPos();

        if (hud.IsFocused()) {
            ImGuiMCP::SetCursorScreenPos(rowScreenPos);
            if (ImGuiMCP::InvisibleButton("##slpp_dec", ImGuiMCP::ImVec2{ btnW, rowH }))
                OnSpeedChange(-0.25f);

            ImGuiMCP::SetCursorScreenPos(ImGuiMCP::ImVec2{ rowScreenPos.x + zoneW - btnW, rowScreenPos.y });
            if (ImGuiMCP::InvisibleButton("##slpp_inc", ImGuiMCP::ImVec2{ btnW, rowH }))
                OnSpeedChange(+0.25f);
        }

        SKSEMenuFramework::PushFont(UI::Theme::Icon::solidFont);
        const ImGuiMCP::ImVec2 leftIconSize = ImGuiMCP::CalcTextSize(UI::Theme::Icon::anglesLeft);
        const ImGuiMCP::ImVec2 rightIconSize = ImGuiMCP::CalcTextSize(UI::Theme::Icon::anglesRight);
        DrawTextShadowed(dl,
            ImGuiMCP::ImVec2{ rowScreenPos.x + (btnW - leftIconSize.x) * 0.5f, rowScreenPos.y + (rowH - leftIconSize.y) * 0.5f },
            UI::Theme::Color::textSecondary, UI::Theme::Icon::anglesLeft);
        DrawTextShadowed(dl,
            ImGuiMCP::ImVec2{ rowScreenPos.x + zoneW - btnW + (btnW - rightIconSize.x) * 0.5f,
                rowScreenPos.y + (rowH - rightIconSize.y) * 0.5f },
            UI::Theme::Color::textSecondary, UI::Theme::Icon::anglesRight);
        FontAwesome::Pop();
        DrawTextShadowed(dl,
            ImGuiMCP::ImVec2{ rowScreenPos.x + (zoneW - valSz.x) * 0.5f,
                rowScreenPos.y + (rowH - valSz.y) * 0.5f },
            UI::Theme::Color::textPrimary, buf);
        ImGuiMCP::SetCursorScreenPos(rowScreenPos);
        ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ zoneW, rowH });

        ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, gap });

        // Stage timer: fills from right toward left as time runs out, hidden when there's no active timer.
        const float dur = _stageDuration;
        const float tmr = _stageTimer;
        if (dur > 0.0f && tmr > 0.0f) {
            const float frac = std::clamp(tmr / dur, 0.0f, 1.0f);
            const float fillW = zoneW * frac;
            const ImGuiMCP::ImVec2 barPos = ImGuiMCP::GetCursorScreenPos();

            ImGuiMCP::ImDrawListManager::AddRectFilled(dl,
                barPos, ImGuiMCP::ImVec2{ barPos.x + zoneW, barPos.y + timerH },
                UI::Theme::Animation::timerTrack, timerH * 0.5f, 0);
            if (fillW > 0.0f) {
                const float fx = barPos.x + zoneW - fillW;
                ImGuiMCP::ImDrawListManager::AddRectFilledMultiColor(dl,
                    ImGuiMCP::ImVec2{ fx, barPos.y }, ImGuiMCP::ImVec2{ barPos.x + zoneW, barPos.y + timerH },
                    UI::Theme::Animation::timerEdge,
                    UI::Theme::Animation::timerCenter,
                    UI::Theme::Animation::timerCenter,
                    UI::Theme::Animation::timerEdge);
            }
            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ zoneW, timerH });
        }

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();
    }
}
