#pragma once

namespace Util
{
    static inline RE::FormID FormFromString(const std::string_view& a_string, int base)
    {
        const auto split = a_string.find('|');
        const auto formIdStr = a_string.substr(0, split);
        RE::FormID formid;
        const auto offset = a_string.starts_with("0x") ? 2 : 0;
        const auto [ptr, res] = std::from_chars(
            formIdStr.data() + offset,
            formIdStr.data() + formIdStr.size(),
            formid, base);
        if (res != std::errc()) {
            // logger::error("Invalid form ID: {} for base: {}", formIdStr, base);
            return 0;
        } else if (split == std::string_view::npos) {
            return formid;
        } else {
            return RE::TESDataHandler::GetSingleton()->LookupFormID(formid, a_string.substr(split + 1));
        }
    }

    static inline RE::FormID FormFromString(const std::string_view& a_string)
    {
        const auto base = a_string.starts_with("0x") ? 16 : 10;
        return FormFromString(a_string, base);
    }

    template <typename T, typename = std::enable_if_t<!std::is_pointer_v<T>>>
    static inline T* FormFromString(const std::string_view& a_string, int base)
    {
        const auto id = FormFromString(a_string, base);
        return id == 0 ? nullptr : RE::TESForm::LookupByID<T>(id);
    }

    template <typename T, typename = std::enable_if_t<!std::is_pointer_v<T>>>
    static inline T* FormFromString(const std::string_view& a_string)
    {
        const auto base = a_string.starts_with("0x") ? 16 : 10;
        return FormFromString<T>(a_string, base);
    }

    template <typename T, typename = std::enable_if_t<std::is_pointer<T>::value>>
    static inline T FormFromString(const std::string_view& a_string, int base)
    {
        using U = std::remove_pointer<T>::type;
        return FormFromString<U>(a_string, base);
    }

    template <typename T, typename = std::enable_if_t<std::is_pointer<T>::value>>
    static inline T FormFromString(const std::string_view& a_string)
    {
        using U = std::remove_pointer<T>::type;
        return FormFromString<U>(a_string);
    }

    static inline std::string FormToString(RE::TESForm* a_form)
    {
        auto file = a_form->GetFile(0);
        if (!file) {
            return std::format("0x{:X}", a_form->GetFormID());
        } else {
            return std::format("0x{:X}|{}", a_form->GetLocalFormID(), file->GetFilename());
        }
    }

}  // namespace Util
