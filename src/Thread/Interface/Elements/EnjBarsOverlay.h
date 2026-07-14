#pragma once
#include "Thread/Interface/UI/Window.h"

namespace Thread::Interface
{
    class EnjBarsOverlay final : public UI::WindowComponent
    {
      public:
        static EnjBarsOverlay& GetSingleton();

        bool Register();
        void Init();
        void Destroy();
        void UpdateSlider(RE::Actor* a_actor, float a_enjoyment, RE::BSFixedString a_interactions);
        void UpdateHighlightedPartner(RE::Actor* a_partner);
        void RegisterRaiseEnjAttempt(RE::Actor* a_actor, float a_nextTimeCycle);

      private:
        EnjBarsOverlay() = default;

        struct ActorEnjBar
        {
            uint32_t formId{};
            char name[64]{};
            float enjoyment{ 0.0f };
            char interactions[128]{};
            bool isTarget{ false };
            bool isGameDpt{ false };
        };

        static void __stdcall RenderCallback();
        void Render();
        void OnSelectPartner(std::uint32_t a_formId);

        static float FillFraction(float a_enjoyment);
        static void FillGradient(float a_enjoyment, ImGuiMCP::ImU32& a_low, ImGuiMCP::ImU32& a_high);
        static float GreenZoneHalfWidth(float a_enjoyment);

        std::vector<ActorEnjBar> _bars;

        static constexpr float kGZoneDefault = 0.125f;
        static constexpr float kGZoneMin = 0.02f;
        static constexpr float kGameEnjThresh = 75.0f;
        static constexpr float kGameEnjDrawMin = 80.0f;
        static constexpr double kFeedbackSec = 0.6;

        bool _needleRunning{ false };
        float _needlePosition{ 0.5f };
        float _needleDirection{ 1.0f };
        float _timeCycle{ 0.0f };
        std::uint32_t _feedbackActorId{};
        bool _feedbackHit{ false };
        double _feedbackUntil{ 0.0 };

        // ── Layout cache
        struct LayoutCache
        {
            float zoneW, barGap, innerGp, frameH, lblPad;
            float nameFt, valFt, intrFt, fbFt;
            float edgeH, edgeV, lblRowH, unitH, winH;
        };
        LayoutCache _layout{};
        float _layoutForFactor{ -1.0f };
        float _layoutForWidth{ -1.0f };
        float _layoutForHeight{ -1.0f };
        std::size_t _layoutForCount{ SIZE_MAX };

        const LayoutCache& GetLayout(std::size_t a_actorCount);
    };
}
