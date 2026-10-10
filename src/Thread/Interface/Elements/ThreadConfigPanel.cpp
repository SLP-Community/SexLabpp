#include "ThreadConfigPanel.h"
#include "Registry/Library.h"
#include "SKSE/Translation.h"

namespace Thread::Interface
{
    using UI::SetWindowFontSize;

    void ThreadConfigPanel::Open(SceneHUD& a_hud)
    {
        _actorStates.clear();
        auto* inst = a_hud.GetThreadInstance();
        if (!inst)
            return;
        for (auto* actor : inst->GetActors()) {
            if (!actor)
                continue;
            _actorStates.push_back({ actor->GetFormID(), actor->IsPlayerRef() });
        }
        // Render() preserves this actor ordering for the lifetime of the panel.
        _sortedActors.clear();
        for (auto* actor : inst->GetActors())
            if (actor)
                _sortedActors.push_back(actor);
        std::ranges::sort(_sortedActors, [](RE::Actor* a, RE::Actor* b) {
            if (a->IsPlayerRef() != b->IsPlayerRef())
                return a->IsPlayerRef();
            return std::string_view{ a->GetDisplayFullName() } < std::string_view{ b->GetDisplayFullName() };
        });
    }

    void ThreadConfigPanel::Close()
    {
        _actorStates.clear();
        _sortedActors.clear();
    }

    // ── Handlers ────────────────────────────────────────────────────────────

    void ThreadConfigPanel::OnRandomScene(SceneHUD& a_hud)
    {
        auto* inst = a_hud.GetThreadInstance();
        if (!inst)
            return;
        const auto* cur = inst->GetActiveScene();
        const auto scenes = inst->GetThreadScenes();
        std::vector<const Registry::Animation::Scene*> pool;
        pool.reserve(scenes.size());
        for (auto* s : scenes)
            if (s != cur)
                pool.push_back(s);
        if (pool.empty())
            return;
        const std::string id{ Random::draw(pool)->GetId() };
        Script::DispatchMethodCall(a_hud.GetThreadScript(), "ResetScene",
            a_hud.GetCallback(), RE::BSFixedString{ id.c_str() });
    }

    void ThreadConfigPanel::OnMoveScene(SceneHUD& a_hud)
    {
        if (!a_hud.GetThreadScript())
            return;
        Script::DispatchMethodCall(a_hud.GetThreadScript(), "MoveScene", a_hud.GetCallback());
    }

    void ThreadConfigPanel::OnAutoPlaySet(SceneHUD& a_hud, bool state)
    {
        auto* inst = a_hud.GetThreadInstance();
        if (inst)
            inst->SetThreadProperty<bool>("AutoAdvance", state);
    }

    void ThreadConfigPanel::OnNextPosition(SceneHUD& a_hud, RE::Actor* actor)
    {
        auto* inst = a_hud.GetThreadInstance();
        if (inst && actor)
            inst->SetNextPermutation(actor);
    }

    void ThreadConfigPanel::OnSetExpression(SceneHUD& a_hud, RE::Actor* actor, const Registry::Expression* expr)
    {
        auto* inst = a_hud.GetThreadInstance();
        if (inst && actor && expr)
            inst->SetExpression(actor, expr);
    }

    void ThreadConfigPanel::OnSetVoice(SceneHUD& a_hud, RE::Actor* actor, const Registry::Voice* voice)
    {
        auto* inst = a_hud.GetThreadInstance();
        if (inst && actor && voice)
            inst->SetVoice(actor, voice);
    }

    void ThreadConfigPanel::OnSetActorAlpha(RE::Actor* actor, int alphaInt)
    {
        if (actor)
            actor->SetAlpha(std::clamp(alphaInt, 0, 100) / 100.0f);
    }

    // ── Actor card ──────────────────────────────────────────────────────────

