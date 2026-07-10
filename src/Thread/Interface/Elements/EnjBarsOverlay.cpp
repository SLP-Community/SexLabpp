#include "EnjBarsOverlay.h"

namespace Thread::Interface
{
    void EnjBarsOverlay::Init()
    {
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (!inst) return;
        if (!inst->GetThreadProperty<bool>("ElementUI_EnjBars")) return;
        if (!inst->GetThreadProperty<bool>("VarUI_SeparateOrgasm")) return;
        {
            std::lock_guard lock(s_mu);
            s_bars.clear();
            for (auto* actor : inst->GetActors()) {
                if (!actor) continue;
                ActorEnjBar b;
                b.formId = actor->GetFormID();
                std::snprintf(b.name, sizeof(b.name), "%s", actor->GetName());
                s_bars.push_back(b);
            }
        }
        s_needlePos.store(0.5f);
        s_needleDir.store(1.0f);
        s_timeCycle.store(0.0f);
        s_greenStart.store(0.5f - kGZoneDefault);
        s_greenEnd.store(0.5f + kGZoneDefault);
        s_needleRunning.store(false);
        s_fbActorId.store(0);
        s_fbHit.store(false);
        s_fbUntil.store(0.0);
        isVisible = true;
        SceneHUD::winEnjBars->IsOpen = true;
    }

    void EnjBarsOverlay::Destroy()
    {
        isVisible = false;
        s_needleRunning.store(false);
        if (SceneHUD::winEnjBars) SceneHUD::winEnjBars->IsOpen = false;
        std::lock_guard lock(s_mu);
        s_bars.clear();
    }

    // ── Pushers (game thread) ───────────────────────────────────────────────

    void EnjBarsOverlay::UpdateSlider(RE::Actor* a_actor, float a_enjoyment, RE::BSFixedString a_interactions)
    {
        if (!isVisible || !a_actor) return;
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

        std::lock_guard lock(s_mu);
        for (auto& b : s_bars) {
            if (b.formId != actorID) continue;
            b.enjoyment = a_enjoyment;
            std::snprintf(b.interactions, sizeof(b.interactions), "%s", intrBuf);
            if (b.isGameDpt && a_enjoyment < kGameEnjThresh)
                b.isGameDpt = false;
            return;
        }
    }

    void EnjBarsOverlay::UpdateHighlightedPartner(RE::Actor* a_partner)
    {
        if (!isVisible || !a_partner) return;
        const uint32_t actorID = a_partner->GetFormID();
        std::lock_guard lock(s_mu);
        for (auto& b : s_bars) b.isTarget = (b.formId == actorID);
    }

    void EnjBarsOverlay::RegisterRaiseEnjAttempt(RE::Actor* a_actor, float a_nextTimeCycle)
    {
        if (!isVisible || !a_actor) return;
        const uint32_t actorID = a_actor->GetFormID();

        const float needlePos = s_needlePos.load(std::memory_order_relaxed);
        const float greenStart = s_greenStart.load(std::memory_order_relaxed);
        const float greenEnd = s_greenEnd.load(std::memory_order_relaxed);

        std::lock_guard lock(s_mu);
        for (auto& b : s_bars) {
            if (b.formId != actorID) continue;
            if (b.enjoyment >= kGameEnjThresh && !b.isGameDpt) {
                b.isGameDpt = true;
                s_needleRunning.store(true, std::memory_order_relaxed);
            }
            const bool hit = (needlePos >= greenStart && needlePos <= greenEnd);
            s_fbActorId.store(actorID, std::memory_order_relaxed);
            s_fbHit.store(hit, std::memory_order_relaxed);
            s_fbUntil.store(ImGuiMCP::GetTime() + kFeedbackSec, std::memory_order_relaxed);
            s_timeCycle.store(a_nextTimeCycle, std::memory_order_relaxed);

            if (!SceneHUD::threadScript) return;
            Script::DispatchMethodCall(SceneHUD::threadScript, "OnRaiseEnjAttemptResult",
                SceneHUD::callbackPtr, hit ? 1 : 0);
        }
    }

    // ── Callbacks ───────────────────────────────────────────────────────────

