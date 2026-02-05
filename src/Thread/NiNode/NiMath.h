#pragma once

#include <Eigen/Dense>

namespace Thread::NiNode::NiMath
{
	struct Segment : public std::pair<RE::NiPoint3, RE::NiPoint3> {
		Segment(RE::NiPoint3 fst, RE::NiPoint3 snd) :
			std::pair<RE::NiPoint3, RE::NiPoint3>(fst, snd), isPoint(fst == snd) {}
		Segment(RE::NiPoint3 fst) :
			std::pair<RE::NiPoint3, RE::NiPoint3>(fst, fst), isPoint(true) {}

		float Length() const { return isPoint ? 0.0f : first.GetDistance(second); }
		RE::NiPoint3 Vector() const { return  isPoint ? RE::NiPoint3::Zero() : second - first; }

		bool isPoint;
	};

	/// @brief Obtain the shortest segment connecting u to v
	Segment ClosestSegmentBetweenSegments(const Segment& u, const Segment& v);

	/// @brief For 3 aligned segments s, u and v if s is between u and v
	/// @return if s is between u and v
	bool IsSegmentBetweenSegments(const Segment& s, const Segment& u, const Segment& v);

	Eigen::Vector3f ToEigen(const RE::NiPoint3& a_point);
	Eigen::Matrix3f ToEigen(const RE::NiMatrix3& a_matrix);
	RE::NiPoint3 ToNiPoint(const Eigen::Vector3f& a_point);
	RE::NiMatrix3 ToNiMatrix(const Eigen::Matrix3f& a_matrix);

	/// @brief Obtain an angle s.t. ret * vector aligns with ideal (0°)
	/// @param vector The vector to align
	/// @param ideal The ideal rotation
	/// @return A rotation to transform vector with
	Eigen::AngleAxisf AlignAxis(const Eigen::Vector3f& vector, const Eigen::Vector3f& ideal);

	/// @brief Create a rotation matrix using Rodrigues Formula
	/// @param v The vector to align, normalized
	/// @param i The ideal rotation of the vector, normalized
	/// @return Rotation matrix s.t. angle(v * M, i) == 0
	RE::NiMatrix3 Rodrigue(const RE::NiPoint3& v, const RE::NiPoint3& i);
	Eigen::Matrix3f Rodrigue(const Eigen::Vector3f& v, const Eigen::Vector3f& i);

	/// @brief Perform least squares on the given set of points
	/// @param a_points The points to perform the analysis on
	/// @param a_minlen The returned segments minimum norm
	/// @return A best-fit segment reaching from the first point and ending at the last
	Segment LeastSquares(const std::vector<RE::NiPoint3>& a_points, float a_minlen);
	Segment LeastSquares(const std::vector<Eigen::Vector3f>& a_points, float a_minlen);

	/// @brief Compute the Angle between v1 and v2, in radians
	/// @param v1 The first vector
	/// @param v2 The second vector
	/// @return The angle, in radians
	float GetCosAngle(const RE::NiPoint3& v1, const RE::NiPoint3& v2);
	float GetAngle(const Eigen::Vector3f& v1, const Eigen::Vector3f& v2);
	float GetAngle(const RE::NiPoint3& v1, const RE::NiPoint3& v2);
	float GetAngleDegree(const RE::NiPoint3& v1, const RE::NiPoint3& v2);

	/// @brief Get the angle when projecting the matrix onto a specific plane
	/// @param rot The rotation matrix to extract the angle from
	/// @return The rotation when viewed from the specified plane
	float GetAngleXZ(const Eigen::Matrix3f& rot);
	float GetAngleXY(const Eigen::Matrix3f& rot);
	float GetAngleYZ(const Eigen::Matrix3f& rot);
	float GetAngleXZ(const RE::NiMatrix3& rot);
	float GetAngleXY(const RE::NiMatrix3& rot);
	float GetAngleYZ(const RE::NiMatrix3& rot);

	/// @brief Compute the projected component of U relative to V
	RE::NiPoint3 ProjectedComponent(RE::NiPoint3 U, RE::NiPoint3 V);

	/// @brief Compute the orthogonal component of U relative to V
	RE::NiPoint3 OrthogonalComponent(RE::NiPoint3 U, RE::NiPoint3 V);

	/// @brief constexpr ceil() function
	constexpr int IntCeil(float f)
	{
		const int i = static_cast<int>(f);
		return f > i ? i + 1 : i;
	}

}	 // namespace Thread::NiNode::NiMath
