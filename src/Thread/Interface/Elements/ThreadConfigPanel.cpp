#include "ThreadConfigPanel.h"
#include "Registry/Library.h"
#include "SKSE/Translation.h"

namespace Thread::Interface
{
    using UI::SetWindowFontSize;

    ThreadConfigPanel& ThreadConfigPanel::GetSingleton()
    {
        static ThreadConfigPanel singleton;
        return singleton;
    }

    bool ThreadConfigPanel::Register()
    {
        return RegisterWindow(RenderCallback);
    }

    void __stdcall ThreadConfigPanel::RenderCallback()
    {
        GetSingleton().Render();
    }

    void ThreadConfigPanel::Open()
    {
        _actorStates.clear();
        auto* inst = SceneHUD::GetSingleton().GetThreadInstance();
        if (!inst)
            return;
        for (auto* actor : inst->GetActors()) {
            if (!actor)
                continue;
            _actorStates.push_back({ actor->GetFormID(), true });
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
        Show();
    }

    void ThreadConfigPanel::Close()
    {
        Hide();
        _actorStates.clear();
        _sortedActors.clear();
    }

    // ── Handlers ────────────────────────────────────────────────────────────

    void ThreadConfigPanel::OnRandomScene()
    {
        auto& hud = SceneHUD::GetSingleton();
        auto* inst = hud.GetThreadInstance();
        if (!inst)
            return;
        const auto* cur = inst->GetActiveScene();
        const auto scenes = inst->GetThreadScenes();
        std::vector<const Registry::Scene*> pool;
        pool.reserve(scenes.size());
        for (auto* s : scenes)
            if (s != cur)
                pool.push_back(s);
        if (pool.empty())
            return;
        const std::string id{ Random::draw(pool)->id };
        Script::DispatchMethodCall(hud.GetThreadScript(), "ResetScene",
            hud.GetCallback(), RE::BSFixedString{ id.c_str() });
    }

    void ThreadConfigPanel::OnMoveScene()
    {
        auto& hud = SceneHUD::GetSingleton();
        if (!hud.GetThreadScript())
            return;
        Script::DispatchMethodCall(hud.GetThreadScript(), "ToggleFocusSceneHUD", hud.GetCallback());
        Script::DispatchMethodCall(hud.GetThreadScript(), "MoveScene", hud.GetCallback());
    }

    void ThreadConfigPanel::OnAutoPlaySet(bool state)
    {
        auto* inst = SceneHUD::GetSingleton().GetThreadInstance();
        if (inst)
            inst->SetThreadProperty<bool>("AutoAdvance", state);
    }

    void ThreadConfigPanel::OnNextPermutation(RE::Actor* actor)
    {
        auto* inst = SceneHUD::GetSingleton().GetThreadInstance();
        if (inst && actor)
            inst->SetNextPermutation(actor);
    }

    void ThreadConfigPanel::OnSetExpression(RE::Actor* actor, const Registry::Expression* expr)
    {
        auto* inst = SceneHUD::GetSingleton().GetThreadInstance();
        if (inst && actor && expr)
            inst->SetExpression(actor, expr);
    }

    void ThreadConfigPanel::OnSetVoice(RE::Actor* actor, const Registry::Voice* voice)
    {
        auto* inst = SceneHUD::GetSingleton().GetThreadInstance();
        if (inst && actor && voice)
            inst->SetVoice(actor, voice);
    }

    void ThreadConfigPanel::OnSetActorAlpha(RE::Actor* actor, int alphaInt)
    {
        if (actor)
            actor->SetAlpha(std::clamp(alphaInt, 0, 100) / 100.0f);
    }

    // ── Actor card ──────────────────────────────────────────────────────────

    void ThreadConfigPanel::RenderActorCard(RE::Actor* actor, ActorState& state)
    {
        if (!actor)
            return;
        auto* lib = Registry::Library::GetSingleton();

        const float panelW = ScaleUI::pxScale(280.0f);
        const float rowMinH = ScaleUI::pxScale(28.0f);
        const float rowPadV = ScaleUI::pxScale(6.0f);
        const float rowPadH = ScaleUI::pxScale(12.0f);
        const float hdrPadV = ScaleUI::pxScale(5.0f);
        const float hdrPadH = ScaleUI::pxScale(10.0f);
        const float lblW = ScaleUI::pxScale(70.0f);
        const float dropW = ScaleUI::pxScale(140.0f);
        const float alphaW = ScaleUI::pxScale(100.0f);

        ImGuiMCP::PushID(static_cast<int>(actor->GetFormID()));

        // ── Card header ─────────────────────────────────────────────────────
        // An invisible-label Selectable provides the real user interaction here.
        // The header's actual content (arrow, name, badge) is drawn on top of it afterward.
        const ImGuiMCP::ImVec2 hdrMin = ImGuiMCP::GetCursorScreenPos();
        const float hdrH = ScaleUI::pxScale(9.0f) + hdrPadV * 2.0f;

        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Header, UI::Theme::Color::cardHeader);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_HeaderHovered, UI::Theme::Color::cardHeader);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_HeaderActive, UI::Theme::Color::cardHeader);
        if (ImGuiMCP::Selectable("##slpp_tcmCardHdr", false, 0, ImGuiMCP::ImVec2{ panelW, hdrH }))
            state.cardOpen = !state.cardOpen;
        ImGuiMCP::PopStyleColor(3);
        const bool hdrHov = ImGuiMCP::IsItemHovered();

        // Idle background when not hovered/focused —> paint a faint background
        auto* dl = ImGuiMCP::GetWindowDrawList();
        if (!hdrHov) {
            const ImGuiMCP::ImVec2 hdrMax{ hdrMin.x + panelW, hdrMin.y + hdrH };
            ImGuiMCP::ImDrawListManager::AddRectFilled(dl, hdrMin, hdrMax, UI::Theme::Color::cardIdle, 0.0f, 0);
        }

        ImGuiMCP::SetCursorScreenPos(hdrMin);
        SetWindowFontSize(ScaleUI::pxScale(UI::Theme::FontSize::caption));

        // Collapse arrow
        ImGuiMCP::SetCursorScreenPos(ImGuiMCP::ImVec2{ hdrMin.x + hdrPadH, hdrMin.y + hdrPadV });
        ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textMuted),
            state.cardOpen ? "\xe2\x96\xbc" : "\xe2\x96\xb2");  // UTF-8 ▼ (open) / ▲ (closed)

        // Name, truncated
        ImGuiMCP::SameLine(0.0f, ScaleUI::pxScale(6.0f));
        ImGuiMCP::TextColored(UI::Theme::ToVec4(hdrHov ? UI::Theme::Color::textSecondary : UI::Theme::Color::textMuted),
            "%s", actor->GetDisplayFullName());

        // Badges flush-right: player / position
        const float badgeX = hdrMin.x + panelW - hdrPadH - (actor->IsPlayerRef() ? ScaleUI::pxScale(40.0f) : 0.0f);
        if (actor->IsPlayerRef()) {
            ImGuiMCP::SetCursorScreenPos(ImGuiMCP::ImVec2{ badgeX, hdrMin.y + hdrPadV });
            ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::accent), "PLAYER");
        }

        ImGuiMCP::SetCursorScreenPos(ImGuiMCP::ImVec2{ hdrMin.x, hdrMin.y + hdrH });

        // ── Card body (collapsible) ─────────────────────────────────────────
        if (!state.cardOpen) {
            ImGuiMCP::PopID();
            return;
        }
        auto* inst = SceneHUD::GetSingleton().GetThreadInstance();
        SetWindowFontSize(ScaleUI::pxScale(UI::Theme::FontSize::compact));

        // ── Permutation row
        {
            const uint32_t cur = inst->GetCurrentPermutation(actor);
            const uint32_t total = inst->GetUniquePermutations(actor);

            ImGuiMCP::SetCursorPosX(rowPadH);
            SetWindowFontSize(ScaleUI::pxScale(UI::Theme::FontSize::caption));
            ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textMuted), "Permutation");
            ImGuiMCP::SameLine(lblW);

            const float btnW = ScaleUI::pxScale(22.0f);
            SetWindowFontSize(ScaleUI::pxScale(UI::Theme::FontSize::caption));
            char permBuf[24];
            std::snprintf(permBuf, sizeof(permBuf), "%u / %u", cur, total);
            const float permW = std::max(ImGuiMCP::CalcTextSize(permBuf).x, ScaleUI::pxScale(30.0f));
            ImGuiMCP::SetNextItemWidth(permW);
            ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textSecondary), "%s", permBuf);

            ImGuiMCP::SameLine(0.0f, ScaleUI::pxScale(6.0f));
            if (ImGuiMCP::Button("▶##slpp_tcmNext", ImGuiMCP::ImVec2{ btnW, rowMinH })) {
                OnNextPermutation(actor);
            }
            ImGuiMCP::SetCursorPosY(ImGuiMCP::GetCursorPosY() + rowPadV);
            ImGuiMCP::ImDrawListManager::AddLine(dl,
                ImGuiMCP::ImVec2{ ImGuiMCP::GetWindowPos().x, ImGuiMCP::GetCursorScreenPos().y },
                ImGuiMCP::ImVec2{ ImGuiMCP::GetWindowPos().x + panelW, ImGuiMCP::GetCursorScreenPos().y },
                UI::Theme::Color::separator, 1.0f);
        }

        // ── Expression combo
        if (Registry::RaceKey(actor).Is(Registry::RaceKey::Value::Human)) {
            const auto* curExpr = inst->GetExpression(actor);
            std::string curLabel = curExpr ? curExpr->GetId().c_str() : "(none)";
            SKSE::Translation::Translate(curLabel, curLabel);

            ImGuiMCP::SetCursorPosX(rowPadH);
            SetWindowFontSize(ScaleUI::pxScale(UI::Theme::FontSize::caption));
            ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textMuted), "Expression");
            ImGuiMCP::SameLine(lblW);

            SetWindowFontSize(ScaleUI::pxScale(UI::Theme::FontSize::caption));
            ImGuiMCP::SetNextItemWidth(dropW);
            if (ImGuiMCP::BeginCombo("##slpp_tcmExpr", curLabel.c_str())) {
                lib->ForEachExpression([&](const auto& expr) {
                    if (!expr.enabled)
                        return false;
                    std::string label{ expr.GetId().c_str() };
                    SKSE::Translation::Translate(label, label);
                    const bool sel = curExpr && curExpr->GetId() == expr.GetId();
                    if (sel)
                        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::Color::selectionText);
                    if (ImGuiMCP::Selectable(label.c_str(), sel)) {
                        OnSetExpression(actor, &expr);
                    }
                    if (sel)
                        ImGuiMCP::PopStyleColor();
                    return false;
                });
                ImGuiMCP::EndCombo();
            }
            ImGuiMCP::SetCursorPosY(ImGuiMCP::GetCursorPosY() + rowPadV);
        }

        // ── Voice combo
        {
            const auto* curVoice = inst->GetVoice(actor);
            std::string curLabel = curVoice ? curVoice->GetId().c_str() : "(none)";
            SKSE::Translation::Translate(curLabel, curLabel);
            const Registry::RaceKey raceKey{ actor };

            ImGuiMCP::SetCursorPosX(rowPadH);
            SetWindowFontSize(ScaleUI::pxScale(UI::Theme::FontSize::caption));
            ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textMuted), "Voice");
            ImGuiMCP::SameLine(lblW);

            SetWindowFontSize(ScaleUI::pxScale(UI::Theme::FontSize::caption));
            ImGuiMCP::SetNextItemWidth(dropW);
            if (ImGuiMCP::BeginCombo("##slpp_tcmVoice", curLabel.c_str())) {
                lib->ForEachVoice([&](const auto& v) {
                    if (!v.HasRace(raceKey))
                        return false;
                    std::string label{ v.GetId().c_str() };
                    SKSE::Translation::Translate(label, label);
                    const bool sel = curVoice && curVoice->GetId() == v.GetId();
                    if (sel)
                        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::Color::selectionText);
                    if (ImGuiMCP::Selectable(label.c_str(), sel)) {
                        OnSetVoice(actor, &v);
                    }
                    if (sel)
                        ImGuiMCP::PopStyleColor();
                    return false;
                });
                ImGuiMCP::EndCombo();
            }
            ImGuiMCP::SetCursorPosY(ImGuiMCP::GetCursorPosY() + rowPadV);
        }

        // ── Alpha slider
        {
            int alphaInt = static_cast<int>(std::round(actor->GetAlpha() * 100.0f));

            ImGuiMCP::SetCursorPosX(rowPadH);
            SetWindowFontSize(ScaleUI::pxScale(UI::Theme::FontSize::caption));
            ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textMuted), "Alpha");
            ImGuiMCP::SameLine(lblW);

            ImGuiMCP::SetNextItemWidth(alphaW);
            ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_FramePadding, ImGuiMCP::ImVec2{ 0.0f, ScaleUI::pxScale(1.5f) });
            if (ImGuiMCP::SliderInt("##slpp_tcmAlpha", &alphaInt, 0, 100, ""))
                OnSetActorAlpha(actor, alphaInt);  // actor's opacity updates live while dragging
            ImGuiMCP::PopStyleVar();

            ImGuiMCP::SameLine(0.0f, ScaleUI::pxScale(6.0f));
            SetWindowFontSize(ScaleUI::pxScale(UI::Theme::FontSize::caption));
            ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textMuted), "%d%%", alphaInt);
        }

        ImGuiMCP::SetCursorPosY(ImGuiMCP::GetCursorPosY() + rowPadV);
        ImGuiMCP::PopID();
    }

    // ── Render ──────────────────────────────────────────────────────────────

    void ThreadConfigPanel::Render()
    {
        auto& hud = SceneHUD::GetSingleton();
        if (!IsVisible() || !hud.IsActive() || !hud.IsPanelOpen(PanelId::kThreadConfig) || !hud.IsFocused())
            return;

        auto* inst = hud.GetThreadInstance();
        if (!inst)
            return;

        auto* io = ImGuiMCP::GetIO();
        const float panelW = ScaleUI::pxScale(280.0f);
        const float offset = ScaleUI::pxScale(UI::Theme::Geometry::panelTabWidth + UI::Theme::Geometry::panelTabGap);
        const float rowMinH = ScaleUI::pxScale(28.0f);
        const float rowPadV = ScaleUI::pxScale(6.0f);
        const float rowPadH = ScaleUI::pxScale(12.0f);
        const float maxBodyH = ScaleUI::pxScale(340.0f);  // before scrolling

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

        // ── THREAD section
        SetWindowFontSize(ScaleUI::pxScale(UI::Theme::FontSize::caption));
        if (ImGuiMCP::Selectable(
                _threadSectionOpen ? "  THREAD  \xe2\x96\xbc" : "  THREAD  \xe2\x96\xb2",
                false, 0, ImGuiMCP::ImVec2{ 0.0f, ScaleUI::pxScale(20.0f) }))
            _threadSectionOpen = !_threadSectionOpen;
        ImGuiMCP::Separator();

        if (_threadSectionOpen) {
            SetWindowFontSize(ScaleUI::pxScale(UI::Theme::FontSize::body));
            ImGuiMCP::SetCursorPosX(rowPadH);
            if (ImGuiMCP::Selectable("Random Scene", false, 0, ImGuiMCP::ImVec2{ panelW - rowPadH * 2.0f, rowMinH }))
                OnRandomScene();
            ImGuiMCP::SetCursorPosX(rowPadH);
            if (ImGuiMCP::Selectable("Move Scene", false, 0, ImGuiMCP::ImVec2{ panelW - rowPadH * 2.0f, rowMinH }))
                OnMoveScene();

            ImGuiMCP::SetCursorPosX(rowPadH);
            ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textMuted), "Auto Advance");
            ImGuiMCP::SameLine(panelW - rowPadH - ScaleUI::pxScale(20.0f));
            bool autoPlay = inst->GetThreadProperty<bool>("AutoAdvance");
            if (ImGuiMCP::Checkbox("##slpp_tcmAutoplay", &autoPlay)) {
                OnAutoPlaySet(autoPlay);
            }

            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, rowPadV });
        }

        ImGuiMCP::Separator();

        // ── ACTORS section
        SetWindowFontSize(ScaleUI::pxScale(UI::Theme::FontSize::caption));
        if (ImGuiMCP::Selectable(
                _actorsSectionOpen ? "  ACTORS  \xe2\x96\xbc" : "  ACTORS  \xe2\x96\xb2",
                false, 0, ImGuiMCP::ImVec2{ 0.0f, ScaleUI::pxScale(20.0f) }))
            _actorsSectionOpen = !_actorsSectionOpen;
        ImGuiMCP::Separator();

        if (_actorsSectionOpen) {
            ImGuiMCP::BeginChild("##slpp_tcmActors", ImGuiMCP::ImVec2{ panelW, maxBodyH }, false,
                ImGuiMCP::ImGuiWindowFlags_NoScrollbar);

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
                RenderActorCard(actor, *st);
            }

            ImGuiMCP::EndChild();
        }

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();
    }
}
