#pragma once

#include <fstream>
#include <string>
#include <type_traits>
#include <vector>

namespace Decode
{
    static inline constexpr size_t HASH_SIZE = 4;
    static inline constexpr size_t ID_SIZE = 8;

    template <typename I, std::enable_if_t<std::is_integral<I>::value, bool> = true>
    void Read(std::ifstream& a_stream, I& a_out)
    {
        constexpr size_t n = sizeof(I);
        uint8_t buffer[n];
        a_stream.read(reinterpret_cast<char*>(buffer), n);
        a_out = 0;
        for (int32_t i = 0; i < n; i++) {
            a_out = (a_out << 8) | buffer[i];
        }
    }

    template <typename F, std::enable_if_t<std::is_floating_point<F>::value, bool> = true>
    void Read(std::ifstream& a_stream, F& a_out)
    {
        int32_t tmp;
        Read(a_stream, tmp);
        a_out = static_cast<float>(tmp) / 1000.0f;
    }

    template <typename S, std::enable_if_t<std::is_same_v<S, std::string> || std::is_same_v<S, RE::BSFixedString>, bool> = true>
    void Read(std::ifstream& a_stream, S& a_out)
    {
        uint64_t u64;
        Read(a_stream, u64);
        std::vector<char> buffer;
        buffer.resize(u64);
        a_stream.read(buffer.data(), u64);
        a_out = std::string{ buffer.begin(), buffer.end() };
    }

    template <typename T>
    T Read(std::ifstream& a_stream)
    {
        T ret;
        Read(a_stream, ret);
        return ret;
    }

}  // namespace Decode
