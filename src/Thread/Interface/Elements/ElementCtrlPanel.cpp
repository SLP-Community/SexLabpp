#include "ElementCtrlPanel.h"
#include "Papyrus/SexLabUtil.h"
#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interface
{
    using UI::SetWindowFontSize;

    namespace
    {
        constexpr auto COLOR_EDIT_FLAGS =
            ImGuiMCP::ImGuiColorEditFlags_NoInputs |
            ImGuiMCP::ImGuiColorEditFlags_AlphaBar |
            ImGuiMCP::ImGuiColorEditFlags_AlphaPreviewHalf;

        // Fit the editor to its reflected labels and controls because text and UI geometry scale independently
        template <class T>
        float MeasureThemeFieldsWidth(T& a_values, ImGuiMCP::ImFont* a_font, float a_fontSize, const ImGuiMCP::ImGuiStyle& a_style)
        {
            const char* valueText = "-00000.000";
            const float valueTextWidth = ImGuiMCP::ImFontManger::CalcTextSizeA(
                a_font, a_fontSize, FLT_MAX, 0.0f, valueText, nullptr, nullptr)
                                             .x;
            const char* bufferText = "0";
            const float bufferWidth = ImGuiMCP::ImFontManger::CalcTextSizeA(
                a_font, a_fontSize, FLT_MAX, 0.0f, bufferText, nullptr, nullptr)
                                          .x;
            const float frameHeight = a_fontSize + a_style.FramePadding.y * 2.0f;
            const float floatFieldWidth = valueTextWidth + bufferWidth + a_style.FramePadding.x * 2.0f +
                                          (frameHeight + a_style.ItemInnerSpacing.x) * 2.0f;
            float width = 0.0f;
            std::size_t index = 0;
            glz::for_each_field(a_values, [&](auto& a_field) {
                using Field = std::remove_cvref_t<decltype(a_field)>;

                const auto fieldName = glz::reflect<T>::keys[index++];
                const float labelWidth = ImGuiMCP::ImFontManger::CalcTextSizeA(
                    a_font, a_fontSize, FLT_MAX, 0.0f, fieldName.data(), fieldName.data() + fieldName.size(), nullptr)
                                             .x;
                if constexpr (std::is_same_v<Field, ImGuiMCP::ImU32>) {
                    width = std::max(width, frameHeight + a_style.ItemInnerSpacing.x + labelWidth);
                } else if constexpr (std::is_same_v<Field, float>) {
                    width = std::max(width, labelWidth + a_style.ItemInnerSpacing.x + floatFieldWidth);
                } else if constexpr (glz::reflectable<Field>) {
                    width = std::max({ width, labelWidth, MeasureThemeFieldsWidth(a_field, a_font, a_fontSize, a_style) });
                }
            });
            return width;
        }

        template <class T>
        void DrawThemeFields(T& a_values)
        {
            std::size_t index = 0;
            glz::for_each_field(a_values, [&](auto& a_field) {
                using Field = std::remove_cvref_t<decltype(a_field)>;

                const auto fieldIndex = index++;
                const auto fieldName = glz::reflect<T>::keys[fieldIndex];
                ImGuiMCP::PushID(static_cast<int>(fieldIndex));

                if constexpr (std::is_same_v<Field, ImGuiMCP::ImU32>) {
                    const float fieldWidth = ImGuiMCP::GetFrameHeight();
                    const float fieldX = ImGuiMCP::GetCursorPosX() + ImGuiMCP::GetContentRegionAvail().x - fieldWidth;
                    auto color = UI::Theme::ToVec4(a_field);
                    ImGuiMCP::AlignTextToFramePadding();
                    ImGuiMCP::TextUnformatted(fieldName.data());
                    ImGuiMCP::SameLine(fieldX);
                    ImGuiMCP::SetNextItemWidth(fieldWidth);
                    if (ImGuiMCP::ColorEdit4("##value", &color.x, COLOR_EDIT_FLAGS))
                        a_field = ImGuiMCP::ColorConvertFloat4ToU32(color);
                } else if constexpr (std::is_same_v<Field, float>) {
                    const auto* style = ImGuiMCP::GetStyle();
                    const float fieldWidth = ImGuiMCP::CalcTextSize("-00000.0000").x + style->FramePadding.x * 2.0f +
                                             (ImGuiMCP::GetFrameHeight() + style->ItemInnerSpacing.x) * 2.0f;
                    const float fieldX = ImGuiMCP::GetCursorPosX() + ImGuiMCP::GetContentRegionAvail().x - fieldWidth;
                    ImGuiMCP::AlignTextToFramePadding();
                    ImGuiMCP::TextUnformatted(fieldName.data());
                    ImGuiMCP::SameLine(fieldX);
                    ImGuiMCP::SetNextItemWidth(fieldWidth);
                    const auto thousandths = std::llround(std::abs(a_field) * 1000.0f);
                    const char* format = thousandths % 100 == 0 ? "%.1f" : thousandths % 10 == 0 ? "%.2f" :
                                                                                                   "%.3f";
                    ImGuiMCP::InputFloat("##value", &a_field, 0.1f, 0.1f, format);
                } else if constexpr (glz::reflectable<Field>) {
                    if (ImGuiMCP::CollapsingHeader(fieldName.data(), ImGuiMCP::ImGuiTreeNodeFlags_DefaultOpen))
                        DrawThemeFields(a_field);
                }

                ImGuiMCP::PopID();
            });
        }
    }

    void ElementCtrlPanel::Open(SceneHUD& a_hud)
    {
        if (auto* inst = a_hud.GetThreadInstance()) {
            _scaleAdjustment = std::clamp(inst->GetThreadProperty<float>("VarUI_MenuScaleMult"), 0.5f, 2.5f);
            const float textScale = inst->GetThreadProperty<float>("VarUI_TextScaleMult");
            _textScaleAdjustment = textScale > 0.0f ? std::clamp(textScale, 0.75f, 2.0f) : 1.0f;
        }
    }
    void ElementCtrlPanel::Close() {}

    void ElementCtrlPanel::OnScaleChange(SceneHUD& a_hud, float a_value)
    {
        auto* inst = a_hud.GetThreadInstance();
        if (!inst)
            return;
        const float value = std::clamp(a_value, 0.5f, 2.5f);
        _scaleAdjustment = value;
        inst->SetThreadProperty<float>("VarUI_MenuScaleMult", value);
        a_hud.GetScale().SetMultiplier(value);
    }

    void ElementCtrlPanel::OnTextScaleChange(SceneHUD& a_hud, float a_value)
    {
        auto* inst = a_hud.GetThreadInstance();
        if (!inst)
            return;
        const float value = std::clamp(a_value, 0.75f, 2.0f);
        _textScaleAdjustment = value;
        inst->SetThreadProperty<float>("VarUI_TextScaleMult", value);
        a_hud.GetScale().SetTextMultiplier(value);
    }

    void ElementCtrlPanel::Render(SceneHUD& a_hud)
    {
        auto* inst = a_hud.GetThreadInstance();
        if (!inst)
            return;
        auto& scale = a_hud.GetScale();

        auto* io = ImGuiMCP::GetIO();
        const float panelOffset = scale.Px(UI::Theme::Geometry.panelTabWidth + UI::Theme::Geometry.panelTabGap);
        ImGuiMCP::SetNextWindowPos(
            ImGuiMCP::ImVec2{ io->DisplaySize.x - panelOffset, io->DisplaySize.y * 0.5f },
            ImGuiMCP::ImGuiCond_Always, ImGuiMCP::ImVec2{ 1.0f, 0.5f });
        ImGuiMCP::SetNextWindowSize({ scale.Px(220.0f), 0.0f }, ImGuiMCP::ImGuiCond_Always);

        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
            ImGuiMCP::ImGuiWindowFlags_NoMove | ImGuiMCP::ImGuiWindowFlags_NoCollapse |
            ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize;

        if (!ImGuiMCP::Begin("Elements##slpp_ECM", nullptr, kFlags)) {
            ImGuiMCP::End();
            return;
        }
        SetWindowFontSize(scale.TextPx(UI::Theme::FontSize.body));
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::ToVec4(UI::Theme::Color.textSecondary));

        ImGuiMCP::SetNextItemWidth(-1.0f);
        ImGuiMCP::SliderFloat("##slpp_ecmScale", &_scaleAdjustment, 0.5f, 2.5f, "UI Scale %.2fx");
        if (ImGuiMCP::IsItemDeactivatedAfterEdit())
            OnScaleChange(a_hud, _scaleAdjustment);
        ImGuiMCP::SetNextItemWidth(-1.0f);
        ImGuiMCP::SliderFloat("##slpp_ecmTextScale", &_textScaleAdjustment, 0.5f, 2.5f, "Text Scale %.2fx");
        if (ImGuiMCP::IsItemDeactivatedAfterEdit())
            OnTextScaleChange(a_hud, _textScaleAdjustment);

        ImGuiMCP::PopStyleColor();

        // Elements
        SetWindowFontSize(scale.TextPx(UI::Theme::FontSize.body));
        UI::PushCheckboxStyle(scale.Factor());
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::ToVec4(UI::Theme::Color.textSecondary));

        const float toggleRowH = scale.Px(24.0f);
        const float rowPadH = scale.Px(12.0f);
        const float availW = ImGuiMCP::GetContentRegionAvail().x - rowPadH * 2.0f;
        const float cbSize = ImGuiMCP::GetFrameHeight();

        auto DrawToggleRow = [&](const char* label, const char* id, bool& state, auto onChange) {
            const ImGuiMCP::ImVec2 toggleRowMin = ImGuiMCP::GetCursorScreenPos();

            // Full row selectable button
            ImGuiMCP::SetCursorScreenPos({ toggleRowMin.x + rowPadH, toggleRowMin.y });
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_HeaderHovered, UI::Theme::ToVec4(UI::Theme::Color.transparent));
            std::string rowId = std::string("##slpp_ecp_row_") + id;
            if (UI::SelectableButton(rowId.c_str(), false, 0, ImGuiMCP::ImVec2{ availW, toggleRowH })) {
                state = !state;
                onChange(state);
            }
            ImGuiMCP::PopStyleColor();

            // Checkbox on the right
            const float cbX = toggleRowMin.x + rowPadH + availW - cbSize;
            const float cbY = toggleRowMin.y + (toggleRowH - cbSize) * 0.5f;
            ImGuiMCP::SetCursorScreenPos({ cbX, cbY });
            std::string cbId = std::string("##slpp_ecp_cb_") + id;
            if (ImGuiMCP::Checkbox(cbId.c_str(), &state)) {
                onChange(state);
            }

            // Label on the left
            const ImGuiMCP::ImVec2 labelSize = ImGuiMCP::CalcTextSize(label);
            const float labelY = toggleRowMin.y + (toggleRowH - labelSize.y) * 0.5f;
            ImGuiMCP::SetCursorScreenPos({ toggleRowMin.x + rowPadH, labelY });
            ImGuiMCP::TextUnformatted(label);

            // Advance cursor for next item
            ImGuiMCP::SetCursorScreenPos({ toggleRowMin.x, toggleRowMin.y + toggleRowH });
        };

        bool state_gameHud = inst->GetThreadProperty<bool>("ElementUI_GameHUD");
        DrawToggleRow("Game HUD", "gameHud", state_gameHud, [&](bool val) {
            inst->SetThreadProperty<bool>("ElementUI_GameHUD", val);
            Papyrus::SexLabUtil::HideElementsGameHUD(nullptr, !val);
        });

        bool state_AnimSpeed = inst->GetThreadProperty<bool>("ElementUI_AnimSpeed");
        DrawToggleRow("Anim Speed Overlay", "animSpeed", state_AnimSpeed, [&](bool val) {
            inst->SetThreadProperty<bool>("ElementUI_AnimSpeed", val);
        });

        bool state_EnjBars = inst->GetThreadProperty<bool>("ElementUI_EnjBars");
        DrawToggleRow("Enj Bars Overlay", "enjBars", state_EnjBars, [&](bool val) {
            inst->SetThreadProperty<bool>("ElementUI_EnjBars", val);
        });

        struct Row
        {
            const char* label;
            const char* property;
            PanelId panel;
        };
        constexpr std::array panelRows{
            Row{ "Thread Config Panel", "ElementUI_ThreadConfig", PanelId::kThreadConfig },
            Row{ "Scene Select Panel", "ElementUI_SceneSelect", PanelId::kSceneSelect },
            Row{ "Offset Adjust Panel", "ElementUI_OffsetAdjust", PanelId::kOffsetAdjust },
        };
        for (const auto& row : panelRows) {
            bool state = inst->GetThreadProperty<bool>(row.property);
            DrawToggleRow(row.label, row.property, state, [&](bool val) {
                inst->SetThreadProperty<bool>(row.property, val);
                if (!val && a_hud.IsPanelOpen(row.panel))
                    a_hud.CloseAllPanels();
            });
        }

        bool state_DebugNodeDraw = inst->GetThreadProperty<bool>("ElementUI_DebugNodeDraw");
        DrawToggleRow("Debug Node Draw", "debugNodeDraw", state_DebugNodeDraw, [&](bool val) {
            inst->SetThreadProperty<bool>("ElementUI_DebugNodeDraw", val);
        });

        ImGuiMCP::PopStyleColor();
        UI::PopCheckboxStyle();

        const float actionGap = scale.Px(UI::Theme::Spacing.sm);
        const float actionWidth = (ImGuiMCP::GetContentRegionAvail().x - actionGap) * 0.5f;
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::ToVec4(UI::Theme::Color.textSecondary));
        if (UI::ActionButton("Theme", actionWidth))
            _showThemeEditor = true;
        ImGuiMCP::SameLine(0.0f, actionGap);
        ImGuiMCP::BeginDisabled(!UI::Theme::IsLoaded());
        if (UI::ActionButton("Save", actionWidth))
            UI::Theme::Save();
        ImGuiMCP::EndDisabled();
        ImGuiMCP::PopStyleColor();

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();
    }

    void ElementCtrlPanel::RenderThemeEditor(SceneHUD& a_hud)
    {
        if (!_showThemeEditor)
            return;

        auto& scale = a_hud.GetScale();
        auto* io = ImGuiMCP::GetIO();
        const float editorMinHeight = scale.Px(100.0f);
        const float editorMaxHeight = std::max(editorMinHeight, io->DisplaySize.y * 0.75f);
        const float panelOffset = scale.Px(UI::Theme::Geometry.panelTabWidth + UI::Theme::Geometry.panelTabGap);
        const float editorRight = io->DisplaySize.x - panelOffset - scale.Px(220.0f) - scale.Px(UI::Theme::Spacing.sm);
        const float editorMaxWidth = std::min(editorRight, io->DisplaySize.x * 0.8f);
        const float fontSize = scale.TextPx(UI::Theme::FontSize.body);
        const auto* style = ImGuiMCP::GetStyle();
        const float editorWidth = std::ceil(std::min(
            MeasureThemeFieldsWidth(UI::Theme::data, ImGuiMCP::GetFont(), fontSize, *style) +
                style->WindowPadding.x * 2.0f + style->ScrollbarSize,
            editorMaxWidth));

        ImGuiMCP::SetNextWindowPos({ editorRight, io->DisplaySize.y * 0.5f }, ImGuiMCP::ImGuiCond_Appearing, { 1.0f, 0.5f });
        ImGuiMCP::SetNextWindowSizeConstraints(
            ImGuiMCP::ImVec2{ editorWidth, editorMinHeight }, ImGuiMCP::ImVec2{ editorWidth, editorMaxHeight });

        constexpr auto kFlags = ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize;
        if (!ImGuiMCP::Begin("Theme##slpp_ThemeEditor", &_showThemeEditor, kFlags)) {
            ImGuiMCP::End();
            return;
        }

        SetWindowFontSize(fontSize);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::ToVec4(UI::Theme::Color.textSecondary));
        DrawThemeFields(UI::Theme::data);
        ImGuiMCP::PopStyleColor();
        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();
    }
}
