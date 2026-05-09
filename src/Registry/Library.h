#pragma once

#include <shared_mutex>

#include "Define/Animation.h"
#include "Define/Expression.h"
#include "Define/Fragment.h"
#include "Define/Furniture.h"
#include "Define/Voice.h"

namespace Registry
{
    class Library : public Singleton<Library>
    {
        static constexpr const char* SCENE_PATH{ CONFIGPATH("Registry") };
        static constexpr const char* SCENE_USER_CONFIG{ USER_CONFIGS("Scenes") };

        static constexpr const char* VOICE_PATH{ CONFIGPATH("Voices\\Voices") };
        static constexpr const char* VOICE_PATH_PITCH{ CONFIGPATH("Voices\\Pitch") };
        static constexpr const char* VOICE_SETTING_PATH{ USER_CONFIGS("Voices.yaml") };
        static constexpr const char* VOICE_SETTINGS_CACHES_PATH{ USER_CONFIGS("Voices_NPC.yaml") };

        static constexpr const char* EXPRESSION_PATH{ USER_CONFIGS("Expressions") };
        static constexpr const char* EXPRESSION_LEGACY_CONFIG{ "Data\\SKSE\\Plugins\\SexLab\\" };

        static constexpr const char* CUM_FX_PATH{ "Data\\Textures\\SexLab\\CumFx\\" };

        static constexpr const char* FURNITURE_PATH{ CONFIGPATH("Furniture") };

      public:
        enum class FxType
        {
            Vaginal = 0,
            Anal,
            Oral,
        };
        constexpr static auto NUM_FX_TYPES = magic_enum::enum_count<FxType>();
        using FxPair = std::pair<RE::BSFixedString, uint8_t>;

      public:
        _NODISCARD std::vector<const Scene*> LookupScenes(const std::vector<RE::Actor*>& a_actors, const std::vector<std::string_view>& tags, const std::vector<RE::Actor*>& a_submissives) const;
        _NODISCARD std::vector<const Scene*> GetByTags(int32_t a_positions, const std::vector<std::string_view>& a_tags) const;

        _NODISCARD const AnimPackage* GetPackageFromScene(const Scene* a_scene) const;
        _NODISCARD const Scene* GetSceneById(const RE::BSFixedString& a_id) const;
        _NODISCARD const Scene* GetSceneByName(const RE::BSFixedString& a_id) const;
        _NODISCARD size_t GetSceneCount() const;

        bool EditScene(const RE::BSFixedString& a_id, const std::function<void(Scene*)>& a_func);
        void EditScene(const Registry::Scene* a_scene, const std::function<void(Scene*)>& a_func);
        bool ForEachPackage(std::function<bool(const AnimPackage*)> a_visitor) const;
        bool ForEachScene(std::function<bool(const Scene*)> a_visitor) const;

      public:
        std::vector<RE::BSFixedString> GetAllVoiceIds(RaceKey a_race) const;
        bool ForEachVoice(std::function<bool(const Voice&)> a_visitor) const;
        const Voice* GetVoice(RE::Actor* a_actor, const TagDetails& a_tags);
        const Voice* GetVoice(const TagDetails& a_tags) const;
        const Voice* GetVoice(RaceKey a_race) const;
        const Voice* GetVoiceById(RE::BSFixedString a_voice) const;
        bool CreateVoice(RE::BSFixedString a_voice);
        void WriteVoiceToFile(RE::BSFixedString a_voice) const;

        std::vector<RE::Actor*> GetSavedActors() const;
        const Voice* GetSavedVoice(RE::FormID a_key) const;
        void SaveVoice(RE::FormID a_key, RE::BSFixedString a_voice);
        void ClearVoice(RE::FormID a_key);

        RE::TESSound* PickSound(RE::BSFixedString a_voice, LegacyVoice a_legacysetting) const;
        RE::TESSound* PickSound(RE::BSFixedString a_voice, uint32_t a_excitement, REX::EnumSet<VoiceAnnotation> a_annotation) const;
        RE::TESSound* PickOrgasmSound(RE::BSFixedString a_voice, REX::EnumSet<VoiceAnnotation> a_annotation) const;

