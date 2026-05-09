#include "Furniture.h"

#include "Registry/Util/RayCast.h"
#include "Registry/Util/RayCast/ObjectBound.h"
#include "Util/StringUtil.h"

namespace Registry
{

#define MAPENTRY(value)                     \
    {                                       \
        #value, FurnitureType::Value::value \
    }

    static inline const std::map<RE::BSFixedString, FurnitureType::Value, FixedStringCompare> FurniTable = {
        MAPENTRY(BedRoll),
        MAPENTRY(BedSingle),
        MAPENTRY(BedDouble),
        MAPENTRY(Wall),
        MAPENTRY(Railing),
        MAPENTRY(CraftCookingPot),
        MAPENTRY(CraftAlchemy),
        MAPENTRY(CraftEnchanting),
        MAPENTRY(CraftSmithing),
        MAPENTRY(CraftAnvil),
        MAPENTRY(CraftWorkbench),
        MAPENTRY(CraftGrindstone),
        MAPENTRY(Table),
        MAPENTRY(TableCounter),
        MAPENTRY(Chair),
        MAPENTRY(ChairCommon),
        MAPENTRY(ChairWood),
        MAPENTRY(ChairBar),
        MAPENTRY(ChairNoble),
        MAPENTRY(ChairMisc),
        MAPENTRY(Bench),
        MAPENTRY(BenchNoble),
        MAPENTRY(BenchMisc),
        MAPENTRY(Throne),
        MAPENTRY(ThroneRiften),
        MAPENTRY(ThroneNordic),
        MAPENTRY(XCross),
        MAPENTRY(Pillory),
    };

#undef MAPENTRY

    FurnitureType::FurnitureType(const RE::BSFixedString& a_value)
    {
        const auto where = FurniTable.find(a_value);
        if (where != FurniTable.end()) {
            value = where->second;
        } else {
            logger::error("Unrecognized Furniture: {}", a_value.c_str());
            value = Value::None;
        }
    }

    RE::BSFixedString FurnitureType::ToString() const
    {
        for (const auto& [key, type] : FurniTable) {
            if (type == this->value) {
                return key;
            }
        }
        return "";
    }

    FurnitureType FurnitureType::GetBedType(const RE::TESObjectREFR* a_reference)
    {
        if (a_reference->HasKeyword(GameForms::FurnitureBedRoll)) {
            return FurnitureType::BedRoll;
        }
        if (std::string name{ a_reference->GetName() }; name.empty() || Util::CastLower(name).find("bed") == std::string::npos)
            return FurnitureType::None;
        const auto root = a_reference->Get3D();
        const auto extra = root ? root->GetExtraData("FRN") : nullptr;
        const auto node = extra ? netimmerse_cast<RE::BSFurnitureMarkerNode*>(extra) : nullptr;
        if (!node) {
            return FurnitureType::None;
        }

        size_t sleepmarkers = 0;
        for (auto&& marker : node->markers) {
            if (marker.animationType.all(RE::BSFurnitureMarker::AnimationType::kSleep)) {
                sleepmarkers += 1;
            }
        }
        switch (sleepmarkers) {
        case 0:
            return FurnitureType::None;
        case 1:
            return FurnitureType::BedSingle;
        default:
            return FurnitureType::BedDouble;
        }
    }

    bool FurnitureType::IsBedType(const RE::TESObjectREFR* a_reference)
    {
        return GetBedType(a_reference).IsBed();
    }

    FurnitureDetails::FurnitureDetails(const YAML::Node& a_node)
    {
        const auto parse_node = [&](const YAML::Node& it) {
            const auto typenode = it["Type"];
            if (!typenode.IsDefined()) {
                logger::error("Missing 'Type' for node {}", a_node.Mark());
                return;
            }
            const auto typestr = typenode.as<std::string>();
            const FurnitureType furniture{ typestr };
            if (furniture.IsNone()) {
                logger::error("Unrecognized Furniture: '{}' {}", typestr, typenode.Mark());
                return;
            }
            const auto offsetnode = it["Offset"];
            if (!offsetnode.IsDefined() || !offsetnode.IsSequence()) {
                logger::error("Missing 'Offset' node for type '{}' {}", typestr, typenode.Mark());
                return;
            }
            const auto dataSize = data.size();
            const auto push = [&](const YAML::Node& node) {
                const auto vec = node.as<std::vector<float>>();
                if (vec.size() != 4) {
                    logger::error("Invalid offset size. Expected 4 but got {} {}", vec.size(), node.Mark());
                    return;
                }
                Coordinate coords{ vec };
                coords.rotation = glm::radians(coords.rotation);
                data.emplace_back(furniture, coords);
            };
            if (offsetnode[0].IsSequence()) {
                for (auto&& offset : offsetnode) {
                    push(offset);
                }
            } else {
                push(offsetnode);
            }
            if (dataSize == data.size()) {
                logger::error("Type '{}' is defined but has no valid offsets {}", typestr, typenode.Mark());
            }
        };
        if (a_node.IsSequence()) {
            for (auto&& it : a_node) {
                parse_node(it);
            }
        } else {
            parse_node(a_node);
        }
    }