    void EnjBarsOverlay::OnSelectPartner(uint32_t formId)
    {
        auto* actor = RE::TESForm::LookupByID<RE::Actor>(formId);
        if (!actor || actor->IsPlayerRef()) return;
        if (!SceneHUD::threadScript) return;
        Script::DispatchMethodCall(SceneHUD::threadScript, "SelectTargetPartner",
            SceneHUD::callbackPtr, std::move(actor));
    }

    float EnjBarsOverlay::FillFraction(float enj)
    {
        if (enj < 0.0f) return std::min(std::abs(enj) / 100.0f, 1.0f);
        const float m = std::fmod(enj, 100.0f);
        return (m == 0.0f && enj > 0.0f) ? 1.0f : std::min(m / 100.0f, 1.0f);
    }

    void EnjBarsOverlay::FillGradient(float enj, ImGuiMCP::ImU32& lo, ImGuiMCP::ImU32& hi)
    {
        if (enj < 0.0f)   { lo = ColorUI::EnjNegHi;  hi = ColorUI::EnjNegLo;  return; }
        if (enj > 100.0f) { lo = ColorUI::EnjOverLo; hi = ColorUI::EnjOverHi; return; }
        lo = ColorUI::EnjNormalLo; hi = ColorUI::EnjNormalHi;
    }

    // ── Layout Cache ────────────────────────────────────────────────────────
    const EnjBarsOverlay::LayoutCache& EnjBarsOverlay::GetLayout(size_t actorCount)
    {
        auto* io = ImGuiMCP::GetIO();
        const float dw = io->DisplaySize.x;
        const float dh = io->DisplaySize.y;
        const float factor = ScaleUI::pxScale(1.0f);
        if (s_layoutForFactor == factor && s_layoutForDw == dw &&
            s_layoutForDh == dh && s_layoutForCount == actorCount) {
            return s_layout;  // skip recompute when scale factor, display size, or actor count stays same
        }

        auto& L   = s_layout;
        L.zoneW   = std::clamp(ScaleUI::pxScale(260.0f), dw * 0.15f, ScaleUI::pxScale(360.0f));
        L.barGap  = ScaleUI::pxScale(4.5f);
        L.innerGp = ScaleUI::pxScale(2.0f);
        L.frameH  = ScaleUI::pxScale(10.0f);
        L.lblPad  = ScaleUI::pxScale(1.5f);
        L.nameFt  = ScaleUI::pxScale(8.5f);
        L.valFt   = ScaleUI::pxScale(8.0f);
        L.intrFt  = ScaleUI::pxScale(7.5f);
        L.fbFt    = ScaleUI::pxScale(10.0f);
        L.edgeH   = ScaleUI::pxScaleClamp(14.0f, 2.5f, 48.0f, dw);
        L.edgeV   = ScaleUI::pxScaleClamp(16.0f, 1.8f, 32.0f, dh);
        L.lblRowH = L.nameFt + L.lblPad * 2.0f;
        L.unitH   = L.lblRowH + L.innerGp + L.frameH + L.barGap;
        L.winH    = L.unitH * static_cast<float>(actorCount) - L.barGap;

        s_layoutForFactor = factor;
        s_layoutForDw     = dw;
        s_layoutForDh     = dh;
        s_layoutForCount  = actorCount;
        return L;
    }

    // ── Render (D3D present thread) ─────────────────────────────────────────

