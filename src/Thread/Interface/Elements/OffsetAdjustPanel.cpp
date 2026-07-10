#include "OffsetAdjustPanel.h"

namespace Thread::Interface
{
    void OffsetAdjustPanel::Init()
    {
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (!inst) return;
        s_axes.clear();
        auto* ctr = inst->GetCenterRef();

        s_centerIsPlayer  = (ctr != nullptr) && ctr->IsPlayerRef();
        s_hasFurniture    = (ctr != nullptr) && !s_centerIsPlayer;
        s_adjustStageOnly = inst->GetThreadProperty<bool>("VarUI_AdjustStage");
        s_selectedId      = std::nullopt;
        s_pickerOpen      = false;
        s_panelOpen       = false;
        s_draggingAxis    = -1;

        RefreshSlots();
        isVisible = true;
        SceneHUD::winOffsetAdjust->IsOpen = true;

        if (s_items.size() == 1) {
            OnActorSelected(s_items[0]);
        } else if (!s_items.empty()) {
            s_pickerOpen = true;
        }
    }

    void OffsetAdjustPanel::Destroy()
    {
        isVisible = false;
        s_axes.clear();
        s_items.clear();
        s_selectedId = std::nullopt;
        s_pickerOpen = false;
        s_panelOpen = false;
        if (SceneHUD::winOffsetAdjust) SceneHUD::winOffsetAdjust->IsOpen = false;
    }

    void OffsetAdjustPanel::RefreshSlots()
    {
        s_items.clear();
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (!inst) return;

        if (s_hasFurniture) {
            ActorItem fi;
            fi.actor   = nullptr;
            fi.formId  = 0;
            fi.label   = "Center";
            fi.isScene = true;
            s_items.push_back(fi);
        }

        const auto actors = inst->GetActors();
        for (size_t i = 0; i < actors.size(); ++i) {
            auto* a = actors[i];
            if (!a) continue;
            ActorItem item;
            item.actor   = a;
            item.formId  = a->GetFormID();
            item.posIdx  = i;
            item.label   = a->GetDisplayFullName();
            item.isScene = false;
            s_items.push_back(item);
        }
        
        std::sort(s_items.begin(), s_items.end(), [](const ActorItem& a, const ActorItem& b){
            if (a.isScene != b.isScene) return a.isScene; // sort center/player first
            if (!a.actor || !b.actor) return !a.actor;
            if (a.actor->IsPlayerRef() != b.actor->IsPlayerRef()) return a.actor->IsPlayerRef();
            return a.label < b.label;
        });
    }

    // ── RefreshValues ────────────────────────────────────────────────────────

    void OffsetAdjustPanel::RefreshValues(uint32_t actorId)
    {
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (!inst) return;
        auto* sc = inst->GetActiveScene();
        auto* st = inst->GetActiveStage();
        if (!sc || !st) return;

        std::array<float, 4> raw{};
        if (actorId == 0) {
            const auto v = sc->furnitureOffset.GetOffset().AsVector();
            std::copy_n(v.begin(), 4, raw.begin());
        } else {
            for (const auto& item : s_items) {
                if (item.formId == actorId) {
                    const auto v = st->positions[item.posIdx].offset.GetOffset().AsVector();
                    std::copy_n(v.begin(), 4, raw.begin());
                    break;
                }
            }
        }

        auto& axes = s_axes[actorId];
        for (int i = 0; i < 4; ++i) {
            const float val = (i == 3) ? raw[i] * kRadToDeg : raw[i];
            if (!axes[i].hasBaseline) {
                axes[i].baseline = val;
                axes[i].hasBaseline = true;
            }
            // Don't overwrite value while user is actively dragging this axis — that would fight the drag.
            if (s_draggingAxis == i && s_draggingId == actorId) continue;
            axes[i].value = val;
        }
    }

    void OffsetAdjustPanel::OnStageChanged()
    {
        if (!isVisible || !s_selectedId) return;
        RefreshValues(*s_selectedId);
    }

    // ── Handlers ──────────────────────────────────────────────────────────────

