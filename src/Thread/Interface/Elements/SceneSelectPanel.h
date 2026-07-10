#pragma once
#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interface
{
    class SceneSelectPanel
    {
      public:
        static void Init();
        static void Destroy();
        static void __stdcall Render();

        static void RebuildEntries();
        static void RebuildFilter();

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

        static void OnSceneSelected(const std::string& sceneId);
        static void OnConfirmSearch();
        static void OnAnnotationSave(SceneEntry& e);

        static bool MatchesFilter(const SceneEntry& e, const std::string& filter);
        
        inline static bool isVisible{ false };
        inline static std::vector<SceneEntry> s_entries;
        inline static char s_searchBuf[128]{};
        inline static int s_hoveredIdx{ -1 };

        inline static char s_lastSearch[128]{};
        inline static std::vector<int> s_filteredIdx;

        inline static bool s_sceneListOpen{ true };
        inline static bool s_searchBoxOpen{ true };
    };
}
