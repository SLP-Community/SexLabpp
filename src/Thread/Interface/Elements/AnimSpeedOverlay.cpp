#include "AnimSpeedOverlay.h"

namespace Thread::Interface
{
    void AnimSpeedOverlay::Init()
    {
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (!inst) return;
        if (!inst->GetThreadProperty<bool>("ElementUI_AnimSpeed")) return;
        isVisible = true;
        SceneHUD::winAnimSpeed->IsOpen = true;
    }

    void AnimSpeedOverlay::Destroy()
    {
        isVisible = false;
        if (SceneHUD::winAnimSpeed) SceneHUD::winAnimSpeed->IsOpen = false;
        speed = 1.0f;
        stageDuration = 0.0f;
        stageTimer = 0.0f;
    }

    void AnimSpeedOverlay::SetAnimSpeedDisplay(float value)
    {
        speed = value;
    }

    void AnimSpeedOverlay::UpdateStageTimerDisplay(float duration, float timer)
    {
        stageDuration = duration;
        stageTimer = timer;
    }

    void AnimSpeedOverlay::OnSpeedChange(float delta)
    {
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (!inst) return;
        const float next = std::clamp(speed.load() + delta, 0.25f, 3.0f);
        speed = next;
        inst->SetAnimationPlaybackSpeed(next);
        Script::DispatchMethodCall(
            Script::GetScriptObject(SceneHUD::linkedThread, "sslThreadModel"),
            "UpdateBaseSpeed", SceneHUD::callbackPtr, static_cast<float>(speed));
    }

    void __stdcall AnimSpeedOverlay::Render()
    {
        if (!isVisible || !SceneHUD::IsActive()) return;

        auto* io = ImGuiMCP::GetIO();
        const float dw = io->DisplaySize.x;
        const float dh = io->DisplaySize.y;

        const float edgeH  = ScaleUI::pxScaleClamp(14.0f, 2.5f, 48.0f, dw);
        const float edgeV  = ScaleUI::pxScaleClamp(16.0f, 1.8f, 32.0f, dh);
        const float zoneW  = ScaleUI::pxScale(100.0f);
        const float zoneH  = ScaleUI::pxScale(60.0f);
        const float timerH = ScaleUI::pxScale(4.0f);
        const float gap    = ScaleUI::pxScale(6.0f);

        // Pinned to the bottom-right corner.
        const float winX = dw - zoneW - edgeH;
        const float winY = dh - zoneH - edgeV;

        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
            ImGuiMCP::ImGuiWindowFlags_NoMove | ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
            ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize | ImGuiMCP::ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiMCP::ImGuiWindowFlags_NoNav | ImGuiMCP::ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiMCP::ImGuiWindowFlags_NoBackground;

        ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{winX, winY}, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{zoneW, zoneH}, ImGuiMCP::ImGuiCond_Always);

        if (!ImGuiMCP::Begin("##slpp_AnimSpeed", nullptr, kFlags)) { ImGuiMCP::End(); return; }

        SetWindowFontSize(ScaleUI::pxScale(13.0f));

        const float spd = speed.load();
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%.2fx", spd);

        // [-]  value  [+]
        const float btnW = ScaleUI::pxScale(20.0f);
        const float rowH = ScaleUI::pxScale(16.0f);
        const ImGuiMCP::ImVec2 valSz = ImGuiMCP::CalcTextSize(buf);
        const float rowStartX = ImGuiMCP::GetCursorPosX();
        auto* dl = ImGuiMCP::GetWindowDrawList();

        if (SceneHUD::focusMode) {
            ImGuiMCP::SetCursorPosX(rowStartX);
            if (ImGuiMCP::Button("-##slpp_dec", ImGuiMCP::ImVec2{ btnW, rowH })) OnSpeedChange(-0.25f);

            ImGuiMCP::SameLine();
            ImGuiMCP::SetCursorPosX(rowStartX + (zoneW - valSz.x) * 0.5f);
            ImGuiMCP::TextUnformatted(buf);

            ImGuiMCP::SameLine();
            ImGuiMCP::SetCursorPosX(rowStartX + zoneW - btnW);
            if (ImGuiMCP::Button("+##slpp_inc", ImGuiMCP::ImVec2{ btnW, rowH })) OnSpeedChange(+0.25f);
        } else {
            const ImGuiMCP::ImVec2 rowScreenPos = ImGuiMCP::GetCursorScreenPos();
            DrawTextShadowed(dl, rowScreenPos, ColorUI::TextSecond, "-");
            DrawTextShadowed(dl,
                ImGuiMCP::ImVec2{ rowScreenPos.x + (zoneW - valSz.x) * 0.5f, rowScreenPos.y },
                ColorUI::TextPrimary, buf);
            DrawTextShadowed(dl,
                ImGuiMCP::ImVec2{ rowScreenPos.x + zoneW - btnW, rowScreenPos.y },
                ColorUI::TextSecond, "+");
            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ zoneW, rowH });
        }

        ImGuiMCP::Dummy(ImGuiMCP::ImVec2{0.0f, gap});

        // Stage timer: fills from right toward left as time runs out, hidden when there's no active timer.
        const float dur = stageDuration.load();
        const float tmr = stageTimer.load();
        if (dur > 0.0f && tmr > 0.0f) {
            const float frac = std::clamp(tmr / dur, 0.0f, 1.0f);
            const float fillW = zoneW * frac;
            const ImGuiMCP::ImVec2 barPos = ImGuiMCP::GetCursorScreenPos();

            ImGuiMCP::ImDrawListManager::AddRectFilled(dl,
                barPos, ImGuiMCP::ImVec2{barPos.x + zoneW, barPos.y + timerH},
                IM_COL32(10, 10, 12, 178), timerH * 0.5f, 0);
            if (fillW > 0.0f) {
                const float fx = barPos.x + zoneW - fillW;
                ImGuiMCP::ImDrawListManager::AddRectFilledMultiColor(dl,
                    ImGuiMCP::ImVec2{fx, barPos.y}, ImGuiMCP::ImVec2{barPos.x + zoneW, barPos.y + timerH},
                    IM_COL32(255,255,255, 38),
                    IM_COL32(255,255,255,217),
                    IM_COL32(255,255,255,217),
                    IM_COL32(255,255,255, 38));
            }
            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{zoneW, timerH});
        }

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();
    }
}
