#pragma once

namespace Thread::NiNode::NiMath
{
	struct Segment : public std::pair<RE::NiPoint3, RE::NiPoint3> {
		Segment(RE::NiPoint3 fst, RE::NiPoint3 snd) :
			std::pair<RE::NiPoint3, RE::NiPoint3>(fst, snd), isPoint(fst == snd) {}
		Segment(RE::NiPoint3 fst) :
			std::pair<RE::NiPoint3, RE::NiPoint3>(fst, fst), isPoint(true) {}

	public:
		bool IsPoint() const { return isPoint; }
		float Length() const { return isPoint ? 0.0f : first.GetDistance(second); }
		RE::NiPoint3 Vector() const { return  isPoint ? RE::NiPoint3::Zero() : second - first; }

		Segment ShortestSegmentTo(const Segment& other) const;
		bool IsBetween(const Segment& u, const Segment& v) const;

	private:
		bool isPoint;
	};

	/// @brief Compute the rotation matrix that rotates vector v towards vector i
	/// @param v The vector to rotate
	/// @param i The vector to rotate towards
	/// @param maxRadians The maximum angle to rotate; i.e. rotation = min(maxRadians, angle between v and i)
	/// @return The rotation matrix that rotates v towards i (V = R * V)
	RE::NiMatrix3 RotateTowards(const RE::NiPoint3& v, const RE::NiPoint3& i, float maxRadians = 0.f);

	/// @brief Compute the best fit line for a set of points using PCA
	/// @param points The points to fit
	Segment BestFit(const std::vector<RE::NiPoint3>& a_points);

	/// @brief Compute the Angle between v1 and v2, in radians
	/// @param v1 The first vector
	/// @param v2 The second vector
	/// @return The angle, in radians
	float GetAngle(const RE::NiPoint3& v1, const RE::NiPoint3& v2);
	float GetAngleCos(const RE::NiPoint3& v1, const RE::NiPoint3& v2);
	float GetAngleDegree(const RE::NiPoint3& v1, const RE::NiPoint3& v2);

	/// @brief Ensure v is pointing in the same general direction as reference; if not, reverse v
	/// @param v The vector to potentially reverse
	/// @param reference The reference vector to compare against
	/// @return v, potentially reversed to point in the same general direction as reference
	void EnsureParallelDirection(RE::NiPoint3& v, const RE::NiPoint3& reference);
	void EnsureAntiParallelDirection(RE::NiPoint3& v, const RE::NiPoint3& reference);

	/// @brief Get the angle when projecting the matrix onto a specific plane
	/// @param rot The rotation matrix to extract the angle from
	/// @return The rotation when viewed from the specified plane
	float GetAngleXZ(const RE::NiMatrix3& rot);
	float GetAngleXY(const RE::NiMatrix3& rot);
	float GetAngleYZ(const RE::NiMatrix3& rot);
	/// @brief Project vectors onto the specified world plane and get the angle between the projections
	float GetAngleXZ(const RE::NiPoint3& u, const RE::NiPoint3& v);
	float GetAngleXY(const RE::NiPoint3& u, const RE::NiPoint3& v);
	float GetAngleYZ(const RE::NiPoint3& u, const RE::NiPoint3& v);

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

	/// @brief Sigmoid function
	inline float Sigmoid(float x)
	{
		return 1.0f / (1.0f + std::exp(-x));
	}

}	 // namespace Thread::NiNode::NiMath
