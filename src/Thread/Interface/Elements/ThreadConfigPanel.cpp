#include "ThreadConfigPanel.h"
#include "Registry/Library.h"
#include "SKSE/Translation.h"

namespace Thread::Interface
{
    void ThreadConfigPanel::Init()
    {
        s_actorStates.clear();
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (!inst) return;
        bool first = true;
        for (auto* actor : inst->GetActors()) {
            if (!actor) continue;
            ActorState st;
            st.formId = actor->GetFormID();
            st.cardOpen = true;
            s_actorStates.push_back(st);
            first = false;
        }
        // Render() iterates s_sortedActors directly, never re-sorts.
        s_sortedActors.clear();
        for (auto* a : inst->GetActors()) if (a) s_sortedActors.push_back(a);
        std::sort(s_sortedActors.begin(), s_sortedActors.end(), [](RE::Actor* a, RE::Actor* b) {
            if (a->IsPlayerRef() != b->IsPlayerRef()) return a->IsPlayerRef();
            return std::string_view{ a->GetDisplayFullName() }
                 < std::string_view{ b->GetDisplayFullName() };
        });
        isVisible = true;
        SceneHUD::winThreadConfig->IsOpen = true;
    }

    void ThreadConfigPanel::Destroy()
    {
        isVisible = false;
        s_actorStates.clear();
        s_sortedActors.clear();
        if (SceneHUD::winThreadConfig) SceneHUD::winThreadConfig->IsOpen = false;
    }

    // ── Handlers ────────────────────────────────────────────────────────────

    void ThreadConfigPanel::OnRandomScene()
    {
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (!inst) return;
        const auto* cur = inst->GetActiveScene();
        const auto scenes = inst->GetThreadScenes();
        std::vector<const Registry::Scene*> pool;
        pool.reserve(scenes.size());
        for (auto* s : scenes) if (s != cur) pool.push_back(s);
        if (pool.empty()) return;
        const std::string id{ pool[rand() % pool.size()]->id };
        Script::DispatchMethodCall(SceneHUD::threadScript, "ResetScene",
            SceneHUD::callbackPtr, RE::BSFixedString{ id.c_str() });
    }

    void ThreadConfigPanel::OnMoveScene()
    {
        if (!SceneHUD::threadScript) return;
        Script::DispatchMethodCall(SceneHUD::threadScript, "ToggleFocusSceneHUD", SceneHUD::callbackPtr);
        Script::DispatchMethodCall(SceneHUD::threadScript, "MoveScene", SceneHUD::callbackPtr);
    }

    void ThreadConfigPanel::OnAutoPlaySet(bool state)
    {
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (inst) inst->SetThreadProperty<bool>("AutoAdvance", state);
    }

    void ThreadConfigPanel::OnNextPermutation(RE::Actor* actor)
    {
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (inst && actor) inst->SetNextPermutation(actor);
    }

    void ThreadConfigPanel::OnSetExpression(RE::Actor* actor, const Registry::Expression* expr)
    {
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (inst && actor && expr) inst->SetExpression(actor, expr);
    }

    void ThreadConfigPanel::OnSetVoice(RE::Actor* actor, const Registry::Voice* voice)
    {
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (inst && actor && voice) inst->SetVoice(actor, voice);
    }

    void ThreadConfigPanel::OnSetActorAlpha(RE::Actor* actor, int alphaInt)
    {
        if (actor) actor->SetAlpha(std::clamp(alphaInt, 0, 100) / 100.0f);
    }

    // ── Actor card ──────────────────────────────────────────────────────────

