#pragma once
#include "SKSEMenuFramework.h"
#include "Thread/Thread.h"
#include "Util/Script.h"

namespace Thread::Interface
{
    enum class IdxHudElement : int32_t
    {
        kGameHUD           = 0,
        kAnimSpeedOverlay  = 1,
        kEnjBarsOverlay    = 2,
        kOffsetAdjustPanel = 3,
        kSceneSelectPanel  = 4,
        kThreadConfigPanel = 5,
        kElementCtrlPanel  = 6,
    };

    enum class IdxTabPanel : int32_t
    {
        kNone         = 0,
        kThreadConfig = 1,
        kSceneSelect  = 2,
        kOffsetAdjust = 3,
        kElementCtrl  = 4,
    };

    class SceneHUD
    {
      public:
        static inline RE::TESQuest* linkedThread{ nullptr };
        static inline Script::ObjectPtr threadScript{ nullptr };
        static inline Script::CallbackPtr callbackPtr{};

        static inline MENU_WINDOW winAnimSpeed{ nullptr };
        static inline MENU_WINDOW winEnjBars{ nullptr };
        static inline MENU_WINDOW winThreadConfig{ nullptr };
        static inline MENU_WINDOW winSceneSelect{ nullptr };
        static inline MENU_WINDOW winOffsetAdjust{ nullptr };
        static inline MENU_WINDOW winElementCtrl{ nullptr };
        static inline MENU_WINDOW winPanelStack{ nullptr };

        static bool Register();
        static void Init(RE::TESQuest* a_qst);
        static void Destroy();
        static bool IsActive() { return linkedThread != nullptr; }

        static inline bool focusMode{ false };
        static void SetFocus(bool state);
        static void ToggleFocus() { SetFocus(!focusMode); }

        static inline IdxTabPanel activePanel{ IdxTabPanel::kNone };
        static void OpenPanel(IdxTabPanel idxPanel);
        static void CloseAllPanels();
        static bool IsPanelOpen(IdxTabPanel idxPanel) { return activePanel == idxPanel; }

        static void OnOverlayToggle(IdxHudElement idxElement, bool state);
    };

    namespace ScaleUI
    {
        inline float s_scaleFactor{ 0.0f };
        inline bool  s_scaleValid{ false };
        inline float s_lastDispW{ -1.0f };
        inline float s_lastDispH{ -1.0f };

        inline void InvalidateScale()
        {
            ScaleUI::s_scaleValid = false;
        }

        inline void RecomputeScale()
        {
            auto* io = ImGuiMCP::GetIO();
            s_lastDispW = io->DisplaySize.x;
            s_lastDispH = io->DisplaySize.y;
            const float minVP = std::min(s_lastDispW, s_lastDispH) * 0.01f;
            float sAdj = 1.5f;
            if (auto* inst = Instance::GetInstance(SceneHUD::linkedThread))
                sAdj = inst->GetThreadProperty<float>("VarUI_MenuScaleMult");
            s_scaleFactor = (minVP / 10.8f) * sAdj;
            s_scaleValid  = true;
        }

        inline void EnsureScaleFresh()
        {
            auto* io = ImGuiMCP::GetIO();
            if (!s_scaleValid || io->DisplaySize.x != s_lastDispW || io->DisplaySize.y != s_lastDispH)
                RecomputeScale();
        }

        inline float pxScale(float pxUnits)
        {
            EnsureScaleFresh();
            return pxUnits * s_scaleFactor;
        }

        inline float pxScaleClamp(float minU, float pct, float maxU, float axisSize)
        {
            return std::clamp(axisSize * pct * 0.01f, pxScale(minU), pxScale(maxU));
        }
    };

    namespace ColorUI
    {
        constexpr ImGuiMCP::ImU32 TextPrimary   = IM_COL32(221, 216, 208, 255);
        constexpr ImGuiMCP::ImU32 TextSecond    = IM_COL32(176, 168, 152, 255);
        constexpr ImGuiMCP::ImU32 TextMuted     = IM_COL32(136, 128, 120, 255);
        constexpr ImGuiMCP::ImU32 BadgeGreen    = IM_COL32(112, 184, 112, 255);

