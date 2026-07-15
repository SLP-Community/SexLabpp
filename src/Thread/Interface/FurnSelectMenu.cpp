#include "FurnSelectMenu.h"
#include "SceneHUD.h"

namespace Thread::Interface
{
    using UI::SetWindowFontSize;

    FurnSelectMenu& FurnSelectMenu::GetSingleton()
    {
        static FurnSelectMenu singleton;
        return singleton;
    }

    bool FurnSelectMenu::Register()
    {
        if (!RegisterWindow(RenderCallback, true)) {
            logger::error("FurnSelectMenu::Register >> AddWindow failed");
            return false;
        }
        return true;
    }

    void FurnSelectMenu::Open(RE::TESQuest* a_quest, const std::vector<Item>& a_items)
    {
        if (!a_quest)
            return;
        _linkedThread = a_quest;
        _items = a_items;
        Show();
    }

    void FurnSelectMenu::HandleSelection(std::size_t a_index)
    {
        Hide();
        auto* quest = std::exchange(_linkedThread, nullptr);
        _items.clear();
        if (!quest)
            return;
        auto* inst = Instance::GetPendingInstance(quest);
        if (!inst) {
            logger::error("FurnSelectMenu::HandleSelection >> instance not found");
            return;
        }
        inst->SetCenterRefSelected(a_index);
    }

    void __stdcall FurnSelectMenu::RenderCallback()
    {
        GetSingleton().Render();
    }

    void FurnSelectMenu::Render()
    {
        if (!IsVisible())
            return;
        auto& scale = SceneHUD::GetSingleton().GetScale();

        const float btnW = scale.Px(260.0f);

        // Centred on screen
        auto* vp = ImGuiMCP::GetMainViewport();
        const ImGuiMCP::ImVec2 centre = ImGuiMCP::ImGuiViewportManager::GetCenter(vp);
        ImGuiMCP::SetNextWindowPos(centre, ImGuiMCP::ImGuiCond_Always, ImGuiMCP::ImVec2{ 0.5f, 0.5f });
        ImGuiMCP::SetNextWindowSize({ btnW + scale.Px(24.0f), 0.0f }, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetNextWindowBgAlpha(0.97f);

        // Begin() needs a plain bool*, so mirror the window's atomic IsOpen through a local and write it back afterward.
        bool isOpen = true;
        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
            ImGuiMCP::ImGuiWindowFlags_NoMove | ImGuiMCP::ImGuiWindowFlags_NoCollapse |
            ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize;

        if (!ImGuiMCP::Begin("##slpp_FurnSelect", &isOpen, kFlags)) {
            ImGuiMCP::End();
            SetVisible(isOpen);
            return;
        }

        // Panel title
        SetWindowFontSize(scale.TextPx(UI::Theme::FontSize::caption));
        ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textSecondary), "CENTER SELECTION");
        ImGuiMCP::Separator();

        SetWindowFontSize(scale.TextPx(UI::Theme::FontSize::body));
        std::optional<std::size_t> selectedIndex;
        if (_items.empty()) {
            ImGuiMCP::TextColored(UI::Theme::ToVec4(UI::Theme::Color::textMuted), "No suitable scene center nearby.");
        } else {
            for (std::size_t i = 0; i < _items.size(); ++i) {
                ImGuiMCP::PushID(static_cast<int>(i));
                const auto label = std::format("{}  ({})", _items[i].name, _items[i].value);
                if (ImGuiMCP::Button(label.c_str(), { btnW, scale.Px(28.0f) }))
                    selectedIndex = i;
                ImGuiMCP::PopID();
            }
        }

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();
        SetVisible(isOpen);
        if (selectedIndex)
            HandleSelection(*selectedIndex);
    }
}
