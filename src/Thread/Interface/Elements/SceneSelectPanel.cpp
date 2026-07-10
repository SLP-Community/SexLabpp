#include "SceneSelectPanel.h"
#include "Registry/Library.h"

namespace Thread::Interface
{
    void SceneSelectPanel::Init()
    {
        RebuildEntries();
        s_searchBuf[0] = '\0';
        s_lastSearch[0] = '\0';
        s_filteredIdx.clear();
        s_hoveredIdx = -1;
        s_sceneListOpen = true;
        s_searchBoxOpen = true;
        RebuildFilter();
        isVisible = true;
        SceneHUD::winSceneSelect->IsOpen = true;
    }

    void SceneSelectPanel::Destroy()
    {
        isVisible = false;
        s_entries.clear();
        s_filteredIdx.clear();
        s_hoveredIdx = -1;
        if (SceneHUD::winSceneSelect) SceneHUD::winSceneSelect->IsOpen = false;
    }

    // ── Handlers ────────────────────────────────────────────────────────────

    void SceneSelectPanel::OnSceneSelected(const std::string& sceneId)
    {
        Script::DispatchMethodCall(SceneHUD::threadScript, "ResetScene",
            SceneHUD::callbackPtr, RE::BSFixedString{ sceneId.c_str() });
        RebuildEntries();
        RebuildFilter();
    }

    void SceneSelectPanel::OnConfirmSearch()
    {
        // Trims the search text, clears the field, and if anything was
        // actually typed, runs the search and closes the panel.
        std::string query{ s_searchBuf };
        const auto lo = query.find_first_not_of(' ');
        if (lo == std::string::npos) { s_searchBuf[0] = '\0'; return; }
        query = query.substr(lo, query.find_last_not_of(' ') - lo + 1);
        s_searchBuf[0] = '\0';
        if (query.empty()) return;

        Script::DispatchMethodCall(SceneHUD::threadScript, "OnSceneResetBySearch",
            SceneHUD::callbackPtr, RE::BSFixedString{ query.c_str() });
        SceneHUD::CloseAllPanels();
    }

    void SceneSelectPanel::OnAnnotationSave(SceneEntry& e)
    {
        const std::string trimmed = [&]{
            std::string s{ e.annotBuf };
            const auto lo = s.find_first_not_of(" \t\r\n");
            if (lo == std::string::npos) return std::string{};
            return s.substr(lo, s.find_last_not_of(" \t\r\n") - lo + 1);
        }();
        if (trimmed == e.annotations) return;
        e.annotations = trimmed;

        auto* lib = Registry::Library::GetSingleton();
        const auto* scene = lib->GetSceneById(RE::BSFixedString{ e.id.c_str() });
        if (!scene) return;
        for (const auto& a : scene->tags.GetAnnotations()) {
            lib->EditScene(RE::BSFixedString{ e.id.c_str() }, [&](Registry::Scene* s) {
                s->tags.RemoveAnnotation(a);
            });
        }
        std::istringstream ss(trimmed);
        std::string token;
        while (std::getline(ss, token, ',')) {
            const auto start = token.find_first_not_of(' ');
            const auto end = token.find_last_not_of(' ');
            if (start != std::string::npos) {
                lib->EditScene(RE::BSFixedString{ e.id.c_str() }, [&](Registry::Scene* s) {
                    s->tags.AddAnnotation(RE::BSFixedString{ 
                        token.substr(start, end - start + 1).c_str() 
                    });
                });
            }
        }
    }

