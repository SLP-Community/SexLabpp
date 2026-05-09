#pragma once

#include "RaceKey.h"
#include "Sex.h"

namespace Registry
{
    /// @brief Fragments are used to represent important data of an actor in the animation system.
    struct ActorFragment
    {
        static inline constexpr size_t MAX_ACTOR_COUNT = 5;
        static inline constexpr size_t MAX_FRAGMENT_BITS = 11;

        enum Value
        {
            None = 0,

            Male = 1 << 0,
            Female = 1 << 1,

            Human = 1 << 2,
            Vampire = 1 << 3,
            Futa = 1 << 4,
            // Unused = 1 << 5,
            // Unused = 1 << 6,
            // Unused = 1 << 7,
            // Unused = 1 << 8,

            CrtBit0 = 1 << 3,
            CrtBit1 = 1 << 4,
            CrtBit2 = 1 << 5,
            CrtBit3 = 1 << 6,
            CrtBit4 = 1 << 7,
            CrtBit5 = 1 << 8,

            Submissive = 1 << 9,
            Unconscious = 1 << 10,
        };
        using ValueType = std::underlying_type_t<Value>;
        using FragmentHash = std::bitset<MAX_ACTOR_COUNT * MAX_FRAGMENT_BITS>;

      public:
        ActorFragment() = default;
        ActorFragment(REX::EnumSet<Sex> a_sex, RaceKey a_race, float a_scale, bool a_vampire, bool a_submissive, bool a_unconscious);
        ActorFragment(REX::EnumSet<Value> a_value) :
          actor(nullptr), value(a_value) {}
        ActorFragment(RE::Actor* a_actor, bool a_submissive);
        ~ActorFragment() = default;

        _NODISCARD RE::Actor* GetActor() const { return actor; }
        _NODISCARD float GetScale() const { return scale; }
        _NODISCARD RaceKey GetRace() const;
        _NODISCARD REX::EnumSet<Sex> GetSex() const;

        _NODISCARD bool IsValid() const { return value != Value::None; }
        _NODISCARD bool IsAbstract() const { return actor == nullptr; }
        _NODISCARD bool IsHuman() const { return value.any(Human); }
        _NODISCARD bool IsVampire() const { return value.all(Human, Vampire); }
        _NODISCARD bool IsSubmissive() const { return value.any(Submissive); };
        _NODISCARD bool IsUnconscious() const { return value.any(Unconscious); };
        _NODISCARD bool IsSex(Sex a_sex) const { return GetSex().all(a_sex); }
        _NODISCARD bool IsNotSex(Sex a_sex) const { return GetSex().none(a_sex); }

        /// @brief Checks how compatible the input fragment is to fill a position referenced by this fragment.
        /// @param a_fragment The fragment to check compatibility with.
        /// @return An integer representing the compatibility score. A higher score indicates better compatibility. 0 indicates no compatibility.
        _NODISCARD int32_t GetCompatibilityScore(const ActorFragment& a_fragment) const;

        /// @brief Abstract fragments may represent multiple distinct actors, so we need to split them into separate fragments.
        /// @return A vector of fragments, each representing a distinct data instance.
        _NODISCARD std::vector<ActorFragment> Split() const;

      public:
        /// @brief Creates a hash from a vector of ActorFragment objects.
        /// @param a_fragments A vector of ActorFragment objects to be merged into a single hash.
        /// @return A FragmentHash representing the combined attributes of the input fragments.
        /// @note The order of the fragments does not matter, as the hash is designed to be independent of the order of the input fragments.
        static FragmentHash MakeFragmentHash(std::vector<ActorFragment> a_fragments);

        /// @brief QoL function to convert a vector of actors to a vector of ActorFragment objects.
        /// @param a_actors A vector of RE::Actor pointers to be converted.
        /// @param a_submissives A vector of RE::Actor pointers representing submissive actors.
        /// @return A vector of ActorFragment objects representing the input actors.
        static std::vector<ActorFragment> MakeFragmentList(std::vector<RE::Actor*> a_actors, std::vector<RE::Actor*> a_submissives);

      public:
        constexpr bool operator==(const ActorFragment& a_other) const noexcept { return value == a_other.value; }
        constexpr bool operator<(const ActorFragment& a_other) const noexcept { return value < a_other.value; }

      private:
        RE::Actor* actor{ nullptr };
        REX::EnumSet<Value> value{ Value::None };
        float scale{ 1.0f };
    };

}  // namespace Registry
