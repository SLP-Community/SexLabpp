#include "OffsetAdjustPanel.h"

namespace Thread::Interface
{
    using UI::SetWindowFontSize;

    OffsetAdjustPanel& OffsetAdjustPanel::GetSingleton()
    {
        static OffsetAdjustPanel singleton;
        return singleton;
    }

    bool OffsetAdjustPanel::Register()
    {
        return RegisterWindow(RenderCallback);
    }

    void __stdcall OffsetAdjustPanel::RenderCallback()
    {
        GetSingleton().Render();
    }

    void OffsetAdjustPanel::Open()
    {
        auto* inst = SceneHUD::GetSingleton().GetThreadInstance();
        if (!inst)
            return;
        _axes.clear();
        auto* ctr = inst->GetCenterRef();

        _hasFurniture = ctr && !ctr->IsPlayerRef();
        _adjustStageOnly = inst->GetThreadProperty<bool>("VarUI_AdjustStage");
        _selectedId.reset();
        _pickerOpen = false;
        _panelOpen = false;
        _draggingAxis = -1;
        _draggingId = 0;

        RefreshSlots();
        Show();

        if (_items.size() == 1) {
            OnActorSelected(_items.front());
        } else if (!_items.empty()) {
            _pickerOpen = true;
        }
    }

    void OffsetAdjustPanel::Close()
    {
        Hide();
        _axes.clear();
        _items.clear();
        _selectedId.reset();
        _pickerOpen = false;
        _panelOpen = false;
        _draggingAxis = -1;
        _draggingId = 0;
    }

    void OffsetAdjustPanel::RefreshSlots()
    {
        _items.clear();
        auto* inst = SceneHUD::GetSingleton().GetThreadInstance();
        if (!inst)
            return;

        if (_hasFurniture) {
            ActorItem fi;
            fi.actor = nullptr;
            fi.formId = 0;
            fi.label = "Center";
            fi.isScene = true;
            _items.push_back(fi);
        }

        const auto actors = inst->GetActors();
        for (size_t i = 0; i < actors.size(); ++i) {
            auto* a = actors[i];
            if (!a)
                continue;
            ActorItem item;
            item.actor = a;
            item.formId = a->GetFormID();
            item.posIdx = i;
            item.label = a->GetDisplayFullName();
            item.isScene = false;
            _items.push_back(item);
        }

        std::ranges::sort(_items, [](const ActorItem& a, const ActorItem& b) {
            if (a.isScene != b.isScene)
                return a.isScene;  // sort center/player first
            if (!a.actor || !b.actor)
                return !a.actor;
            if (a.actor->IsPlayerRef() != b.actor->IsPlayerRef())
                return a.actor->IsPlayerRef();
            return a.label < b.label;
        });
    }

    // ── RefreshValues ────────────────────────────────────────────────────────

    void OffsetAdjustPanel::RefreshValues(uint32_t actorId)
    {
        auto* inst = SceneHUD::GetSingleton().GetThreadInstance();
        if (!inst)
            return;
        auto* sc = inst->GetActiveScene();
        auto* st = inst->GetActiveStage();
        if (!sc || !st)
            return;

        std::array<float, 4> raw{};
        if (actorId == 0) {
            const auto v = sc->furnitureOffset.GetOffset().AsVector();
            std::copy_n(v.begin(), 4, raw.begin());
        } else {
            for (const auto& item : _items) {
                if (item.formId == actorId) {
                    const auto v = st->positions[item.posIdx].offset.GetOffset().AsVector();
                    std::copy_n(v.begin(), 4, raw.begin());
                    break;
                }
            }
        }

        auto& axes = _axes[actorId];
        for (int i = 0; i < 4; ++i) {
            const float val = (i == 3) ? raw[i] * kRadToDeg : raw[i];
            if (!axes[i].hasBaseline) {
                axes[i].baseline = val;
                axes[i].hasBaseline = true;
            }
            // Don't overwrite value while user is actively dragging this axis — that would fight the drag.
            if (_draggingAxis == i && _draggingId == actorId)
                continue;
            axes[i].value = val;
        }
    }

    void OffsetAdjustPanel::OnStageChanged()
    {
        if (!IsVisible() || !_selectedId)
            return;
        RefreshValues(*_selectedId);
    }

    // ── Handlers ──────────────────────────────────────────────────────────────

