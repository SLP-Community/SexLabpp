#pragma once
#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interface
{
    class OffsetAdjustPanel final
    {
      public:
        void Open(SceneHUD& a_hud);
        void Close();
        void Render(SceneHUD& a_hud);
        void OnStageChanged(SceneHUD& a_hud);

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
            float baseline{ 0.0f };  // default value
            bool hasBaseline{ false };
            float dragStartValue{ 0.0f };
        };

        void OnActorSelected(SceneHUD& a_hud, const ActorItem& a_item);
        void OnSetOffset(SceneHUD& a_hud, Registry::CoordinateType a_axis, std::uint32_t a_actorId, float a_value);
        void OnResetOffsets(SceneHUD& a_hud);
        void OnSetAdjustStageOnly(SceneHUD& a_hud, bool a_state);

        void RefreshSlots(SceneHUD& a_hud);
        void RefreshValues(SceneHUD& a_hud, std::uint32_t a_actorId);

        // The centered drag slider used for each axis; Returns true if value changed this frame and C++ should be notified.
        bool OffsetTrack(UI::Scale& a_scale, const char* a_id, AxisState& a_state, float a_range, bool& a_draggingOut);

        std::vector<ActorItem> _items;

        bool _hasFurniture{ false };
        bool _adjustStageOnly{ false };
        bool _pickerOpen{ false };
        bool _panelOpen{ false };

        std::unordered_map<std::uint32_t, std::array<AxisState, 4>> _axes;
        std::optional<std::uint32_t> _selectedId;

        // Which axis is currently being dragged, and by whom;
        // Used so a background refresh doesn't fight the player's own drag in progress.
        int _draggingAxis{ -1 };
        std::uint32_t _draggingId{};

        static constexpr float kRangeXYZ = 200.0f;
        static constexpr float kRangeR = 180.0f;
        static constexpr float kDragScale = 300.0f;
        static constexpr float kRadToDeg = 180.0f / 3.14159265358979f;
    };
}