    void ThreadConfigPanel::RenderActorCard(RE::Actor* actor, ActorState& state)
    {
        if (!actor) return;
        auto* lib = Registry::Library::GetSingleton();

        const float panelW   = ScaleUI::pxScale(280.0f);
        const float rowMinH  = ScaleUI::pxScale(28.0f);
        const float rowPadV  = ScaleUI::pxScale(6.0f);
        const float rowPadH  = ScaleUI::pxScale(12.0f);
        const float hdrPadV  = ScaleUI::pxScale(5.0f);
        const float hdrPadH  = ScaleUI::pxScale(10.0f);
        const float badgeGap = ScaleUI::pxScale(4.0f);
        const float lblW     = ScaleUI::pxScale(70.0f);
        const float dropW    = ScaleUI::pxScale(140.0f);
        const float alphaW   = ScaleUI::pxScale(100.0f);

        ImGuiMCP::PushID(static_cast<int>(actor->GetFormID()));

        // ── Card header ─────────────────────────────────────────────────────
        // An invisible-label Selectable provides the real user interaction here.
        // The header's actual content (arrow, name, badge) is drawn on top of it afterward.
        const ImGuiMCP::ImVec2 hdrMin = ImGuiMCP::GetCursorScreenPos();
        const float hdrH = ScaleUI::pxScale(9.0f) + hdrPadV * 2.0f;

        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Header,        IM_COL32(24, 24, 30, 140));
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_HeaderHovered, IM_COL32(24, 24, 30, 140));
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_HeaderActive,  IM_COL32(24, 24, 30, 140));
        if (ImGuiMCP::Selectable("##slpp_tcmCardHdr", false, 0, ImGuiMCP::ImVec2{ panelW, hdrH }))
            state.cardOpen = !state.cardOpen;
        ImGuiMCP::PopStyleColor(3);
        const bool hdrHov = ImGuiMCP::IsItemHovered();

        // Idle background when not hovered/focused —> paint a faint background
        auto* dl = ImGuiMCP::GetWindowDrawList();
        if (!hdrHov) {
            const ImGuiMCP::ImVec2 hdrMax{ hdrMin.x + panelW, hdrMin.y + hdrH };
            ImGuiMCP::ImDrawListManager::AddRectFilled(dl, hdrMin, hdrMax, IM_COL32(10, 10, 14, 89), 0.0f, 0);
        }

        ImGuiMCP::SetCursorScreenPos(hdrMin);
        ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());

        // Collapse arrow
        ImGuiMCP::SetCursorScreenPos(ImGuiMCP::ImVec2{hdrMin.x + hdrPadH, hdrMin.y + hdrPadV});
        ImGuiMCP::TextColored(ToVec4(ColorUI::TextMuted),
            state.cardOpen ? "\xe2\x96\xbc" : "\xe2\x96\xb2");  // UTF-8 ▼ (open) / ▲ (closed)

        // Name, truncated
        ImGuiMCP::SameLine(0.0f, ScaleUI::pxScale(6.0f));
        ImGuiMCP::TextColored(ToVec4(hdrHov ? ColorUI::TextSecond : ColorUI::TextMuted),
            "%s", actor->GetDisplayFullName());

        // Badges flush-right: player / position
        const float badgeX = hdrMin.x + panelW - hdrPadH - (actor->IsPlayerRef() ? ScaleUI::pxScale(40.0f) : 0.0f);
        if (actor->IsPlayerRef()) {
            ImGuiMCP::SetCursorScreenPos(ImGuiMCP::ImVec2{badgeX, hdrMin.y + hdrPadV});
            ImGuiMCP::TextColored(ToVec4(ColorUI::BadgeGreen), "PLAYER");
        }

        ImGuiMCP::SetCursorScreenPos(ImGuiMCP::ImVec2{hdrMin.x, hdrMin.y + hdrH});

        // ── Card body (collapsible) ─────────────────────────────────────────
        if (!state.cardOpen) { ImGuiMCP::PopID(); return; }
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.5f) / ImGuiMCP::GetFontSize());

        // ── Permutation row
        {
            const uint32_t cur = inst->GetCurrentPermutation(actor);
            const uint32_t total = inst->GetUniquePermutations(actor);

            ImGuiMCP::SetCursorPosX(rowPadH);
            ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());
            ImGuiMCP::TextColored(ToVec4(ColorUI::TextMuted), "Permutation");
            ImGuiMCP::SameLine(lblW);

            const float btnW = ScaleUI::pxScale(22.0f);
            ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());
            char permBuf[24];
            std::snprintf(permBuf, sizeof(permBuf), "%u / %u", cur, total);
            const float permW = std::max(ImGuiMCP::CalcTextSize(permBuf).x, ScaleUI::pxScale(30.0f));
            ImGuiMCP::SetNextItemWidth(permW);
            ImGuiMCP::TextColored(ToVec4(ColorUI::TextSecond), "%s", permBuf);

            ImGuiMCP::SameLine(0.0f, ScaleUI::pxScale(6.0f));
            if (ImGuiMCP::Button("▶##slpp_tcmNext", ImGuiMCP::ImVec2{btnW, rowMinH})) {
                OnNextPermutation(actor);
            }
            ImGuiMCP::SetCursorPosY(ImGuiMCP::GetCursorPosY() + rowPadV);
            ImGuiMCP::ImDrawListManager::AddLine(dl,
                ImGuiMCP::ImVec2{ImGuiMCP::GetWindowPos().x, ImGuiMCP::GetCursorScreenPos().y},
                ImGuiMCP::ImVec2{ImGuiMCP::GetWindowPos().x + panelW, ImGuiMCP::GetCursorScreenPos().y},
                IM_COL32(55, 55, 58, 128), 1.0f);
        }

        // ── Expression combo
        if (Registry::RaceKey(actor).Is(Registry::RaceKey::Value::Human)) {
            const auto* curExpr = inst->GetExpression(actor);
            std::string curLabel = curExpr ? curExpr->GetId().c_str() : "(none)";
            SKSE::Translation::Translate(curLabel, curLabel);

            ImGuiMCP::SetCursorPosX(rowPadH);
            ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());
            ImGuiMCP::TextColored(ToVec4(ColorUI::TextMuted), "Expression");
            ImGuiMCP::SameLine(lblW);

            ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());
            ImGuiMCP::SetNextItemWidth(dropW);
            if (ImGuiMCP::BeginCombo("##slpp_tcmExpr", curLabel.c_str())) {
                lib->ForEachExpression([&](const auto& expr) {
                    if (!expr.enabled) return false;
                    std::string label{ expr.GetId().c_str() };
                    SKSE::Translation::Translate(label, label);
                    const bool sel = curExpr && curExpr->GetId() == expr.GetId();
                    if (sel) ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, IM_COL32(0x90, 0xb0, 0xc8, 255));
                    if (ImGuiMCP::Selectable(label.c_str(), sel)) {
                        OnSetExpression(actor, &expr);
                    }
                    if (sel) ImGuiMCP::PopStyleColor();
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
            ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());
            ImGuiMCP::TextColored(ToVec4(ColorUI::TextMuted), "Voice");
            ImGuiMCP::SameLine(lblW);

            ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());
            ImGuiMCP::SetNextItemWidth(dropW);
            if (ImGuiMCP::BeginCombo("##slpp_tcmVoice", curLabel.c_str())) {
                lib->ForEachVoice([&](const auto& v) {
                    if (!v.HasRace(raceKey)) return false;
                    std::string label{ v.GetId().c_str() };
                    SKSE::Translation::Translate(label, label);
                    const bool sel = curVoice && curVoice->GetId() == v.GetId();
                    if (sel) ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, IM_COL32(0x90, 0xb0, 0xc8, 255));
                    if (ImGuiMCP::Selectable(label.c_str(), sel)) {
                        OnSetVoice(actor, &v);
                    }
                    if (sel) ImGuiMCP::PopStyleColor();
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
            ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());
            ImGuiMCP::TextColored(ToVec4(ColorUI::TextMuted), "Alpha");
            ImGuiMCP::SameLine(lblW);

            ImGuiMCP::SetNextItemWidth(alphaW);
            ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_FramePadding, ImGuiMCP::ImVec2{0.0f, ScaleUI::pxScale(1.5f)});
            if (ImGuiMCP::SliderInt("##slpp_tcmAlpha", &alphaInt, 0, 100, ""))
                OnSetActorAlpha(actor, alphaInt); // actor's opacity updates live while dragging
            ImGuiMCP::PopStyleVar();

            ImGuiMCP::SameLine(0.0f, ScaleUI::pxScale(6.0f));
            ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());
            ImGuiMCP::TextColored(ToVec4(ColorUI::TextMuted), "%d%%", alphaInt);
        }

        ImGuiMCP::SetCursorPosY(ImGuiMCP::GetCursorPosY() + rowPadV);
        ImGuiMCP::PopID();
    }

    // ── Render ──────────────────────────────────────────────────────────────

    void __stdcall ThreadConfigPanel::Render()
    {
        if (!isVisible || !SceneHUD::IsActive()) return;
        if (!SceneHUD::IsPanelOpen(IdxTabPanel::kThreadConfig)) return;
        if (!SceneHUD::focusMode) return;

        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (!inst) return;

        auto* io = ImGuiMCP::GetIO();
        const float panelW   = ScaleUI::pxScale(280.0f);
        const float offset   = ScaleUI::pxScale(40.0f);   // from right edge
        const float rowMinH  = ScaleUI::pxScale(28.0f);
        const float rowPadV  = ScaleUI::pxScale(6.0f);
        const float rowPadH  = ScaleUI::pxScale(12.0f);
        const float maxBodyH = ScaleUI::pxScale(340.0f);  // before scrolling

        ImGuiMCP::SetNextWindowPos(
            ImGuiMCP::ImVec2{io->DisplaySize.x - offset, io->DisplaySize.y * 0.5f},
            ImGuiMCP::ImGuiCond_Always, ImGuiMCP::ImVec2{1.0f, 0.5f});
        ImGuiMCP::SetNextWindowSizeConstraints(
            ImGuiMCP::ImVec2{panelW, rowMinH},
            ImGuiMCP::ImVec2{panelW, io->DisplaySize.y * 0.8f});
        ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{panelW, 0.0f}, ImGuiMCP::ImGuiCond_Always);

        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
            ImGuiMCP::ImGuiWindowFlags_NoMove     | ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
            ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize;

        if (!ImGuiMCP::Begin("##slpp_TCM", nullptr, kFlags)) { ImGuiMCP::End(); return; }

        // ── THREAD section
        ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());
        if (ImGuiMCP::Selectable(
                s_threadSectionOpen ? "  THREAD  \xe2\x96\xbc" : "  THREAD  \xe2\x96\xb2",
                false, 0, ImGuiMCP::ImVec2{0.0f, ScaleUI::pxScale(20.0f)}))
            s_threadSectionOpen = !s_threadSectionOpen;
        ImGuiMCP::Separator();

        if (s_threadSectionOpen) {
            ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(10.0f) / ImGuiMCP::GetFontSize());
            ImGuiMCP::SetCursorPosX(rowPadH);
            if (ImGuiMCP::Selectable("Random Scene", false, 0, ImGuiMCP::ImVec2{panelW - rowPadH * 2.0f, rowMinH}))
                OnRandomScene();
            ImGuiMCP::SetCursorPosX(rowPadH);
            if (ImGuiMCP::Selectable("Move Scene", false, 0, ImGuiMCP::ImVec2{panelW - rowPadH * 2.0f, rowMinH}))
                OnMoveScene();

            ImGuiMCP::SetCursorPosX(rowPadH);
            ImGuiMCP::TextColored(ToVec4(ColorUI::TextMuted), "Auto Advance");
            ImGuiMCP::SameLine(panelW - rowPadH - ScaleUI::pxScale(20.0f));
            bool autoPlay = inst->GetThreadProperty<bool>("AutoAdvance");
            if (ImGuiMCP::Checkbox("##slpp_tcmAutoplay", &autoPlay)) {
                OnAutoPlaySet(autoPlay);
            }

            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{0.0f, rowPadV});
        }

        ImGuiMCP::Separator();

        // ── ACTORS section
        ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());
        if (ImGuiMCP::Selectable(
                s_actorsSectionOpen ? "  ACTORS  \xe2\x96\xbc" : "  ACTORS  \xe2\x96\xb2",
                false, 0, ImGuiMCP::ImVec2{0.0f, ScaleUI::pxScale(20.0f)}))
            s_actorsSectionOpen = !s_actorsSectionOpen;
        ImGuiMCP::Separator();

        if (s_actorsSectionOpen) {
            ImGuiMCP::BeginChild("##slpp_tcmActors", ImGuiMCP::ImVec2{panelW, maxBodyH}, false,
                ImGuiMCP::ImGuiWindowFlags_NoScrollbar);

            for (auto* actor : s_sortedActors) {
                const uint32_t fid = actor->GetFormID();
                ActorState* st = nullptr;
                for (auto& s : s_actorStates) if (s.formId == fid) { st = &s; break; }
                if (!st) {
                    s_actorStates.push_back({ fid, false });
                    st = &s_actorStates.back();
                }
                RenderActorCard(actor, *st);
            }

            ImGuiMCP::EndChild();
        }

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();
    }
}