    void OffsetAdjustPanel::OnActorSelected(const ActorItem& item)
    {
        _selectedId = item.formId;
        _pickerOpen = false;
        _panelOpen = true;
        if (!_axes.contains(item.formId) || !_axes[item.formId][0].hasBaseline)
            RefreshValues(item.formId);
    }

    void OffsetAdjustPanel::OnSetOffset(Registry::CoordinateType axis, uint32_t actorId, float value)
    {
        auto* inst = SceneHUD::GetSingleton().GetThreadInstance();
        if (inst)
            inst->OffsetAdjustSet(actorId, axis, value);
    }

    void OffsetAdjustPanel::OnResetOffsets()
    {
        auto* inst = SceneHUD::GetSingleton().GetThreadInstance();
        if (!inst || !_selectedId)
            return;
        inst->OffsetAdjustReset();
        _axes.erase(*_selectedId);
        RefreshValues(*_selectedId);
    }

    void OffsetAdjustPanel::OnSetAdjustStageOnly(bool state)
    {
        _adjustStageOnly = state;
        auto* inst = SceneHUD::GetSingleton().GetThreadInstance();
        if (inst)
            inst->SetThreadProperty<bool>("VarUI_AdjustStage", state);
    }

    // ── The offset track widget ───────────────────────────────────────────────
    bool OffsetAdjustPanel::OffsetTrack(const char* axisLabel, AxisState& state, float range, bool& draggingOut)
    {
        bool changed = false;
        draggingOut = false;

        const float rowPadV = ScaleUI::pxScale(4.0f);
        const float rowPadH = ScaleUI::pxScale(12.0f);
        const float trackH = ScaleUI::pxScale(4.0f);   // track thickness
        const float needleW = ScaleUI::pxScale(3.0f);  // needle thickness
        const float hitExt = ScaleUI::pxScale(10.0f);  // extends clickable/draggable area above and below visible track
        const float valW = ScaleUI::pxScale(52.0f);    // width of the numeric value field
        const float labelFt = ScaleUI::pxTextScale(UI::Theme::FontSize::body);
        const float valFt = ScaleUI::pxTextScale(UI::Theme::FontSize::body);
        const float trackW = ImGuiMCP::GetContentRegionAvail().x - rowPadH * 2.0f;

        ImGuiMCP::SetCursorPosX(rowPadH);

        // ── Label (double-click resets to baseline)
        SetWindowFontSize(labelFt);
        const ImGuiMCP::ImVec2 lblMin = ImGuiMCP::GetCursorScreenPos();
        ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textSecondary), "%s", axisLabel);
        const ImGuiMCP::ImVec2 lblMax = ImGuiMCP::ImVec2{ lblMin.x + ImGuiMCP::CalcTextSize(axisLabel).x, lblMin.y + labelFt };
        if (ImGuiMCP::IsMouseHoveringRect(lblMin, lblMax) &&
            ImGuiMCP::IsMouseDoubleClicked(ImGuiMCP::ImGuiMouseButton_Left)) {
            state.value = state.baseline;
            changed = true;
        }

        // ── Value input field (flush right of header row)
        ImGuiMCP::SameLine(rowPadH + trackW - valW);
        SetWindowFontSize(valFt);
        char valBuf[16];
        std::snprintf(valBuf, sizeof(valBuf),
            "%+d", static_cast<int>(std::round(state.value)));
        ImGuiMCP::SetNextItemWidth(valW);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_FrameBg, UI::Theme::Color::transparent);
        const ImGuiMCP::ImU32 valTextCol = std::abs(state.value) >= 1.0f ?
                                               UI::Theme::Color::textSecondary :
                                               UI::Theme::Color::textMuted;
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, valTextCol);
        if (ImGuiMCP::InputText("##slpp_oamVal", valBuf, sizeof(valBuf),
                ImGuiMCP::ImGuiInputTextFlags_EnterReturnsTrue | ImGuiMCP::ImGuiInputTextFlags_CharsDecimal)) {
            float v = std::round(std::strtof(valBuf, nullptr));
            if (std::isnan(v))
                v = state.value;
            if (range == kRangeR)
                v = std::clamp(v, -kRangeR, kRangeR);
            state.value = v;
            changed = true;
        }
        if (ImGuiMCP::IsItemDeactivatedAfterEdit()) {
            float v = std::round(std::strtof(valBuf, nullptr));
            if (!std::isnan(v)) {
                if (range == kRangeR)
                    v = std::clamp(v, -kRangeR, kRangeR);
                state.value = v;
                changed = true;
            }
        }
        ImGuiMCP::PopStyleColor(2);

        ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, rowPadV });

        // ── Track + needle + fill drawn on NEXT line
        ImGuiMCP::SetCursorPosX(rowPadH);
        const ImGuiMCP::ImVec2 trackMin = ImGuiMCP::GetCursorScreenPos();
        const ImGuiMCP::ImVec2 trackMax = ImGuiMCP::ImVec2{ trackMin.x + trackW, trackMin.y + trackH };

        // Invisible button with extended hit area
        const ImGuiMCP::ImVec2 hitMin = ImGuiMCP::ImVec2{ trackMin.x, trackMin.y - hitExt };
        const ImGuiMCP::ImVec2 hitMax = ImGuiMCP::ImVec2{ trackMax.x, trackMax.y + hitExt };
        ImGuiMCP::SetCursorScreenPos(hitMin);
        ImGuiMCP::InvisibleButton("##slpp_oamTrack", ImGuiMCP::ImVec2{ trackW, trackH + hitExt * 2.0f });
        const bool hovered = ImGuiMCP::IsItemHovered();
        const bool active = ImGuiMCP::IsItemActive();

        // Drag (activated on first frame, captures start value)
        if (ImGuiMCP::IsItemActivated()) {
            state.dragStartValue = state.value;
            _draggingAxis = -1;  // will be set by caller
            draggingOut = true;
        }
        if (active) {
            const float dx = ImGuiMCP::GetMouseDragDelta(ImGuiMCP::ImGuiMouseButton_Left).x;
            float v = std::round(state.dragStartValue + dx * (range / kDragScale));
            if (range == kRangeR)
                v = std::clamp(v, -kRangeR, kRangeR);
            state.value = v;
            draggingOut = true;  // Notify on every frame while dragging
            changed = true;
        }

        // Drag released: final notification
        if (ImGuiMCP::IsItemDeactivated() && !active) {
            changed = true;
        }

        // Arrow keys nudge the value by 1 while the slider is hovered.
        if (hovered) {
            float delta = 0.0f;
            if (ImGuiMCP::IsKeyPressed(ImGuiMCP::ImGuiKey_RightArrow))
                delta = 1.0f;
            else if (ImGuiMCP::IsKeyPressed(ImGuiMCP::ImGuiKey_LeftArrow))
                delta = -1.0f;
            if (delta != 0.0f) {
                float v = state.value + delta;
                if (range == kRangeR)
                    v = std::clamp(v, -kRangeR, kRangeR);
                state.value = v;
                changed = true;
            }
        }

        // ── Draw track ─────────────────────────────────────────────────────────
        auto* dl = ImGuiMCP::GetWindowDrawList();

        // Track background
        ImGuiMCP::ImDrawListManager::AddRectFilled(dl, trackMin, trackMax, UI::Theme::Offset::track, ScaleUI::pxScale(2.0f), 0);
        ImGuiMCP::ImDrawListManager::AddRect(dl, trackMin, trackMax, UI::Theme::Offset::trackBorder, ScaleUI::pxScale(2.0f), 0, 1.0f);

        // Center tick
        const float cx = trackMin.x + trackW * 0.5f;
        ImGuiMCP::ImDrawListManager::AddLine(dl,
            ImGuiMCP::ImVec2{ cx, trackMin.y - ScaleUI::pxScale(2.0f) }, ImGuiMCP::ImVec2{ cx, trackMax.y + ScaleUI::pxScale(2.0f) },
            UI::Theme::Offset::centerTick, 1.0f);

        // Fill grows from the center out toward the needle, in either direction.
        const float pct = std::clamp(0.5f + (state.value / range) * 0.5f, 0.0f, 1.0f);
        const float needleX = trackMin.x + pct * trackW;
        if (state.value != 0.0f) {
            const float fillL = std::min(cx, needleX);
            const float fillR = std::max(cx, needleX);
            ImGuiMCP::ImDrawListManager::AddRectFilled(dl,
                ImGuiMCP::ImVec2{ fillL, trackMin.y }, ImGuiMCP::ImVec2{ fillR, trackMax.y },
                UI::Theme::Offset::fill, 0.0f, 0);
        }

        // Needle extends slightly above and below the track so it stays visible
        const float nTop = trackMin.y - ScaleUI::pxScale(5.0f);
        const float nBot = trackMax.y + ScaleUI::pxScale(5.0f);
        const ImGuiMCP::ImU32 nCol = active ? UI::Theme::Offset::needleActive : UI::Theme::Offset::needle;
        ImGuiMCP::ImDrawListManager::AddRectFilled(dl,
            ImGuiMCP::ImVec2{ needleX - needleW * 0.5f, nTop },
            ImGuiMCP::ImVec2{ needleX + needleW * 0.5f, nBot },
            nCol, ScaleUI::pxScale(1.5f), 0);

        ImGuiMCP::SetCursorScreenPos(ImGuiMCP::ImVec2{ trackMin.x, trackMax.y + ScaleUI::pxScale(4.0f) });

        // Separator
        ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ trackW, rowPadV });
        const ImGuiMCP::ImVec2 sepPos = ImGuiMCP::GetCursorScreenPos();
        ImGuiMCP::ImDrawListManager::AddLine(dl,
            ImGuiMCP::ImVec2{ sepPos.x, sepPos.y }, ImGuiMCP::ImVec2{ sepPos.x + trackW + rowPadH * 2.0f, sepPos.y },
            UI::Theme::Offset::separator, 1.0f);

        return changed;
    }

    // ── Render ────────────────────────────────────────────────────────────────

    void OffsetAdjustPanel::Render()
    {
        auto& hud = SceneHUD::GetSingleton();
        if (!IsVisible() || !hud.ShouldRender() || !hud.IsPanelOpen(PanelId::kOffsetAdjust) || !hud.IsFocused())
            return;

        auto* io = ImGuiMCP::GetIO();
        const float dw = io->DisplaySize.x;
        const float dh = io->DisplaySize.y;

        const float offset = ScaleUI::pxScale(UI::Theme::Geometry::panelTabWidth + UI::Theme::Geometry::panelTabGap);
        const float pickerW = ScaleUI::pxScale(200.0f);
        const float panelW = ScaleUI::pxScale(300.0f);
        const float sectionH = std::max(ScaleUI::pxScale(20.0f),
            ScaleUI::pxTextScale(UI::Theme::FontSize::sectionHeader) + ScaleUI::pxScale(UI::Theme::Spacing::xs));

        // ── Target picker
        if (!_panelOpen && !_items.empty()) {
            ImGuiMCP::SetNextWindowPos(
                ImGuiMCP::ImVec2{ dw - offset, dh * 0.5f }, ImGuiMCP::ImGuiCond_Always, ImGuiMCP::ImVec2{ 1.0f, 0.5f });
            ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{ pickerW, 0.0f }, ImGuiMCP::ImGuiCond_Always);
            constexpr auto pFlags =
                ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
                ImGuiMCP::ImGuiWindowFlags_NoMove | ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
                ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize;

            if (ImGuiMCP::Begin("##slpp_OAMPicker", nullptr, pFlags)) {
                SetWindowFontSize(ScaleUI::pxTextScale(UI::Theme::FontSize::sectionHeader));
                if (UI::CollapsibleSectionHeader(
                        "PICK TARGET", "##slpp_oamTargetSection", _pickerOpen, { 0.0f, sectionH }))
                    _pickerOpen = !_pickerOpen;
                ImGuiMCP::Separator();

                if (_pickerOpen) {
                    SetWindowFontSize(ScaleUI::pxTextScale(UI::Theme::FontSize::body));
                    for (const auto& item : _items) {
                        ImGuiMCP::PushID(static_cast<int>(item.formId));
                        const bool isSel = _selectedId && *_selectedId == item.formId;

                        std::string lbl = item.label;
                        if (item.isScene)
                            lbl += " [SCENE]";

                        if (isSel)
                            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Header, UI::Theme::Color::selectionFill);
                        if (UI::SelectableButton(lbl.c_str(), isSel, 0, ImGuiMCP::ImVec2{ 0, ScaleUI::pxScale(28.0f) }))
                            OnActorSelected(item);
                        if (isSel)
                            ImGuiMCP::PopStyleColor();
                        ImGuiMCP::PopID();
                    }
                }
                ImGuiMCP::SetWindowFontScale(1.0f);
            }
            ImGuiMCP::End();
        }

        // ── Adjustment panel
        if (_panelOpen && _selectedId) {
            const uint32_t actorId = *_selectedId;
            auto& axes = _axes[actorId];
            if (!axes[0].hasBaseline)
                RefreshValues(actorId);

            std::string panelTitle;
            for (const auto& item : _items) {
                if (item.formId == actorId) {
                    panelTitle = item.label;
                    if (item.isScene)
                        panelTitle += " [SCENE]";
                    break;
                }
            }

            ImGuiMCP::SetNextWindowPos(
                ImGuiMCP::ImVec2{ dw - offset, dh * 0.5f }, ImGuiMCP::ImGuiCond_Always, ImGuiMCP::ImVec2{ 1.0f, 0.5f });
            ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{ panelW, 0.0f }, ImGuiMCP::ImGuiCond_Always);
            constexpr auto panelFlags =
                ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
                ImGuiMCP::ImGuiWindowFlags_NoMove | ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
                ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize;

            if (ImGuiMCP::Begin("##slpp_OAMPanel", nullptr, panelFlags)) {
                // Panel title: centered and uppercase
                SetWindowFontSize(ScaleUI::pxTextScale(UI::Theme::FontSize::caption));
                const float titleW = ImGuiMCP::CalcTextSize(panelTitle.c_str()).x;
                ImGuiMCP::SetCursorPosX((panelW - titleW) * 0.5f);
                ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textMuted), "%s", panelTitle.c_str());
                ImGuiMCP::Separator();

                // Stage-only toggle row
                SetWindowFontSize(ScaleUI::pxTextScale(UI::Theme::FontSize::body));
                ImGuiMCP::SetCursorPosX(ScaleUI::pxScale(12.0f));
                ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textSecondary), "Adjust Stage Only");
                ImGuiMCP::SameLine(panelW - ScaleUI::pxScale(12.0f) - ScaleUI::pxScale(16.0f));
                bool stageOnly = _adjustStageOnly;
                UI::PushCheckboxStyle(hud.GetScale().Factor());
                if (ImGuiMCP::Checkbox("##slpp_oamStageOnly", &stageOnly))
                    OnSetAdjustStageOnly(stageOnly);
                UI::PopCheckboxStyle();

                // Reset row
                ImGuiMCP::SetCursorPosX(ScaleUI::pxScale(12.0f));
                const ImGuiMCP::ImVec2 resetMin = ImGuiMCP::GetCursorScreenPos();
                const ImGuiMCP::ImVec2 resetSize{ panelW - ScaleUI::pxScale(24.0f), ScaleUI::pxScale(28.0f) };
                const bool resetOffsets = UI::SelectableButton("Reset Offsets", false, 0, resetSize);
                const ImGuiMCP::ImVec2 cursorAfterReset = ImGuiMCP::GetCursorPos();
                SKSEMenuFramework::PushFont(UI::Theme::Icon::solidFont);
                const ImGuiMCP::ImVec2 resetIconSize = ImGuiMCP::CalcTextSize(UI::Theme::Icon::rotateLeft);
                ImGuiMCP::SetCursorScreenPos({ resetMin.x + resetSize.x - resetIconSize.x - ScaleUI::pxScale(8.0f),
                    resetMin.y + (resetSize.y - resetIconSize.y) * 0.5f });
                ImGuiMCP::TextUnformatted(UI::Theme::Icon::rotateLeft);
                FontAwesome::Pop();
                ImGuiMCP::SetCursorPos(cursorAfterReset);
                if (resetOffsets)
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

                    if (OffsetTrack(kLabels[i], axes[i], range, drag)) {
                        OnSetOffset(kAxisEnums[i], actorId, axes[i].value);
                    }

                    if (drag) {
                        _draggingAxis = i;
                        _draggingId = actorId;
                    } else if (_draggingAxis == i && _draggingId == actorId && !ImGuiMCP::IsMouseDown(ImGuiMCP::ImGuiMouseButton_Left)) {
                        _draggingAxis = -1;
                    }
                    ImGuiMCP::PopID();
                }
                ImGuiMCP::SetWindowFontScale(1.0f);
            }
            ImGuiMCP::End();
        }
    }
}
