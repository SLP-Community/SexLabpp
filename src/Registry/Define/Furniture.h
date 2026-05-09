#pragma once

#include "Transform.h"

namespace Registry
{
    struct FurnitureType
    {
        enum Value : uint32_t
        {
            None = 0,

            BedRoll = 1 << 0,
            BedSingle = 1 << 1,
            BedDouble = 1 << 2,

            Wall = 1 << 3,
            Railing = 1 << 4,

            CraftCookingPot = 1 << 5,
            CraftAlchemy = 1 << 6,
            CraftEnchanting = 1 << 7,
            CraftSmithing = 1 << 8,
            CraftAnvil = 1 << 9,
            CraftWorkbench = 1 << 10,
            CraftGrindstone = 1 << 11,

            Table = 1 << 12,
            TableCounter = 1 << 13,

            Chair = 1 << 14,        // No arm, high back
            ChairCommon = 1 << 15,  // Common chair
            ChairWood = 1 << 16,    // Wooden Chair
            ChairBar = 1 << 17,     // Bar stool
            ChairNoble = 1 << 18,   // Noble Chair
            ChairMisc = 1 << 19,    // Unspecified

            Bench = 1 << 20,       // With back
            BenchNoble = 1 << 21,  // Noble Bench (no back, with arm)
            BenchMisc = 1 << 20,   // No specification on back or arm

            Throne = 1 << 22,
            ThroneRiften = 1 << 23,
            ThroneNordic = 1 << 24,
            // COMEBACK: These might want to be removed?
            XCross = 1 << 25,
            Pillory = 1 << 26,

            All = static_cast<std::underlying_type_t<Value>>(-1)
        };

      public:
        constexpr FurnitureType() = default;
        constexpr FurnitureType(Value a_value) :
          value(a_value) {}
        FurnitureType(const RE::BSFixedString& a_value);

        _NODISCARD RE::BSFixedString ToString() const;
        _NODISCARD constexpr bool Is(FurnitureType::Value a_value) const { return value == a_value; }
        _NODISCARD constexpr bool IsBed() const { return value == Value::BedSingle || value == Value::BedDouble || value == Value::BedRoll; }
        _NODISCARD constexpr bool IsNone() const { return value == Value::None; }

      public:
        _NODISCARD static FurnitureType GetBedType(const RE::TESObjectREFR* a_reference);
        _NODISCARD static bool IsBedType(const RE::TESObjectREFR* a_reference);

        template <Value a_value>
        _NODISCARD static constexpr std::string_view ToString()
        {
            return magic_enum::enum_name<a_value>();
        }

      public:
        _NODISCARD constexpr bool operator==(const FurnitureType& a_rhs) const { return value == a_rhs.value; }
        _NODISCARD constexpr bool operator!=(const FurnitureType& a_rhs) const { return value != a_rhs.value; }
        _NODISCARD constexpr bool operator<(const FurnitureType& a_rhs) const { return value < a_rhs.value; }

        constexpr operator Value() const { return value; }

      public:
        Value value{ Value::None };
    };

    struct FurnitureOffset
    {
        FurnitureType type{ FurnitureType::None };
        Coordinate offset{ 0.0f, 0.0f, 0.0f, 0.0f };
    };

    class FurnitureDetails
    {
      public:
        FurnitureDetails(const YAML::Node& a_node);
        FurnitureDetails(FurnitureType a_type, Coordinate a_coordinate) :
          data({ { a_type, a_coordinate } }) {}
        ~FurnitureDetails() = default;

        std::vector<FurnitureOffset> GetCoordinatesInBound(RE::TESObjectREFR* a_ref, REX::EnumSet<FurnitureType::Value> a_filter) const;
        std::vector<FurnitureOffset> GetClosestCoordinatesInBound(RE::TESObjectREFR* a_ref, REX::EnumSet<FurnitureType::Value> a_filter, RE::TESObjectREFR* a_center) const;

        template <typename T, typename S>
        bool HasType(T a_container, S a_projection) const
        {
            return std::ranges::any_of(data, a_container, [a_projection](auto&& it) {
                FurnitureType type = a_projection(it);
                return HasType(type);
            });
        }
        bool HasType(FurnitureType a_type) const
        {
            return std::ranges::contains(data, a_type, [](auto&& it) { return it.type; });
        }
        REX::EnumSet<FurnitureType::Value> GetTypes() const
        {
            REX::EnumSet<FurnitureType::Value> ret{ FurnitureType::None };
            for (auto&& it : data) {
                ret.set(it.type.value);
            }
            return ret;
        }

      private:
        std::vector<FurnitureOffset> data;
    };

}  // namespace Registry
