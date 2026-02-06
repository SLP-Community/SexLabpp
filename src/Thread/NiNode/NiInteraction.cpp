#include "NiInteraction.h"

#include "NiMath.h"

namespace Thread::NiNode
{
	NiInteraction EvaluateKissing(const NiMotion& a_motionA, const NiMotion& a_motionB)
	{
		NiInteraction result{ NiInteraction::Type::Kissing };
		assert(a_motionA.HasSufficientData() && a_motionB.HasSufficientData());

		const auto mouthA = a_motionA.DescribeMotion(NiMotion::pMouth);
		const auto mouthB = a_motionB.DescribeMotion(NiMotion::pMouth);
		if (!mouthA.Valid() || !mouthB.Valid())
			return result;

		const float duration = std::min(mouthA.duration, mouthB.duration);
		if (duration < Settings::fMinKissDuration)
			return result;

		const float mouthDistance = mouthA.Mean().GetDistance(mouthB.Mean());
		if (mouthDistance > Settings::fDistanceMouth)
			return result;

		const auto headYA = a_motionA.DescribeMotion(NiMotion::vHeadY);
		const auto headYB = a_motionB.DescribeMotion(NiMotion::vHeadY);
		float facingScore = 0.0f;
		if (headYA.Valid() && headYB.Valid()) {
			// Facing each other <=> cos ~ -1
			RE::NiPoint3 dirA = headYA.trajectory.Vector();
			RE::NiPoint3 dirB = headYB.trajectory.Vector();
			const float cos = NiMath::GetCosAngle(dirA, dirB);
			facingScore = std::clamp(-cos, 0.0f, 1.0f);
		}

		// Scores
		const float distanceScore = 1.0f - std::clamp(mouthDistance / Settings::fDistanceMouth, 0.0f, 1.0f);
		const float avgVelocity = 0.5f * (mouthA.avgSpeed + mouthB.avgSpeed);
		const float velocityScore = 1.0f - std::clamp(avgVelocity / Settings::fMaxKissSpeed, 0.0f, 1.0f);
		const float impulseScore = 1.0f - std::clamp(0.5f * (mouthA.impulse + mouthB.impulse), 0.0f, 1.0f);
		const float oscillationScore = 1.0f - std::clamp(0.5f * (mouthA.oscillation + mouthB.oscillation), 0.0f, 1.0f);
		const float stabilityScore = 0.5f * impulseScore + 0.5f * oscillationScore;
		const float timeScore = std::clamp(duration / Settings::fMinKissDuration, 0.0f, 1.0f);

		// Save results
		result.confidence =
		  0.35f * distanceScore +
		  0.25f * facingScore +
		  0.20f * timeScore +
		  0.10f * velocityScore +
		  0.10f * stabilityScore;

		result.duration = duration;
		result.velocity = avgVelocity;
		result.active = result.confidence > Settings::fEnterThreshold;

		return result;
	}

}  // namespace Thread::NiNode
