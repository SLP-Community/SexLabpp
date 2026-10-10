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
        _filteredIndices.clear();
        _hoveredIndex = -1;
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
        if (!scene) {
            logger::error("Scene not found: {}", e.id);
            return;
        }
        for (const auto& a : scene->GetTags().GetAnnotations()) {
            lib->EditScene(RE::BSFixedString{ e.id.c_str() }, [&](Registry::Animation::Scene* s) {
                s->GetTags().RemoveAnnotation(a);
            });
        }
        std::istringstream ss(trimmed);
        std::string token;
        while (std::getline(ss, token, ',')) {
            const auto start = token.find_first_not_of(' ');
            const auto end = token.find_last_not_of(' ');
            if (start != std::string::npos) {
                lib->EditScene(RE::BSFixedString{ e.id.c_str() }, [&](Registry::Animation::Scene* s) {
                    s->GetTags().AddAnnotation(RE::BSFixedString{
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
            e.id = sc->GetId();
            e.name = sc->GetName();
            e.isActive = (sc == active);
            if (const auto* pkg = lib->GetPackageFromScene(sc)) {
                e.packageName = pkg->GetName().c_str();
                e.author = pkg->GetAuthor().c_str();
            }
            bool first = true;
            for (const auto& t : sc->GetTags().AsVector()) {
                if (!first)
                    e.tags += ", ";
                e.tags += t.c_str();
                first = false;
            }
            first = true;
            for (const auto& a : sc->GetTags().GetAnnotations()) {
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

        const float panelW = scale.Px(240.0f);
        const float offset = scale.Px(UI::Theme::Geometry.panelTabWidth + UI::Theme::Geometry.panelTabGap);
        const float maxH = dh * 0.8f;
        const float rowH = scale.TextPx(UI::Theme::FontSize.body) + scale.Px(UI::Theme::Spacing.xs);
        const float panelPadding = scale.Px(10.0f);

        // Calculate dynamic list height based on number of entries
        const float maxListH = scale.Px(280.0f);
        const float calculatedListH = std::min(std::max(static_cast<float>(_filteredIndices.size()), 1.0f) * rowH, maxListH);

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

        // ── Scene List with padding
        ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, scale.Px(4.0f) });
        ImGuiMCP::SetCursorPosX(panelPadding);

        SetWindowFontSize(scale.TextPx(UI::Theme::FontSize.compact));

        // Modern scrollbar styling
        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_ScrollbarSize, scale.Px(6.0f));
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ScrollbarBg, UI::Theme::Color.panelBackground);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ScrollbarGrab, UI::Theme::ToVec4(UI::Theme::Color.borderSubtle));
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ScrollbarGrabHovered, UI::Theme::ToVec4(UI::Theme::Color.borderHovered));
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_ScrollbarGrabActive, UI::Theme::ToVec4(UI::Theme::Color.borderActive));

        ImGuiMCP::BeginChild("##slpp_smmSceneList", ImGuiMCP::ImVec2{ panelW - panelPadding * 2.0f, calculatedListH },
            ImGuiMCP::ImGuiChildFlags_None, ImGuiMCP::ImGuiWindowFlags_None);

        int hoveredRowIndex = -1;
        std::optional<std::string> selectedScene;
        for (int i : _filteredIndices) {
            auto& e = _entries[i];
            ImGuiMCP::PushID(i);

            if (e.isActive)
                ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::Color.accent);
            else
                ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::ToVec4(UI::Theme::Color.textSecondary));

            ImGuiMCP::SetCursorPosX(ImGuiMCP::GetCursorPosX() + scale.Px(8.0f));
            const bool clicked = UI::SelectableButton(e.name.c_str(), e.isActive,
                ImGuiMCP::ImGuiSelectableFlags_AllowOverlap, ImGuiMCP::ImVec2{ 0.0f, rowH });
            ImGuiMCP::PopStyleColor();

            if (ImGuiMCP::IsItemHovered())
                hoveredRowIndex = i;
            if (clicked && !e.isActive)
                selectedScene = e.id;
            ImGuiMCP::PopID();
        }

        ImGuiMCP::EndChild();

        // Restore scrollbar styles
        ImGuiMCP::PopStyleColor(4);
        ImGuiMCP::PopStyleVar();

        if (selectedScene)
            OnSceneSelected(a_hud, *selectedScene);

        ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, scale.Px(8.0f) });

        // ── Search Section with padding
        SetWindowFontSize(scale.TextPx(UI::Theme::FontSize.body));
        ImGuiMCP::SetCursorPosX(panelPadding);

        const float searchAreaW = panelW - panelPadding * 2.0f;

        // Style search input
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::ToVec4(UI::Theme::Color.textSecondary));
        SetWindowFontSize(scale.TextPx(UI::Theme::FontSize.caption));

        ImGuiMCP::SetNextItemWidth(searchAreaW);
        if (ImGuiMCP::InputTextWithHint("##slpp_smmSearch", "Tag or scene name...",
                _searchBuffer, sizeof(_searchBuffer))) {
            RebuildFilter();
            _hoveredIndex = -1;
        }

        const bool searchInputHasFocus = ImGuiMCP::IsItemFocused();

        ImGuiMCP::PopStyleColor();
        SetWindowFontSize(scale.TextPx(UI::Theme::FontSize.body));

        if (searchInputHasFocus && _searchBuffer[0] != '\0' &&
            ImGuiMCP::IsKeyPressed(ImGuiMCP::ImGuiKey_Escape, false)) {
            _searchBuffer[0] = '\0';
            RebuildFilter();
            _hoveredIndex = -1;
        }

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, scale.Px(4.0f) });
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
            const float keyFont = scale.TextPx(UI::Theme::FontSize.smallText);
            const float rowFont = scale.TextPx(UI::Theme::FontSize.compact);
            const float tagFont = scale.TextPx(UI::Theme::FontSize.metadata);

            const float cardX = winPos.x - cardW - 6.0f;

            ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{ cardX, _infoCardY }, ImGuiMCP::ImGuiCond_Always, ImGuiMCP::ImVec2{ 0.0f, 0.0f });
            ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{ cardW, 0.0f }, ImGuiMCP::ImGuiCond_Always);
            ImGuiMCP::SetNextWindowBgAlpha(0.4f);

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
                    ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color.textSecondary), "%s", key);
                    ImGuiMCP::SameLine(keyW);
                    SetWindowFontSize(valFont);
                    ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color.textMuted), "%s",
                        val.empty() ? "\xE2\x80\x94" : val.c_str());
                    SetWindowFontSize(keyFont);
                    ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, rowGap });
                };

                infoRow("PACK:", e.packageName, rowFont);
                infoRow("AUTHOR:", e.author, rowFont);

                ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color.textSecondary), "TAGS:");
                ImGuiMCP::SameLine(keyW);
                SetWindowFontSize(tagFont);
                ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_Text, UI::Theme::ToVec4(UI::Theme::Color.textMuted));
                ImGuiMCP::TextWrapped("%s", e.tags.empty() ? "\xE2\x80\x94" : e.tags.c_str());
                ImGuiMCP::PopStyleColor();
                SetWindowFontSize(keyFont);
                ImGuiMCP::Dummy(ImGuiMCP::ImVec2{ 0.0f, rowGap });

                ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color.textSecondary), "ANNOTATIONS");
                SetWindowFontSize(rowFont);
                const float fieldW = cardW - scale.Px(12.0f);
                const ImGuiMCP::ImVec2 annotSz{ fieldW,
                    std::clamp(ImGuiMCP::CalcTextSize(e.annotBuf, nullptr, false, fieldW).y + scale.Px(10.0f),
                        scale.Px(26.0f), scale.Px(60.0f)) };

                const float annotCenterX = (cardW - fieldW) * 0.5f;
                ImGuiMCP::SetCursorPosX(annotCenterX);
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
        bool isHoveringList = ImGuiMCP::IsWindowHovered(ImGuiMCP::ImGuiHoveredFlags_RootAndChildWindows);
        if (!keepInfoCardOpen && !isHoveringList) {
            _hoveredIndex = -1;
        }
    }
}
