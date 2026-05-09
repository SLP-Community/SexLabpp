#pragma once

#include "RayCast/ObjectBound.h"

namespace SAT
{
    struct OrientedObjectBound
    {
        OrientedObjectBound(RE::NiNode* a_origin, const ObjectBound& a_bound) :
          origin(a_origin), box(a_bound) {}
        OrientedObjectBound(RE::NiNode* a_origin) :
          origin(a_origin), box([&]() {
				const auto ret = ObjectBound::MakeBoundingBox(a_origin);
				return ret ? *ret : ObjectBound{}; }()) {}
        ~OrientedObjectBound() = default;

        std::vector<glm::vec3> GetCorners() const
        {
            const auto center = box.GetCenterWorld();
            const auto halfsize = glm::abs(box.worldBoundMax - center);
            const auto& rotate = origin->world.rotate;
            const auto x = rotate.GetVectorX(), y = rotate.GetVectorY(), z = rotate.GetVectorZ();
            const auto ex = halfsize.x * glm::vec3{ x.x, x.y, x.z };
            const auto ey = halfsize.y * glm::vec3{ y.x, y.y, y.z };
            const auto ez = halfsize.z * glm::vec3{ z.x, z.y, z.z };

            std::vector<glm::vec3> corners;
            corners.push_back(center - ex - ey - ez);
            corners.push_back(center + ex - ey - ez);
            corners.push_back(center - ex + ey - ez);
            corners.push_back(center + ex + ey - ez);
            corners.push_back(center - ex - ey + ez);
            corners.push_back(center + ex - ey + ez);
            corners.push_back(center - ex + ey + ez);
            corners.push_back(center + ex + ey + ez);
            return corners;
        }

        RE::NiNode* origin;
        ObjectBound box;
    };

    struct SATResult
    {
        float mtv{ std::numeric_limits<float>::max() };
        glm::vec3 mtv_axis{};
    };

    inline std::vector<glm::vec3> GetAxes(const OrientedObjectBound& a_obb1, const OrientedObjectBound& a_obb2)
    {
        const auto& rot1 = a_obb1.origin->world.rotate.entry;
        const auto& rot2 = a_obb2.origin->world.rotate.entry;
        std::array<std::array<glm::vec3, 3>, 2> rotations;
        for (int i = 0; i < rotations[0].size(); i++) {
            rotations[0][i] = glm::vec3(rot1[i][0], rot1[i][1], rot1[i][2]);
            rotations[1][i] = glm::vec3(rot2[i][0], rot2[i][1], rot2[i][2]);
        }

        std::vector<glm::vec3> axes;
        for (size_t n = 0; n < rotations.size(); n++) {
            for (int i = 0; i < rotations[0].size(); i++) {
                axes.push_back(glm::normalize(rotations[n][i]));
            }
        }
        for (int i = 0; i < rotations[0].size(); i++) {
            for (int j = 0; j < rotations[0].size(); j++) {
                glm::vec3 axis = glm::cross(rotations[0][i], rotations[1][j]);
                if (glm::length(axis) > 0) {
                    axes.push_back(glm::normalize(axis));
                }
            }
        }
        return axes;
    }

    inline std::optional<SATResult> SAT(const OrientedObjectBound& a_obb1, const OrientedObjectBound& a_obb2)
    {
        const auto corner1 = a_obb1.GetCorners(), corner2 = a_obb2.GetCorners();
        const auto axes = GetAxes(a_obb1, a_obb2);

        SATResult ret;
        for (auto&& axis : axes) {
            const auto project = [&axis](const std::vector<glm::vec3>& points) -> std::pair<float, float> {
                float min = std::numeric_limits<float>::max(), max = 0.0f;
                for (const auto& point : points) {
                    float projection = glm::dot(axis, point);
                    min = std::min(projection, min);
                    max = std::max(projection, max);
                }
                return { min, max };
            };

            const auto [start1, end1] = project(corner1);
            const auto [start2, end2] = project(corner2);

            const auto minend = std::min(end1, end2), maxstart = std::max(start1, start2);
            const auto overlap = minend - maxstart;
            if (overlap > 0)
                return std::nullopt;

            if (ret.mtv > overlap) {
                ret.mtv = overlap;
                ret.mtv_axis = axis;
            }
        }
        return ret;
    }

}