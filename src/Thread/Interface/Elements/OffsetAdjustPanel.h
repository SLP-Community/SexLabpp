#pragma once
#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interface
{
    class OffsetAdjustPanel
    {
      public:
        static void Init();
        static void Destroy();
        static void __stdcall Render();
        static void OnStageChanged();

      private:
        struct ActorItem
        {
            RE::Actor* actor{};
            uint32_t formId{};
            size_t posIdx{};
            std::string label;
            bool isScene{ false };
        };

        struct AxisState
        {
            float value{ 0.0f };
            float baseline{ 0.0f }; // default value
            bool hasBaseline{ false };
            float dragStartValue{ 0.0f };
        };

        static void OnActorSelected(const ActorItem& item);
        static void OnSetOffset(Registry::CoordinateType axis, uint32_t actorId, float value);
        static void OnResetOffsets();
        static void OnSetAdjustStageOnly(bool state);

        static void RefreshSlots();
        static void RefreshValues(uint32_t actorId);

        // The centered drag slider used for each axis; Returns true if value changed this frame and C++ should be notified.
        static bool OffsetTrack(const char* id, AxisState& state, float range, bool /*isDragging*/, bool& draggingOut);

        inline static bool isVisible{ false };
        inline static std::vector<ActorItem> s_items;

        inline static bool s_centerIsPlayer{ false };
        inline static bool s_hasFurniture{ false };
        inline static bool s_adjustStageOnly{ false };
        inline static bool s_pickerOpen{ false };
        inline static bool s_panelOpen{ false };

        inline static std::unordered_map<uint32_t, std::array<AxisState, 4>> s_axes; // Per furn/actor XYZR states map
        inline static std::optional<uint32_t> s_selectedId{ std::nullopt }; // nullptr=none; 0=furniture, else formId

        // Which axis is currently being dragged, and by whom;
        // Used so a background refresh doesn't fight the player's own drag in progress.
        inline static int s_draggingAxis{ -1 };
        inline static uint32_t s_draggingId{ 0 };

        static constexpr float kRangeXYZ = 200.0f;
        static constexpr float kRangeR   = 180.0f;
        static constexpr float kDragScale= 300.0f;
        static constexpr float kRadToDeg = 180.0f / 3.14159265358979f;
    };
}
