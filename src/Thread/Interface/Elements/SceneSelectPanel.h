#pragma once
namespace Thread::Interface
{
    class SceneHUD;

    class SceneSelectPanel final
    {
      public:
        void Open(SceneHUD& a_hud);
        void Close();
        void Render(SceneHUD& a_hud);
        void RebuildEntries(SceneHUD& a_hud);
        void RebuildFilter();

      private:
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

        void OnSceneSelected(SceneHUD& a_hud, const std::string& a_sceneId);
        void OnConfirmSearch(SceneHUD& a_hud);
        void OnAnnotationSave(SceneEntry& a_entry);

        static bool MatchesFilter(const SceneEntry& a_entry, std::string_view a_filter);

        std::vector<SceneEntry> _entries;
        char _searchBuffer[128]{};
        int _hoveredIndex{ -1 };
        float _infoCardY{ 0.0f };

        char _lastSearch[128]{};
        std::vector<int> _filteredIndices;

        bool _sceneListOpen{ true };
        bool _searchBoxOpen{ true };
    };
}
