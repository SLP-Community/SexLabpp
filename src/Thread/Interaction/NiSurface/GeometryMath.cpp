#include "GeometryMath.h"

namespace Thread::Interaction::NiSurface::GeometryMath
{
    Segment ClosestSegmentBetweenSegments(const Segment& a_lhs, const Segment& a_rhs)
    {
        if (a_lhs.IsPoint() && a_rhs.IsPoint()) {
            return { a_lhs.start, a_rhs.start };
        }

        const auto lhsVector = a_lhs.Vector();
        const auto rhsVector = a_rhs.Vector();
        const auto offset = a_rhs.start - a_lhs.start;
        const auto lhsLengthSq = lhsVector.SqrLength();
        const auto rhsLengthSq = rhsVector.SqrLength();
        const auto lhsOffset = lhsVector.Dot(offset);
        const auto rhsOffset = rhsVector.Dot(offset);

        float lhsFactor;
        float rhsFactor;
        if (a_lhs.IsPoint()) {
            lhsFactor = 0.0f;
            rhsFactor = std::clamp(-rhsOffset / rhsLengthSq, 0.0f, 1.0f);
        } else if (a_rhs.IsPoint()) {
            lhsFactor = std::clamp(lhsOffset / lhsLengthSq, 0.0f, 1.0f);
            rhsFactor = 0.0f;
        } else {
            const auto vectorsDot = rhsVector.Dot(lhsVector);
            const auto determinant = lhsLengthSq * rhsLengthSq - vectorsDot * vectorsDot;
            if (determinant < FLT_EPSILON * lhsLengthSq * rhsLengthSq) {
                lhsFactor = std::clamp(lhsOffset / lhsLengthSq, 0.0f, 1.0f);
                rhsFactor = 0.0f;
            } else {
                lhsFactor = std::clamp((lhsOffset * rhsLengthSq - rhsOffset * vectorsDot) / determinant, 0.0f, 1.0f);
                rhsFactor = std::clamp((lhsOffset + lhsFactor * vectorsDot) / rhsLengthSq, 0.0f, 1.0f);
            }
        }

        return { a_lhs.start + lhsVector * lhsFactor, a_rhs.start + rhsVector * rhsFactor };
    }

    Eigen::Vector3f ToEigen(const RE::NiPoint3& a_point)
    {
        return { a_point.x, a_point.y, a_point.z };
    }

    Eigen::Matrix3f ToEigen(const RE::NiMatrix3& a_matrix)
    {
        return Eigen::Matrix3f{
            { a_matrix.entry[0][0], a_matrix.entry[1][0], a_matrix.entry[2][0] },
            { a_matrix.entry[0][1], a_matrix.entry[1][1], a_matrix.entry[2][1] },
            { a_matrix.entry[0][2], a_matrix.entry[1][2], a_matrix.entry[2][2] },
        };
    }

    RE::NiMatrix3 ToNiMatrix(const Eigen::Matrix3f& a_matrix)
    {
        return RE::NiMatrix3{
            { a_matrix(0, 0), a_matrix(1, 0), a_matrix(2, 0) },
            { a_matrix(0, 1), a_matrix(1, 1), a_matrix(2, 1) },
            { a_matrix(0, 2), a_matrix(1, 2), a_matrix(2, 2) },
        };
    }

    Segment LeastSquares(const std::vector<Eigen::Vector3f>& a_points, float a_minimumLength)
    {
        Eigen::MatrixXf matrix(3, a_points.size());
        for (std::size_t i = 0; i < a_points.size(); ++i) {
            matrix.col(i) = a_points[i];
        }

        const Eigen::Vector3f mean = matrix.rowwise().mean();
        const auto centered = matrix.colwise() - mean;
        const Eigen::JacobiSVD<Eigen::Matrix3f> decomposition{ centered * centered.transpose(), Eigen::ComputeFullU };
        const Eigen::Vector3f axis = decomposition.matrixU().col(0);
        const Eigen::Vector3f start = mean + axis * centered.col(0).dot(axis);
        Eigen::Vector3f end = mean + axis * centered.col(a_points.size() - 1).dot(axis);

        const Eigen::Vector3f vector = end - start;
        if (vector.squaredNorm() <= FLT_EPSILON) {
            end = start + axis * a_minimumLength;
        } else if (const auto deficit = a_minimumLength - vector.norm(); deficit > 0.0f) {
            end += vector.normalized() * deficit;
        }
        return { { start.x(), start.y(), start.z() }, { end.x(), end.y(), end.z() } };
    }

    float GetAngleDegree(const RE::NiPoint3& a_lhs, const RE::NiPoint3& a_rhs)
    {
        const auto denominator = a_lhs.Length() * a_rhs.Length();
        if (denominator <= FLT_EPSILON) {
            return 0.0f;
        }
        return RE::rad_to_deg(std::acos(std::clamp(a_lhs.Dot(a_rhs) / denominator, -1.0f, 1.0f)));
    }

    RE::NiPoint3 ProjectedComponent(RE::NiPoint3 a_vector, RE::NiPoint3 a_axis)
    {
        const auto axisLengthSq = a_axis.SqrLength();
        return axisLengthSq > FLT_EPSILON ? a_axis * (a_vector.Dot(a_axis) / axisLengthSq) : RE::NiPoint3{};
    }
}
