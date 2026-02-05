#include "NiMath.h"

namespace Thread::NiNode::NiMath
{
	Segment ClosestSegmentBetweenSegments(const Segment& u, const Segment& v)
	{
		if (u.isPoint && v.isPoint) {
			return Segment{ u.first, v.first };
		}
		const auto vU = u.second - u.first, vV = v.second - v.first, vR = v.first - u.first;
		const auto lU = vU.SqrLength(), lV = vV.SqrLength();
		const auto vUvR = vU.Dot(vR), vVvR = vV.Dot(vR);

		float s, t;
		if (u.isPoint) {
			s = 0.0f;
			t = std::clamp(-vVvR / lV, 0.0f, 1.0f);
		} else if (v.isPoint) {
			s = std::clamp(vUvR / lU, 0.0f, 1.0f);
			t = 0.0f;
		} else {
			const auto vUvV = vV.Dot(vU);
			const auto det = lU * lV - vUvV * vUvV;

			if (det < FLT_EPSILON * lU * lV) {
				s = std::clamp(vUvR / lU, 0.0f, 1.0f);
				t = 0.0f;
			} else {
				s = std::clamp((vUvR * lV - vVvR * vUvV) / det, 0.0f, 1.0f);
				t = std::clamp((vUvR + s * vUvV) / lV, 0.0f, 1.0f);
			}
		}

		const auto c1 = u.first + (vU * s);
		const auto c2 = v.first + (vV * t);
		return Segment { c1, c2 };
	}

	bool IsSegmentBetweenSegments(const Segment& a_s, const Segment& a_u, const Segment& a_v)
	{
		auto s = a_s.Vector(), u = a_u.Vector(), v = a_v.Vector();
		s.Unitize();
		u.Unitize();
		v.Unitize();

		const auto cosVs = v.Dot(s);
		const auto cosUs = u.Dot(s);
		const auto cosUv = u.Dot(v);
		if (cosVs <= FLT_EPSILON || cosUs <= FLT_EPSILON || cosUv <= FLT_EPSILON) {
			return false;
		}
		return cosVs + cosUs >= cosUv - FLT_EPSILON;
	}

	Eigen::Vector3f ToEigen(const RE::NiPoint3& a_point)
	{
		return { a_point.x, a_point.y, a_point.z };
	}

	Eigen::Matrix3f ToEigen(const RE::NiMatrix3& a_mat)
	{
		return Eigen::Matrix3f{
			{ a_mat.entry[0][0], a_mat.entry[1][0], a_mat.entry[2][0] },
			{ a_mat.entry[0][1], a_mat.entry[1][1], a_mat.entry[2][1] },
			{ a_mat.entry[0][2], a_mat.entry[1][2], a_mat.entry[2][2] },
		};
	}

	RE::NiPoint3 ToNiPoint(const Eigen::Vector3f& a_point)
	{
		return { a_point[0], a_point[1], a_point[2] };
	}

	RE::NiMatrix3 ToNiMatrix(const Eigen::Matrix3f& a_mat)
	{
		return RE::NiMatrix3{
			{ a_mat(0, 0), a_mat(1, 0), a_mat(2, 0) },
			{ a_mat(0, 1), a_mat(1, 1), a_mat(2, 1) },
			{ a_mat(0, 2), a_mat(1, 2), a_mat(2, 2) },
		};
	}

	Eigen::AngleAxisf AlignAxis(const Eigen::Vector3f& vector, const Eigen::Vector3f& ideal)
	{
		const auto rotationAxis = vector.cross(ideal);
		const auto angle = GetAngle(vector, ideal);
		Eigen::AngleAxisf rotation{ angle, rotationAxis.normalized() };
		return rotation;
	}

	Segment LeastSquares(const std::vector<RE::NiPoint3>& a_points, float a_minlen)
	{
		std::vector<Eigen::Vector3f> points{};
		points.reserve(a_points.size());
		for (auto&& point : a_points) {
			points.push_back(ToEigen(point));
		}
		return LeastSquares(points, a_minlen);
	}

