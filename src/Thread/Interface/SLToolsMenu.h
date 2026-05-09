#pragma once
#include "PrismaUtil.h"

namespace Thread::PrismaUI
{
    class SLToolsMenu
    {
      public:
        static bool Initialize();
        static void Open(Script::ObjectPtr a_scriptObj, RE::BSFixedString activeSceneName, const std::vector<RE::BSFixedString>& playingScenesNames);

      private:
        static inline constexpr std::string_view FILEPATH{ "SexLab\\SLToolsMenu.html" };
        static inline PrismaView view{ 0 };
        static inline std::array<std::string, 2> s_result;
        static inline Script::ObjectPtr s_threadScript{ nullptr };
        static void HandleSelection(std::string s_select);
    };
}