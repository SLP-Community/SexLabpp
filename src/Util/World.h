#pragma once

namespace Util
{
    inline void ForEachObjectInRange(RE::TESObjectREFR* a_center, float a_radius, std::function<RE::BSContainer::ForEachResult(RE::TESObjectREFR*)> a_forEachFunc)
    {
        const auto TES = RE::TES::GetSingleton();
        const auto center_coords = a_center->GetPosition();
        if (const auto interior = TES->interiorCell; interior) {
            interior->ForEachReferenceInRange(center_coords, a_radius, a_forEachFunc);
        } else if (const auto grids = TES->gridCells; grids) {
            auto gridLength = grids->length;
            if (gridLength > 0) {
                float yPlus = center_coords.y + a_radius;
                float yMinus = center_coords.y - a_radius;
                float xPlus = center_coords.x + a_radius;
                float xMinus = center_coords.x - a_radius;
                for (uint32_t x = 0, y = 0; (x < gridLength && y < gridLength); x++, y++) {
                    const auto gridcell = grids->GetCell(x, y);
                    if (gridcell && gridcell->IsAttached()) {
                        auto cellCoords = gridcell->GetCoordinates();
                        if (!cellCoords)
                            continue;
                        float worldX = cellCoords->worldX;
                        float worldY = cellCoords->worldY;
                        if (worldX < xPlus && (worldX + 4096.0) > xMinus && worldY < yPlus && (worldY + 4096.0) > yMinus) {
                            gridcell->ForEachReferenceInRange(center_coords, a_radius, a_forEachFunc);
                        }
                    }
                }
            }
        }
    }
}  // namespace CellCrawler