    std::vector<FurnitureOffset> FurnitureDetails::GetCoordinatesInBound(RE::TESObjectREFR* a_ref, REX::EnumSet<FurnitureType::Value> a_filter) const
    {
        if (a_ref->GetAngleX() > Settings::fFurnitureTiltTolerance || a_ref->GetAngleY() > Settings::fFurnitureTiltTolerance) {
            logger::error("GetCoordinatesInBound: Reference {} is tilted too much. X: {}, Y: {}", a_ref->GetFormID(), a_ref->GetAngleX(), a_ref->GetAngleY());
            return {};
        }
        const auto niObj = a_ref->Get3D();
        const auto niNode = niObj ? niObj->AsNode() : nullptr;
        if (!niNode) {
            logger::error("GetCoordinatesInBound: No 3D object found for reference {}", a_ref->GetFormID());
            return {};
        }
        const auto bBox = ObjectBound::MakeBoundingBox(niNode);
        if (!bBox) {
            logger::error("GetCoordinatesInBound: No bounding box found for reference {}", a_ref->GetFormID());
            return {};
        }
        const auto bBoxWorld = bBox->GetCenterWorld();
        const glm::vec4 traceStart{
            bBoxWorld.x, bBoxWorld.y,
            bBox->worldBoundMax.z, 0.0f
        };
        const Coordinate referenceCoordinates{ a_ref };
        std::vector<RE::NiAVObject*> hitList{ niObj };
        const auto castRay = [&](glm::vec4 start, const glm::vec4& end) {
            do {
                auto res = Raycast::hkpCastRay(start, end, hitList);
                if (!res.hit) {
                    return true;
                }
                auto hitRef = res.hitObject ? res.hitObject->GetUserData() : nullptr;
                auto base = hitRef ? hitRef->GetBaseObject() : nullptr;
                if (!base || base->Is(RE::FormType::Static, RE::FormType::MovableStatic, RE::FormType::Furniture, RE::FormType::Door)) {
                    break;
                }
                const auto hitRefNi = hitRef->Get3D();
                if (!hitRefNi) {
                    logger::error("GetCoordinatesInBound: No 3D object found for reference {}", hitRef->GetFormID());
                    break;
                }
                hitList.push_back(res.hitObject);
                start = res.hitPos;
            } while (true);
            return false;
        };
        std::vector<FurnitureOffset> ret{};
        for (auto&& it : data) {
            const auto& [type, offset] = it;
            if (!a_filter.any(type.value)) {
                continue;
            }
            constexpr auto& radius = Settings::fFurnitureSquare;
            constexpr auto& step = Settings::fFurnitureSquareStepSize;
            const auto offsetLocation = referenceCoordinates.ApplyReturn(offset);
            auto raycastTarget = offsetLocation.AsVec4(0.0f), raycastStart = offsetLocation.AsVec4(0.0f);
            raycastTarget.z += Settings::fFurnitureSquareHeight;
            raycastStart.z += Settings::fFurnitureSquareFloorSkip;
            // check the place surrounding the center to see if there is anything occupying it (walls, furniture, etc)
            for (float x = raycastStart.x - radius; x <= raycastStart.x + radius; x += step) {
                for (float y = raycastStart.y - radius; y <= raycastStart.y + radius; y += step) {
                    glm::vec4 point(x, y, raycastStart.z, 0.0f);
                    if (glm::distance(point, raycastStart) > radius)
                        continue;
                    if (!castRay(point, raycastTarget)) {
                        goto Next_Iteration;
                    }
                }
            }
            ret.push_back(it);
Next_Iteration:;
        }
        return ret;
    }

    std::vector<FurnitureOffset> FurnitureDetails::GetClosestCoordinatesInBound(RE::TESObjectREFR* a_ref, REX::EnumSet<FurnitureType::Value> a_filter, RE::TESObjectREFR* a_center) const
    {
        const auto center = Coordinate(a_center);
        const auto valids = GetCoordinatesInBound(a_ref, a_filter);
        std::map<FurnitureType, std::vector<Coordinate>> valid_map{};
        for (auto&& [type, coordinates] : valids) {
            valid_map[type].push_back(coordinates);
        }
        std::vector<FurnitureOffset> ret{};
        for (auto&& [type, coordinates] : valid_map) {
            const auto min = std::ranges::min_element(coordinates, [&](const Coordinate& a, const Coordinate& b) {
                return a.GetDistance(center) < b.GetDistance(center);
            });
            if (min != coordinates.end()) {
                ret.emplace_back(type, *min);
            }
        }
        return ret;
    }

}  // namespace Registry