	Segment LeastSquares(const std::vector<Eigen::Vector3f>& a_points, float a_minlen)
	{
		Eigen::MatrixXf A(3, a_points.size());
		for (size_t i = 0; i < a_points.size(); ++i) {
			A.col(i) = a_points[i];
		}
		const Eigen::Vector3f a = A.rowwise().mean();
		const auto centered = A.colwise() - a;
		const auto covariance = centered * centered.transpose();
		Eigen::JacobiSVD<Eigen::Matrix3f> svd{ covariance, Eigen::ComputeFullU };
		const Eigen::Vector3f b = svd.matrixU().col(0);

		const auto f = [&](float x) -> Eigen::Vector3f { return a + b * x; };
		auto s1 = f(centered.col(0).dot(b));
		auto s2 = f(centered.col(a_points.size() - 1).dot(b));

		const auto vS = s2 - s1;
		if (const auto dif = a_minlen - vS.norm(); dif > 0.0f) {
			s2 += vS.normalized() * dif;
		}

		[[maybe_unused]] auto angle = RE::rad_to_deg(GetAngle(vS, a_points.back() - a_points.front()));

		return Segment{ ToNiPoint(s1), ToNiPoint(s2) };
	}


	RE::NiMatrix3 Rodrigue(const RE::NiPoint3& v, const RE::NiPoint3& i)
	{
		RE::NiPoint3 cross = v.Cross(i);
		const auto sin_theta = cross.Length();
		const auto cos_theta = v.Dot(i);
		cross.Unitize();

		RE::NiMatrix3 skew{
			{ 0, cross.z, -cross.y },
			{ -cross.z, 0, cross.x },
			{ cross.y, -cross.x, 0 },
		};

		RE::NiMatrix3 transform = RE::NiMatrix3{} + (skew * sin_theta) + (skew * skew * (1 - cos_theta));
		if (cos_theta < 0) {
			transform = transform * -1;
		}
		return transform;
	}

	Eigen::Matrix3f Rodrigue(const Eigen::Vector3f& v, const Eigen::Vector3f& i)
	{
		Eigen::Vector3f cross = v.cross(i);
		const auto sin_theta = cross.norm();
		const auto cos_theta = v.dot(i);
		cross.normalize();

		Eigen::Matrix3f skew{
			{ 0, -cross.z(), cross.y() },
			{ cross.z(), 0, -cross.x() },
			{ -cross.y(), cross.x(), 0 },
		};

		Eigen::Matrix3f transform = Eigen::Matrix3f::Identity() + (skew * sin_theta) + (skew * skew * (1 - cos_theta));
		if (cos_theta < 0) {
			transform = -transform;
		}
		return transform;
	}

	float GetCosAngle(const RE::NiPoint3& v1, const RE::NiPoint3& v2)
	{
		const auto dot = v1.Dot(v2);
		const auto l = v1.Length() * v2.Length();
		return std::clamp(dot / l, -1.0f, 1.0f);
	}

	float GetAngle(const Eigen::Vector3f& v1, const Eigen::Vector3f& v2)
	{
		const auto cos_theta = v1.dot(v2);
		const auto l = v1.norm() * v2.norm();
		const auto x = std::clamp(cos_theta / l, -1.0f, 1.0f);
		return acos(x);
	}
	
	float GetAngle(const RE::NiPoint3& v1, const RE::NiPoint3& v2)
	{
		return std::acosf(GetCosAngle(v1, v2));
	}

	float GetAngleDegree(const RE::NiPoint3& v1, const RE::NiPoint3& v2)
	{
		return RE::rad_to_deg(GetAngle(v1, v2));
	}

	float GetAngleXY(const RE::NiMatrix3& rot)
	{
		return std::atan2(rot.entry[0][1], rot.entry[0][0]);
	}
	float GetAngleXY(const Eigen::Matrix3f& rot)
	{
		return std::atan2(rot(1, 0), rot(0, 0));
	}

	float GetAngleXZ(const RE::NiMatrix3& rot)
	{
		return std::atan2(-rot.entry[0][2], rot.entry[0][0]);
	}
	float GetAngleXZ(const Eigen::Matrix3f& rot)
	{
		return std::atan2(-rot(2, 0), rot(0, 0));
	}

	float GetAngleYZ(const RE::NiMatrix3& rot)
	{
		return std::atan2(-rot.entry[1][2], rot.entry[1][1]);
	}
	float GetAngleYZ(const Eigen::Matrix3f& rot)
	{
		return std::atan2(-rot(2, 1), rot(0, 0));
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