    void __stdcall EnjBarsOverlay::Render()
    {
        if (!isVisible || !SceneHUD::IsActive()) return;

        // ── Snapshot s_bars (hold lock as briefly as possible) ────────────────
        // The lock covers ONLY the copy. All drawing happens against the snapshot
        // so the game thread is never blocked during actual rendering.
        std::vector<ActorEnjBar> snap;
        {
            std::lock_guard lock(s_mu);
            snap = s_bars;
        }
        if (snap.empty()) return;

        // ── Advance needle (atomic reads/writes, no lock needed) ──────────────
        const float dt        = ImGuiMCP::GetIO()->DeltaTime;
        const float timeCycle = s_timeCycle.load(std::memory_order_relaxed);
        const bool  running   = s_needleRunning.load(std::memory_order_relaxed);

        const bool anyQualify = std::any_of(snap.begin(), snap.end(),
            [](const ActorEnjBar& b){ return b.isGameDpt && b.enjoyment >= kGameEnjDrawMin; });

        if (!anyQualify && running)
            s_needleRunning.store(false, std::memory_order_relaxed);

        if (anyQualify && running && timeCycle > 0.0f) {
            float pos = s_needlePos.load(std::memory_order_relaxed);
            float dir = s_needleDir.load(std::memory_order_relaxed);
            pos += dir * (1.0f / timeCycle) * std::min(dt, 0.1f);
            if (pos >= 1.0f) { pos = 1.0f; dir = -1.0f; }
            if (pos <= 0.0f) { pos = 0.0f; dir =  1.0f; }
            s_needlePos.store(pos, std::memory_order_relaxed);
            s_needleDir.store(dir, std::memory_order_relaxed);
        }

        // Read all atomic display state once before drawing
        const float needlePos  = s_needlePos.load(std::memory_order_relaxed);
        const float greenStart = s_greenStart.load(std::memory_order_relaxed);
        const float greenEnd   = s_greenEnd.load(std::memory_order_relaxed);
        const uint32_t fbId    = s_fbActorId.load(std::memory_order_relaxed);
        const bool fbHit       = s_fbHit.load(std::memory_order_relaxed);
        const double fbUntil   = s_fbUntil.load(std::memory_order_relaxed);
        const double now       = ImGuiMCP::GetTime();

        // Read cached layout
        auto* io = ImGuiMCP::GetIO();
        const float dh = io->DisplaySize.y;
        const auto& L = GetLayout(snap.size());
        const float zoneW   = L.zoneW;
        const float barGap  = L.barGap;
        const float innerGp = L.innerGp;
        const float frameH  = L.frameH;
        const float lblPad  = L.lblPad;
        const float nameFt  = L.nameFt;
        const float valFt   = L.valFt;
        const float intrFt  = L.intrFt;
        const float fbFt    = L.fbFt;
        const float edgeH   = L.edgeH;
        const float edgeV   = L.edgeV;
        const float lblRowH = L.lblRowH;
        const float winH    = L.winH;

        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize | ImGuiMCP::ImGuiWindowFlags_NoMove |
            ImGuiMCP::ImGuiWindowFlags_NoScrollbar | ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiMCP::ImGuiWindowFlags_NoFocusOnAppearing | ImGuiMCP::ImGuiWindowFlags_NoNav |
            ImGuiMCP::ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiMCP::ImGuiWindowFlags_NoBackground;

        ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{edgeH, dh - winH - edgeV}, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{zoneW, winH}, ImGuiMCP::ImGuiCond_Always);

        if (!ImGuiMCP::Begin("##slpp_EnjBars", nullptr, kFlags)) { ImGuiMCP::End(); return; }

        auto* dl = ImGuiMCP::GetWindowDrawList();

