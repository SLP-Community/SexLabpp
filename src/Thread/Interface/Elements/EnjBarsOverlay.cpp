#include "EnjBarsOverlay.h"
#include "Thread/Interface/SceneHUD.h"

#include <array>
#include <limits>

namespace Thread::Interface
{
    using UI::DrawTextShadowed;
    using UI::SetWindowFontSize;

    namespace
    {
        float SmoothStep(float a_edge0, float a_edge1, float a_value)
        {
            const float amount = std::clamp((a_value - a_edge0) / (a_edge1 - a_edge0), 0.0f, 1.0f);
            return amount * amount * (3.0f - 2.0f * amount);
        }

        // Computes a seed from a formId
        float PhaseFromId(std::uint32_t a_formId, std::uint32_t a_salt)
        {
            std::uint32_t value = a_formId ^ a_salt;
            value ^= value >> 16;
            value *= 0x7FEB352Du;
            value ^= value >> 15;
            value *= 0x846CA68Bu;
            value ^= value >> 16;
            return static_cast<float>(value) / static_cast<float>(std::numeric_limits<std::uint32_t>::max()) * 6.283185307f;
        }
    }

    void EnjBarsOverlay::Init(Instance& a_instance)
    {
        _bars.clear();
        for (auto* actor : a_instance.GetActors()) {
            if (!actor)
                continue;
            ActorEnjBar bar;
            bar.formId = actor->GetFormID();
            std::snprintf(bar.name, sizeof(bar.name), "%s", actor->GetName());
            if (actor->IsPlayerRef())
                _bars.insert(_bars.begin(), bar);
            else
                _bars.push_back(bar);
        }
        _needlePosition = 0.5f;
        _needleDirection = 1.0f;
        _timeCycle = 0.0f;
        _needleRunning = false;
        _feedbackActorId = 0;
        _feedbackHit = false;
        _feedbackUntil = 0.0;
    }

    // ── Pushers (game thread) ───────────────────────────────────────────────

