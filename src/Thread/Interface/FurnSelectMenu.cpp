#include "FurnSelectMenu.h"

namespace Thread::Interface
{
    bool FurnSelectMenu::Register()
    {
        window = SKSEMenuFramework::AddWindow(FurnSelectMenu::Render, true);
        if (!window) {
            logger::error("FurnSelectMenu::Register >> AddWindow failed");
            return false;
        }
        window->IsOpen = false;
        return true;
    }

    void FurnSelectMenu::Open(RE::TESQuest* a_qst, const std::vector<Item>& a_items)
    {
        if (!window || !a_qst) return;
        s_linkedThread = a_qst;
        s_items = a_items;
        window->IsOpen = true;
    }

    void FurnSelectMenu::HandleSelection(size_t index)
    {
        window->IsOpen = false;
        auto* qst = s_linkedThread;
        s_linkedThread = nullptr;
        s_items.clear();
        if (!qst) return;
        auto* inst = Instance::GetPendingInstance(qst);
        if (!inst) {
            logger::error("FurnSelectMenu::HandleSelection >> instance not found");
            return;
        }
        inst->SetCenterRefSelected(index);
    }

    void __stdcall FurnSelectMenu::Render()
    {
        if (!window->IsOpen) return;

        const float btnW = ScaleUI::pxScale(260.0f);

        // Centred on screen
        auto* vp = ImGuiMCP::GetMainViewport();
        const ImGuiMCP::ImVec2 centre = ImGuiMCP::ImGuiViewportManager::GetCenter(vp);
        ImGuiMCP::SetNextWindowPos(centre, ImGuiMCP::ImGuiCond_Always, ImGuiMCP::ImVec2{0.5f, 0.5f});
        ImGuiMCP::SetNextWindowSize(ImGuiMCP::ImVec2{btnW + ScaleUI::pxScale(24.0f), 0.0f}, ImGuiMCP::ImGuiCond_Always);
        ImGuiMCP::SetNextWindowBgAlpha(0.97f);

        // Begin() needs a plain bool*, so mirror the window's atomic IsOpen through a local and write it back afterward.
        bool isOpen = true;
        constexpr auto kFlags =
            ImGuiMCP::ImGuiWindowFlags_NoTitleBar | ImGuiMCP::ImGuiWindowFlags_NoResize |
            ImGuiMCP::ImGuiWindowFlags_NoMove     | ImGuiMCP::ImGuiWindowFlags_NoCollapse |
            ImGuiMCP::ImGuiWindowFlags_AlwaysAutoResize;

        if (!ImGuiMCP::Begin("##slpp_FurnSelect", &isOpen, kFlags)) {
            ImGuiMCP::End();
            window->IsOpen = isOpen;
            return;
        }

        // Panel title
        SetWindowFontSize(ScaleUI::pxScale(9.0f));
        ImGuiMCP::TextColored(ToVec4(ColorUI::TextSecond), "CENTER SELECTION");
        ImGuiMCP::Separator();

        SetWindowFontSize(ScaleUI::pxScale(10.0f));
        if (s_items.empty()) {
            ImGuiMCP::TextColored(ToVec4(ColorUI::TextMuted), "No suitable scene center nearby.");
        } else {
            for (size_t i = 0; i < s_items.size(); ++i) {
                ImGuiMCP::PushID(static_cast<int>(i));
                const std::string lbl = s_items[i].name + "  (" + s_items[i].value + ")";
                if (ImGuiMCP::Button(lbl.c_str(), ImGuiMCP::ImVec2{btnW, ScaleUI::pxScale(28.0f)}))
                    HandleSelection(i);
                ImGuiMCP::PopID();
            }
        }

        ImGuiMCP::SetWindowFontScale(1.0f);
        ImGuiMCP::End();
        window->IsOpen = isOpen;
    }
}
