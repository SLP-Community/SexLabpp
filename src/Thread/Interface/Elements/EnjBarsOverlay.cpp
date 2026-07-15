#include "EnjBarsOverlay.h"
#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interface
{
    using UI::DrawTextShadowed;
    using UI::SetWindowFontSize;

    EnjBarsOverlay& EnjBarsOverlay::GetSingleton()
    {
        static EnjBarsOverlay singleton;
        return singleton;
    }

    bool EnjBarsOverlay::Register()
    {
        return RegisterWindow(RenderCallback);
    }

    void __stdcall EnjBarsOverlay::RenderCallback()
    {
        GetSingleton().Render();
    }

    void EnjBarsOverlay::Init()
    {
        auto* inst = SceneHUD::GetSingleton().GetThreadInstance();
        if (!inst)
            return;
        if (!inst->GetThreadProperty<bool>("ElementUI_EnjBars"))
            return;
        if (!inst->GetThreadProperty<bool>("VarUI_SeparateOrgasm"))
            return;
        _bars.clear();
        for (auto* actor : inst->GetActors()) {
            if (!actor)
                continue;
            ActorEnjBar bar;
            bar.formId = actor->GetFormID();
            std::snprintf(bar.name, sizeof(bar.name), "%s", actor->GetName());
            _bars.push_back(bar);
        }
        _needlePosition = 0.5f;
        _needleDirection = 1.0f;
        _timeCycle = 0.0f;
        _needleRunning = false;
        _feedbackActorId = 0;
        _feedbackHit = false;
        _feedbackUntil = 0.0;
        Show();
    }

    void EnjBarsOverlay::Destroy()
    {
        Hide();
        _needleRunning = false;
        _bars.clear();
    }

    // ── Pushers (game thread) ───────────────────────────────────────────────

    void EnjBarsOverlay::UpdateSlider(RE::Actor* a_actor, float a_enjoyment, RE::BSFixedString a_interactions)
    {
        if (!IsVisible() || !a_actor)
            return;
        const uint32_t actorID = a_actor->GetFormID();

        // "a,b,c" ──> "a · b · c"
        char intrBuf[128] = {};
        const char* src = a_interactions.c_str();
        size_t outLen = 0;
        for (const char* p = src; *p && outLen < sizeof(intrBuf) - 4; ++p) {
            if (*p == ',') {
                std::memcpy(intrBuf + outLen, " \xC2\xB7 ", 3);
                outLen += 3;
            } else {
                intrBuf[outLen++] = *p;
            }
        }
        intrBuf[outLen] = '\0';

        for (auto& b : _bars) {
            if (b.formId != actorID)
                continue;
            b.enjoyment = a_enjoyment;
            std::snprintf(b.interactions, sizeof(b.interactions), "%s", intrBuf);
            if (b.isGameDpt && a_enjoyment < kGameEnjThresh)
                b.isGameDpt = false;
            return;
        }
    }

    void EnjBarsOverlay::UpdateHighlightedPartner(RE::Actor* a_partner)
    {
        if (!IsVisible() || !a_partner)
            return;
        const uint32_t actorID = a_partner->GetFormID();
        for (auto& b : _bars) b.isTarget = (b.formId == actorID);
    }

    void EnjBarsOverlay::RegisterRaiseEnjAttempt(RE::Actor* a_actor, float a_nextTimeCycle)
    {
        if (!IsVisible() || !a_actor)
            return;
        const uint32_t actorID = a_actor->GetFormID();

        for (auto& b : _bars) {
            if (b.formId != actorID)
                continue;
            if (b.enjoyment >= kGameEnjThresh && !b.isGameDpt) {
                b.isGameDpt = true;
                _needleRunning = true;
            }
            const float halfWidth = GreenZoneHalfWidth(b.enjoyment);
            const bool hit = _needlePosition >= 0.5f - halfWidth && _needlePosition <= 0.5f + halfWidth;
            _feedbackActorId = actorID;
            _feedbackHit = hit;
            _feedbackUntil = ImGuiMCP::GetTime() + kFeedbackSec;
            _timeCycle = a_nextTimeCycle;

            auto& hud = SceneHUD::GetSingleton();
            if (!hud.GetThreadScript())
                return;
            Script::DispatchMethodCall(hud.GetThreadScript(), "OnRaiseEnjAttemptResult",
                hud.GetCallback(), hit ? 1 : 0);
            return;
        }
    }

    // ── Callbacks ───────────────────────────────────────────────────────────

    void EnjBarsOverlay::OnSelectPartner(uint32_t formId)
    {
        auto* actor = RE::TESForm::LookupByID<RE::Actor>(formId);
        if (!actor || actor->IsPlayerRef())
            return;
        auto& hud = SceneHUD::GetSingleton();
        if (!hud.GetThreadScript())
            return;
        Script::DispatchMethodCall(hud.GetThreadScript(), "SelectTargetPartner",
            hud.GetCallback(), std::move(actor));
    }

    float EnjBarsOverlay::FillFraction(float enj)
    {
        if (enj < 0.0f)
            return std::min(std::abs(enj) / 100.0f, 1.0f);
        const float m = std::fmod(enj, 100.0f);
        return (m == 0.0f && enj > 0.0f) ? 1.0f : std::min(m / 100.0f, 1.0f);
    }

    void EnjBarsOverlay::FillGradient(float enj, ImGuiMCP::ImU32& lo, ImGuiMCP::ImU32& hi)
    {
        if (enj < 0.0f) {
            lo = UI::Theme::Enjoyment::negativeHigh;
            hi = UI::Theme::Enjoyment::negativeLow;
            return;
        }
        if (enj > 100.0f) {
            lo = UI::Theme::Enjoyment::overflowLow;
            hi = UI::Theme::Enjoyment::overflowHigh;
            return;
        }
        lo = UI::Theme::Enjoyment::normalLow;
        hi = UI::Theme::Enjoyment::normalHigh;
    }

    float EnjBarsOverlay::GreenZoneHalfWidth(float a_enjoyment)
    {
        return std::max(kGZoneMin, kGZoneDefault - (a_enjoyment - kGameEnjDrawMin) * 0.00375f);
    }

    // ── Layout Cache ────────────────────────────────────────────────────────
    const EnjBarsOverlay::LayoutCache& EnjBarsOverlay::GetLayout(size_t actorCount)
    {
        auto* io = ImGuiMCP::GetIO();
        const float dw = io->DisplaySize.x;
        const float dh = io->DisplaySize.y;
        const float factor = ScaleUI::pxScale(1.0f);
        const float textFactor = ScaleUI::pxTextScale(1.0f);
        if (_layoutForFactor == factor && _layoutForTextFactor == textFactor &&
            _layoutForWidth == dw && _layoutForHeight == dh && _layoutForCount == actorCount) {
            return _layout;
        }

        auto& L = _layout;
        L.zoneW = std::clamp(ScaleUI::pxScale(260.0f), dw * 0.15f, ScaleUI::pxScale(360.0f));
        L.barGap = ScaleUI::pxScale(4.5f);
        L.innerGp = ScaleUI::pxScale(2.0f);
        L.frameH = ScaleUI::pxScale(UI::Theme::FontSize::body);
        L.lblPad = ScaleUI::pxScale(1.5f);
        L.nameFt = ScaleUI::pxTextScale(UI::Theme::FontSize::metadata);
        L.valFt = ScaleUI::pxTextScale(UI::Theme::FontSize::smallText);
        L.intrFt = ScaleUI::pxTextScale(UI::Theme::FontSize::detail);
        L.fbFt = std::min(ScaleUI::pxTextScale(UI::Theme::FontSize::body), L.frameH);
        L.edgeH = ScaleUI::pxScaleClamp(14.0f, 2.5f, 48.0f, dw);
        L.edgeV = ScaleUI::pxScaleClamp(16.0f, 1.8f, 32.0f, dh);
        L.lblRowH = std::max(ScaleUI::pxScale(UI::Theme::FontSize::metadata), L.nameFt) + L.lblPad * 2.0f;
        L.unitH = L.lblRowH + L.innerGp + L.frameH + L.barGap;
        L.winH = L.unitH * static_cast<float>(actorCount) - L.barGap;

        _layoutForFactor = factor;
        _layoutForTextFactor = textFactor;
        _layoutForWidth = dw;
        _layoutForHeight = dh;
        _layoutForCount = actorCount;
        return L;
    }

    void EnjBarsOverlay::Render()
    {
        auto& hud = SceneHUD::GetSingleton();
        if (!IsVisible() || !hud.ShouldRender() || _bars.empty())
            return;

        const float deltaTime = ImGuiMCP::GetIO()->DeltaTime;

        const bool anyQualify = std::ranges::any_of(_bars,
            [](const ActorEnjBar& b) { return b.isGameDpt && b.enjoyment >= kGameEnjDrawMin; });

        if (!anyQualify)
            _needleRunning = false;

        if (anyQualify && _needleRunning && _timeCycle > 0.0f) {
            _needlePosition += _needleDirection * (1.0f / _timeCycle) * std::min(deltaTime, 0.1f);
            if (_needlePosition >= 1.0f) {
                _needlePosition = 1.0f;
                _needleDirection = -1.0f;
            }
            if (_needlePosition <= 0.0f) {
                _needlePosition = 0.0f;
                _needleDirection = 1.0f;
            }
        }

        const double now = ImGuiMCP::GetTime();

        // Read cached layout
        auto* io = ImGuiMCP::GetIO();
        const float dh = io->DisplaySize.y;
        const auto& L = GetLayout(_bars.size());
        const float zoneW = L.zoneW;
        const float barGap = L.barGap;
        const float innerGp = L.innerGp;
        const float frameH = L.frameH;
        const float lblPad = L.lblPad;
        const float nameFt = L.nameFt;
        const float valFt = L.valFt;
        const float intrFt = L.intrFt;
        const float fbFt = L.fbFt;
        const float edgeH = L.edgeH;
        const float edgeV = L.edgeV;
        const float lblRowH = L.lblRowH;
        const float winH = L.winH;

        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize | ImGuiMCP::ImGuiWindowFlags_NoMove |
            ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
            ImGuiMCP::ImGuiWindowFlags_NoFocusOnAppearing | ImGuiMCP::ImGuiWindowFlags_NoNav |
            ImGuiMCP::ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiMCP::ImGuiWindowFlags_NoBackground;

        ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{ edgeH, dh - winH - edgeV }, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{ zoneW, winH }, ImGuiMCP::ImGuiCond_Always);

        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_WindowPadding, ImGuiMCP::ImVec2{ 0.0f, 0.0f });
        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_ItemSpacing, ImGuiMCP::ImVec2{ 0.0f, 0.0f });
        if (!ImGuiMCP::Begin("##slpp_EnjBars", nullptr, kFlags)) {
            ImGuiMCP::End();
            ImGuiMCP::PopStyleVar(2);
            return;
        }

        auto* dl = ImGuiMCP::GetWindowDrawList();

        for (size_t barIndex = 0; barIndex < _bars.size(); ++barIndex) {
            auto& b = _bars[barIndex];
            ImGuiMCP::PushID(static_cast<int>(b.formId));
            const ImGuiMCP::ImVec2 rowStart = ImGuiMCP::GetCursorScreenPos();

            // ── Label row ───────────────────────────────────────────────────
            // An invisible-label selectable placed underneath accepts user interaction.
            // This makes label row selectable (to pick actor as target partner) when focused.
            if (hud.IsFocused() && !b.isTarget) {
                if (ImGuiMCP::Selectable("##slpp_eboPartner", false, 0, ImGuiMCP::ImVec2{ zoneW, lblRowH }))
                    OnSelectPartner(b.formId);
                ImGuiMCP::SetCursorScreenPos(rowStart);
            }

            const float nameMaxX = rowStart.x + zoneW * 0.4f;
            const float valueMinX = rowStart.x + zoneW * 0.8f;
            const float rowMaxY = rowStart.y + lblRowH;

            // actor name (left)
            SetWindowFontSize(nameFt);
            ImGuiMCP::ImDrawListManager::PushClipRect(dl, rowStart,
                ImGuiMCP::ImVec2{ nameMaxX, rowMaxY }, true);
            DrawTextShadowed(dl,
                ImGuiMCP::ImVec2{ rowStart.x + lblPad, rowStart.y + lblPad },
                b.isTarget ? UI::Theme::Color::textPrimary : UI::Theme::Color::textSecondary,
                b.name);
            ImGuiMCP::ImDrawListManager::PopClipRect(dl);

            // enjoyment value (right)
            SetWindowFontSize(valFt);
            char valBuf[12];
            std::snprintf(valBuf, sizeof(valBuf), "%d",
                static_cast<int>(std::round(b.enjoyment)));
            const ImGuiMCP::ImU32 valCol = b.enjoyment > 100.0f ? UI::Theme::Enjoyment::overflowLow : b.enjoyment < 0.0f ? UI::Theme::Enjoyment::negativeHigh :
                                                                                                                           UI::Theme::Color::textSecondary;
            const float valX = rowStart.x + zoneW - ImGuiMCP::CalcTextSize(valBuf).x - lblPad;
            ImGuiMCP::ImDrawListManager::PushClipRect(dl,
                ImGuiMCP::ImVec2{ valueMinX, rowStart.y }, ImGuiMCP::ImVec2{ rowStart.x + zoneW, rowMaxY }, true);
            DrawTextShadowed(dl, ImGuiMCP::ImVec2{ valX, rowStart.y + lblPad }, valCol, valBuf);
            ImGuiMCP::ImDrawListManager::PopClipRect(dl);

            // interaction string (centre)
            if (b.interactions[0] != '\0') {
                SetWindowFontSize(intrFt);
                const float intrX = nameMaxX +
                                    (valueMinX - nameMaxX - ImGuiMCP::CalcTextSize(b.interactions).x) * 0.5f;
                ImGuiMCP::ImDrawListManager::PushClipRect(dl,
                    ImGuiMCP::ImVec2{ nameMaxX, rowStart.y }, ImGuiMCP::ImVec2{ valueMinX, rowMaxY }, true);
                DrawTextShadowed(dl, ImGuiMCP::ImVec2{ intrX, rowStart.y + lblPad },
                    UI::Theme::Color::textMuted, b.interactions);
                ImGuiMCP::ImDrawListManager::PopClipRect(dl);
            }

            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ zoneW, lblRowH });
            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, innerGp });

            // ── Enj Bar frame ───────────────────────────────────────────────
            const ImGuiMCP::ImVec2 frameMin = ImGuiMCP::GetCursorScreenPos();
            const ImGuiMCP::ImVec2 frameMax{ frameMin.x + zoneW, frameMin.y + frameH };

            // drop shadow
            ImGuiMCP::ImDrawListManager::AddRectFilled(dl,
                ImGuiMCP::ImVec2{ frameMin.x - 2, frameMin.y + 2 },
                ImGuiMCP::ImVec2{ frameMax.x + 2, frameMax.y + 6 },
                UI::Theme::Enjoyment::frameShadow, 2.0f, 0);

            // track
            ImGuiMCP::ImDrawListManager::AddRectFilled(dl, frameMin, frameMax, UI::Theme::Enjoyment::frameSurface, 0.0f, 0);
            ImGuiMCP::ImDrawListManager::AddRect(dl, frameMin, frameMax, UI::Theme::Enjoyment::frameBorder, 0.0f, 0, 1.0f);

            // outer white rim
            ImGuiMCP::ImDrawListManager::AddRect(dl,
                ImGuiMCP::ImVec2{ frameMin.x - 1, frameMin.y - 1 },
                ImGuiMCP::ImVec2{ frameMax.x + 1, frameMax.y + 1 },
                UI::Theme::Enjoyment::frameRim, 0.0f, 0, 1.0f);

            // inner top shine
            ImGuiMCP::ImDrawListManager::AddLine(dl,
                ImGuiMCP::ImVec2{ frameMin.x + 1, frameMin.y + 1 },
                ImGuiMCP::ImVec2{ frameMax.x - 1, frameMin.y + 1 },
                UI::Theme::Enjoyment::frameShine, 1.0f);

            // fill
            const float frac = FillFraction(b.enjoyment);
            if (frac > 0.0f) {
                ImGuiMCP::ImU32 cLo, cHi;
                FillGradient(b.enjoyment, cLo, cHi);
                const float fillW = zoneW * frac;
                if (b.enjoyment < 0.0f) {
                    ImGuiMCP::ImDrawListManager::AddRectFilledMultiColor(dl,
                        ImGuiMCP::ImVec2{ frameMax.x - fillW, frameMin.y }, frameMax,
                        cLo, cHi, cHi, cLo);
                } else {
                    ImGuiMCP::ImDrawListManager::AddRectFilledMultiColor(dl,
                        frameMin, ImGuiMCP::ImVec2{ frameMin.x + fillW, frameMax.y },
                        cLo, cHi, cHi, cLo);
                }
            }

            // ── Green zone + needle ─────────────────────────────────────────
            if (b.isGameDpt && b.enjoyment >= kGameEnjDrawMin) {
                const float halfWidth = GreenZoneHalfWidth(b.enjoyment);
                const float greenStart = 0.5f - halfWidth;
                const float greenEnd = 0.5f + halfWidth;
                const float gx0 = frameMin.x + greenStart * zoneW;
                const float gx1 = frameMin.x + greenEnd * zoneW;
                const bool inZone = _needlePosition >= greenStart && _needlePosition <= greenEnd;

                ImGuiMCP::ImDrawListManager::AddRectFilled(dl,
                    ImGuiMCP::ImVec2{ gx0, frameMin.y }, ImGuiMCP::ImVec2{ gx1, frameMax.y },
                    inZone ? UI::Theme::Enjoyment::zoneActive : UI::Theme::Enjoyment::zoneIdle, 0.0f, 0);
                ImGuiMCP::ImDrawListManager::AddLine(dl,
                    ImGuiMCP::ImVec2{ gx0, frameMin.y }, ImGuiMCP::ImVec2{ gx0, frameMax.y },
                    inZone ? UI::Theme::Enjoyment::zoneFocused : UI::Theme::Enjoyment::zoneBorder, 1.0f);
                ImGuiMCP::ImDrawListManager::AddLine(dl,
                    ImGuiMCP::ImVec2{ gx1, frameMin.y }, ImGuiMCP::ImVec2{ gx1, frameMax.y },
                    inZone ? UI::Theme::Enjoyment::zoneFocused : UI::Theme::Enjoyment::zoneBorder, 1.0f);

                // zone center line
                const float zoneCx = (gx0 + gx1) * 0.5f;
                ImGuiMCP::ImDrawListManager::AddLine(dl,
                    ImGuiMCP::ImVec2{ zoneCx, frameMin.y }, ImGuiMCP::ImVec2{ zoneCx, frameMax.y },
                    inZone ? UI::Theme::Enjoyment::zoneCenterActive : UI::Theme::Enjoyment::zoneCenter, 1.0f);

                // needle rect
                const float nx = frameMin.x + _needlePosition * zoneW;
                const float nw = innerGp * 0.667f;
                const float nTop = frameMin.y - innerGp * 0.667f;
                const float nBot = frameMax.y + innerGp * 0.667f;
                const ImGuiMCP::ImU32 nCol = inZone ? UI::Theme::Enjoyment::needleActive : UI::Theme::Enjoyment::needle;
                ImGuiMCP::ImDrawListManager::AddRectFilled(dl,
                    ImGuiMCP::ImVec2{ nx - nw * 0.5f, nTop }, ImGuiMCP::ImVec2{ nx + nw * 0.5f, nBot },
                    nCol, 0.0f, 0);

                // small downward-pointing triangle beneath the needle
                ImGuiMCP::ImDrawListManager::AddTriangleFilled(dl,
                    ImGuiMCP::ImVec2{ nx - innerGp, nBot },
                    ImGuiMCP::ImVec2{ nx + innerGp, nBot },
                    ImGuiMCP::ImVec2{ nx, nBot + lblPad * 2.0f },
                    nCol);

                // feedback flash
                if (_feedbackActorId == b.formId && now < _feedbackUntil) {
                    ImGuiMCP::ImDrawListManager::AddRectFilled(dl, frameMin, frameMax,
                        _feedbackHit ? UI::Theme::Enjoyment::feedbackHit : UI::Theme::Enjoyment::feedbackMiss,
                        0.0f, 0);
                    SetWindowFontSize(fbFt);
                    const char* fbStr = _feedbackHit ? "HIT" : "MISS";
                    const ImGuiMCP::ImU32 fbCol = _feedbackHit ? UI::Theme::Enjoyment::hit : UI::Theme::Enjoyment::miss;
                    const ImGuiMCP::ImVec2 fbSz = ImGuiMCP::CalcTextSize(fbStr);
                    DrawTextShadowed(dl,
                        ImGuiMCP::ImVec2{ frameMax.x - fbSz.x - innerGp * 1.667f,
                            frameMin.y + (frameH - fbSz.y) * 0.5f },
                        fbCol, fbStr);
                }
            }

            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ zoneW, frameH });

            // highlight border for whichever actor is currently targeted
            if (b.isTarget) {
                constexpr auto tc = UI::Theme::Enjoyment::targetBorder;
                ImGuiMCP::ImDrawListManager::AddRect(dl, frameMin, frameMax, tc, 0.0f, 0, 1.0f);
                ImGuiMCP::ImDrawListManager::AddRect(dl,
                    ImGuiMCP::ImVec2{ frameMin.x - 1, frameMin.y - 1 },
                    ImGuiMCP::ImVec2{ frameMax.x + 1, frameMax.y + 1 },
                    tc, 0.0f, 0, 1.0f);
            }

            if (barIndex + 1 < _bars.size())
                ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, barGap });
            ImGuiMCP::PopID();
        }

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();
        ImGuiMCP::PopStyleVar(2);
    }
}
