#pragma once
#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interface
{
    class EnjBarsOverlay
    {
      public:
        static void Init();
        static void Destroy();
        static void __stdcall Render();

        static void UpdateSlider(RE::Actor* a_actor, float a_enjoyment, RE::BSFixedString a_interactions);
        static void UpdateHighlightedPartner(RE::Actor* a_partner);
        static void RegisterRaiseEnjAttempt(RE::Actor* a_actor, float a_nextTimeCycle);

      private:
        struct ActorEnjBar
        {
            uint32_t formId{};
            char name[64]{};
            float enjoyment{ 0.0f };
            char interactions[128]{};
            bool isTarget{ false };
            bool isGameDpt{ false };
        };

        static void OnSelectPartner(uint32_t formId);

        static float FillFraction(float enj);
        static void FillGradient (float enj, ImGuiMCP::ImU32& lo, ImGuiMCP::ImU32& hi);

        inline static bool isVisible{ false };

        // s_mu protects s_bars only, ensuring that the game thread writes whereas D3D thread reads
        inline static std::mutex s_mu;
        inline static std::vector<ActorEnjBar> s_bars;

        static constexpr float  kGZoneDefault   = 0.125f;
        static constexpr float  kGZoneMin       = 0.02f;
        static constexpr float  kGameEnjThresh  = 75.0f;
        static constexpr float  kGameEnjDrawMin = 80.0f;
        static constexpr double kFeedbackSec    = 0.6;

        // Needle state uses atomics so that:
        // (a) RegisterRaiseEnjAttempt can write s_needlePos/_needleDir/s_timeCycle without holding s_mu during Render()
        // (b) so pusher calls from the game thread never race with the render thread's needle advance.
        inline static std::atomic<bool> s_needleRunning{ false };
        inline static std::atomic<float> s_needlePos{ 0.5f };
        inline static std::atomic<float> s_needleDir{ 1.0f };
        inline static std::atomic<float> s_timeCycle{ 0.0f };
        inline static std::atomic<float> s_greenStart{ 0.5f - kGZoneDefault };
        inline static std::atomic<float> s_greenEnd{ 0.5f + kGZoneDefault };
        
        // Feedback (written by RegisterRaiseEnjAttempt under s_mu, read by Render)
        inline static std::atomic<uint32_t> s_fbActorId{ 0 };
        inline static std::atomic<bool> s_fbHit{ false };
        inline static std::atomic<double> s_fbUntil{ 0.0 };

        // ── Layout cache
        struct LayoutCache
        {
            float zoneW, barGap, innerGp, frameH, lblPad;
            float nameFt, valFt, intrFt, fbFt;
            float edgeH, edgeV, lblRowH, unitH, winH;
        };
        inline static LayoutCache s_layout{};
        inline static float s_layoutForFactor{ -1.0f };
        inline static float s_layoutForDw{ -1.0f };
        inline static float s_layoutForDh{ -1.0f };
        inline static size_t s_layoutForCount { SIZE_MAX };

        static const LayoutCache& GetLayout(size_t actorCount);
    };
}
