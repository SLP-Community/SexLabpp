#pragma once
#include "Thread/Interface/UI/Window.h"

namespace Thread::Interface
{
    class SceneSelectPanel final : public UI::WindowComponent, public UI::Panel
    {
      public:
        static SceneSelectPanel& GetSingleton();

        bool Register();
        void Open() override;
        void Close() override;
        void RebuildEntries();
        void RebuildFilter();

      private:
        SceneSelectPanel() = default;

        struct SceneEntry
        {
            std::string id;
            std::string name;
            std::string packageName;
            std::string author;
            std::string tags;
            std::string annotations;
            bool isActive{ false };
            char annotBuf[256]{};
        };

        static void __stdcall RenderCallback();
        void Render();
        void OnSceneSelected(const std::string& a_sceneId);
        void OnConfirmSearch();
        void OnAnnotationSave(SceneEntry& a_entry);

        static bool MatchesFilter(const SceneEntry& a_entry, std::string_view a_filter);

        std::vector<SceneEntry> _entries;
        char _searchBuffer[128]{};
        int _hoveredIndex{ -1 };

        char _lastSearch[128]{};
        std::vector<int> _filteredIndices;

        bool _sceneListOpen{ true };
        bool _searchBoxOpen{ true };
    };
}