    void SceneSelectPanel::RebuildEntries()
    {
        s_entries.clear();
        auto* inst = Instance::GetInstance(SceneHUD::linkedThread);
        if (!inst) return;
        const auto* active = inst->GetActiveScene();
        auto* lib = Registry::Library::GetSingleton();

        for (const auto* sc : inst->GetThreadScenes()) {
            SceneEntry e;
            e.id = sc->id;
            e.name = sc->name;
            e.isActive = (sc == active);
            if (const auto* pkg = lib->GetPackageFromScene(sc)) {
                e.packageName = pkg->GetName().c_str();
                e.author = pkg->GetAuthor().c_str();
            }
            bool first = true;
            for (const auto& t : sc->tags.AsVector()) {
                if (!first) e.tags += ", ";
                e.tags += t.c_str();
                first = false;
            }
            first = true;
            for (const auto& a : sc->tags.GetAnnotations()) {
                if (!first) e.annotations += ", ";
                e.annotations += a.c_str();
                first = false;
            }
            std::snprintf(e.annotBuf, sizeof(e.annotBuf), "%s", e.annotations.c_str());
            s_entries.push_back(std::move(e));
        }

        std::sort(s_entries.begin(), s_entries.end(), [](const SceneEntry& a, const SceneEntry& b){
            if (a.isActive != b.isActive) return a.isActive;
            return a.name < b.name;
        });
    }

    bool SceneSelectPanel::MatchesFilter(const SceneEntry& e, const std::string& filter)
    {
        if (filter.empty()) return true;
        auto icontains = [&](const std::string& hay) {
            return std::search(hay.begin(), hay.end(), filter.begin(), filter.end(),
                [](char a, char b){ return std::tolower(a) == std::tolower(b); }) != hay.end();
        };
        return icontains(e.name) || icontains(e.tags) || icontains(e.author);
    }

    void SceneSelectPanel::RebuildFilter()
    {
        std::memcpy(s_lastSearch, s_searchBuf, sizeof(s_searchBuf));
        s_filteredIdx.clear();
        const std::string filter{ s_searchBuf };
        for (int i = 0; i < static_cast<int>(s_entries.size()); ++i)
            if (MatchesFilter(s_entries[i], filter))
                s_filteredIdx.push_back(i);
    }

    // ── Render ────────────────────────────────────────────────────────────────

    void __stdcall SceneSelectPanel::Render()
    {
        if (!isVisible || !SceneHUD::IsActive()) return;
        if (!SceneHUD::IsPanelOpen(IdxTabPanel::kSceneSelect)) return;
        if (!SceneHUD::focusMode) return;

        auto* io = ImGuiMCP::GetIO();
        const float dw = io->DisplaySize.x;
        const float dh = io->DisplaySize.y;

        const float panelW   = ScaleUI::pxScale(270.0f);
        const float offset   = ScaleUI::pxScale(40.0f);
        const float maxH     = dh * 0.8f;
        const float listMaxH = ScaleUI::pxScale(200.0f);
        const float rowH     = ScaleUI::pxScale(10.0f) + ScaleUI::pxScale(6.0f) * 2.0f;

        ImGuiMCP::SetNextWindowPos(
            ImGuiMCP::ImVec2{dw - offset, dh * 0.5f}, ImGuiMCP::ImGuiCond_Always, ImGuiMCP::ImVec2{1.0f, 0.5f});
        ImGuiMCP::SetNextWindowSizeConstraints(ImGuiMCP::ImVec2{panelW, rowH}, ImGuiMCP::ImVec2{panelW, maxH});
        ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{panelW, 0.0f}, ImGuiMCP::ImGuiCond_Always);

        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
            ImGuiMCP::ImGuiWindowFlags_NoMove     | ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
            ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize;

        if (!ImGuiMCP::Begin("##slpp_SSM", nullptr, kFlags)) { ImGuiMCP::End(); return; }

        const ImGuiMCP::ImVec2 winPos = ImGuiMCP::GetWindowPos();

