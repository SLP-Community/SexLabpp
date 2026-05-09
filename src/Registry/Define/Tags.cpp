#include "Tags.h"

#include "Registry/Util/Decode.h"
#include "Util/StringUtil.h"

namespace Registry
{
#define MAPENTRY(value) \
    {                   \
        #value, value   \
    }

    using enum Tag;
    static inline const std::map<RE::BSFixedString, Tag, FixedStringCompare> TagTable = {
        MAPENTRY(SixtyNine),
        MAPENTRY(Anal),
        MAPENTRY(Asphyxiation),
        MAPENTRY(Blowjob),
        MAPENTRY(Boobjob),
        MAPENTRY(BreastSucking),
        MAPENTRY(Buttjob),
        MAPENTRY(Cowgirl),
        MAPENTRY(Cunnilingus),
        MAPENTRY(Deepthroat),
        MAPENTRY(Doggy),
        MAPENTRY(Dominant),
        MAPENTRY(DoublePenetration),
        MAPENTRY(FaceSitting),
        MAPENTRY(Facial),
        MAPENTRY(Feet),
        MAPENTRY(Fingering),
        MAPENTRY(Fisting),
        MAPENTRY(Footjob),
        MAPENTRY(Forced),
        MAPENTRY(Grinding),
        MAPENTRY(Handjob),
        MAPENTRY(Humiliation),
        MAPENTRY(LeadIn),
        MAPENTRY(LotusPosition),
        MAPENTRY(Masturbation),
        MAPENTRY(Missionary),
        MAPENTRY(Oral),
        MAPENTRY(Penetration),
        MAPENTRY(ProneBone),
        MAPENTRY(ReverseCowgirl),
        MAPENTRY(ReverseSpitroast),
        MAPENTRY(Rimming),
        MAPENTRY(Spanking),
        MAPENTRY(Spitroast),
        MAPENTRY(Teasing),
        MAPENTRY(Toys),
        MAPENTRY(Tribadism),
        MAPENTRY(TriplePenetration),
        MAPENTRY(Vaginal),

        MAPENTRY(Holding),
        MAPENTRY(Spooning),
        MAPENTRY(Hugging),
        MAPENTRY(Kissing),
        MAPENTRY(Sitting),
        MAPENTRY(Kneeling),
        MAPENTRY(Standing),
        MAPENTRY(Loving),
        MAPENTRY(Lying),
        MAPENTRY(Behind),
        MAPENTRY(Facing),
        MAPENTRY(Magic),

        MAPENTRY(Ryona),
        MAPENTRY(Gore),
        MAPENTRY(Oviposition)
    };

#undef MAPENTRY

    TagData::TagData(std::ifstream& a_stream)
    {
        uint64_t tag_count;
        Decode::Read(a_stream, tag_count);
        for (size_t j = 0; j < tag_count; j++) {
            RE::BSFixedString tag;
            Decode::Read(a_stream, tag);
            AddTag(tag);
        }
    }

    void TagData::AddTag(Tag a_tag)
    {
        _basetags.set(a_tag);
    }

    void TagData::AddTag(const TagData& a_tag)
    {
        _basetags.set(a_tag._basetags.get());
        _extratags.reserve(_extratags.size() + a_tag._extratags.size());
        _annotations.reserve(_annotations.size() + a_tag._annotations.size());
        for (auto&& tag : a_tag._extratags) {
            AddExtraTag(tag);
        }
        for (auto&& tag : a_tag._annotations) {
            AddAnnotation(tag);
        }
    }

    void TagData::AddTag(RE::BSFixedString a_tag)
    {
        const auto where = TagTable.find(a_tag);
        if (where != TagTable.end()) {
            _basetags.set(where->second);
        } else {
            AddExtraTag(a_tag);
        }
    }

    void TagData::RemoveTag(Tag a_tag)
    {
        _basetags.reset(a_tag);
    }

    void TagData::RemoveTag(const TagData& a_tag)
    {
        _basetags.reset(a_tag._basetags.get());
        for (auto&& tag : a_tag._extratags) {
            RemoveExtraTag(tag);
        }
    }

    void TagData::RemoveTag(const RE::BSFixedString& a_tag)
    {
        const auto where = TagTable.find(a_tag);
        if (where != TagTable.end()) {
            _basetags.reset(where->second);
        } else {
            RemoveExtraTag(a_tag);
        }
    }

    void TagData::IntersectTags(const TagData& a_tags)
    {
        _basetags.set((_basetags & a_tags._basetags).get());
        std::erase_if(_extratags, [&](const auto& tag) { return !a_tags.HasTag(tag); });
        std::erase_if(_annotations, [&](const auto& tag) { return !a_tags.HasTag(tag); });
    }