    void OffsetAdjustPanel::OnActorSelected(const ActorItem& item)
    {
        s_selectedId = item.formId;
        s_pickerOpen = false;
        s_panelOpen = true;
        if (!s_axes.count(item.formId) || !s_axes[item.formId][0].hasBaseline)
            RefreshValues(item.formId);
    }

    void OffsetAdjustPanel::OnSetOffset(Registry::CoordinateType axis, uint32_t actorId, float value)
    {
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (inst) inst->OffsetAdjustSet(actorId, axis, value);
    }

    void OffsetAdjustPanel::OnResetOffsets()
    {
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (!inst || !s_selectedId) return;
        inst->OffsetAdjustReset();
        s_axes.erase(*s_selectedId);
        RefreshValues(*s_selectedId);
    }

    void OffsetAdjustPanel::OnSetAdjustStageOnly(bool state)
    {
        s_adjustStageOnly = state;
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (inst) inst->SetThreadProperty<bool>("VarUI_AdjustStage", state);
    }

    // ── The offset track widget ───────────────────────────────────────────────
    bool OffsetAdjustPanel::OffsetTrack(const char* axisLabel, AxisState& state, float range, bool /*isDragging*/, bool& draggingOut)
    {
        bool changed = false;
        draggingOut  = false;

        const float rowPadV = ScaleUI::pxScale(4.0f);
        const float rowPadH = ScaleUI::pxScale(12.0f);
        const float trackH  = ScaleUI::pxScale(4.0f);    // track thickness
        const float needleW = ScaleUI::pxScale(3.0f);    // needle thickness
        const float hitExt  = ScaleUI::pxScale(10.0f);   // extends clickable/draggable area above and below visible track
        const float valW    = ScaleUI::pxScale(52.0f);   // width of the numeric value field
        const float labelFt = ScaleUI::pxScale(10.0f);   // axis label font size
        const float valFt   = ScaleUI::pxScale(10.0f);
        const float trackW  = ImGuiMCP::GetContentRegionAvail().x - rowPadH * 2.0f;

        ImGuiMCP::SetCursorPosX(rowPadH);

        // ── Label (double-click resets to baseline)
        ImGuiMCP::SetWindowFontScale(labelFt / ImGuiMCP::GetFontSize());
        const ImGuiMCP::ImVec2 lblMin = ImGuiMCP::GetCursorScreenPos();
        ImGuiMCP::TextColored(ToVec4(ColorUI::TextSecond), "%s", axisLabel);
        const ImGuiMCP::ImVec2 lblMax = ImGuiMCP::ImVec2{lblMin.x + ImGuiMCP::CalcTextSize(axisLabel).x, lblMin.y + labelFt};
        if (ImGuiMCP::IsMouseHoveringRect(lblMin, lblMax) &&
            ImGuiMCP::IsMouseDoubleClicked(ImGuiMCP::ImGuiMouseButton_Left)) {
            state.value = state.baseline;
            changed = true;
        }

        // ── Value input field (flush right of header row)
        ImGuiMCP::SameLine(rowPadH + trackW - valW);
        ImGuiMCP::SetWindowFontScale(valFt / ImGuiMCP::GetFontSize());
        char valBuf[16];
        std::snprintf(valBuf, sizeof(valBuf),
            "%+d", static_cast<int>(std::round(state.value)));
        ImGuiMCP::SetNextItemWidth(valW);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_FrameBg, IM_COL32(0,0,0,0));
        const ImGuiMCP::ImU32 valTextCol = std::abs(state.value) >= 1.0f ? ColorUI::TextSecond : ColorUI::TextMuted;
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, valTextCol);
        if (ImGuiMCP::InputText("##slpp_oamVal", valBuf, sizeof(valBuf),
                ImGuiMCP::ImGuiInputTextFlags_EnterReturnsTrue | ImGuiMCP::ImGuiInputTextFlags_CharsDecimal)) {
            float v = std::round(std::strtof(valBuf, nullptr));
            if (std::isnan(v)) v = state.value;
            if (range == kRangeR) v = std::clamp(v, -kRangeR, kRangeR);
            state.value = v;
            changed = true;
        }
        if (ImGuiMCP::IsItemDeactivatedAfterEdit()) {
            float v = std::round(std::strtof(valBuf, nullptr));
            if (!std::isnan(v)) {
                if (range == kRangeR) v = std::clamp(v, -kRangeR, kRangeR);
                state.value = v;
                changed = true;
            }
        }
        ImGuiMCP::PopStyleColor(2);

        ImGuiMCP::Dummy(ImGuiMCP::ImVec2{0.0f, rowPadV});

        // ── Track + needle + fill drawn on NEXT line
        ImGuiMCP::SetCursorPosX(rowPadH);
        const ImGuiMCP::ImVec2 trackMin = ImGuiMCP::GetCursorScreenPos();
        const ImGuiMCP::ImVec2 trackMax = ImGuiMCP::ImVec2{trackMin.x + trackW, trackMin.y + trackH};

        // Invisible button with extended hit area
        const ImGuiMCP::ImVec2 hitMin = ImGuiMCP::ImVec2{trackMin.x, trackMin.y - hitExt};
        const ImGuiMCP::ImVec2 hitMax = ImGuiMCP::ImVec2{trackMax.x, trackMax.y + hitExt};
        ImGuiMCP::SetCursorScreenPos(hitMin);
        ImGuiMCP::InvisibleButton("##slpp_oamTrack", ImGuiMCP::ImVec2{trackW, trackH + hitExt * 2.0f});
        const bool hovered = ImGuiMCP::IsItemHovered();
        const bool active  = ImGuiMCP::IsItemActive();

        // Drag (activated on first frame, captures start value)
        if (ImGuiMCP::IsItemActivated()) {
            state.dragStartValue = state.value;
            s_draggingAxis = -1;  // will be set by caller
            draggingOut = true;
        }
        if (active) {
            const float dx = ImGuiMCP::GetMouseDragDelta(ImGuiMCP::ImGuiMouseButton_Left).x;
            float v = std::round(state.dragStartValue + dx * (range / kDragScale));
            if (range == kRangeR) v = std::clamp(v, -kRangeR, kRangeR);
            state.value = v;
            draggingOut = true; // Notify on every frame while dragging
            changed = true;
        }

        // Drag released: final notification
        if (ImGuiMCP::IsItemDeactivated() && !active) {
            changed = true;
        }

        // Arrow keys nudge the value by 1 while the slider is hovered.
        if (hovered) {
            float delta = 0.0f;
            if (ImGuiMCP::IsKeyPressed(ImGuiMCP::ImGuiKey_RightArrow)) delta =  1.0f;
            else if (ImGuiMCP::IsKeyPressed(ImGuiMCP::ImGuiKey_LeftArrow)) delta = -1.0f;
            if (delta != 0.0f) {
                float v = state.value + delta;
                if (range == kRangeR) v = std::clamp(v, -kRangeR, kRangeR);
                state.value = v;
                changed = true;
            }
        }

        // ── Draw track ─────────────────────────────────────────────────────────
        auto* dl = ImGuiMCP::GetWindowDrawList();

        // Track background
        ImGuiMCP::ImDrawListManager::AddRectFilled(dl, trackMin, trackMax, IM_COL32(255,255,255,10), ScaleUI::pxScale(2.0f), 0);
        ImGuiMCP::ImDrawListManager::AddRect(dl, trackMin, trackMax, IM_COL32(58,58,58,128), ScaleUI::pxScale(2.0f), 0, 1.0f);

        // Center tick
        const float cx = trackMin.x + trackW * 0.5f;
        ImGuiMCP::ImDrawListManager::AddLine(dl,
            ImGuiMCP::ImVec2{cx, trackMin.y - ScaleUI::pxScale(2.0f)}, ImGuiMCP::ImVec2{cx, trackMax.y + ScaleUI::pxScale(2.0f)},
            IM_COL32(255,255,255,26), 1.0f);

        // Fill grows from the center out toward the needle, in either direction.
        const float pct = std::clamp(0.5f + (state.value / range) * 0.5f, 0.0f, 1.0f);
        const float needleX = trackMin.x + pct * trackW;
        if (state.value != 0.0f) {
            const float fillL = std::min(cx, needleX);
            const float fillR = std::max(cx, needleX);
            ImGuiMCP::ImDrawListManager::AddRectFilled(dl,
                ImGuiMCP::ImVec2{fillL, trackMin.y}, ImGuiMCP::ImVec2{fillR, trackMax.y},
                ColorUI::OamFill, 0.0f, 0);
        }

        // Needle extends slightly above and below the track so it stays visible
        const float nTop = trackMin.y - ScaleUI::pxScale(5.0f);
        const float nBot = trackMax.y + ScaleUI::pxScale(5.0f);
        const ImGuiMCP::ImU32 nCol = active ? ColorUI::OamNeedleDrg : ColorUI::OamNeedle;
        ImGuiMCP::ImDrawListManager::AddRectFilled(dl,
            ImGuiMCP::ImVec2{needleX - needleW * 0.5f, nTop},
            ImGuiMCP::ImVec2{needleX + needleW * 0.5f, nBot},
            nCol, ScaleUI::pxScale(1.5f), 0);

        ImGuiMCP::SetCursorScreenPos(ImGuiMCP::ImVec2{trackMin.x, trackMax.y + ScaleUI::pxScale(4.0f)});

        // Separator
        ImGuiMCP::Dummy(ImGuiMCP::ImVec2{trackW, rowPadV});
        const ImGuiMCP::ImVec2 sepPos = ImGuiMCP::GetCursorScreenPos();
        ImGuiMCP::ImDrawListManager::AddLine(dl,
            ImGuiMCP::ImVec2{sepPos.x, sepPos.y}, ImGuiMCP::ImVec2{sepPos.x + trackW + rowPadH * 2.0f, sepPos.y},
            IM_COL32(38,38,38,115), 1.0f);

        return changed;
    }

    // ── Render ────────────────────────────────────────────────────────────────

    void __stdcall OffsetAdjustPanel::Render()
    {
        if (!isVisible || !SceneHUD::IsActive()) return;
        if (!SceneHUD::IsPanelOpen(IdxTabPanel::kOffsetAdjust)) return;
        if (!SceneHUD::focusMode) return;

        auto* io = ImGuiMCP::GetIO();
        const float dw = io->DisplaySize.x;
        const float dh = io->DisplaySize.y;

        const float offset  = ScaleUI::pxScale(40.0f);
        const float pickerW = ScaleUI::pxScale(200.0f);
        const float panelW  = ScaleUI::pxScale(300.0f);

        // ── Target picker
        if (s_pickerOpen) {
            ImGuiMCP::SetNextWindowPos(
                ImGuiMCP::ImVec2{dw - offset, dh * 0.5f}, ImGuiMCP::ImGuiCond_Always, ImGuiMCP::ImVec2{1.0f, 0.5f});
            ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{pickerW, 0.0f}, ImGuiMCP::ImGuiCond_Always);
            ImGuiMCP::SetNextWindowBgAlpha(0.98f);

            constexpr auto pFlags =
                ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
                ImGuiMCP::ImGuiWindowFlags_NoMove     | ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
                ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize;

            if (ImGuiMCP::Begin("##slpp_OAMPicker", nullptr, pFlags)) {
                ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());
                ImGuiMCP::TextColored(ToVec4(ColorUI::TextSecond), "  PICK TARGET");
                ImGuiMCP::Separator();

                ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(10.0f) / ImGuiMCP::GetFontSize());
                for (const auto& item : s_items) {
                    ImGuiMCP::PushID(static_cast<int>(item.formId));
                    const bool isSel = s_selectedId && *s_selectedId == item.formId;

                    std::string lbl = item.label;
                    if (item.isScene) lbl += " [SCENE]";

                    if (isSel) ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Header, IM_COL32(30,40,30,128));
                    if (ImGuiMCP::Selectable(lbl.c_str(), isSel, 0, ImGuiMCP::ImVec2{0, ScaleUI::pxScale(28.0f)}))
                        OnActorSelected(item);
                    if (isSel) ImGuiMCP::PopStyleColor();
                    ImGuiMCP::PopID();
                }
            }
            ImGuiMCP::End();
        }

        // ── Adjustment panel
        if (s_panelOpen && s_selectedId) {
            const uint32_t actorId = *s_selectedId;
            auto& axes = s_axes[actorId];
            if (!axes[0].hasBaseline) RefreshValues(actorId);

            std::string panelTitle;
            for (const auto& item : s_items) {
                if (item.formId == actorId) {
                    panelTitle = item.label;
                    if (item.isScene) panelTitle += " [SCENE]";
                    break;
                }
            }

            ImGuiMCP::SetNextWindowPos(
                ImGuiMCP::ImVec2{dw - offset, dh * 0.5f}, ImGuiMCP::ImGuiCond_Always, ImGuiMCP::ImVec2{1.0f, 0.5f});
            ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{panelW, 0.0f}, ImGuiMCP::ImGuiCond_Always);
            ImGuiMCP::SetNextWindowBgAlpha(0.98f);

            constexpr auto panelFlags =
                ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
                ImGuiMCP::ImGuiWindowFlags_NoMove     | ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
                ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize;

            if (ImGuiMCP::Begin("##slpp_OAMPanel", nullptr, panelFlags)) {

                // Panel title: centered and uppercase
                ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());
                const float titleW = ImGuiMCP::CalcTextSize(panelTitle.c_str()).x;
                ImGuiMCP::SetCursorPosX((panelW - titleW) * 0.5f);
                ImGuiMCP::TextColored(ToVec4(ColorUI::TextMuted), "%s", panelTitle.c_str());
                ImGuiMCP::Separator();

                // Stage-only toggle row
                ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(10.0f) / ImGuiMCP::GetFontSize());
                ImGuiMCP::SetCursorPosX(ScaleUI::pxScale(12.0f));
                ImGuiMCP::TextColored(ToVec4(ColorUI::TextSecond), "Adjust Stage Only");
                ImGuiMCP::SameLine(panelW - ScaleUI::pxScale(12.0f) - ScaleUI::pxScale(16.0f));
                bool stageOnly = s_adjustStageOnly;
                if (ImGuiMCP::Checkbox("##slpp_oamStageOnly", &stageOnly))
                    OnSetAdjustStageOnly(stageOnly);

                // Reset row
                ImGuiMCP::SetCursorPosX(ScaleUI::pxScale(12.0f));
                if (ImGuiMCP::Selectable("Reset Offsets ↺", false, 0,
                        ImGuiMCP::ImVec2{panelW - ScaleUI::pxScale(24.0f), ScaleUI::pxScale(28.0f)}))
                    OnResetOffsets();
                ImGuiMCP::Separator();

                // ── Axis Sliders
                static constexpr const char* kLabels[4] = { "X", "Y", "Z", "R" };
                static constexpr Registry::CoordinateType kAxisEnums[4] = {
                    Registry::CoordinateType::X, Registry::CoordinateType::Y,
                    Registry::CoordinateType::Z, Registry::CoordinateType::R
                };

                for (int i = 0; i < 4; ++i) {
                    ImGuiMCP::PushID(i);
                    const float range = (i == 3) ? kRangeR : kRangeXYZ;
                    bool drag = false;

                    if (OffsetTrack(kLabels[i], axes[i], range, s_draggingAxis == i && s_draggingId == actorId, drag)) {
                        OnSetOffset(kAxisEnums[i], actorId, axes[i].value);
                    }

                    if (drag) {
                        s_draggingAxis = i; s_draggingId = actorId;
                    } else if (s_draggingAxis == i && s_draggingId == actorId && !ImGuiMCP::IsMouseDown(ImGuiMCP::ImGuiMouseButton_Left)) {
                        s_draggingAxis = -1;
                    }
                    ImGuiMCP::PopID();
                }
            }
            ImGuiMCP::End();
        }

        ImGuiMCP::SetWindowFontScale(1.0f);
    }
}
