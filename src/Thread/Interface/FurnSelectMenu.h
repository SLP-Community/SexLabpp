#pragma once
#include "Thread/Interface/SceneHUD.h"

namespace Thread::Interface
{
    class FurnSelectMenu
    {
      public:
        struct Item {
            std::string name;
            std::string value;
        };

        static bool Register();
        static void Open(RE::TESQuest* a_qst, const std::vector<Item>& a_items);

      private:
        static void __stdcall Render();
        static void HandleSelection(size_t index);

        inline static MENU_WINDOW window{ nullptr };
        inline static RE::TESQuest* s_linkedThread{ nullptr };
        inline static std::vector<Item> s_items{};
    };
}
