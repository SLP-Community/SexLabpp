#include "StageSelectMenu.h"
#include "SceneHUD.h"

namespace Thread::Interface
{
    using UI::DrawTextShadowed;
    using UI::SetWindowFontSize;

    namespace
    {
        constexpr float kGraphColumnSpacing = 88.0f;
        constexpr float kGraphRowSpacing = 36.0f;
        constexpr float kGraphNodeRadius = 6.0f;

        std::string ResolveNavTextPlaceholders(std::string_view a_text, const std::vector<RE::Actor*>& a_actors)
        {
            std::string out;
            out.reserve(a_text.size());
            size_t i = 0;
            while (i < a_text.size()) {
                if (a_text[i] == '{') {
                    const size_t close = a_text.find('}', i + 1);
                    if (close != std::string_view::npos) {
                        const std::string_view digits = a_text.substr(i + 1, close - i - 1);
                        const bool isIndex = !digits.empty() &&
                                             std::ranges::all_of(digits, [](char c) { return std::isdigit(static_cast<unsigned char>(c)); });
                        if (isIndex) {
                            const size_t idx = static_cast<size_t>(std::stoul(std::string{ digits }));
                            const RE::Actor* actor = idx < a_actors.size() ? a_actors[idx] : nullptr;
                            out += actor ? actor->GetName() : "{}";
                            i = close + 1;
                            continue;
                        }
                    }
                }
                out += a_text[i++];
            }
            return out;
        }
    }

    StageSelectMenu& StageSelectMenu::GetSingleton()
    {
        static StageSelectMenu singleton;
        return singleton;
    }

    bool StageSelectMenu::Register()
    {
        if (!RegisterWindow(RenderCallback, false)) {
            logger::error("StageSelectMenu::Register >> AddWindow failed");
            return false;
        }
        return true;
    }

    // ── Native entry points

    bool StageSelectMenu::OpenStageSelectMenu(RE::TESQuest* a_quest)
    {
        if (!a_quest)
            return false;
        auto* inst = Instance::GetInstance(a_quest);
        if (!inst)
            return false;

        const auto* scene = inst->GetActiveScene();
        const auto* stage = inst->GetActiveStage();
        if (!scene || !stage)
            return false;

        const auto adjacent = stage->GetOutgoingEdges();
        if (adjacent.size() < 2) {
            return false;
        }

        _choices.clear();
        _choices.reserve(adjacent.size());
        int choiceIdx = 1;
        for (const auto* next : adjacent) {
            std::string navText = !next->GetNavigationText().empty() ?
                                      ResolveNavTextPlaceholders(next->GetNavigationText(), inst->GetActors()) :
                                      std::string{ next->GetId() };
            std::string prefix = std::to_string(choiceIdx++) + ".  ";
            _choices.push_back({ next, std::move(prefix), std::move(navText) });
        }
        _choiceScrollOffsets.assign(_choices.size(), 0.0f);
        _choiceOrigin = stage;
        _choicePending = true;
        _linkedThread = a_quest;
        _choiceStartTime = ImGuiMCP::GetTime();
        _selectedChoiceIndex = 0;
        SetBlocksInput(true);
        Show();
        return true;
    }

    void StageSelectMenu::SetVisibilitySceneGraph(RE::TESQuest* a_quest, bool a_open)
    {
        if (!a_quest || _graphOpen == a_open)
            return;

        if (a_open) {
            _graphOpen = true;
            _linkedThread = a_quest;
            if (auto* inst = Instance::GetInstance(a_quest))
                BuildSceneGraph(*inst);
            _graphFitPending = true;
            SetBlocksInput(true);
            Show();
        } else {
            CloseGraphView();
        }
    }

    void StageSelectMenu::RefreshSceneGraphView(RE::TESQuest* a_quest)
    {
        if (!a_quest)
            return;
        auto* inst = Instance::GetInstance(a_quest);
        if (!inst)
            return;

        if (_choicePending && inst->GetActiveStage() != _choiceOrigin)
            ClosePendingChoice();

        if (_graphOpen) {
            if (inst->GetActiveScene() != _graphScene)
                BuildSceneGraph(*inst);
            else if (const auto* activeStage = inst->GetActiveStage()) {
                const auto it = std::ranges::find(_graphNodes, activeStage, &GraphNode::stage);
                _graphCurrentIndex = it != _graphNodes.end() ? static_cast<int>(std::distance(_graphNodes.begin(), it)) : -1;
            }
        }
    }

