#pragma once
#include "SKSEMenuFramework.h"

namespace Thread
{
    class Instance;
}

namespace Thread::Interface
{
    class SceneHUD;
    namespace UI
    {
        class Scale;
    }

    class EnjBarsOverlay final
    {
      public:
        void Init(Instance& a_instance);
        void Render(SceneHUD& a_hud);
        void UpdateSlider(RE::Actor* a_actor, float a_enjoyment, RE::BSFixedString a_interactions);
        void UpdateHighlightedPartner(RE::Actor* a_partner);
        void RegisterRaiseEnjAttempt(SceneHUD& a_hud, RE::Actor* a_actor, float a_nextTimeCycle);

      private:
        struct ActorEnjBar
        {
            uint32_t formId{};
            char name[64]{};
            float enjoyment{ 0.0f };
            float targetFill{ 0.0f };
            float displayedFill{ 0.0f };
            float trailingFill{ 0.0f };
            char interactions[128]{};
            bool isTarget{ false };
            bool isGameDpt{ false };
        };

        void OnSelectPartner(SceneHUD& a_hud, std::uint32_t a_formId);

        static float FillFraction(float a_enjoyment);
        static void FillGradient(float a_enjoyment, ImGuiMCP::ImU32& a_low, ImGuiMCP::ImU32& a_high);
        static float GreenZoneHalfWidth(float a_enjoyment);

        std::vector<ActorEnjBar> _bars;

        static constexpr float kGZoneDefault = 0.125f;
        static constexpr float kGZoneMin = 0.02f;
        static constexpr float kGameEnjThresh = 80.0f;
        static constexpr float kGameEnjDrawMin = 80.0f;
        // At 100 enjoyment, the needle cycle is 0.8s. It shouldn't go any faster than that.
        static constexpr float kGameMinTimeCycle = 0.8f;
        static constexpr double kFeedbackSec = 0.6;

        bool _needleRunning{ false };
        float _needlePosition{ 0.5f };
        float _needleDirection{ 1.0f };
        float _timeCycle{ 0.0f };
        std::uint32_t _feedbackActorId{};
        bool _feedbackHit{ false };
        double _feedbackUntil{ 0.0 };

        struct Layout
        {
            float zoneW, barGap, innerGp, frameH, lblPad;
            float nameFt, valFt, intrFt, fbFt;
            float edgeH, edgeV, lblRowH, unitH, winH;
        };

        static Layout GetLayout(UI::Scale& a_scale, std::size_t a_actorCount);
    };
}
