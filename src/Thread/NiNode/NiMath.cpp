#include "NiMath.h"

namespace Thread::NiNode::NiMath
{
	namespace
	{
		inline RE::NiPoint3 ProjectToXY(const RE::NiPoint3& v)
		{
			return { v.x, v.y, 0.0f };
		}

		inline RE::NiPoint3 ProjectToXZ(const RE::NiPoint3& v)
		{
			return { v.x, 0.0f, v.z };
		}

		inline RE::NiPoint3 ProjectToYZ(const RE::NiPoint3& v)
		{
			return { 0.0f, v.y, v.z };
		}
	}

	Segment Segment::ShortestSegmentTo(const Segment& other) const
	{
		if (IsPoint() && other.IsPoint()) {
			return Segment{ first, other.first };
		}
		const auto vSelf = Vector();
		const auto vOther = other.Vector();
		const auto vFirst = other.first - first;

		const auto lenSelf = vSelf.SqrLength();
		const auto lenOther = vOther.SqrLength();

		const auto dotSelfFirst = vSelf.Dot(vFirst);
		const auto dotOtherFirst = vOther.Dot(vFirst);

		float tSelf, tOther;
		if (IsPoint()) {
			tSelf = 0.0f;
			tOther = std::clamp(-dotOtherFirst / lenOther, 0.0f, 1.0f);
		} else if (other.IsPoint()) {
			tSelf = std::clamp(dotSelfFirst / lenSelf, 0.0f, 1.0f);
			tOther = 0.0f;
		} else {
			const auto dotSelfOther = vSelf.Dot(vOther);
			const auto det = lenSelf * lenOther - dotSelfOther * dotSelfOther;

			if (det < FLT_EPSILON * lenSelf * lenOther) {
				tSelf = std::clamp(dotSelfFirst / lenSelf, 0.0f, 1.0f);
				tOther = 0.0f;
			} else {
				tSelf = std::clamp((dotSelfFirst * lenOther - dotOtherFirst * dotSelfOther) / det, 0.0f, 1.0f);
				tOther = std::clamp((dotSelfFirst + tSelf * dotSelfOther) / lenOther, 0.0f, 1.0f);
			}
		}

		const auto c1 = first + (vSelf * tSelf);
		const auto c2 = other.first + (vOther * tOther);
		return Segment { c1, c2 };
	}

	bool Segment::IsBetween(const Segment& u, const Segment& v) const
	{
		auto s = Vector();
		auto uVec = u.Vector();
		auto vVec = v.Vector();

		if (s.SqrLength() < FLT_EPSILON || uVec.SqrLength() < FLT_EPSILON || vVec.SqrLength() < FLT_EPSILON)
			return false;

		s.Unitize();
		uVec.Unitize();
		vVec.Unitize();

		const auto n = uVec.Cross(vVec);
		if (n.SqrLength() < FLT_EPSILON)
			return false;

		float sideU = n.Dot(uVec.Cross(s));
		float sideV = n.Dot(s.Cross(vVec));
		return sideU >= -FLT_EPSILON && sideV >= -FLT_EPSILON;
	}


	Segment BestFit(const std::vector<RE::NiPoint3>& a_points)
	{
		switch (a_points.size()) {
		case 0:
			return { RE::NiPoint3::Zero(), RE::NiPoint3::Zero() };
		case 1:
			return { a_points[0], a_points[0] };
		case 2:
			return { a_points[0], a_points[1] };
		}

		// 1) Centroid
		RE::NiPoint3 centroid{};
		for (const auto& p : a_points) {
			centroid += p;
		}
		centroid /= static_cast<float>(a_points.size());

		// 2) Covariance matrix
		RE::NiMatrix3 cov{
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f },
		};
		for (const auto& p : a_points) {
			RE::NiPoint3 d = p - centroid;
			for (size_t i = 0; i < 3; ++i) {
				for (size_t j = 0; j < 3; ++j) {
					cov.entry[i][j] += d[i] * d[j];
				}
			}
		}
		float invN = 1.0f / a_points.size();
		cov = cov * invN;

		// 3) Power iteration for principal eigenvector
		constexpr int   PCA_MAX_ITERATIONS          = 50;
		constexpr float PCA_DIRECTION_TOLERANCE_SQR = 1e-6f;
		constexpr float PCA_MIN_VECTOR_NORM_SQR     = 1e-12f;

		RE::NiPoint3 dir{ 1.0f, 1.0f, 1.0f };
		dir.Unitize();

		for (int i = 0; i < PCA_MAX_ITERATIONS; ++i) {
			RE::NiPoint3 newDir = cov * dir;
			if (newDir.SqrLength() < PCA_MIN_VECTOR_NORM_SQR) {
				// Covariance matrix is near-zero; fall back to a default axis.
				dir = RE::NiPoint3{ 1.0f, 0.0f, 0.0f };
				break;
			}

			newDir.Unitize();

			RE::NiPoint3 diff = newDir - dir;
			if (diff.SqrLength() < PCA_DIRECTION_TOLERANCE_SQR) {
				dir = newDir;
				break;
			}

			dir = newDir;
		}