    void StageSelectMenu::Close()
    {
        _choicePending = false;
        _choices.clear();
        _choiceScrollOffsets.clear();
        _choiceOrigin = nullptr;
        if (_graphOpen) {
            _graphOpen = false;
        }
        SetBlocksInput(false);
        Hide();
    }

    // ── Internals

    void StageSelectMenu::HideIfIdle()
    {
        if (_choicePending || _graphOpen)
            return;
        SetBlocksInput(false);
        Hide();
    }

    void StageSelectMenu::ClosePendingChoice()
    {
        _choicePending = false;
        _choices.clear();
        _choiceScrollOffsets.clear();
        _choiceOrigin = nullptr;
        HideIfIdle();
    }

    void StageSelectMenu::CloseGraphView()
    {
        _graphOpen = false;
        HideIfIdle();
    }

    void StageSelectMenu::OnNextBranchSelected(int a_index)
    {
        const auto script = Script::GetScriptObject(_linkedThread, "sslThreadController");
        Script::DispatchMethodCall(script, "BranchTo", Script::CallbackPtr{}, static_cast<int>(a_index));
        ClosePendingChoice();
    }

    void StageSelectMenu::BuildSceneGraph(Instance& a_inst)
    {
        _graphNodes.clear();
        _graphEdges.clear();
        _graphCurrentIndex = -1;
        _graphFitPending = true;
        _graphNameScrollOffset = 0.0f;

        const auto* scene = a_inst.GetActiveScene();
        if (!scene)
            return;
        _graphScene = scene;

        const auto allStages = scene->GetAllStages();
        if (allStages.empty())
            return;

        std::unordered_map<const Registry::Animation::Stage*, int> indexOf;
        indexOf.reserve(allStages.size());
        _graphNodes.reserve(allStages.size());
        for (const auto* s : allStages) {
            indexOf.emplace(s, static_cast<int>(_graphNodes.size()));
            _graphNodes.push_back(GraphNode{ s, 0.0f, 0.0f, -1 });
        }

        // BFS layering from the scene's start stage; determines column (x) placement.
        std::vector<int> layerOf(_graphNodes.size(), -1);
        std::vector<int> queue;
        queue.reserve(_graphNodes.size());

        const auto* root = scene->GetStageById(RE::BSFixedString{});
        if (root) {
            if (const auto it = indexOf.find(root); it != indexOf.end()) {
                layerOf[it->second] = 0;
                queue.push_back(it->second);
            }
        }

        for (size_t head = 0; head < queue.size(); ++head) {
            const int cur = queue[head];
            const auto* curStage = _graphNodes[cur].stage;
            const auto adjacent = curStage->GetOutgoingEdges();
            for (const auto* next : adjacent) {
                const auto it = indexOf.find(next);
                if (it == indexOf.end())
                    continue;
                const int nextIdx = it->second;
                _graphEdges.push_back(GraphEdge{ cur, nextIdx });
                if (layerOf[nextIdx] == -1) {
                    layerOf[nextIdx] = layerOf[cur] + 1;
                    queue.push_back(nextIdx);
                }
            }
        }

        // Anything BFS never reached (disconnected islands) still gets drawn, in its own lane.
        int maxLayer = 0;
        for (const int l : layerOf)
            maxLayer = std::max(maxLayer, l);
        for (auto& l : layerOf) {
            if (l == -1)
                l = ++maxLayer;
        }

        std::unordered_map<int, int> rowsInLayer;
        for (size_t i = 0; i < _graphNodes.size(); ++i) {
            const int layer = layerOf[i];
            const int row = rowsInLayer[layer]++;
            _graphNodes[i].layer = layer;
            _graphNodes[i].x = static_cast<float>(layer) * kGraphColumnSpacing;
            _graphNodes[i].y = static_cast<float>(row) * kGraphRowSpacing;
        }
        std::unordered_map<int, float> layerHeight;
        for (auto&& [layer, count] : rowsInLayer)
            layerHeight[layer] = static_cast<float>(count - 1) * kGraphRowSpacing;
        for (auto& node : _graphNodes)
            node.y -= layerHeight[node.layer] * 0.5f;

        if (const auto* active = a_inst.GetActiveStage()) {
            if (const auto it = indexOf.find(active); it != indexOf.end())
                _graphCurrentIndex = it->second;
        }
        _graphShowLabels = _graphNodes.size() < 20;
    }