    void EnjBarsOverlay::UpdateSlider(RE::Actor* a_actor, float a_enjoyment, RE::BSFixedString a_interactions)
    {
        if (!a_actor)
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
            // Enjoyment stays between -100 and 100. This also keeps stale script values from showing as overflow.
            b.enjoyment = std::clamp(a_enjoyment, -100.0f, 100.0f);
            std::snprintf(b.interactions, sizeof(b.interactions), "%s", intrBuf);
            if (b.isGameDpt && a_enjoyment < kGameEnjThresh)
                b.isGameDpt = false;
            return;
        }
    }

    void EnjBarsOverlay::UpdateHighlightedPartner(RE::Actor* a_partner)
    {
        if (!a_partner || a_partner->IsPlayerRef())
            return;
        const uint32_t actorID = a_partner->GetFormID();
        for (auto& b : _bars) b.isTarget = (b.formId == actorID);
    }

    void EnjBarsOverlay::RegisterRaiseEnjAttempt(SceneHUD& a_hud, RE::Actor* a_actor, float a_nextTimeCycle)
    {
        if (!a_actor)
            return;
        const uint32_t actorID = a_actor->GetFormID();

        for (auto& b : _bars) {
            if (b.formId != actorID)
                continue;
            if (b.enjoyment < kGameEnjThresh)
                return;
            _timeCycle = std::max(a_nextTimeCycle, kGameMinTimeCycle);
            if (!b.isGameDpt) {
                b.isGameDpt = true;
                _needleRunning = true;
                _needlePosition = 0.0f;
                _needleDirection = 1.0f;
                // First input starts the needle. Scoring the centered initial position would always hit.
                return;
            }
            const float halfWidth = GreenZoneHalfWidth(b.enjoyment);
            const bool hit = _needlePosition >= 0.5f - halfWidth && _needlePosition <= 0.5f + halfWidth;
            _feedbackActorId = actorID;
            _feedbackHit = hit;
            _feedbackUntil = ImGuiMCP::GetTime() + kFeedbackSec;

            if (!a_hud.GetThreadScript())
                return;
            Script::DispatchMethodCall(a_hud.GetThreadScript(), "OnRaiseEnjAttemptResult",
                a_hud.GetCallback(), hit ? 1 : 0);
            return;
        }
    }

    // ── Callbacks ───────────────────────────────────────────────────────────

    void EnjBarsOverlay::OnSelectPartner(SceneHUD& a_hud, uint32_t formId)
    {
        auto* actor = RE::TESForm::LookupByID<RE::Actor>(formId);
        if (!actor || actor->IsPlayerRef())
            return;
        if (!a_hud.GetThreadScript())
            return;
        Script::DispatchMethodCall(a_hud.GetThreadScript(), "SelectTargetPartner",
            a_hud.GetCallback(), std::move(actor));
    }

    float EnjBarsOverlay::FillFraction(float a_enjoyment)
    {
        // Negative enjoyment fills from the right, but both sides still stop at a full bar.
        if (a_enjoyment < 0.0f)
            return std::min(std::abs(a_enjoyment) / 100.0f, 1.0f);
        return std::min(a_enjoyment / 100.0f, 1.0f);
    }

    void EnjBarsOverlay::FillGradient(float a_enjoyment, ImGuiMCP::ImU32& a_low, ImGuiMCP::ImU32& a_high)
    {
        if (a_enjoyment < 0.0f) {
            a_low = UI::Theme::Enjoyment.negativeHigh;
            a_high = UI::Theme::Enjoyment.negativeLow;
            return;
        }
        a_low = UI::Theme::Enjoyment.normalLow;
        a_high = UI::Theme::Enjoyment.normalHigh;
    }

    float EnjBarsOverlay::GreenZoneHalfWidth(float a_enjoyment)
    {
        return std::max(kGZoneMin, kGZoneDefault - (a_enjoyment - kGameEnjDrawMin) * 0.00375f);
    }

    EnjBarsOverlay::Layout EnjBarsOverlay::GetLayout(UI::Scale& a_scale, size_t actorCount)
    {
        auto* io = ImGuiMCP::GetIO();
        const float dw = io->DisplaySize.x;
        const float dh = io->DisplaySize.y;

        Layout layout;
        auto& L = layout;
        L.zoneW = std::clamp(a_scale.Px(260.0f), dw * 0.15f, a_scale.Px(360.0f));
        L.barGap = a_scale.Px(4.5f);
        L.innerGp = a_scale.Px(2.0f);
        L.frameH = a_scale.Px(UI::Theme::FontSize.body);
        L.lblPad = a_scale.Px(1.5f);
        L.nameFt = a_scale.TextPx(UI::Theme::FontSize.metadata);
        L.valFt = a_scale.TextPx(UI::Theme::FontSize.smallText);
        L.intrFt = a_scale.TextPx(UI::Theme::FontSize.detail);
        L.fbFt = std::min(a_scale.TextPx(UI::Theme::FontSize.body), L.frameH);
        L.edgeH = a_scale.Clamp(14.0f, 2.5f, 48.0f, dw);
        L.edgeV = a_scale.Clamp(16.0f, 1.8f, 32.0f, dh);
        L.lblRowH = std::max(a_scale.Px(UI::Theme::FontSize.metadata), L.nameFt) + L.lblPad * 2.0f;
        L.unitH = L.lblRowH + L.innerGp + L.frameH + L.barGap;
        L.winH = L.unitH * static_cast<float>(actorCount) - L.barGap;

        return layout;
    }

    void EnjBarsOverlay::Render(SceneHUD& a_hud)
    {
        if (_bars.empty())
            return;
        auto* inst = a_hud.GetThreadInstance();
        if (!inst)
            return;
        auto& scale = a_hud.GetScale();

        const float deltaTime = ImGuiMCP::GetIO()->DeltaTime;
        const float animationDelta = std::min(deltaTime, 0.1f);

        const bool anyQualify = std::ranges::any_of(_bars,
            [](const ActorEnjBar& b) { return b.isGameDpt && b.enjoyment >= kGameEnjDrawMin; });

        if (!anyQualify)
            _needleRunning = false;

        if (anyQualify && _needleRunning && _timeCycle > 0.0f) {
            _needlePosition += _needleDirection * (1.0f / _timeCycle) * animationDelta;
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

        // Calculate layout
        auto* io = ImGuiMCP::GetIO();
        const float dh = io->DisplaySize.y;
        const auto L = GetLayout(scale, _bars.size());
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
        const float frameRounding = scale.Px(UI::Theme::Geometry.roundingEnjBar);
        const float clipPadding = frameRounding + scale.Px(2.0f);

        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize | ImGuiMCP::ImGuiWindowFlags_NoMove |
            ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
            ImGuiMCP::ImGuiWindowFlags_NoFocusOnAppearing | ImGuiMCP::ImGuiWindowFlags_NoNav |
            ImGuiMCP::ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiMCP::ImGuiWindowFlags_NoBackground;

        ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{ edgeH - clipPadding, dh - winH - edgeV - clipPadding }, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{ zoneW + clipPadding * 2.0f, winH + clipPadding * 2.0f }, ImGuiMCP::ImGuiCond_Always);

        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_WindowPadding, ImGuiMCP::ImVec2{ clipPadding, clipPadding });
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
            if (a_hud.IsFocused() && !b.isTarget) {
                if (ImGuiMCP::Selectable("##slpp_eboPartner", false, 0, ImGuiMCP::ImVec2{ zoneW, lblRowH }))
                    OnSelectPartner(a_hud, b.formId);
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
                b.isTarget ? UI::Theme::Color.textPrimary : UI::Theme::Color.textSecondary,
                b.name);
            ImGuiMCP::ImDrawListManager::PopClipRect(dl);

            // enjoyment value (right)
            SetWindowFontSize(valFt);
            char valBuf[12];
            std::snprintf(valBuf, sizeof(valBuf), "%d",
                static_cast<int>(std::round(b.enjoyment)));
            const ImGuiMCP::ImU32 valCol = b.enjoyment < 0.0f ? UI::Theme::Enjoyment.negativeHigh : UI::Theme::Color.textSecondary;
            const float valX = rowStart.x + zoneW - ImGuiMCP::CalcTextSize(valBuf).x - lblPad;
            ImGuiMCP::ImDrawListManager::PushClipRect(dl,
                ImGuiMCP::ImVec2{ valueMinX, rowStart.y }, ImGuiMCP::ImVec2{ rowStart.x + zoneW, rowMaxY }, true);
            DrawTextShadowed(dl, ImGuiMCP::ImVec2{ valX, rowStart.y + lblPad }, valCol, valBuf);
            ImGuiMCP::ImDrawListManager::PopClipRect(dl);

            // interaction string (centre)
            if (b.interactions[0] != '\0') {
                const bool showInterText = inst->GetThreadProperty<bool>("VarUI_EnjInterText");
                const ImGuiMCP::ImU32 interStrCol = showInterText
                    ? UI::Theme::Enjoyment.interactionText
                    : (UI::Theme::Enjoyment.interactionText & ~IM_COL32_A_MASK);  // zero alpha

                SetWindowFontSize(intrFt);
                const float intrX = nameMaxX +
                                    (valueMinX - nameMaxX - ImGuiMCP::CalcTextSize(b.interactions).x) * 0.5f;
                ImGuiMCP::ImDrawListManager::PushClipRect(dl,
                    ImGuiMCP::ImVec2{ nameMaxX, rowStart.y }, ImGuiMCP::ImVec2{ valueMinX, rowMaxY }, true);
                DrawTextShadowed(dl, ImGuiMCP::ImVec2{ intrX, rowStart.y + lblPad },
                    interStrCol, b.interactions);
                ImGuiMCP::ImDrawListManager::PopClipRect(dl);
            }

            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ zoneW, lblRowH });
            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, innerGp });

            // ── Enj Bar frame ───────────────────────────────────────────────
            const ImGuiMCP::ImVec2 frameMin = ImGuiMCP::GetCursorScreenPos();
            const ImGuiMCP::ImVec2 frameMax{ frameMin.x + zoneW, frameMin.y + frameH };
            const ImGuiMCP::ImVec2 borderMin{ frameMin.x - 0.5f, frameMin.y - 0.5f };
            const ImGuiMCP::ImVec2 borderMax{ frameMax.x + 0.5f, frameMax.y + 0.5f };
            const float frameClipRadius = frameRounding >= 0.5f
                                              ? std::min(frameRounding, std::max(std::min(frameH, zoneW) * 0.5f - 1.0f, 0.0f))
                                              : 0.0f;

            const float targetFill = FillFraction(b.enjoyment);
            if (std::abs(targetFill - b.targetFill) > 0.0005f) {
                if (targetFill > b.targetFill)
                    b.trailingFill = targetFill;
                b.targetFill = targetFill;
            }
            b.displayedFill += (targetFill - b.displayedFill) * (1.0f - std::exp(-UI::Theme::Enjoyment.fillEaseRate * animationDelta));
            b.trailingFill += (targetFill - b.trailingFill) * (1.0f - std::exp(-UI::Theme::Enjoyment.trailEaseRate * animationDelta));
            if (std::abs(targetFill - b.displayedFill) < 0.0005f)
                b.displayedFill = targetFill;
            if (std::abs(targetFill - b.trailingFill) < 0.0005f)
                b.trailingFill = targetFill;

            // track
            ImGuiMCP::ImDrawListManager::AddRectFilled(dl, frameMin, frameMax,
                UI::Theme::Enjoyment.frameSurface, frameRounding, ImGuiMCP::ImDrawFlags_RoundCornersAll);

            // trailing fill
            if (b.trailingFill > b.displayedFill + 0.0005f) {
                const float trailW = zoneW * b.trailingFill;
                const auto trailCorners = trailW >= zoneW - frameClipRadius ? ImGuiMCP::ImDrawFlags_RoundCornersAll : b.enjoyment < 0.0f ? ImGuiMCP::ImDrawFlags_RoundCornersRight :
                                                                                                                                          ImGuiMCP::ImDrawFlags_RoundCornersLeft;
                const ImGuiMCP::ImVec2 trailMin = b.enjoyment < 0.0f ? ImGuiMCP::ImVec2{ frameMax.x - trailW, frameMin.y } : frameMin;
                const ImGuiMCP::ImVec2 trailMax = b.enjoyment < 0.0f ? frameMax : ImGuiMCP::ImVec2{ frameMin.x + trailW, frameMax.y };
                ImGuiMCP::ImDrawListManager::AddRectFilled(dl, trailMin, trailMax,
                    UI::Theme::Enjoyment.fillTrail, frameRounding, trailCorners);
            }

            // fill
            const float frac = b.displayedFill;
            if (frac > 0.0f) {
                ImGuiMCP::ImU32 cLo, cHi;
                FillGradient(b.enjoyment, cLo, cHi);
                const float fillW = zoneW * frac;
                const bool negative = b.enjoyment < 0.0f;
                const auto fillCorners = frac >= 1.0f ? ImGuiMCP::ImDrawFlags_RoundCornersAll : b.enjoyment < 0.0f ? ImGuiMCP::ImDrawFlags_RoundCornersRight :
                                                                                                                     ImGuiMCP::ImDrawFlags_RoundCornersLeft;
                const ImGuiMCP::ImVec2 fillMin = negative ? ImGuiMCP::ImVec2{ frameMax.x - fillW, frameMin.y } : frameMin;
                const ImGuiMCP::ImVec2 fillMax = negative ? frameMax : ImGuiMCP::ImVec2{ frameMin.x + fillW, frameMax.y };
                if (fillW <= 1.5f) {
                    UI::DrawRoundedGradientRect(dl, fillMin, fillMax, cLo, cHi, frameRounding, fillCorners);
                } else {
                    float capRadius = 0.0f;
                    if (frameClipRadius > 0.0f) {
                        const float widthLimit = std::max(fillW - 1.0f, 0.0f);
                        capRadius = std::min(frameClipRadius, widthLimit);
                    }

                    const float subtle = SmoothStep(70.0f, 82.0f, b.enjoyment);
                    const float strong = SmoothStep(85.0f, 100.0f, b.enjoyment);
                    const float frequencyFactor = (0.5f + subtle * 0.4f + strong * 1.4f) * UI::Theme::Enjoyment.waveSpeed;
                    const float desiredAmplitude = scale.Px(0.75f + SmoothStep(50.0f, 82.0f, b.enjoyment) * 0.5f + strong * 1.75f) * UI::Theme::Enjoyment.waveIntensity;
                    const float edgeAmplitude = std::min(desiredAmplitude, std::max(fillW - capRadius - 0.5f, 0.0f) * 0.5f);
                    const float phaseA = PhaseFromId(b.formId, 0x68E31DA4u);
                    const float phaseB = PhaseFromId(b.formId, 0xB7E15162u);
                    const float time = static_cast<float>(now);
                    const auto edgeOffset = [&](float a_yFraction) {
                        const float yPhase = a_yFraction * 100.0f * UI::Theme::Enjoyment.waveSpatialFrequency;
                        return edgeAmplitude *
                               (0.7f * std::sin(1.6f * frequencyFactor * time + phaseA + yPhase * 0.06f) +
                                   0.4f * UI::Theme::Enjoyment.waveSecondaryStrength *
                                       std::sin(3.9f * frequencyFactor * time + phaseB - yPhase * 0.09f));
                    };

                    constexpr int edgeSamples = 16;
                    constexpr int arcSegments = 4;
                    std::array<ImGuiMCP::ImVec2, 32> points;
                    int pointCount = 0;
                    const float anchorX = negative ? frameMax.x : frameMin.x;
                    const float direction = negative ? -1.0f : 1.0f;
                    const auto point = [&](float a_distance, float a_y) {
                        return ImGuiMCP::ImVec2{ anchorX + direction * a_distance, a_y };
                    };

                    points[pointCount++] = point(capRadius, frameMin.y);
                    for (int sample = 0; sample <= edgeSamples; ++sample) {
                        const float yFraction = static_cast<float>(sample) / static_cast<float>(edgeSamples);
                        const float y = std::lerp(frameMin.y, frameMax.y, yFraction);
                        const float distanceFromHorizontalEdge = std::min(y - frameMin.y, frameMax.y - y);
                        float roundedFrameInset = 0.0f;
                        if (distanceFromHorizontalEdge < frameClipRadius) {
                            const float radiusOffset = frameClipRadius - distanceFromHorizontalEdge;
                            roundedFrameInset = frameClipRadius - std::sqrt(std::max(frameClipRadius * frameClipRadius - radiusOffset * radiusOffset, 0.0f));
                        }
                        const float edgeDistance = std::clamp(fillW + edgeOffset(yFraction), capRadius + 0.25f, zoneW - roundedFrameInset);
                        points[pointCount++] = point(edgeDistance, y);
                    }
                    points[pointCount++] = point(capRadius, frameMax.y);

                    if (capRadius > 0.0f) {
                        for (int segment = 1; segment <= arcSegments; ++segment) {
                            const float angle = 1.570796327f + 1.570796327f * static_cast<float>(segment) / static_cast<float>(arcSegments);
                            points[pointCount++] = point(capRadius + std::cos(angle) * capRadius,
                                frameMax.y - capRadius + std::sin(angle) * capRadius);
                        }
                        points[pointCount++] = point(0.0f, frameMin.y + capRadius);
                        for (int segment = 1; segment < arcSegments; ++segment) {
                            const float angle = 3.141592654f + 1.570796327f * static_cast<float>(segment) / static_cast<float>(arcSegments);
                            points[pointCount++] = point(capRadius + std::cos(angle) * capRadius,
                                frameMin.y + capRadius + std::sin(angle) * capRadius);
                        }
                    }

                    if (negative)
                        std::reverse(points.begin(), points.begin() + pointCount);

                    ImGuiMCP::ImDrawListManager::PushClipRect(dl, frameMin, frameMax, true);
                    const int vertexStart = dl->VtxBuffer.Size;
                    ImGuiMCP::ImDrawListManager::AddConcavePolyFilled(dl, points.data(), pointCount, IM_COL32(255, 255, 255, 255));
                    ImGuiMCP::ImDrawListManager::PopClipRect(dl);

                    const auto low = UI::Theme::ToVec4(cLo);
                    const auto high = UI::Theme::ToVec4(cHi);
                    for (int vertexIndex = vertexStart; vertexIndex < dl->VtxBuffer.Size; ++vertexIndex) {
                        auto& vertex = dl->VtxBuffer.Data[vertexIndex];
                        const float coverage = UI::Theme::ToVec4(vertex.col).w;
                        const float factor = std::clamp((vertex.pos.x - fillMin.x) / fillW, 0.0f, 1.0f);
                        vertex.col = ImGuiMCP::ColorConvertFloat4ToU32({ std::lerp(low.x, high.x, factor), std::lerp(low.y, high.y, factor),
                            std::lerp(low.z, high.z, factor), std::lerp(low.w, high.w, factor) * coverage });
                    }
                }
            }
            ImGuiMCP::ImDrawListManager::AddRect(dl, borderMin, borderMax,
                UI::Theme::Enjoyment.frameBorder, frameRounding, ImGuiMCP::ImDrawFlags_RoundCornersAll, 1.0f);

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
                    inZone ? UI::Theme::Enjoyment.zoneActive : UI::Theme::Enjoyment.zoneIdle, 0.0f, 0);
                ImGuiMCP::ImDrawListManager::AddLine(dl,
                    ImGuiMCP::ImVec2{ gx0, frameMin.y }, ImGuiMCP::ImVec2{ gx0, frameMax.y },
                    inZone ? UI::Theme::Enjoyment.zoneFocused : UI::Theme::Enjoyment.zoneBorder, 1.0f);
                ImGuiMCP::ImDrawListManager::AddLine(dl,
                    ImGuiMCP::ImVec2{ gx1, frameMin.y }, ImGuiMCP::ImVec2{ gx1, frameMax.y },
                    inZone ? UI::Theme::Enjoyment.zoneFocused : UI::Theme::Enjoyment.zoneBorder, 1.0f);

                // zone center line
                const float zoneCx = (gx0 + gx1) * 0.5f;
                ImGuiMCP::ImDrawListManager::AddLine(dl,
                    ImGuiMCP::ImVec2{ zoneCx, frameMin.y }, ImGuiMCP::ImVec2{ zoneCx, frameMax.y },
                    inZone ? UI::Theme::Enjoyment.zoneCenterActive : UI::Theme::Enjoyment.zoneCenter, 1.0f);

                // needle rect
                const float nx = frameMin.x + _needlePosition * zoneW;
                const float nw = innerGp * 0.667f;
                const float nTop = frameMin.y - innerGp * 0.667f;
                const float nBot = frameMax.y + innerGp * 0.667f;
                const ImGuiMCP::ImU32 nCol = inZone ? UI::Theme::Enjoyment.needleActive : UI::Theme::Enjoyment.needle;
                ImGuiMCP::ImDrawListManager::AddRectFilled(dl,
                    ImGuiMCP::ImVec2{ nx - nw * 0.5f, nTop }, ImGuiMCP::ImVec2{ nx + nw * 0.5f, nBot },
                    nCol, 0.0f, 0);

                // feedback flash
                if (_feedbackActorId == b.formId && now < _feedbackUntil) {
                    ImGuiMCP::ImDrawListManager::AddRectFilled(dl, frameMin, frameMax,
                        _feedbackHit ? UI::Theme::Enjoyment.feedbackHit : UI::Theme::Enjoyment.feedbackMiss,
                        frameRounding, ImGuiMCP::ImDrawFlags_RoundCornersAll);
                    SetWindowFontSize(fbFt);
                    const char* fbStr = _feedbackHit ? "HIT" : "MISS";
                    const ImGuiMCP::ImU32 fbCol = _feedbackHit ? UI::Theme::Enjoyment.hit : UI::Theme::Enjoyment.miss;
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
                const auto tc = UI::Theme::Enjoyment.targetBorder;
                ImGuiMCP::ImDrawListManager::AddRect(dl, borderMin, borderMax,
                    tc, frameRounding, ImGuiMCP::ImDrawFlags_RoundCornersAll, 2.0f);
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
