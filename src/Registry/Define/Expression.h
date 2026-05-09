#pragma once

#include "Registry/Define/Tags.h"

namespace Registry
{
    struct Expression
    {
        enum class DefaultExpression
        {
            Afraid,
            Angry,
            Happy,
            Joy,
            Pained,
            Pleasure,
            Sad,
            Shy
        };

        enum class Scaling
        {
            Linear,
            Square,
            Cubic,
            Exponential,

            Total
        };

        enum ValueType
        {
            Phoneme = 0,
            Modifier = 16,
            MoodType = 30,
            MoodValue = 31,

            Total = 32
        };

      public:
        Expression(const RE::BSFixedString& a_id) :
          id(a_id) { assert(!a_id.empty()); };
        Expression(DefaultExpression a_default);
        Expression(const YAML::Node& a_src);
        Expression(const nlohmann::json& a_src);
        ~Expression() = default;

        bool IsEnabled() const { return enabled; }
        RE::BSFixedString GetId() const { return id; }
        RE::BSFixedString GetName() const { return id; }
        const TagData& GetTags() const { return tags; }
        std::array<float, Total> GetData(RE::SEXES::SEX a_sex, float a_strength) const;

        void UpdateValues(bool a_female, int a_level, std::vector<float> a_values);
        void UpdateTags(const TagData& a_newtags);
        void SetScaling(Expression::Scaling a_scaling);
        void SetEnabled(bool a_enabled);

        void Save(std::string_view a_fileLocation, bool force) const;

      public:
        RE::BSFixedString id;
        uint8_t version{ 0 };
        bool enabled{ true };
        mutable bool has_edits{ false };

        TagData tags{};
        Scaling scaling{ Scaling::Linear };
        std::vector<std::array<float, Total>> data[RE::SEXES::kTotal]{};
    };

}  // namespace Registry
