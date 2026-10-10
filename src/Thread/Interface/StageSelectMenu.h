#pragma once
#include "Thread/Interface/UI/Window.h"

namespace Thread
{
    class Instance;
}

namespace Registry::Animation
{
    struct Stage;
    struct Scene;
}

namespace Thread::Interface
{
    class StageSelectMenu final : public UI::WindowComponent
    {
      public:
        static StageSelectMenu& GetSingleton();

        bool Register();

        bool OpenStageSelectMenu(RE::TESQuest* a_quest);                   // called by sslThreadModel.GoToStage()
        void SetVisibilitySceneGraph(RE::TESQuest* a_quest, bool a_open);  // called by hotkey iToggleSceneGraph
        void RefreshSceneGraphView(RE::TESQuest* a_quest);                 // syncs the graph's current-node highlight
        void Close();

        [[nodiscard]] bool IsGraphViewOpen() const { return _graphOpen; }
        [[nodiscard]] bool IsChoicePending() const { return _choicePending; }

      private:
        struct ChoiceOption
        {
            const Registry::Animation::Stage* stage;
            std::string prefix;
            std::string label;
        };

        struct GraphNode
        {
            const Registry::Animation::Stage* stage;
            float x{ 0.0f };
            float y{ 0.0f };
            int layer{ 0 };
        };

        struct GraphEdge
        {
            int from;
            int to;
        };

        StageSelectMenu() = default;
        StageSelectMenu(const StageSelectMenu&) = delete;
        StageSelectMenu& operator=(const StageSelectMenu&) = delete;

        static void __stdcall RenderCallback();
        void Render();

        void RenderStageChoiceCard();
        void OnNextBranchSelected(int a_index);

        void BuildSceneGraph(Instance& a_inst);
        void RenderSceneGraphView();

        void ClosePendingChoice();
        void CloseGraphView();
        void HideIfIdle();

        RE::TESQuest* _linkedThread{ nullptr };

        // Branch-choice modal
        bool _choicePending{ false };
        std::vector<ChoiceOption> _choices;
        const Registry::Animation::Stage* _choiceOrigin{ nullptr };
        int _selectedChoiceIndex = 0;
        std::vector<float> _choiceScrollOffsets;
        double _choiceStartTime = 0.0;
        float _choiceTimeLimitSec = 5.0f; // 0.0f disable auto-select timeout

        // Graph view
        bool _graphOpen{ false };
        const Registry::Animation::Scene* _graphScene{ nullptr };
        std::vector<GraphNode> _graphNodes;
        std::vector<GraphEdge> _graphEdges;
        int _graphCurrentIndex{ -1 };
        float _graphZoom{ 1.0f };
        float _graphPanX{ 0.0f };
        float _graphPanY{ 0.0f };
        bool _graphFitPending{ true };
        bool _graphShowLabels{ false };
        float _graphNameScrollOffset{ 0.0f };
    };
}