		// 4) Find line extents
		float minT = FLT_MAX;
		float maxT = -FLT_MAX;
		for (const auto& p : a_points) {
			float t = (p - centroid).Dot(dir);
			minT = std::min(minT, t);
			maxT = std::max(maxT, t);
		}

		const auto start = centroid + dir * minT;
		const auto end   = centroid + dir * maxT;
		return { start, end };
	}

	RE::NiMatrix3 RotateTowards(const RE::NiPoint3& v, const RE::NiPoint3& i, float maxRadians)
	{
		RE::NiPoint3 axis = v.Cross(i);
		float sin_theta = axis.Length();
		float cos_theta = v.Dot(i);

		if (sin_theta < FLT_EPSILON && cos_theta > 0.0f) {
			return RE::NiMatrix3{}; // parallel or zero
		} else if (sin_theta < FLT_EPSILON && cos_theta < 0.0f) {
			// antiparallel
			RE::NiPoint3 perp = v.Cross(RE::NiPoint3{1,0,0});
			if (perp.SqrLength() < 1e-6f)
				perp = v.Cross(RE::NiPoint3{0,1,0});
			perp.Unitize();

			// 180° rotation: R = -I + 2 * k kᵀ
			RE::NiMatrix3 m{};
			for (size_t r = 0; r < 3; ++r) {
				for (size_t c = 0; c < 3; ++c) {
					m.entry[r][c] = (r == c) ? -1.0f : 0.0f;
					m.entry[r][c] += 2.0f * perp[r] * perp[c];
				}
			}
			return m;
		}
		float theta = std::atan2(sin_theta, cos_theta);
		float step = maxRadians != 0.0f ? std::min(theta, maxRadians) : theta;

		axis /= sin_theta; // normalize

		RE::NiMatrix3 K{
			{ 0,        -axis.z,  axis.y },
			{ axis.z,    0,      -axis.x },
			{ -axis.y,   axis.x,  0 }
		};

		return RE::NiMatrix3{}
			+ K * std::sinf(step)
			+ (K * K) * (1.0f - std::cosf(step));
	}
	
	float GetAngle(const RE::NiPoint3& v1, const RE::NiPoint3& v2)
	{
		return std::acosf(GetAngleCos(v1, v2));
	}
	
	float GetAngleCos(const RE::NiPoint3& v1, const RE::NiPoint3& v2)
	{
		const auto dot = v1.Dot(v2);
		const auto l = v1.Length() * v2.Length();
		return std::clamp(dot / l, -1.0f, 1.0f);
	}

	float GetAngleDegree(const RE::NiPoint3& v1, const RE::NiPoint3& v2)
	{
		return RE::rad_to_deg(GetAngle(v1, v2));
	}

	void EnsureParallelDirection(RE::NiPoint3& v, const RE::NiPoint3& reference)
	{
		if (v.Dot(reference) < 0.0f) {
			v = -v;
		}
	}

	void EnsureAntiParallelDirection(RE::NiPoint3& v, const RE::NiPoint3& reference)
	{
		if (v.Dot(reference) > 0.0f) {
			v = -v;
		}
	}

	float GetAngleXY(const RE::NiMatrix3& rot)
	{
		return std::atan2(rot.entry[0][1], rot.entry[0][0]);
	}

	float GetAngleXZ(const RE::NiMatrix3& rot)
	{
		return std::atan2(-rot.entry[0][2], rot.entry[0][0]);
	}

	float GetAngleYZ(const RE::NiMatrix3& rot)
	{
		return std::atan2(-rot.entry[1][2], rot.entry[1][1]);
	}
	
	float GetAngleXZ(const RE::NiPoint3& u, const RE::NiPoint3& v)
	{
		return GetAngleCos(ProjectToXZ(u), ProjectToXZ(v));
	}

	float GetAngleXY(const RE::NiPoint3& u, const RE::NiPoint3& v)
	{
		return GetAngleCos(ProjectToXY(u), ProjectToXY(v));
	}

	float GetAngleYZ(const RE::NiPoint3& u, const RE::NiPoint3& v)
	{
		return GetAngleCos(ProjectToYZ(u), ProjectToYZ(v));
	}
	
	RE::NiPoint3 ProjectedComponent(RE::NiPoint3 U, RE::NiPoint3 V)
	{
		return V * (U.Dot(V) / V.SqrLength());
	}

	RE::NiPoint3 OrthogonalComponent(RE::NiPoint3 U, RE::NiPoint3 V)
	{
		return U - ProjectedComponent(U, V);
	}

}	 // namespace Thread::NiNode::NiMath
