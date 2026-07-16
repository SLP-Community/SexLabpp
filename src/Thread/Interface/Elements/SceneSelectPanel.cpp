#include "SceneSelectPanel.h"
#include "Registry/Library.h"
#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interface
{
    using UI::SetWindowFontSize;

    void SceneSelectPanel::Open(SceneHUD& a_hud)
    {
        RebuildEntries(a_hud);
        _searchBuffer[0] = '\0';
        _lastSearch[0] = '\0';
        _filteredIndices.clear();
        _hoveredIndex = -1;
        _sceneListOpen = true;
        _searchBoxOpen = true;
        RebuildFilter();
    }

    void SceneSelectPanel::Close()
    {
        _entries.clear();
        _filteredIndices.clear();
        _hoveredIndex = -1;
    }

    // ── Handlers ────────────────────────────────────────────────────────────

    void SceneSelectPanel::OnSceneSelected(SceneHUD& a_hud, const std::string& a_sceneId)
    {
        Script::DispatchMethodCall(a_hud.GetThreadScript(), "ResetScene",
            a_hud.GetCallback(), RE::BSFixedString{ a_sceneId.c_str() });
        RebuildEntries(a_hud);
        RebuildFilter();
        _hoveredIndex = -1;
    }

    void SceneSelectPanel::OnConfirmSearch(SceneHUD& a_hud)
    {
        // Trims the search text, clears the field, and if anything was
        // actually typed, runs the search and closes the panel.
        std::string query{ _searchBuffer };
        const auto lo = query.find_first_not_of(' ');
        if (lo == std::string::npos) {
            _searchBuffer[0] = '\0';
            return;
        }
        query = query.substr(lo, query.find_last_not_of(' ') - lo + 1);
        _searchBuffer[0] = '\0';
        if (query.empty())
            return;

        Script::DispatchMethodCall(a_hud.GetThreadScript(), "OnSceneResetBySearch",
            a_hud.GetCallback(), RE::BSFixedString{ query.c_str() });
        a_hud.CloseAllPanels();
    }

    void SceneSelectPanel::OnAnnotationSave(SceneEntry& e)
    {
        const std::string trimmed = [&] {
            std::string s{ e.annotBuf };
            const auto lo = s.find_first_not_of(" \t\r\n");
            if (lo == std::string::npos)
                return std::string{};
            return s.substr(lo, s.find_last_not_of(" \t\r\n") - lo + 1);
        }();
        if (trimmed == e.annotations)
            return;
        e.annotations = trimmed;

        auto* lib = Registry::Library::GetSingleton();
        const auto* scene = lib->GetSceneById(RE::BSFixedString{ e.id.c_str() });
        if (!scene)
            return;
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
                        token.substr(start, end - start + 1).c_str() });
                });
            }
        }
    }

    void SceneSelectPanel::RebuildEntries(SceneHUD& a_hud)
    {
        _entries.clear();
        auto* inst = a_hud.GetThreadInstance();
        if (!inst)
            return;
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
                if (!first)
                    e.tags += ", ";
                e.tags += t.c_str();
                first = false;
            }
            first = true;
            for (const auto& a : sc->tags.GetAnnotations()) {
                if (!first)
                    e.annotations += ", ";
                e.annotations += a.c_str();
                first = false;
            }
            std::snprintf(e.annotBuf, sizeof(e.annotBuf), "%s", e.annotations.c_str());
            _entries.push_back(std::move(e));
        }

        std::ranges::sort(_entries, [](const SceneEntry& a, const SceneEntry& b) {
            if (a.isActive != b.isActive)
                return a.isActive;
            return a.name < b.name;
        });
    }

    bool SceneSelectPanel::MatchesFilter(const SceneEntry& a_entry, std::string_view a_filter)
    {
        if (a_filter.empty())
            return true;
        auto containsCaseInsensitive = [&](std::string_view a_text) {
            return std::ranges::search(a_text, a_filter, {}, [](char a_character) { return static_cast<char>(std::tolower(static_cast<unsigned char>(a_character))); }, [](char a_character) { return static_cast<char>(std::tolower(static_cast<unsigned char>(a_character))); }).begin() != a_text.end();
        };
        return containsCaseInsensitive(a_entry.name) || containsCaseInsensitive(a_entry.tags) || containsCaseInsensitive(a_entry.author);
    }

    void SceneSelectPanel::RebuildFilter()
    {
        std::memcpy(_lastSearch, _searchBuffer, sizeof(_searchBuffer));
        _filteredIndices.clear();
        const std::string_view filter{ _searchBuffer };
        for (int i = 0; i < static_cast<int>(_entries.size()); ++i)
            if (MatchesFilter(_entries[i], filter))
                _filteredIndices.push_back(i);
    }

    // ── Render ────────────────────────────────────────────────────────────────

    void SceneSelectPanel::Render(SceneHUD& a_hud)
    {
        auto& scale = a_hud.GetScale();

        auto* io = ImGuiMCP::GetIO();
        const float dw = io->DisplaySize.x;
        const float dh = io->DisplaySize.y;

        const float panelW = scale.Px(270.0f);
        const float offset = scale.Px(UI::Theme::Geometry::panelTabWidth + UI::Theme::Geometry::panelTabGap);
        const float maxH = dh * 0.8f;
        const float listMaxH = scale.Px(200.0f);
        const float rowH = scale.TextPx(UI::Theme::FontSize::body) + scale.Px(UI::Theme::Spacing::sm) * 2.0f;
        const float sectionH = std::max(scale.Px(20.0f),
            scale.TextPx(UI::Theme::FontSize::sectionHeader) + scale.Px(UI::Theme::Spacing::xs));

        ImGuiMCP::SetNextWindowPos(
            ImGuiMCP::ImVec2{ dw - offset, dh * 0.5f }, ImGuiMCP::ImGuiCond_Always, ImGuiMCP::ImVec2{ 1.0f, 0.5f });
        ImGuiMCP::SetNextWindowSizeConstraints(ImGuiMCP::ImVec2{ panelW, rowH }, ImGuiMCP::ImVec2{ panelW, maxH });
        ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{ panelW, 0.0f }, ImGuiMCP::ImGuiCond_Always);

        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
            ImGuiMCP::ImGuiWindowFlags_NoMove | ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
            ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize;

        if (!ImGuiMCP::Begin("##slpp_SSM", nullptr, kFlags)) {
            ImGuiMCP::End();
            return;
        }

        const ImGuiMCP::ImVec2 winPos = ImGuiMCP::GetWindowPos();

        // ── Section: Scenes List
        SetWindowFontSize(scale.TextPx(UI::Theme::FontSize::sectionHeader));
        if (UI::CollapsibleSectionHeader("CHANGE ACTIVE SCENE", "##slpp_ssmSceneListSection", _sceneListOpen,
                { 0.0f, sectionH }))
            _sceneListOpen = !_sceneListOpen;
        ImGuiMCP::Separator();

        int hoveredRowIndex = -1;

        if (_sceneListOpen) {
            SetWindowFontSize(scale.TextPx(UI::Theme::FontSize::body));
            ImGuiMCP::BeginChild("##slpp_smmSceneList", ImGuiMCP::ImVec2{ panelW, listMaxH }, false);

            if (std::memcmp(_searchBuffer, _lastSearch, sizeof(_searchBuffer)) != 0)
                RebuildFilter();  // rebuild filtered index only when search text changed since last frame

            std::optional<std::string> selectedScene;
            for (int i : _filteredIndices) {
                auto& e = _entries[i];
                ImGuiMCP::PushID(i);

                if (e.isActive)
                    ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::Color::accent);
                const bool clicked = UI::SelectableButton(e.name.c_str(), e.isActive,
                    ImGuiMCP::ImGuiSelectableFlags_AllowOverlap, ImGuiMCP::ImVec2{ 0.0f, rowH });
                if (e.isActive)
                    ImGuiMCP::PopStyleColor();

                if (ImGuiMCP::IsItemHovered())
                    hoveredRowIndex = i;
                if (clicked && !e.isActive)
                    selectedScene = e.id;
                ImGuiMCP::PopID();
            }

            ImGuiMCP::EndChild();
            if (selectedScene)
                OnSceneSelected(a_hud, *selectedScene);
        }

        ImGuiMCP::Separator();

        // ── Section: Search Scenes
        SetWindowFontSize(scale.TextPx(UI::Theme::FontSize::sectionHeader));
        if (UI::CollapsibleSectionHeader("CHANGE SCENES BY TAG / NAME", "##slpp_ssmSearchSection", _searchBoxOpen,
                { 0.0f, sectionH }))
            _searchBoxOpen = !_searchBoxOpen;
        ImGuiMCP::Separator();

        if (_searchBoxOpen) {
            SetWindowFontSize(scale.TextPx(UI::Theme::FontSize::body));
            ImGuiMCP::SetNextItemWidth(panelW - scale.Px(20.0f));
            ImGuiMCP::InputTextWithHint("##slpp_smmSearch", "Tag or scene name...",
                _searchBuffer, sizeof(_searchBuffer));

            ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, scale.Px(4.0f) });
            const float btnW = (panelW - scale.Px(20.0f) - scale.Px(6.0f)) * 0.5f;
            // Cancel closes the whole panel rather than just clearing the text box.
            if (UI::ActionButton("Cancel##slpp_smmCancel", btnW)) {
                _searchBuffer[0] = '\0';
                a_hud.CloseAllPanels();
            }
            ImGuiMCP::SameLine(0.0f, scale.Px(6.0f));
            if (UI::ActionButton("Search##slpp_smmConfirm", btnW))
                OnConfirmSearch(a_hud);
        }

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();

        // ── Info card, retained while hovering it or editing annotations
        if (hoveredRowIndex >= 0) {
            _hoveredIndex = hoveredRowIndex;
            _infoCardY = ImGuiMCP::GetMousePos().y - scale.Px(40.0f);
        }
        bool keepInfoCardOpen = hoveredRowIndex >= 0;
        if (_hoveredIndex >= 0 && _hoveredIndex < static_cast<int>(_entries.size())) {
            auto& e = _entries[_hoveredIndex];

            const float cardW = scale.Px(190.0f);
            const float keyW = scale.Px(46.0f);
            const float rowGap = scale.Px(4.0f);
            const float keyFont = scale.TextPx(UI::Theme::FontSize::smallText);
            const float rowFont = scale.TextPx(UI::Theme::FontSize::compact);
            const float tagFont = scale.TextPx(UI::Theme::FontSize::metadata);

            const float cardX = winPos.x - cardW - 6.0f;

            ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{ cardX, _infoCardY }, ImGuiMCP::ImGuiCond_Always, ImGuiMCP::ImVec2{ 0.0f, 0.0f });
            ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{ cardW, 0.0f }, ImGuiMCP::ImGuiCond_Always);
            ImGuiMCP::SetNextWindowBgAlpha(0.97f);

            constexpr auto cardFlags =
                ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
                ImGuiMCP::ImGuiWindowFlags_NoMove | ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
                ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiMCP::ImGuiWindowFlags_NoFocusOnAppearing | ImGuiMCP::ImGuiWindowFlags_NoNav;

            if (ImGuiMCP::Begin("##slpp_smmInfoCard", nullptr, cardFlags)) {
                constexpr auto cardHoverFlags =
                    ImGuiMCP::ImGuiHoveredFlags_ChildWindows |
                    ImGuiMCP::ImGuiHoveredFlags_AllowWhenBlockedByActiveItem;
                const auto cardPos = ImGuiMCP::GetWindowPos();
                const auto cardSize = ImGuiMCP::GetWindowSize();
                const auto mousePos = ImGuiMCP::GetMousePos();
                const bool isHoveringBridge =
                    mousePos.x >= cardPos.x + cardSize.x &&
                    mousePos.x <= winPos.x + ImGuiMCP::GetStyle()->WindowPadding.x &&
                    mousePos.y >= cardPos.y && mousePos.y <= cardPos.y + cardSize.y;
                keepInfoCardOpen |= ImGuiMCP::IsWindowHovered(cardHoverFlags) || isHoveringBridge;
                SetWindowFontSize(keyFont);

                auto infoRow = [&](const char* key, const std::string& val, float valFont) {
                    ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textMuted), "%s", key);
                    ImGuiMCP::SameLine(keyW);
                    SetWindowFontSize(valFont);
                    ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textSecondary), "%s",
                        val.empty() ? "\xE2\x80\x94" : val.c_str());
                    SetWindowFontSize(keyFont);
                    ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, rowGap });
                };

                infoRow("PACK", e.packageName, rowFont);
                infoRow("AUTHOR", e.author, rowFont);

                ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textMuted), "TAGS");
                ImGuiMCP::SameLine(keyW);
                SetWindowFontSize(tagFont);
                ImGuiMCP::TextWrapped("%s", e.tags.empty() ? "\xE2\x80\x94" : e.tags.c_str());
                SetWindowFontSize(keyFont);
                ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, rowGap });

                ImGuiMCP::Separator();
                ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, scale.Px(5.0f) });

                ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textMuted), "ANNOTATIONS");
                SetWindowFontSize(rowFont);
                const float fieldW = cardW - scale.Px(24.0f);
                const ImGuiMCP::ImVec2 annotSz{ fieldW,
                    std::clamp(ImGuiMCP::CalcTextSize(e.annotBuf, nullptr, false, fieldW).y + scale.Px(10.0f),
                        scale.Px(26.0f), scale.Px(60.0f)) };
                if (ImGuiMCP::InputTextMultiline("##slpp_annot", e.annotBuf, sizeof(e.annotBuf), annotSz,
                        ImGuiMCP::ImGuiInputTextFlags_EnterReturnsTrue)) {
                    OnAnnotationSave(e);  // Save on Enter
                } else if (ImGuiMCP::IsItemDeactivatedAfterEdit()) {
                    OnAnnotationSave(e);  // Save on focus loss
                }
                keepInfoCardOpen |= ImGuiMCP::IsItemActive();
            }
            ImGuiMCP::End();
        }
        if (!keepInfoCardOpen)
            _hoveredIndex = -1;
    }
}