    void TagData::RemoveAnnotation(const RE::BSFixedString& a_tag)
    {
        const auto where = std::find(_annotations.begin(), _annotations.end(), a_tag);
        if (where == _annotations.end())
            return;
        _annotations.erase(where);
    }

    bool TagData::HasTag(Tag a_tag) const
    {
        return _basetags.all(a_tag);
    }

    bool TagData::HasTag(const RE::BSFixedString& a_tag) const
    {
        const auto where = TagTable.find(a_tag);
        if (where != TagTable.end()) {
            return _basetags.all(where->second);
        }
        return HasExtraTag(a_tag) || HasAnnotation(a_tag);
    }

    bool TagData::HasTags(const TagData& a_tag, bool a_all) const
    {
        if (a_all) {
            return _basetags.all(a_tag._basetags.get()) &&
                   std::ranges::all_of(a_tag._extratags, [&](const auto& tag) { return HasTag(tag); }) &&
                   std::ranges::all_of(a_tag._annotations, [&](const auto& tag) { return HasTag(tag); });
        } else {
            return a_tag.IsEmpty() ||
                   _basetags.any(a_tag._basetags.get()) ||
                   std::ranges::any_of(a_tag._extratags, [&](const auto& tag) { return HasTag(tag); }) ||
                   std::ranges::any_of(a_tag._annotations, [&](const auto& tag) { return HasTag(tag); });
        }
    }

    uint32_t TagData::CountTags(const TagData& a_tag) const
    {
        return std::popcount((a_tag._basetags & _basetags).underlying()) +
               static_cast<uint32_t>(std::ranges::count_if(_extratags, [&](const auto& tag) { return a_tag.HasTag(tag); })) +
               static_cast<uint32_t>(std::ranges::count_if(_annotations, [&](const auto& tag) { return a_tag.HasTag(tag); }));
    }

    bool TagData::IsEmpty() const
    {
        return _basetags.underlying() == 0 && _extratags.empty() && _annotations.empty();
    }

    bool TagData::HasAnnotation(const RE::BSFixedString& a_tag) const
    {
        return std::find(_annotations.begin(), _annotations.end(), a_tag) != _annotations.end();
    }

    void TagData::AddAnnotation(RE::BSFixedString a_tag)
    {
        if (std::find(_annotations.begin(), _annotations.end(), a_tag) != _annotations.end())
            return;
        _annotations.push_back(a_tag);
    }

    void TagData::ForEachExtra(std::function<bool(const std::string_view)> a_visitor) const
    {
        for (auto&& tag : _extratags) {
            if (a_visitor(tag.data()))
                return;
        }
    }

    std::vector<RE::BSFixedString> TagData::AsVector() const
    {
        std::vector<RE::BSFixedString> ret{ _extratags.begin(), _extratags.end() };
        for (auto&& [tag_str, tag] : TagTable)
            if (_basetags.all(tag))
                ret.push_back(tag_str);

        return ret;
    }

    void TagData::AddExtraTag(const RE::BSFixedString& a_tag)
    {
        if (HasExtraTag(a_tag))
            return;
        _extratags.push_back(a_tag);
    }

    void TagData::RemoveExtraTag(const RE::BSFixedString& a_tag)
    {
        const auto where = std::find(_extratags.begin(), _extratags.end(), a_tag);
        if (where == _extratags.end())
            return;
        _extratags.erase(where);
    }

    bool TagData::HasExtraTag(const RE::BSFixedString& a_tag) const
    {
        return std::find(_extratags.begin(), _extratags.end(), a_tag) != _extratags.end();
    }

    TagDetails::TagDetails(const std::string_view a_tags) :
      TagDetails(Util::StringSplit(a_tags, ",")) {}

    TagDetails::TagDetails(const std::vector<std::string_view> a_tags)
    {
        for (auto&& tag : a_tags) {
            if (tag.empty())
                continue;

            switch (tag[0]) {
            case '!':  // Scene Meta for Papyrus, ignore
                continue;
            case '~':
                _tags[TagType::Optional].AddTag(std::string(tag.substr(1)));
                break;
            case '-':
                _tags[TagType::Disallow].AddTag(std::string(tag.substr(1)));
                break;
            default:
                _tags[TagType::Required].AddTag(std::string(tag));
                break;
            }
        }
    }

    TagDetails::TagDetails(const std::array<TagData, TagType::Total> a_tags)
    {
        for (size_t i = 0; i < TagType::Total; i++)
            _tags[i] = a_tags[i];
    }

    bool TagDetails::MatchTags(const TagData& a_data) const
    {
        if (!_tags[TagType::Disallow].IsEmpty() && a_data.HasTags(_tags[TagType::Disallow], false))
            return false;
        if (!_tags[TagType::Optional].IsEmpty() && !a_data.HasTags(_tags[TagType::Optional], false))
            return false;
        return _tags[TagType::Required].IsEmpty() || a_data.HasTags(_tags[TagType::Required], true);
    }

}