    void ThreadConfigPanel::RenderActorCard(SceneHUD& a_hud, RE::Actor* actor, ActorState& state)
    {
        if (!actor)
            return;
        auto* lib = Registry::Library::GetSingleton();
        auto& scale = a_hud.GetScale();

        const float nestedScale = UI::Theme::Geometry.nestedMenuScale;
        const float contentX = ImGuiMCP::GetCursorPosX();
        const float availableW = ImGuiMCP::GetContentRegionAvail().x;
        const float cardW = availableW * nestedScale;
        const float cardX = contentX + (availableW - cardW) * 0.5f;
        const float rowPadV = scale.Px(6.0f) * nestedScale;
        const float rowPadH = scale.Px(12.0f) * nestedScale;
        const float fieldGap = scale.Px(UI::Theme::Spacing.sm) * nestedScale;
        const float captionSize = scale.TextPx(UI::Theme::FontSize.caption) * nestedScale;
        const float subsectionHeaderSize = scale.TextPx(UI::Theme::FontSize.subsectionHeader) * nestedScale;

        SetWindowFontSize(captionSize);
        const float labelW = ImGuiMCP::CalcTextSize("Scene Position").x + fieldGap;
        const float fieldX = cardX + rowPadH + labelW;
        const float fieldW = cardW - rowPadH * 2.0f - labelW;
        const float alphaValueW = ImGuiMCP::CalcTextSize("100%").x;
        const float alphaW = fieldW - fieldGap - alphaValueW;

        const auto* style = ImGuiMCP::GetStyle();
        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_FramePadding,
            ImGuiMCP::ImVec2{ style->FramePadding.x * nestedScale, style->FramePadding.y * nestedScale });
        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_ItemSpacing,
            ImGuiMCP::ImVec2{ style->ItemSpacing.x * nestedScale, style->ItemSpacing.y * nestedScale });
        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_ItemInnerSpacing,
            ImGuiMCP::ImVec2{ style->ItemInnerSpacing.x * nestedScale, style->ItemInnerSpacing.y * nestedScale });
        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_GrabMinSize, style->GrabMinSize * nestedScale);

        ImGuiMCP::PushID(static_cast<int>(actor->GetFormID()));

        ImGuiMCP::SetCursorPosX(cardX);
        SetWindowFontSize(subsectionHeaderSize);
        if (UI::CollapsibleCardHeader(actor->GetDisplayFullName(), actor->IsPlayerRef() ? "PLAYER" : nullptr,
                "##slpp_tcmCardHdr", state.cardOpen, cardW, scale.Factor() * nestedScale))
            state.cardOpen = !state.cardOpen;
        auto* dl = ImGuiMCP::GetWindowDrawList();

        // ── Card body (collapsible) ─────────────────────────────────────────
        if (!state.cardOpen) {
            ImGuiMCP::SetCursorPosX(contentX);
            ImGuiMCP::PopStyleVar(4);
            ImGuiMCP::PopID();
            return;
        }
        auto* inst = a_hud.GetThreadInstance();
        const ImGuiMCP::ImVec2 bodyMin = ImGuiMCP::GetCursorScreenPos();
        ImGuiMCP::SetCursorScreenPos(bodyMin);
        ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, scale.Px(UI::Theme::Spacing.xs) * nestedScale });
        ImGuiMCP::ImDrawListManager::ChannelsSplit(dl, 2);
        ImGuiMCP::ImDrawListManager::ChannelsSetCurrent(dl, 1);

        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_FrameBg, UI::Theme::Color.nestedControl);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_FrameBgHovered, UI::Theme::Color.nestedControlHovered);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_FrameBgActive, UI::Theme::Color.nestedControlActive);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Button, UI::Theme::Color.nestedControl);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ButtonHovered, UI::Theme::Color.nestedControlHovered);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ButtonActive, UI::Theme::Color.nestedControlActive);
        SetWindowFontSize(scale.TextPx(UI::Theme::FontSize.compact) * nestedScale);

        // ── Expression combo
        if (Registry::RaceKey(actor).Is(Registry::RaceKey::Value::Human)) {
            const auto* curExpr = inst->GetExpression(actor);
            std::string curLabel = curExpr ? curExpr->GetId().c_str() : "(none)";
            SKSE::Translation::Translate(curLabel, curLabel);

            ImGuiMCP::SetCursorPosX(cardX + rowPadH);
            SetWindowFontSize(captionSize);
            ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color.textSecondary), "Expression");
            ImGuiMCP::SameLine(fieldX);

            SetWindowFontSize(captionSize);
            ImGuiMCP::SetNextItemWidth(fieldW);
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_PopupBg, UI::Theme::Color.nestedPopup);
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::ToVec4(UI::Theme::Color.textMuted));
            if (ImGuiMCP::BeginCombo("##slpp_tcmExpr", curLabel.c_str())) {
                ImGuiMCP::PopStyleColor();
                lib->ForEachExpression([&](const auto& expr) {
                    if (!expr.enabled)
                        return false;
                    std::string label{ expr.GetId().c_str() };
                    SKSE::Translation::Translate(label, label);
                    const bool sel = curExpr && curExpr->GetId() == expr.GetId();
                    if (sel)
                        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::Color.selectionText);
                    else
                        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::ToVec4(UI::Theme::Color.textMuted));
                    SetWindowFontSize(captionSize);
                    if (ImGuiMCP::Selectable(label.c_str(), sel)) {
                        OnSetExpression(a_hud, actor, &expr);
                    }
                    ImGuiMCP::PopStyleColor();
                    return false;
                });
                ImGuiMCP::EndCombo();
            } else {
                ImGuiMCP::PopStyleColor();
            }
            ImGuiMCP::PopStyleColor();
            ImGuiMCP::SetCursorPosY(ImGuiMCP::GetCursorPosY() + rowPadV);
        }

        // ── Voice combo
        {
            const auto* curVoice = inst->GetVoice(actor);
            std::string curLabel = curVoice ? curVoice->GetId().c_str() : "(none)";
            SKSE::Translation::Translate(curLabel, curLabel);
            const Registry::RaceKey raceKey{ actor };

            ImGuiMCP::SetCursorPosX(cardX + rowPadH);
            SetWindowFontSize(captionSize);
            ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color.textSecondary), "Voice");
            ImGuiMCP::SameLine(fieldX);

            SetWindowFontSize(captionSize);
            ImGuiMCP::SetNextItemWidth(fieldW);
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_PopupBg, UI::Theme::Color.nestedPopup);
            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::ToVec4(UI::Theme::Color.textMuted));
            if (ImGuiMCP::BeginCombo("##slpp_tcmVoice", curLabel.c_str())) {
                ImGuiMCP::PopStyleColor();
                lib->ForEachVoice([&](const auto& v) {
                    if (!v.HasRace(raceKey))
                        return false;
                    std::string label{ v.GetId().c_str() };
                    SKSE::Translation::Translate(label, label);
                    const bool sel = curVoice && curVoice->GetId() == v.GetId();
                    if (sel)
                        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::Color.selectionText);
                    else
                        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::ToVec4(UI::Theme::Color.textMuted));
                    SetWindowFontSize(captionSize);
                    if (ImGuiMCP::Selectable(label.c_str(), sel)) {
                        OnSetVoice(a_hud, actor, &v);
                    }
                    ImGuiMCP::PopStyleColor();
                    return false;
                });
                ImGuiMCP::EndCombo();
            } else {
                ImGuiMCP::PopStyleColor();
            }
            ImGuiMCP::PopStyleColor();
            ImGuiMCP::SetCursorPosY(ImGuiMCP::GetCursorPosY() + rowPadV);
        }

        // ── Alpha slider
        {
            int alphaInt = static_cast<int>(std::round(actor->GetAlpha() * 100.0f));

            ImGuiMCP::SetCursorPosX(cardX + rowPadH);
            SetWindowFontSize(captionSize);
            ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color.textSecondary), "Alpha");
            ImGuiMCP::SameLine(fieldX);

            ImGuiMCP::SetNextItemWidth(alphaW);
            ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_FramePadding,
                ImGuiMCP::ImVec2{ 0.0f, scale.Px(0.5f) * nestedScale });
            ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_GrabMinSize, scale.Px(6.0f) * nestedScale);
            if (ImGuiMCP::SliderInt("##slpp_tcmAlpha", &alphaInt, 0, 100, ""))
                OnSetActorAlpha(actor, alphaInt);  // actor's opacity updates live while dragging
            ImGuiMCP::PopStyleVar(2);

            ImGuiMCP::SameLine(0.0f, fieldGap);
            SetWindowFontSize(captionSize);
            ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color.textMuted), "%d%%", alphaInt);
        }

        // ── Scene position row
        {
            const int32_t current = inst->GetCurrentPermutation(actor);
            const int32_t total = inst->GetUniquePermutations(actor);
            const bool canCycle = total > 1;

            if (canCycle) {
                char permBuf[24];
                std::snprintf(permBuf, sizeof(permBuf), "%d of %d", current, total);
                const float btnRoundedH = scale.Px(20.0f) * nestedScale;
                const float btnRoundedW = btnRoundedH + scale.Px(8.0f) * nestedScale;

                SetWindowFontSize(captionSize);
                const float labelH = ImGuiMCP::CalcTextSize("Scene Position").y;
                const float rowH = std::max(labelH, btnRoundedH);
                const float textOffsetY = (rowH - labelH) * 0.5f;

                ImGuiMCP::SetCursorPosY(ImGuiMCP::GetCursorPosY() + rowPadV + textOffsetY);
                ImGuiMCP::SetCursorPosX(cardX + rowPadH);

                // Label (left)
                ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color.textSecondary), "Scene Position");

                // Count (center, same line)
                const float permTextW = ImGuiMCP::CalcTextSize(permBuf).x;
                const float centerX = cardX + cardW * 0.5f - permTextW * 0.5f;
                ImGuiMCP::SameLine();
                ImGuiMCP::SetCursorPosX(centerX);
                ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color.textMuted), "%s", permBuf);

                // Next-perm button (right, same line)
                const float rightBtnX = cardX + cardW - rowPadH - btnRoundedW;
                ImGuiMCP::SameLine();
                ImGuiMCP::SetCursorPosX(rightBtnX);
                SKSEMenuFramework::PushFont(UI::Theme::Icon::solidFont);
                const bool nextPosition = ImGuiMCP::Button(UI::Theme::Icon::nextPerm, ImGuiMCP::ImVec2{ btnRoundedW, btnRoundedH });
                FontAwesome::Pop();
                if (ImGuiMCP::IsItemHovered())
                    ImGuiMCP::SetTooltip("Move actor to the next compatible scene position");
                if (nextPosition)
                    OnNextPosition(a_hud, actor);
            }
        }

        ImGuiMCP::SetCursorPosY(ImGuiMCP::GetCursorPosY() + rowPadV);
        ImGuiMCP::PopStyleColor(6);

        const ImGuiMCP::ImVec2 bodyMax{ cardX + cardW, ImGuiMCP::GetCursorScreenPos().y };
        ImGuiMCP::ImDrawListManager::ChannelsSetCurrent(dl, 0);
        ImGuiMCP::ImDrawListManager::AddRectFilled(
            dl, bodyMin, bodyMax, UI::Theme::Color.nestedSurface, 0.0f, 0);
        ImGuiMCP::ImDrawListManager::ChannelsMerge(dl);

        ImGuiMCP::SetCursorPosX(contentX);
        ImGuiMCP::PopStyleVar(4);
        ImGuiMCP::PopID();
    }

    // ── Render ──────────────────────────────────────────────────────────────

    void ThreadConfigPanel::Render(SceneHUD& a_hud)
    {
        auto* inst = a_hud.GetThreadInstance();
        if (!inst)
            return;

        auto& scale = a_hud.GetScale();
        auto* io = ImGuiMCP::GetIO();
        const float panelW = scale.Px(280.0f);
        const float offset = scale.Px(UI::Theme::Geometry.panelTabWidth + UI::Theme::Geometry.panelTabGap);
        const float rowMinH = scale.Px(28.0f);
        const float rowPadH = scale.Px(UI::Theme::Spacing.lg);
        const float maxBodyH = scale.Px(340.0f);  // before scrolling

        ImGuiMCP::SetNextWindowPos(
            ImGuiMCP::ImVec2{ io->DisplaySize.x - offset, io->DisplaySize.y * 0.5f },
            ImGuiMCP::ImGuiCond_Always, ImGuiMCP::ImVec2{ 1.0f, 0.5f });
        ImGuiMCP::SetNextWindowSizeConstraints(
            ImGuiMCP::ImVec2{ panelW, rowMinH },
            ImGuiMCP::ImVec2{ panelW, io->DisplaySize.y * 0.8f });
        ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{ panelW, 0.0f }, ImGuiMCP::ImGuiCond_Always);

        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
            ImGuiMCP::ImGuiWindowFlags_NoMove | ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
            ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize;

        if (!ImGuiMCP::Begin("##slpp_TCM", nullptr, kFlags)) {
            ImGuiMCP::End();
            return;
        }

        // ── Auto Advance toggle
        SetWindowFontSize(scale.TextPx(UI::Theme::FontSize.body));
        bool autoPlay = inst->GetThreadProperty<bool>("AutoAdvance");
        if (UI::CheckboxRow("Auto Advance", "slpp_tcmAutoAdvance", autoPlay, scale.Factor()))
            OnAutoPlaySet(a_hud, autoPlay);

        ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, scale.Px(UI::Theme::Spacing.md) });

        // ── Actions
        SetWindowFontSize(scale.TextPx(UI::Theme::FontSize.body));
        const float actionGap = scale.Px(UI::Theme::Spacing.sm);
        const float actionW = (panelW - rowPadH * 2.0f - actionGap) * 0.5f;
        ImGuiMCP::SetCursorPosX(rowPadH);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::ToVec4(UI::Theme::Color.textSecondary));
        if (UI::ActionButton("Random Scene", actionW))
            OnRandomScene(a_hud);
        ImGuiMCP::SameLine(0.0f, actionGap);
        if (UI::ActionButton("Move Scene", actionW))
            OnMoveScene(a_hud);
        ImGuiMCP::PopStyleColor();

        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Separator, UI::Theme::Color.nestedSeparator);
        ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, scale.Px(UI::Theme::Spacing.sm) });
        ImGuiMCP::Separator();
        ImGuiMCP::PopStyleColor();

        // ── Actors list with scrolling
        ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, scale.Px(UI::Theme::Spacing.xs) });

        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_ScrollbarSize, scale.Px(6.0f));
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ScrollbarBg, UI::Theme::Color.panelBackground);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ScrollbarGrab, UI::Theme::ToVec4(UI::Theme::Color.borderSubtle));
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ScrollbarGrabHovered, UI::Theme::ToVec4(UI::Theme::Color.borderHovered));
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ScrollbarGrabActive, UI::Theme::ToVec4(UI::Theme::Color.borderActive));

        ImGuiMCP::SetNextWindowSizeConstraints(
            ImGuiMCP::ImVec2{ 0.0f, 0.0f }, ImGuiMCP::ImVec2{ FLT_MAX, maxBodyH });
        ImGuiMCP::BeginChild("##slpp_tcmActors", ImGuiMCP::ImVec2{ -FLT_MIN, 0.0f },
            ImGuiMCP::ImGuiChildFlags_AutoResizeY, ImGuiMCP::ImGuiWindowFlags_None);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Separator, UI::Theme::Color.nestedSeparator);

        bool first = true;
        for (auto* actor : _sortedActors) {
            const uint32_t fid = actor->GetFormID();
            ActorState* st = nullptr;
            for (auto& state : _actorStates)
                if (state.formId == fid) {
                    st = &state;
                    break;
                }
            if (!st) {
                _actorStates.push_back({ fid, false });
                st = &_actorStates.back();
            }
            if (!first)
                ImGuiMCP::Separator();
            first = false;
            RenderActorCard(a_hud, actor, *st);
        }

        ImGuiMCP::PopStyleColor();
        ImGuiMCP::EndChild();

        ImGuiMCP::PopStyleColor(4);
        ImGuiMCP::PopStyleVar();

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();
    }
}