        for (auto& b : snap) {
            ImGuiMCP::PushID(static_cast<int>(b.formId));
            const ImGuiMCP::ImVec2 rowStart = ImGuiMCP::GetCursorScreenPos();

            // ── Label row ───────────────────────────────────────────────────
            // An invisible-label selectable placed underneath accepts user interaction.
            // This makes label row selectable (to pick actor as target partner) when focused.
            if (SceneHUD::focusMode && !b.isTarget) {
                if (ImGuiMCP::Selectable("##slpp_eboPartner", false, 0, ImGuiMCP::ImVec2{ zoneW, lblRowH }))
                    OnSelectPartner(b.formId);
                ImGuiMCP::SetCursorScreenPos(rowStart);
            }

            // actor name (left)
            ImGuiMCP::SetWindowFontScale(nameFt / ImGuiMCP::GetFontSize());
            DrawTextShadowed(dl,
                ImGuiMCP::ImVec2{rowStart.x + lblPad, rowStart.y + lblPad},
                b.isTarget ? ColorUI::TextPrimary : ColorUI::TextSecond,
                b.name);

            // enjoyment value (right)
            ImGuiMCP::SetWindowFontScale(valFt / ImGuiMCP::GetFontSize());
            char valBuf[12];
            std::snprintf(valBuf, sizeof(valBuf), "%d",
                static_cast<int>(std::round(b.enjoyment)));
            const ImGuiMCP::ImU32 valCol = b.enjoyment > 100.0f ? ColorUI::EnjOverLo
                               : b.enjoyment < 0.0f   ? ColorUI::EnjNegHi
                               : ColorUI::TextSecond;
            const float valX = rowStart.x + zoneW - ImGuiMCP::CalcTextSize(valBuf).x - lblPad;
            DrawTextShadowed(dl, ImGuiMCP::ImVec2{valX, rowStart.y + lblPad}, valCol, valBuf);

            // interaction string (centre)
            if (b.interactions[0] != '\0') {
                ImGuiMCP::SetWindowFontScale(intrFt / ImGuiMCP::GetFontSize());
                const float intrX = rowStart.x +
                    (zoneW - std::min(ImGuiMCP::CalcTextSize(b.interactions).x, zoneW * 0.39f)) * 0.5f;
                DrawTextShadowed(dl, ImGuiMCP::ImVec2{intrX, rowStart.y + lblPad},
                    ColorUI::TextMuted, b.interactions);
            }

            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{zoneW, lblRowH});
            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{0.0f, innerGp});

            // ── Enj Bar frame ───────────────────────────────────────────────
            const ImGuiMCP::ImVec2 frameMin = ImGuiMCP::GetCursorScreenPos();
            const ImGuiMCP::ImVec2 frameMax{ frameMin.x + zoneW, frameMin.y + frameH };

            // drop shadow
            ImGuiMCP::ImDrawListManager::AddRectFilled(dl,
                ImGuiMCP::ImVec2{frameMin.x - 2, frameMin.y + 2},
                ImGuiMCP::ImVec2{frameMax.x + 2, frameMax.y + 6},
                IM_COL32(0, 0, 0, 150), 2.0f, 0);

            // track
            ImGuiMCP::ImDrawListManager::AddRectFilled(dl, frameMin, frameMax, IM_COL32(16, 16, 18, 255), 0.0f, 0);
            ImGuiMCP::ImDrawListManager::AddRect(dl, frameMin, frameMax, IM_COL32(40, 40, 48, 255), 0.0f, 0, 1.0f);

            // outer white rim
            ImGuiMCP::ImDrawListManager::AddRect(dl,
                ImGuiMCP::ImVec2{frameMin.x - 1, frameMin.y - 1},
                ImGuiMCP::ImVec2{frameMax.x + 1, frameMax.y + 1},
                IM_COL32(255, 255, 255, 20), 0.0f, 0, 1.0f);

            // inner top shine
            ImGuiMCP::ImDrawListManager::AddLine(dl,
                ImGuiMCP::ImVec2{frameMin.x + 1, frameMin.y + 1},
                ImGuiMCP::ImVec2{frameMax.x - 1, frameMin.y + 1},
                IM_COL32(255, 255, 255, 10), 1.0f);

            // fill
            const float frac = FillFraction(b.enjoyment);
            if (frac > 0.0f) {
                ImGuiMCP::ImU32 cLo, cHi;
                FillGradient(b.enjoyment, cLo, cHi);
                const float fillW = zoneW * frac;
                if (b.enjoyment < 0.0f) {
                    ImGuiMCP::ImDrawListManager::AddRectFilledMultiColor(dl,
                        ImGuiMCP::ImVec2{frameMax.x - fillW, frameMin.y}, frameMax,
                        cLo, cHi, cHi, cLo);
                } else {
                    ImGuiMCP::ImDrawListManager::AddRectFilledMultiColor(dl,
                        frameMin, ImGuiMCP::ImVec2{frameMin.x + fillW, frameMax.y},
                        cLo, cHi, cHi, cLo);
                }
            }

            // ── Green zone + needle ─────────────────────────────────────────
            if (b.isGameDpt && b.enjoyment >= kGameEnjDrawMin) {
                
                const float doff = std::max(kGZoneMin, kGZoneDefault - (b.enjoyment - kGameEnjDrawMin) * 0.00375f);
                s_greenStart.store(0.5f - doff, std::memory_order_relaxed);
                s_greenEnd.store(0.5f + doff, std::memory_order_relaxed);

                const float gx0 = frameMin.x + greenStart * zoneW;
                const float gx1 = frameMin.x + greenEnd * zoneW;
                const bool inZone = (needlePos >= greenStart && needlePos <= greenEnd);

                ImGuiMCP::ImDrawListManager::AddRectFilled(dl,
                    ImGuiMCP::ImVec2{gx0, frameMin.y}, ImGuiMCP::ImVec2{gx1, frameMax.y},
                    inZone ? ColorUI::EnjZoneActive : ColorUI::EnjZoneIdle, 0.0f, 0);
                ImGuiMCP::ImDrawListManager::AddLine(dl,
                    ImGuiMCP::ImVec2{gx0, frameMin.y}, ImGuiMCP::ImVec2{gx0, frameMax.y},
                    inZone ? ColorUI::EnjZoneBdAct : ColorUI::EnjZoneBd, 1.0f);
                ImGuiMCP::ImDrawListManager::AddLine(dl,
                    ImGuiMCP::ImVec2{gx1, frameMin.y}, ImGuiMCP::ImVec2{gx1, frameMax.y},
                    inZone ? ColorUI::EnjZoneBdAct : ColorUI::EnjZoneBd, 1.0f);

                // zone center line
                const float zoneCx = (gx0 + gx1) * 0.5f;
                ImGuiMCP::ImDrawListManager::AddLine(dl,
                    ImGuiMCP::ImVec2{zoneCx, frameMin.y}, ImGuiMCP::ImVec2{zoneCx, frameMax.y},
                    inZone ? IM_COL32(100, 230, 80, 77) : IM_COL32(80, 180, 60, 51), 1.0f);

                // needle rect
                const float nx   = frameMin.x + needlePos * zoneW;
                const float nw   = innerGp * 0.667f;
                const float nTop = frameMin.y - innerGp * 0.667f;
                const float nBot = frameMax.y + innerGp * 0.667f;
                const ImGuiMCP::ImU32 nCol = inZone ? ColorUI::EnjNeedleAct : ColorUI::EnjNeedle;
                ImGuiMCP::ImDrawListManager::AddRectFilled(dl,
                    ImGuiMCP::ImVec2{nx - nw * 0.5f, nTop}, ImGuiMCP::ImVec2{nx + nw * 0.5f, nBot},
                    nCol, 0.0f, 0);

                // small downward-pointing triangle beneath the needle
                ImGuiMCP::ImDrawListManager::AddTriangleFilled(dl,
                    ImGuiMCP::ImVec2{nx - innerGp, nBot},
                    ImGuiMCP::ImVec2{nx + innerGp, nBot},
                    ImGuiMCP::ImVec2{nx, nBot + lblPad * 2.0f},
                    nCol);

                // feedback flash
                if (fbId == b.formId && now < fbUntil) {
                    ImGuiMCP::ImDrawListManager::AddRectFilled(dl, frameMin, frameMax,
                        fbHit ? IM_COL32(60, 200, 80, 89) : IM_COL32(200, 60, 40, 97),
                        0.0f, 0);
                    ImGuiMCP::SetWindowFontScale(fbFt / ImGuiMCP::GetFontSize());
                    const char* fbStr = fbHit ? "HIT" : "MISS";
                    const ImGuiMCP::ImU32 fbCol = fbHit ? ColorUI::EnjHitText : ColorUI::EnjMissText;
                    const ImGuiMCP::ImVec2 fbSz = ImGuiMCP::CalcTextSize(fbStr);
                    DrawTextShadowed(dl,
                        ImGuiMCP::ImVec2{frameMax.x - fbSz.x - innerGp * 1.667f,
                               frameMin.y + (frameH - fbSz.y) * 0.5f},
                        fbCol, fbStr);
                }
            }

            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{zoneW, frameH});

            // highlight border for whichever actor is currently targeted
            if (b.isTarget) {
                constexpr ImGuiMCP::ImU32 tc = IM_COL32(106, 96, 85, 255);
                ImGuiMCP::ImDrawListManager::AddRect(dl, frameMin, frameMax, tc, 0.0f, 0, 1.0f);
                ImGuiMCP::ImDrawListManager::AddRect(dl,
                    ImGuiMCP::ImVec2{frameMin.x - 1, frameMin.y - 1},
                    ImGuiMCP::ImVec2{frameMax.x + 1, frameMax.y + 1},
                    tc, 0.0f, 0, 1.0f);
            }

            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{0.0f, barGap});
            ImGuiMCP::PopID();
        }

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();
    }
}
