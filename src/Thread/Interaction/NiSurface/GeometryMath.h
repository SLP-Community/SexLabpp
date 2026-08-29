#pragma once

#include <Eigen/Dense>

namespace Thread::Interaction::NiSurface::GeometryMath
{
    struct Segment
    {
        explicit Segment(RE::NiPoint3 a_point) :
          start(a_point), end(a_point) {}
        Segment(RE::NiPoint3 a_start, RE::NiPoint3 a_end) :
          start(a_start), end(a_end) {}

        float Length() const { return start.GetDistance(end); }
        RE::NiPoint3 Vector() const { return end - start; }
        bool IsPoint() const { return start == end; }

        RE::NiPoint3 start;
        RE::NiPoint3 end;
    };

    /// Obtain the shortest segment connecting two segments.
    Segment ClosestSegmentBetweenSegments(const Segment& a_lhs, const Segment& a_rhs);

    Eigen::Vector3f ToEigen(const RE::NiPoint3& a_point);
    Eigen::Matrix3f ToEigen(const RE::NiMatrix3& a_matrix);
    RE::NiMatrix3 ToNiMatrix(const Eigen::Matrix3f& a_matrix);

    /// Return the best-fit segment through the points, extended to at least the requested length.
    Segment LeastSquares(const std::vector<Eigen::Vector3f>& a_points, float a_minimumLength);
    float GetAngleDegree(const RE::NiPoint3& a_lhs, const RE::NiPoint3& a_rhs);
    /// Compute the component of a vector parallel to an axis.
    RE::NiPoint3 ProjectedComponent(RE::NiPoint3 a_vector, RE::NiPoint3 a_axis);
}