    // ── Render

    void __stdcall StageSelectMenu::RenderCallback()
    {
        GetSingleton().Render();
    }

    void StageSelectMenu::Render()
    {
        if (!IsVisible())
            return;
        if (!_choicePending && !_graphOpen)
            return;
        if (!_linkedThread)
            return;

        if (_graphOpen)
            RenderSceneGraphView();
        if (_choicePending && !_graphOpen)
            RenderStageChoiceCard();
    }

    void StageSelectMenu::RenderStageChoiceCard()
    {
        auto& scale = SceneHUD::GetSingleton().GetScale();
        auto* io = ImGuiMCP::GetIO();
        const float dh = io->DisplaySize.y;

        const float fontSize = scale.TextPx(UI::Theme::FontSize.body);
        const float padV = scale.Px(5.0f);
        const float accentW = scale.Px(3.0f);
        const float padLeft = scale.Px(10.0f);
        const float rowH = fontSize + padV * 2.0f;
        const float maxW = io->DisplaySize.x * 0.20f;  // hard 20% width cap
        const float scrollSpd = scale.Px(10.0f);       // pixels per second for marquee

        const float titleFontSize = scale.TextPx(UI::Theme::FontSize.sectionHeader);
        const float titleH = titleFontSize + padV * 2.0f;

        const float timerBarH = scale.Px(2.0f);
        const float timerDummyGap = scale.Px(1.0f);
        const bool timerActive = _choiceTimeLimitSec > 0.0f;
        const float timerBlockH = timerActive ? (timerDummyGap + timerBarH + timerDummyGap) : 0.0f;

        const int rowCount = static_cast<int>(_choices.size());
        const float totalH = titleH + timerBlockH + rowH * rowCount;
        const float winY = (dh - totalH) * 0.5f;

        ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{ 0.0f, winY }, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{ maxW, totalH }, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetNextWindowBgAlpha(0.0f);

        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
            ImGuiMCP::ImGuiWindowFlags_NoMove | ImGuiMCP::ImGuiWindowFlags_NoCollapse |
            ImGuiMCP::ImGuiWindowFlags_NoScrollbar | ImGuiMCP::ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiMCP::ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiMCP::ImGuiWindowFlags_NoDecoration;

        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_WindowPadding, ImGuiMCP::ImVec2{ 0.0f, 0.0f });
        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_WindowBg, ImGuiMCP::ImVec4{ 0, 0, 0, 0 });
        if (!ImGuiMCP::Begin("##slpp_StageMenuChoice", nullptr, kFlags)) {
            ImGuiMCP::End();
            ImGuiMCP::PopStyleColor(1);
            ImGuiMCP::PopStyleVar(2);
            return;
        }

        auto* dl = ImGuiMCP::GetWindowDrawList();
        const ImGuiMCP::ImVec2 winPos = ImGuiMCP::GetWindowPos();
        const double timeD = ImGuiMCP::GetTime();

        std::optional<int> clickedIndex;

        const ImGuiMCP::ImU32 panelClear = UI::Theme::Color.panelBackground & 0x00FFFFFFu;
        ImGuiMCP::ImDrawListManager::AddRectFilledMultiColor(dl,
            winPos, ImGuiMCP::ImVec2{ winPos.x + maxW, winPos.y + totalH },
            UI::Theme::Color.panelBackground, panelClear, panelClear, UI::Theme::Color.panelBackground);

        // Title
        SetWindowFontSize(titleFontSize);
        DrawTextShadowed(dl,
            ImGuiMCP::ImVec2{ winPos.x + padLeft, winPos.y + (titleH - titleFontSize) * 0.5f },
            UI::Theme::Color.textSecondary, "Choose Next Stage:");
        SetWindowFontSize(fontSize);

        // Depleting timer bar
        if (timerActive) {
            const float elapsed = static_cast<float>(timeD - _choiceStartTime);
            const float remainFrac = std::clamp(1.0f - elapsed / _choiceTimeLimitSec, 0.0f, 1.0f);

            const float barY = winPos.y + titleH + timerDummyGap;
            const ImGuiMCP::ImVec2 barMin{ winPos.x, barY };
            const ImGuiMCP::ImVec2 barMax{ winPos.x + maxW, barY + timerBarH };

            const ImGuiMCP::ImU32 trackClear = UI::Theme::Animation.timerTrack & 0x00FFFFFFu;
            ImGuiMCP::ImDrawListManager::AddRectFilledMultiColor(dl, barMin, barMax,
                UI::Theme::Animation.timerTrack, trackClear, trackClear, UI::Theme::Animation.timerTrack);

            const float filledW = maxW * remainFrac;
            const ImGuiMCP::ImVec2 fillMin{ barMin.x, barMin.y };
            const ImGuiMCP::ImVec2 fillMax{ barMin.x + filledW, barMax.y };
            const ImGuiMCP::ImU32 fillClear = (UI::Theme::Animation.timerCenter & 0x00FFFFFFu);
            ImGuiMCP::ImDrawListManager::AddRectFilledMultiColor(dl, fillMin, fillMax,
                UI::Theme::Animation.timerCenter, fillClear, fillClear, UI::Theme::Animation.timerCenter);

            if (filledW > 0.0f) {
                const float edgeW = std::min(filledW, scale.Px(3.0f));
                const ImGuiMCP::ImU32 edgeClear = UI::Theme::Animation.timerEdge & 0x00FFFFFFu;
                ImGuiMCP::ImDrawListManager::AddRectFilledMultiColor(dl,
                    ImGuiMCP::ImVec2{ fillMax.x - edgeW, barMin.y }, fillMax,
                    UI::Theme::Animation.timerEdge, edgeClear, edgeClear, UI::Theme::Animation.timerEdge);
            }

            if (remainFrac <= 0.0f && rowCount > 0) {
                static std::mt19937 rng{ std::random_device{}() };
                std::uniform_int_distribution<int> dist(0, rowCount - 1);
                clickedIndex = dist(rng);
            }
        }

        for (int i = 0; i < rowCount; ++i) {
            const auto& choice = _choices[i];
            ImGuiMCP::PushID(i);

            const float rowY = winPos.y + titleH + timerBlockH + rowH * i;
            const ImGuiMCP::ImVec2 rowMin{ winPos.x, rowY };
            const ImGuiMCP::ImVec2 rowMax{ winPos.x + maxW, rowY + rowH };

            // Stage branch selection
            ImGuiMCP::SetCursorScreenPos(rowMin);
            ImGuiMCP::InvisibleButton("##slpp_ssmRow", ImGuiMCP::ImVec2{ maxW, rowH });
            if (i == 0)
                ImGuiMCP::SetItemDefaultFocus();
            const bool mouseHov = ImGuiMCP::IsItemHovered();
            const bool navFocused = ImGuiMCP::IsItemFocused();
            if (mouseHov || navFocused)
                _selectedChoiceIndex = i;
            if (ImGuiMCP::IsItemClicked())
                clickedIndex = i;

            const bool isSelected = (i == _selectedChoiceIndex);

            // Left accent bar that fades to the right
            const ImGuiMCP::ImU32 accentCol = isSelected ? UI::Theme::Color.accent : UI::Theme::Color.borderSubtle;
            const ImGuiMCP::ImU32 accentFade = (accentCol & 0x00FFFFFFu);  // same color, alpha=0
            ImGuiMCP::ImDrawListManager::AddRectFilledMultiColor(dl,
                rowMin, ImGuiMCP::ImVec2{ rowMin.x + accentW, rowMax.y },
                accentCol, accentFade, accentFade, accentCol);

            const float textY = rowY + (rowH - fontSize) * 0.5f;
            const ImGuiMCP::ImU32 textCol = isSelected ? UI::Theme::Color.textPrimary : UI::Theme::Color.textSecondary;

            const float prefixX = rowMin.x + accentW + padLeft;
            const float prefixW = ImGuiMCP::CalcTextSize(choice.prefix.c_str()).x;
            const float navX_base = prefixX + prefixW;
            const float availW = maxW - accentW - padLeft;
            const float navAvailW = availW - prefixW;
            const float navTextW = ImGuiMCP::CalcTextSize(choice.label.c_str()).x;

            // Static prefix
            ImGuiMCP::ImDrawListManager::PushClipRect(dl, rowMin, ImGuiMCP::ImVec2{ navX_base, rowMax.y }, true);
            DrawTextShadowed(dl, ImGuiMCP::ImVec2{ prefixX, textY }, textCol, choice.prefix.c_str());
            ImGuiMCP::ImDrawListManager::PopClipRect(dl);

            // Scrollable NavText
            ImGuiMCP::ImDrawListManager::PushClipRect(dl, ImGuiMCP::ImVec2{ navX_base, rowMin.y }, rowMax, true);

            float navX = navX_base;
            if (isSelected && navTextW > navAvailW) {
                if (i < static_cast<int>(_choiceScrollOffsets.size()))
                    _choiceScrollOffsets[i] += io->DeltaTime * scrollSpd;
                const float cycle = navTextW + navAvailW * 0.5f;
                const float scroll = std::fmod(std::max(0.0f, _choiceScrollOffsets[i]), cycle);
                navX = navX_base - scroll;
            } else if (i < static_cast<int>(_choiceScrollOffsets.size())) {
                _choiceScrollOffsets[i] = 0.0f;
            }
            DrawTextShadowed(dl, ImGuiMCP::ImVec2{ navX, textY }, textCol, choice.label.c_str());

            ImGuiMCP::ImDrawListManager::PopClipRect(dl);
            ImGuiMCP::PopID();
        }

        // Number-key shortcuts (1-9) + Esc selects first option
        if (!clickedIndex) {
            for (int i = 0; i < rowCount && i < 9; ++i) {
                const auto key = static_cast<ImGuiMCP::ImGuiKey>(static_cast<int>(ImGuiMCP::ImGuiKey_1) + i);
                if (ImGuiMCP::IsKeyReleased(key)) {
                    clickedIndex = i;
                    break;
                }
            }
        }
        if (!clickedIndex && _selectedChoiceIndex >= 0 && _selectedChoiceIndex < rowCount &&
            ImGuiMCP::IsKeyReleased(ImGuiMCP::ImGuiKey_Space))
            clickedIndex = _selectedChoiceIndex;
        if (!clickedIndex && rowCount > 0 && ImGuiMCP::IsKeyReleased(ImGuiMCP::ImGuiKey_Escape))
            clickedIndex = 0;

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();
        ImGuiMCP::PopStyleColor(1);
        ImGuiMCP::PopStyleVar(2);

        if (clickedIndex)
            OnNextBranchSelected(*clickedIndex);
    }

    void StageSelectMenu::RenderSceneGraphView()
    {
        auto& scale = SceneHUD::GetSingleton().GetScale();
        auto* io = ImGuiMCP::GetIO();
        const float dw = io->DisplaySize.x;
        const float dh = io->DisplaySize.y;

        const float margin = scale.Px(100.0f);
        const float rawW = dw - margin * 2.0f;
        const float rawH = dh - margin * 2.0f;
        constexpr float kPanelShrink = 0.9f;
        const float viewW = rawW * kPanelShrink;
        const float viewH = rawH * kPanelShrink;
        const float originX = margin + (rawW - viewW) * 0.5f;
        const float originY = margin + (rawH - viewH) * 0.5f;

        // Layout constants
        const float bodyTop = originY;
        const float bodyBot = originY + viewH;
        const float bodyLeft = originX;
        const float bodyRight = originX + viewW;

        ImGuiMCP::SetNextWindowPos(ImGuiMCP::ImVec2{ originX, originY }, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{ viewW, viewH }, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetNextWindowBgAlpha(0.0f);

        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
            ImGuiMCP::ImGuiWindowFlags_NoMove | ImGuiMCP::ImGuiWindowFlags_NoScrollbar |
            ImGuiMCP::ImGuiWindowFlags_NoCollapse | ImGuiMCP::ImGuiWindowFlags_NoBackground |
            ImGuiMCP::ImGuiWindowFlags_NoFocusOnAppearing;

        if (!ImGuiMCP::Begin("##slpp_StageMenuGraph", nullptr, kFlags)) {
            ImGuiMCP::End();
            return;
        }

        auto* dl = ImGuiMCP::GetWindowDrawList();
        const ImGuiMCP::ImVec2 winPos = ImGuiMCP::GetWindowPos();

        // ── Panel fill: rounded body
        const ImGuiMCP::ImU32 bodyFill = (UI::Theme::Color.panelBackground & 0x00FFFFFFu) | (148u << 24);
        const float bodyRounding = scale.Px(8.0f);
        ImGuiMCP::ImDrawListManager::AddRectFilled(dl,
            ImGuiMCP::ImVec2{ bodyLeft, bodyTop },
            ImGuiMCP::ImVec2{ bodyRight, bodyBot },
            bodyFill, bodyRounding, ImGuiMCP::ImDrawFlags_RoundCornersAll);

        // ── Edge vignette
        const ImGuiMCP::ImU32 vigFull = (UI::Theme::Color.panelBackground & 0x00FFFFFFu) | (160u << 24);
        const ImGuiMCP::ImU32 vigClear = (UI::Theme::Color.panelBackground & 0x00FFFFFFu);
        const float vigW = scale.Px(48.0f);  // horizontal vignette depth
        const float vigH = scale.Px(56.0f);  // vertical vignette depth

        const auto Vig = [&](float x0, float y0, float x1, float y1,
                             ImGuiMCP::ImU32 tl, ImGuiMCP::ImU32 tr, ImGuiMCP::ImU32 br, ImGuiMCP::ImU32 bl) {
            ImGuiMCP::ImDrawListManager::AddRectFilledMultiColor(dl, { x0, y0 }, { x1, y1 }, tl, tr, br, bl);
        };
        Vig(bodyLeft, bodyTop, bodyRight, bodyTop + vigH, vigFull, vigFull, vigClear, vigClear);   // Top
        Vig(bodyLeft, bodyBot - vigH, bodyRight, bodyBot, vigClear, vigClear, vigFull, vigFull);   // Bottom
        Vig(bodyLeft, bodyTop, bodyLeft + vigW, bodyBot, vigFull, vigClear, vigClear, vigFull);    // Left
        Vig(bodyRight - vigW, bodyTop, bodyRight, bodyBot, vigClear, vigFull, vigFull, vigClear);  // Right

        // ── Scene name
        const float nameFontSize = scale.TextPx(UI::Theme::FontSize.sectionHeader);
        const float namePadH = scale.Px(8.0f);
        const float namePadSide = scale.Px(12.0f);
        const char* sceneName = _graphScene ? _graphScene->GetName().data() : "Scene Graph";

        SetWindowFontSize(nameFontSize);
        const float nameAvailW = viewW * 0.50f - namePadSide * 2.0f;
        const float nameTextW = ImGuiMCP::CalcTextSize(sceneName).x;
        const float nameY = bodyTop + namePadH;

        ImGuiMCP::ImDrawListManager::PushClipRect(dl,
            ImGuiMCP::ImVec2{ bodyLeft + namePadSide, bodyTop },
            ImGuiMCP::ImVec2{ bodyLeft + namePadSide + nameAvailW, bodyTop + nameFontSize + namePadH * 2.0f }, true);

        float nameX = bodyLeft + namePadSide;
        if (nameTextW > nameAvailW) {
            _graphNameScrollOffset += io->DeltaTime * scale.Px(12.0f);
            const float cycle = nameTextW + nameAvailW * 0.5f;
            _graphNameScrollOffset = std::fmod(_graphNameScrollOffset, cycle);
            nameX = bodyLeft + namePadSide - _graphNameScrollOffset;
        } else {
            _graphNameScrollOffset = 0.0f;
        }
        DrawTextShadowed(dl, ImGuiMCP::ImVec2{ nameX, nameY }, UI::Theme::Color.textPrimary, sceneName);
        ImGuiMCP::ImDrawListManager::PopClipRect(dl);

        // ── Buttons
        const float btnFontSize = scale.TextPx(UI::Theme::FontSize.caption);
        const float btnRounding = scale.Px(4.0f);
        const float btnPadSide = scale.Px(12.0f);
        const float btnPadBot = scale.Px(10.0f);
        const float btnGap = scale.Px(4.0f);
        SetWindowFontSize(btnFontSize);
        ImGuiMCP::PushStyleVar(ImGuiMCP::ImGuiStyleVar_FrameRounding, btnRounding);

        ImGuiMCP::SetWindowFontScale(1.0f);  // reset scale so GetFrameHeight works
        SetWindowFontSize(btnFontSize);
        const float btnH = ImGuiMCP::GetFrameHeight();
        const float btnW = scale.Px(80.0f);

        const float closeBtnY = bodyBot - btnPadBot - btnH;
        const float labelBtnY = closeBtnY - btnGap - btnH;
        const float btnX = bodyRight - btnPadSide - btnW;

        const char* lblToggleLabel = _graphShowLabels ? "Labels: On" : "Labels: Off";
        ImGuiMCP::SetCursorScreenPos(ImGuiMCP::ImVec2{ btnX, labelBtnY });
        if (UI::ActionButton(lblToggleLabel, btnW))
            _graphShowLabels = !_graphShowLabels;

        ImGuiMCP::SetCursorScreenPos(ImGuiMCP::ImVec2{ btnX, closeBtnY });
        const bool closeClicked = UI::ActionButton("Close", btnW);

        ImGuiMCP::PopStyleVar();

        // ── Canvas
        const ImGuiMCP::ImVec2 canvasMin{ bodyLeft, bodyTop };
        const ImGuiMCP::ImVec2 canvasMax{ bodyRight, bodyBot };
        const ImGuiMCP::ImVec2 canvasSize{ viewW, viewH };

        ImGuiMCP::SetCursorScreenPos(canvasMin);
        ImGuiMCP::InvisibleButton("##slpp_graphCanvas", canvasSize);
        const bool canvasHovered = ImGuiMCP::IsItemHovered();
        const bool canvasActive = ImGuiMCP::IsItemActive();

        if (canvasHovered && io->MouseWheel != 0.0f)
            _graphZoom = std::clamp(_graphZoom + io->MouseWheel * 0.1f, 0.3f, 2.5f);
        if (canvasActive && ImGuiMCP::IsMouseDown(ImGuiMCP::ImGuiMouseButton_Left)) {
            _graphPanX += io->MouseDelta.x;
            _graphPanY += io->MouseDelta.y;
            _graphFitPending = false;
        }

        // Fit-to-view on first open
        if (_graphFitPending && !_graphNodes.empty()) {
            float minX = _graphNodes[0].x, maxX = _graphNodes[0].x;
            float minY = _graphNodes[0].y, maxY = _graphNodes[0].y;
            for (const auto& n : _graphNodes) {
                minX = std::min(minX, n.x);
                maxX = std::max(maxX, n.x);
                minY = std::min(minY, n.y);
                maxY = std::max(maxY, n.y);
            }
            const float graphW = std::max(1.0f, maxX - minX);
            const float graphH = std::max(1.0f, maxY - minY);
            _graphZoom = std::clamp(std::min(canvasSize.x / (graphW + kGraphColumnSpacing), canvasSize.y / (graphH + kGraphRowSpacing * 2.0f)), 0.3f, 1.5f);
            _graphPanX = canvasSize.x * 0.5f - (minX + graphW * 0.5f) * _graphZoom;
            _graphPanY = canvasSize.y * 0.5f - (minY + graphH * 0.5f) * _graphZoom;
            _graphFitPending = false;
        }

        ImGuiMCP::ImDrawListManager::PushClipRect(dl, canvasMin, canvasMax, true);

        const auto worldToScreen = [&](float a_x, float a_y) {
            return ImGuiMCP::ImVec2{ canvasMin.x + _graphPanX + a_x * _graphZoom, canvasMin.y + _graphPanY + a_y * _graphZoom };
        };

        const float radius = kGraphNodeRadius * scale.Factor() * std::clamp(_graphZoom, 0.5f, 1.0f);

        // Edges
        for (const auto& edge : _graphEdges) {
            const auto& from = _graphNodes[edge.from];
            const auto& to = _graphNodes[edge.to];
            const ImGuiMCP::ImVec2 a = worldToScreen(from.x, from.y);
            const ImGuiMCP::ImVec2 b = worldToScreen(to.x, to.y);
            ImGuiMCP::ImDrawListManager::AddLine(dl, a, b, UI::Theme::Color.borderSubtle, scale.Px(1.2f));

            const float dx = b.x - a.x, dy = b.y - a.y;
            const float len = std::sqrt(dx * dx + dy * dy);
            if (len > 1.0f) {
                const float ux = dx / len, uy = dy / len;
                const ImGuiMCP::ImVec2 tip{ b.x - ux * radius, b.y - uy * radius };
                const float wingSize = scale.Px(4.0f);
                const ImGuiMCP::ImVec2 wLeft{ tip.x - ux * wingSize - uy * wingSize * 0.6f, tip.y - uy * wingSize + ux * wingSize * 0.6f };
                const ImGuiMCP::ImVec2 wRight{ tip.x - ux * wingSize + uy * wingSize * 0.6f, tip.y - uy * wingSize - ux * wingSize * 0.6f };
                ImGuiMCP::ImDrawListManager::AddTriangleFilled(dl, tip, wLeft, wRight, UI::Theme::Color.borderHovered);
            }
        }

        auto* inst = Instance::GetInstance(_linkedThread);
        if (!inst)
            return;

        // Nodes
        int graphClickedIndex = -1;
        const ImGuiMCP::ImVec2 mouse = ImGuiMCP::GetMousePos();
        for (int i = 0; i < static_cast<int>(_graphNodes.size()); ++i) {
            const auto& node = _graphNodes[i];
            const ImGuiMCP::ImVec2 p = worldToScreen(node.x, node.y);
            if (p.x < canvasMin.x - radius || p.x > canvasMax.x + radius || p.y < canvasMin.y - radius || p.y > canvasMax.y + radius)
                continue;

            const float dx = mouse.x - p.x, dy = mouse.y - p.y;
            const bool hovered = canvasHovered && (dx * dx + dy * dy) <= (radius + scale.Px(4.0f)) * (radius + scale.Px(4.0f));
            if (hovered && ImGuiMCP::IsMouseClicked(ImGuiMCP::ImGuiMouseButton_Left))
                graphClickedIndex = i;

            const bool isCurrent = i == _graphCurrentIndex;
            const auto isSinkNode = _graphScene && node.stage && node.stage->GetOutgoingEdges().empty();
            const auto fill = isCurrent  ? UI::Theme::Color.accent :
                              hovered    ? UI::Theme::Color.borderHovered :
                              isSinkNode ? UI::Theme::Color.textMuted :
                                           UI::Theme::Color.buttonIdle;

            ImGuiMCP::ImDrawListManager::AddCircleFilled(dl, p, radius, fill, 20);

            // Show labels when toggle is on (always current + hovered regardless).
            const bool showLabel = _graphShowLabels || isCurrent || hovered;
            if (showLabel) {
                std::string label = !node.stage->GetNavigationText().empty() ?
                                        ResolveNavTextPlaceholders(node.stage->GetNavigationText(), inst->GetActors()) :
                                        std::string{ node.stage->GetId() };
                SetWindowFontSize(scale.TextPx(UI::Theme::FontSize.metadata) * 0.88f);
                const ImGuiMCP::ImVec2 labelSz = ImGuiMCP::CalcTextSize(label.c_str());
                DrawTextShadowed(dl, ImGuiMCP::ImVec2{ p.x - labelSz.x * 0.5f, p.y + radius + scale.Px(2.0f) },
                    isCurrent ? UI::Theme::Color.textPrimary : UI::Theme::Color.textSecondary, label.c_str());
            }
        }

        ImGuiMCP::ImDrawListManager::PopClipRect(dl);

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();

        // Node click
        const auto script = Script::GetScriptObject(_linkedThread, "sslThreadController");
        if (graphClickedIndex >= 0 && graphClickedIndex < static_cast<int>(_graphNodes.size())) {
            const auto* clickedStage = _graphNodes[graphClickedIndex].stage;
            if (_graphScene && clickedStage) {
                Script::DispatchMethodCall(script, "ToggleVisibilitySceneGraph", Script::CallbackPtr{}, -1);
                Script::DispatchMethodCall(script, "SkipTo", Script::CallbackPtr{}, std::string{ clickedStage->GetId() });
                return;
            }
        }
        if (closeClicked || ImGuiMCP::IsKeyReleased(ImGuiMCP::ImGuiKey_Escape))
            Script::DispatchMethodCall(script, "ToggleVisibilitySceneGraph", Script::CallbackPtr{}, -1);
    }
}
