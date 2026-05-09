#pragma once

namespace UserData
{
    enum class Strip
    {
        NoStrip = -1,
        Always = 1,

        None = 0
    };

    class StripData :
      public Singleton<StripData>
    {
      public:
        Strip CheckStrip(RE::TESForm* a_form);
        Strip CheckKeywords(RE::TESForm* a_form);

      public:
        void Load();
        void Save();

        void AddArmor(RE::TESForm* a_form, Strip a_type) { strips[a_form->formID] = a_type; }
        void RemoveArmor(RE::TESForm* a_form) { strips.erase(a_form->formID); }
        void RemoveArmorAll() { strips.clear(); }

      private:
        std::map<RE::FormID, Strip> strips{};
        YAML::Node _root;
    };
}  // namespace UserData