        void SetVoiceEnabled(RE::BSFixedString a_voice, bool a_enabled);
        void SetVoiceSound(RE::BSFixedString a_voice, LegacyVoice a_legacysetting, RE::TESSound* a_sound);
        void SetVoiceTags(RE::BSFixedString a_voice, const std::vector<RE::BSFixedString>& a_tags);
        void SetVoiceRace(RE::BSFixedString a_voice, const std::vector<RaceKey>& a_races);
        void SetVoiceSex(RE::BSFixedString a_voice, RE::SEXES::SEX a_sex);

      public:
        _NODISCARD const Expression* GetExpressionById(const RE::BSFixedString& a_id) const;
        _NODISCARD const Expression* GetExpression(const TagDetails& a_details) const;
        bool ForEachExpression(std::function<bool(const Expression&)> a_visitor) const;
        bool CreateExpression(const RE::BSFixedString& a_id);

        void UpdateExpressionValues(RE::BSFixedString a_id, bool a_female, int a_level, std::vector<float> a_values);
        void UpdateExpressionTags(RE::BSFixedString a_id, const TagData& a_newtags);
        void SetExpressionScaling(RE::BSFixedString a_id, Expression::Scaling a_scaling);
        void SetExpressionEnabled(RE::BSFixedString a_id, bool a_enabled);

      public:
        RE::BSFixedString PickRandomFxSet(FxType a_type) const;
        uint8_t GetFxCount(FxType a_type, RE::BSFixedString a_set) const;

      public:
        _NODISCARD const FurnitureDetails* GetFurnitureDetails(const RE::TESObjectREFR* a_ref) const;

      public:
        void Initialize() noexcept;
        void Save() const noexcept;

      private:
        bool FolderExists(const char* path, bool notifyUser) const noexcept;
        void InitializeScenes() noexcept;
        void InitializeSceneSettings() noexcept;
        void InitializeFurnitures() noexcept;
        void InitializeExpressions() noexcept;
        void InitializeExpressionsImpl() noexcept;
        void InitializeExpressionsLegacy() noexcept;
        void InitializeVoice() noexcept;
        void InitializeVoiceImpl() noexcept;
        void InitializeVoicePitches() noexcept;
        void InitializeVoiceSettings() noexcept;
        void InitializeVoiceCache() noexcept;
        void InitializeCumFx() noexcept;
        uint8_t InitializeCumFxType(const fs::directory_entry& a_typePath) const noexcept;

        void SaveScenes() const noexcept;
        void SaveExpressions() const noexcept;
        void SaveVoices() const noexcept;

      private:
        mutable std::shared_mutex _mScenes{};
        std::vector<std::unique_ptr<AnimPackage>> packages;
        std::map<RE::BSFixedString, Scene*, FixedStringCompare> sceneMap;             // SceneId -> Scene
        std::unordered_map<ActorFragment::FragmentHash, std::vector<Scene*>> scenes;  // Hashes -> Scenes

        mutable std::shared_mutex _mVoice{};
        std::map<RE::BSFixedString, Voice, FixedStringCompare> voices{};
        std::map<RE::FormID, VoicePitch> savedPitches{};   // VoiceType -> Pitch (or fixed Voice)
        std::map<RE::FormID, const Voice*> savedVoices{};  // NPC -> Voice

        mutable std::shared_mutex _mExpressions{};
        std::map<RE::BSFixedString, Expression, FixedStringCompare> expressions{};

        mutable std::shared_mutex _mCumFx{};
        std::array<std::vector<FxPair>, NUM_FX_TYPES> fxList;

        mutable std::shared_mutex _mFurniture{};
        FurnitureDetails offsetDefaultBedroll{ FurnitureType::BedRoll, Coordinate(std::vector{ 0.0f, 0.0f, 7.5f, 180.0f }) };
        FurnitureDetails offsetDefaultBedsingle{ FurnitureType::BedSingle, Coordinate(std::vector{ 0.0f, -31.0f, 42.5f, 0.0f }) };
        FurnitureDetails offsetDefaultBeddouble{ FurnitureType::BedDouble, Coordinate(std::vector{ 0.0f, -31.0f, 42.5f, 0.0f }) };
        std::map<RE::BSFixedString, std::unique_ptr<FurnitureDetails>, FixedStringCompare> furnitures;
    };
}
