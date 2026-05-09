#pragma once

#include <random>

template <typename T>
concept Container = requires(T a) {
    typename T::value_type;
    { a.begin() } -> std::input_iterator;
    { a.end() } -> std::input_iterator;
    { a.size() } -> std::convertible_to<size_t>;
};

struct Random
{
    Random() = delete;

    static inline std::mt19937 eng{ std::random_device{}() };

    template <class T>
    static inline T draw(T a_min, T a_max)
    {
        if constexpr (std::is_integral_v<T>)
            return std::uniform_int_distribution<T>{ a_min, a_max }(eng);
        else
            return std::uniform_real_distribution<T>{ a_min, a_max }(eng);
    }

    template <Container T>
    static inline typename T::value_type draw(const T& a_container)
    {
        if (a_container.empty()) {
            throw std::out_of_range("Cannot draw from an empty container.");
        }
        std::uniform_int_distribution<size_t> dist{ 0, a_container.size() - 1 };
        return *(a_container.begin() + dist(eng));
    }

    template <Container V>
    static inline void shuffle(V& a_container)
    {
        std::ranges::shuffle(a_container, eng);
    }

    static inline std::string generateUUID()
    {
        constexpr std::string_view v = "0123456789abcdef";
        constexpr std::string_view templateStr{ "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx" };
        std::uniform_int_distribution<size_t> dist{ 0, v.size() };

        std::string ret{ templateStr };
        for (int i = 0; i < ret.size(); i++) {
            if (ret[i] == 'x') {
                ret[i] = v[dist(eng)];
            }
        }
        return ret;
    }
};