        // ── Section: Scenes List
        ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());
        if (ImGuiMCP::Selectable(
                s_sceneListOpen ? "  CHANGE ACTIVE SCENE  \xe2\x96\xbc" : "  CHANGE ACTIVE SCENE  \xe2\x96\xb2",
                false, 0, ImGuiMCP::ImVec2{0.0f, ScaleUI::pxScale(20.0f)}))
            s_sceneListOpen = !s_sceneListOpen;
        ImGuiMCP::Separator();

        s_hoveredIdx = -1;

        if (s_sceneListOpen) {
            ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(10.0f) / ImGuiMCP::GetFontSize());
            ImGuiMCP::BeginChild("##slpp_smmSceneList", ImGuiMCP::ImVec2{panelW, listMaxH}, false);

            if (std::memcmp(s_searchBuf, s_lastSearch, sizeof(s_searchBuf)) != 0)
                RebuildFilter(); // rebuild filtered index only when search text changed since last frame

            for (int i : s_filteredIdx) {
                auto& e = s_entries[i];
                ImGuiMCP::PushID(i);

                if (e.isActive) ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, IM_COL32(0x70, 0xb8, 0x70, 255));
                const bool clicked = ImGuiMCP::Selectable(e.name.c_str(), e.isActive,
                    ImGuiMCP::ImGuiSelectableFlags_AllowOverlap, ImGuiMCP::ImVec2{0.0f, rowH});
                if (e.isActive) ImGuiMCP::PopStyleColor();

                if (ImGuiMCP::IsItemHovered()) s_hoveredIdx = i;
                if (clicked && !e.isActive) {
                    OnSceneSelected(e.id);
                }
                ImGuiMCP::PopID();
            }

            ImGuiMCP::EndChild();
        }

        ImGuiMCP::Separator();

        // ── Section: Search Scenes
        ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(9.0f) / ImGuiMCP::GetFontSize());
        if (ImGuiMCP::Selectable(
                s_searchBoxOpen ? "  CHANGE SCENES BY TAG / NAME  \xe2\x96\xbc" : "  CHANGE SCENES BY TAG / NAME  \xe2\x96\xb2",
                false, 0, ImGuiMCP::ImVec2{0.0f, ScaleUI::pxScale(20.0f)}))
            s_searchBoxOpen = !s_searchBoxOpen;
        ImGuiMCP::Separator();

        if (s_searchBoxOpen) {
            ImGuiMCP::SetWindowFontScale(ScaleUI::pxScale(10.0f) / ImGuiMCP::GetFontSize());
            ImGuiMCP::SetNextItemWidth(panelW - ScaleUI::pxScale(20.0f));
            ImGuiMCP::InputTextWithHint("##slpp_smmSearch", "Tag or scene name...",
                s_searchBuf, sizeof(s_searchBuf));

            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{0.0f, ScaleUI::pxScale(4.0f)});
            const float btnW = (panelW - ScaleUI::pxScale(20.0f) - ScaleUI::pxScale(6.0f)) * 0.5f;
            // Cancel closes the whole panel rather than just clearing the text box.
            if (ImGuiMCP::Button("Cancel##slpp_smmCancel", ImGuiMCP::ImVec2{btnW, 0.0f})) {
                s_searchBuf[0] = '\0';
                SceneHUD::CloseAllPanels();
            }
            ImGuiMCP::SameLine(0.0f, ScaleUI::pxScale(6.0f));
            if (ImGuiMCP::Button("Search##slpp_smmConfirm", ImGuiMCP::ImVec2{btnW, 0.0f}))
                OnConfirmSearch();
        }

        const ImGuiMCP::ImVec2 winSize = ImGuiMCP::GetWindowSize();
        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();

        // ── Info card, anchored left of the panel while hovering a row
        if (s_hoveredIdx >= 0 && s_hoveredIdx < static_cast<int>(s_entries.size())) {
            auto& e = s_entries[s_hoveredIdx];

            const float cardW   = ScaleUI::pxScale(190.0f);
            const float keyW    = ScaleUI::pxScale(46.0f);
            const float rowGap  = ScaleUI::pxScale(4.0f);
            const float keyFont = ScaleUI::pxScale(8.0f);
            const float rowFont = ScaleUI::pxScale(9.5f);
            const float tagFont = ScaleUI::pxScale(8.5f);

            const float cardX = winPos.x - cardW - 6.0f;
            const float cardY = ImGuiMCP::GetMousePos().y - ScaleUI::pxScale(40.0f);

            ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{cardX, cardY}, ImGuiMCP::ImGuiCond_Always, ImGuiMCP::ImVec2{0.0f, 0.0f});
            ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{cardW, 0.0f}, ImGuiMCP::ImGuiCond_Always);
            ImGuiMCP::SetNextWindowBgAlpha(0.97f);

            constexpr auto cardFlags =
                ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
                ImGuiMCP::ImGuiWindowFlags_NoMove     | ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
                ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiMCP::ImGuiWindowFlags_NoFocusOnAppearing | ImGuiMCP::ImGuiWindowFlags_NoNav;

            if (ImGuiMCP::Begin("##slpp_smmInfoCard", nullptr, cardFlags)) {
                ImGuiMCP::SetWindowFontScale(keyFont / ImGuiMCP::GetFontSize());

                auto infoRow = [&](const char* key, const std::string& val, float valFont){
                    ImGuiMCP::TextColored(ToVec4(ColorUI::TextMuted), "%s", key);
                    ImGuiMCP::SameLine(keyW);
                    ImGuiMCP::SetWindowFontScale(valFont / ImGuiMCP::GetFontSize());
                    ImGuiMCP::TextColored(ToVec4(ColorUI::TextSecond), "%s",
                        val.empty() ? "\xE2\x80\x94" : val.c_str());
                    ImGuiMCP::SetWindowFontScale(keyFont / ImGuiMCP::GetFontSize());
                    ImGuiMCP::Dummy(ImGuiMCP::ImVec2{0.0f, rowGap});
                };

                infoRow("PACK", e.packageName, rowFont);
                infoRow("AUTHOR", e.author, rowFont);

                ImGuiMCP::TextColored(ToVec4(ColorUI::TextMuted), "TAGS");
                ImGuiMCP::SameLine(keyW);
                ImGuiMCP::SetWindowFontScale(tagFont / ImGuiMCP::GetFontSize());
                ImGuiMCP::TextWrapped("%s", e.tags.empty() ? "\xE2\x80\x94" : e.tags.c_str());
                ImGuiMCP::SetWindowFontScale(keyFont / ImGuiMCP::GetFontSize());
                ImGuiMCP::Dummy(ImGuiMCP::ImVec2{0.0f, rowGap});

                ImGuiMCP::Separator();
                ImGuiMCP::Dummy(ImGuiMCP::ImVec2{0.0f, ScaleUI::pxScale(5.0f)});

                ImGuiMCP::TextColored(ToVec4(ColorUI::TextMuted), "ANNOTATIONS");
                ImGuiMCP::SetWindowFontScale(rowFont / ImGuiMCP::GetFontSize());
                const float fieldW = cardW - ScaleUI::pxScale(24.0f);
                const ImGuiMCP::ImVec2 annotSz{ fieldW,
                    std::clamp(ImGuiMCP::CalcTextSize(e.annotBuf, nullptr, false, fieldW).y + ScaleUI::pxScale(10.0f),
                        ScaleUI::pxScale(26.0f), ScaleUI::pxScale(60.0f)) };
                if (ImGuiMCP::InputTextMultiline("##slpp_annot", e.annotBuf, sizeof(e.annotBuf), annotSz,
                        ImGuiMCP::ImGuiInputTextFlags_EnterReturnsTrue)) {
                    OnAnnotationSave(e);  // Save on Enter
                } else if (ImGuiMCP::IsItemDeactivatedAfterEdit()) {
                    OnAnnotationSave(e);  // Save on focus loss
                }
            }
            ImGuiMCP::End();
        }
    }
}
