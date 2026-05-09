#pragma once

namespace Registry
{
    enum class Tag : uint64_t
    {
        SixtyNine = 1ULL << 0,
        Anal = 1ULL << 1,
        Asphyxiation = 1ULL << 2,
        Blowjob = 1ULL << 3,
        Boobjob = 1ULL << 4,
        BreastSucking = 1ULL << 5,
        Buttjob = 1ULL << 6,
        Cowgirl = 1ULL << 7,
        Cunnilingus = 1ULL << 8,
        Deepthroat = 1ULL << 9,
        Doggy = 1ULL << 10,
        Dominant = 1ULL << 11,
        DoublePenetration = 1ULL << 12,
        FaceSitting = 1ULL << 13,
        Facial = 1ULL << 14,
        Feet = 1ULL << 15,
        Fingering = 1ULL << 16,
        Fisting = 1ULL << 17,
        Footjob = 1ULL << 18,
        Forced = 1ULL << 19,
        Grinding = 1ULL << 20,
        Handjob = 1ULL << 21,
        Humiliation = 1ULL << 22,
        LeadIn = 1ULL << 23,
        LotusPosition = 1ULL << 24,
        Masturbation = 1ULL << 25,
        Missionary = 1ULL << 26,
        Oral = 1ULL << 27,
        Penetration = 1ULL << 28,
        ProneBone = 1ULL << 29,
        ReverseCowgirl = 1ULL << 30,
        ReverseSpitroast = 1ULL << 31,
        Rimming = 1ULL << 32,
        Spanking = 1ULL << 33,
        Spitroast = 1ULL << 34,
        Teasing = 1ULL << 35,
        Toys = 1ULL << 36,
        Tribadism = 1ULL << 37,
        TriplePenetration = 1ULL << 38,
        Vaginal = 1ULL << 39,

        Behind = 1ULL << 40,
        Facing = 1ULL << 41,
        Holding = 1ULL << 42,
        Hugging = 1ULL << 43,
        Kissing = 1ULL << 44,
        Kneeling = 1ULL << 45,
        Loving = 1ULL << 46,
        Lying = 1ULL << 47,
        Magic = 1ULL << 48,
        Sitting = 1ULL << 49,
        Spooning = 1ULL << 50,
        Standing = 1ULL << 51,

        Ryona = 1ULL << 52,
        Gore = 1ULL << 53,
        Oviposition = 1ULL << 54,
    };

    class TagData
    {
      public:
        template <class T>
        TagData(const std::vector<T>& a_tags)
        {
            for (auto&& it : a_tags) {
                AddTag(it);
            }
        }
        TagData(std::ifstream& a_stream);
        TagData() = default;
        ~TagData() = default;

      public:
        /// @brief Add (all of) the arguments tags to this
        void AddTag(Tag a_tag);
        void AddTag(const TagData& a_tag);
        void AddTag(RE::BSFixedString a_tag);

        /// @brief Remove (all of) the arguments tags from this
        void RemoveTag(Tag a_tag);
        void RemoveTag(const TagData& a_tag);
        void RemoveTag(const RE::BSFixedString& a_tag);

        void IntersectTags(const TagData& a_tags);

        /// @brief If this has (all of) the arguments tags
        _NODISCARD bool HasTag(Tag a_tag) const;
        _NODISCARD bool HasTag(const RE::BSFixedString& a_tag) const;

        /// @brief Checks if this has any or all of the arguments tags
        _NODISCARD bool HasTags(const TagData& a_tag, bool a_all) const;
        _NODISCARD uint32_t CountTags(const TagData& a_tag) const;

        /// @brief If this data contains any tags
        _NODISCARD bool IsEmpty() const;

      public:
        bool HasAnnotation(const RE::BSFixedString& a_tag) const;
        void AddAnnotation(RE::BSFixedString a_tag);
        void RemoveAnnotation(const RE::BSFixedString& a_tag);
        void SetAnnotations(const std::vector<RE::BSFixedString>& a_tags) { _annotations = a_tags; }

        /// @brief Get the annotated (editable) tags
        std::vector<RE::BSFixedString>& GetAnnotations() { return _annotations; }
        const std::vector<RE::BSFixedString>& GetAnnotations() const { return _annotations; }

      public:
        /// @brief visitor returns true to stop cycling
        void ForEachExtra(std::function<bool(const std::string_view)> a_visitor) const;

        /// @brief get all tags in this data in a single vector
        std::vector<RE::BSFixedString> AsVector() const;

      private:
        void AddExtraTag(const RE::BSFixedString& a_tag);
        void RemoveExtraTag(const RE::BSFixedString& a_tag);
        bool HasExtraTag(const RE::BSFixedString& a_tag) const;

        stl::enumeration<Tag> _basetags;
        std::vector<RE::BSFixedString> _extratags;
        std::vector<RE::BSFixedString> _annotations;
    };

    class TagDetails
    {
      public:
        enum TagType
        {
            Required = 0,
            Disallow,
            Optional,

            Total
        };

      public:
        TagDetails(const std::string_view a_tags);
        TagDetails(const std::vector<std::string_view> a_tags);
        TagDetails(const std::array<TagData, TagType::Total> a_tags);
        ~TagDetails() = default;

        /// @brief If the given tag data matches all of the this's tags
        _NODISCARD bool MatchTags(const TagData& a_data) const;

      private:
        TagData _tags[TagType::Total];
    };

}  // namespace Registry