        constexpr ImGuiMCP::ImU32 EnjNormalLo   = IM_COL32(122,  40,  40, 255);
        constexpr ImGuiMCP::ImU32 EnjNormalHi   = IM_COL32(208, 104,  88, 255);
        constexpr ImGuiMCP::ImU32 EnjOverLo     = IM_COL32(138,  96,  16, 255);
        constexpr ImGuiMCP::ImU32 EnjOverHi     = IM_COL32(232, 184,  64, 255);
        constexpr ImGuiMCP::ImU32 EnjNegLo      = IM_COL32( 30,  58,  88, 255);
        constexpr ImGuiMCP::ImU32 EnjNegHi      = IM_COL32( 72, 128, 192, 255);
        constexpr ImGuiMCP::ImU32 EnjZoneIdle   = IM_COL32( 50, 155,  60,  41);
        constexpr ImGuiMCP::ImU32 EnjZoneActive = IM_COL32( 60, 185,  65,  71);
        constexpr ImGuiMCP::ImU32 EnjZoneBd     = IM_COL32( 90, 200,  70, 128);
        constexpr ImGuiMCP::ImU32 EnjZoneBdAct  = IM_COL32(110, 250,  90, 242);
        constexpr ImGuiMCP::ImU32 EnjNeedle     = IM_COL32(200, 216, 184, 255);
        constexpr ImGuiMCP::ImU32 EnjNeedleAct  = IM_COL32(144, 248, 120, 255);
        constexpr ImGuiMCP::ImU32 EnjHitText    = IM_COL32( 96, 204,  80, 255);
        constexpr ImGuiMCP::ImU32 EnjMissText   = IM_COL32(224,  96,  80, 255);

        constexpr ImGuiMCP::ImU32 OamFill       = IM_COL32(160, 160, 160,  56);
        constexpr ImGuiMCP::ImU32 OamNeedle     = IM_COL32(176, 168, 152, 255);
        constexpr ImGuiMCP::ImU32 OamNeedleDrg  = IM_COL32(221, 216, 208, 255);

        constexpr ImGuiMCP::ImU32 BgTab         = IM_COL32(24, 24, 26, 235);
        constexpr ImGuiMCP::ImU32 BgTabHover    = IM_COL32(36, 36, 40, 245);
        constexpr ImGuiMCP::ImU32 BgPanel       = IM_COL32(20, 20, 22, 245);
    };

    inline void DrawTextShadowed(ImGuiMCP::ImDrawList* dl, ImGuiMCP::ImVec2 pos, ImGuiMCP::ImU32 col,
                                 const char* text, bool withGlow = false)
    {
        constexpr ImGuiMCP::ImU32 sh = IM_COL32(0, 0, 0, 210);
        ImGuiMCP::ImDrawListManager::AddText(dl, ImGuiMCP::ImVec2{pos.x - 1, pos.y - 1}, sh, text, nullptr);
        ImGuiMCP::ImDrawListManager::AddText(dl, ImGuiMCP::ImVec2{pos.x + 1, pos.y - 1}, sh, text, nullptr);
        ImGuiMCP::ImDrawListManager::AddText(dl, ImGuiMCP::ImVec2{pos.x - 1, pos.y + 1}, sh, text, nullptr);
        ImGuiMCP::ImDrawListManager::AddText(dl, ImGuiMCP::ImVec2{pos.x + 1, pos.y + 1}, sh, text, nullptr);
        if (withGlow) {
            const float r = ScaleUI::pxScale(3.0f);
            constexpr ImGuiMCP::ImU32 g = IM_COL32(0, 0, 0, 200);
            ImGuiMCP::ImDrawListManager::AddText(dl, ImGuiMCP::ImVec2{pos.x - r, pos.y - r}, g, text, nullptr);
            ImGuiMCP::ImDrawListManager::AddText(dl, ImGuiMCP::ImVec2{pos.x + r, pos.y - r}, g, text, nullptr);
            ImGuiMCP::ImDrawListManager::AddText(dl, ImGuiMCP::ImVec2{pos.x - r, pos.y + r}, g, text, nullptr);
            ImGuiMCP::ImDrawListManager::AddText(dl, ImGuiMCP::ImVec2{pos.x + r, pos.y + r}, g, text, nullptr);
        }
        ImGuiMCP::ImDrawListManager::AddText(dl, pos, col, text, nullptr);
    }

    inline ImGuiMCP::ImVec4 ToVec4(ImGuiMCP::ImU32 col)
    {
        return ImGuiMCP::ImVec4{
            static_cast<float>((col >> IM_COL32_R_SHIFT) & 0xFF) / 255.0f,
            static_cast<float>((col >> IM_COL32_G_SHIFT) & 0xFF) / 255.0f,
            static_cast<float>((col >> IM_COL32_B_SHIFT) & 0xFF) / 255.0f,
            static_cast<float>((col >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f,
        };
    }

}  // namespace Thread::Interface
