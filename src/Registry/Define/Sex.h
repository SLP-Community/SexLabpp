#pragma once

namespace Registry
{
    enum class PapyrusSex
    {
        None = 0,

        Male = 1 << 0,
        Female = 1 << 1,
        Futa = 1 << 2,
        CrtMale = 1 << 3,
        CrtFemale = 1 << 4,
    };

    enum class Sex : uint8_t
    {
        None = 0,

        Male = 1 << 0,
        Female = 1 << 1,
        Futa = 1 << 2,
    };

    enum class Sexuality : uint8_t
    {
        Hetero = 0,
        Homo = 1,
        Bi = 2,

        None
    };

    /// @brief Get the (1 dimensional) sex for this actor
    Sex GetSex(RE::Actor* a_actor, bool a_skipfactions = false);
    /// @brief If this (female) actor is a futa
    bool IsFuta(RE::Actor* a_actor);

}  // namespace Registry
