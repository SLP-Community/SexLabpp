#include "NiInteraction.h"

#include "NiMath.h"

namespace Thread::NiNode
{
	namespace
	{
		template <typename NiDescriptorType>
		void AddAngleScores(NiDescriptorType& descriptor, const RE::NiPoint3& vecA, const RE::NiPoint3& vecB)
		{
			const auto scoreXY = NiMath::GetAngleXY(vecA, vecB);
			const auto scoreXZ = NiMath::GetAngleXZ(vecA, vecB);
			const auto scoreYZ = NiMath::GetAngleYZ(vecA, vecB);

			descriptor.AddValue(INiDescriptor::Feature::AngleXY, scoreXY);
			descriptor.AddValue(INiDescriptor::Feature::AngleXZ, scoreXZ);
			descriptor.AddValue(INiDescriptor::Feature::AngleYZ, scoreYZ);
		}

		template <typename NiDescriptorType>
		void AddBasicPairedScores(NiDescriptorType& descriptor, const MotionDescriptor& motionA, const MotionDescriptor& motionB)
		{
			const float duration = std::min(motionA.duration, motionB.duration);
			const float timeScore = duration / Settings::fMinTypeDuration;
			const float oscillationScore = 0.5f * (motionA.oscillation + motionB.oscillation);
			const float impulseScore = 0.5f * (motionA.impulse + motionB.impulse);
			const float stabilityScore = 0.5f * (motionA.positionalVariance + motionB.positionalVariance);
			const float stabilityVarScore = 0.5f * (motionA.directionalVariance + motionB.directionalVariance);

			descriptor.AddValue(INiDescriptor::Feature::Time, timeScore);
			descriptor.AddValue(INiDescriptor::Feature::Oscillation, oscillationScore);
			descriptor.AddValue(INiDescriptor::Feature::Impulse, impulseScore);
			descriptor.AddValue(INiDescriptor::Feature::Stability, stabilityScore);
			descriptor.AddValue(INiDescriptor::Feature::StabilityVariance, stabilityVarScore);
		}
	}

	NiInteraction EvaluateVaginal(const NiMotion& a_motionA, const NiMotion& a_motionB)
	{
		NiInteraction result{};
		assert(a_motionA.HasSufficientData() && a_motionB.HasSufficientData());
		if (!a_motionA.HasMomentData(NiMotion::pVaginalStart) || !a_motionB.HasMomentData(NiMotion::pSchlongTip)) {
			return result;
		}

		// a_motionA: receiving actor (vagina/anal)
		// a_motionB: penetrating actor (with schlong)
		const auto vaginalEntryMotion = a_motionA.DescribeMotion(NiMotion::pVaginalStart);
		const auto schlongTipMotion = a_motionB.DescribeMotion(NiMotion::pSchlongTip);
		if (!vaginalEntryMotion.DescribesMotion() && !schlongTipMotion.DescribesMotion()) {
			return result;
		}

		const auto pVaginaStart = a_motionA.GetLatestMoment(NiMotion::pVaginalStart);
		const auto pSchlongStart = a_motionB.GetLatestMoment(NiMotion::pSchlongBase);
		const auto pSchlongEnd = a_motionB.GetLatestMoment(NiMotion::pSchlongTip);
		const NiMath::Segment sSchlong{ pSchlongStart, pSchlongEnd };

		const auto sDistance = sSchlong.ShortestSegmentTo(pVaginaStart);
		const auto distance = sDistance.Length();
		if (distance > Settings::fDistanceCrotch * 2.0f) {
			return result;
		}

		const float avgVelocity = 0.5f * (vaginalEntryMotion.avgSpeed + schlongTipMotion.avgSpeed);
		const auto vSchlong = schlongTipMotion.DescribesMotion() ? schlongTipMotion.trajectory.Vector() : sSchlong.Vector();
		const auto pVaginaEnd = a_motionA.GetLatestMoment(NiMotion::pVaginalEnd);
		auto vVagina = vaginalEntryMotion.DescribesMotion() ? vaginalEntryMotion.trajectory.Vector() : (pVaginaEnd - pVaginaStart);
		NiMath::EnsureParallelDirection(vVagina, vSchlong);

		const float distanceScore = distance / Settings::fDistanceCrotch;
		const float velocityScore = avgVelocity / Settings::fMinSpeedPenetration;

		auto descriptor = NiDescriptor<NiType::Vaginal>();
		AddAngleScores(descriptor, vVagina, vSchlong);
		AddBasicPairedScores(descriptor, vaginalEntryMotion, schlongTipMotion);
		descriptor.AddValue(INiDescriptor::Feature::Distance, distanceScore);
		descriptor.AddValue(INiDescriptor::Feature::Velocity, velocityScore);

		result.descriptor = std::make_unique<NiDescriptor<NiType::Vaginal>>(std::move(descriptor));
		result.velocity = avgVelocity;

		return result;
	}

	NiInteraction EvaluateAnal(const NiMotion& a_motionA, const NiMotion& a_motionB)
	{
		NiInteraction result{};
		assert(a_motionA.HasSufficientData() && a_motionB.HasSufficientData());
		if (!a_motionA.HasMomentData(NiMotion::pAnalStart) || !a_motionB.HasMomentData(NiMotion::pSchlongTip)) {
			return result;
		}

		// a_motionA: receiving actor (anal)
		// a_motionB: penetrating actor (with schlong)
		const auto analEntryMotion = a_motionA.DescribeMotion(NiMotion::pAnalStart);
		const auto schlongTipMotion = a_motionB.DescribeMotion(NiMotion::pSchlongTip);
		if (!analEntryMotion.DescribesMotion() && !schlongTipMotion.DescribesMotion()) {
			return result;
		}

		const auto pAnalStart = a_motionA.GetLatestMoment(NiMotion::pAnalStart);
		const auto pSchlongStart = a_motionB.GetLatestMoment(NiMotion::pSchlongBase);
		const auto pSchlongEnd = a_motionB.GetLatestMoment(NiMotion::pSchlongTip);
		const NiMath::Segment sSchlong{ pSchlongStart, pSchlongEnd };

		const auto sDistance = sSchlong.ShortestSegmentTo(pAnalStart);
		const auto distance = sDistance.Length();
		if (distance > Settings::fDistanceCrotch * 2.0f) {
			return result;
		}

		const float avgVelocity = 0.5f * (analEntryMotion.avgSpeed + schlongTipMotion.avgSpeed);
		const auto vSchlong = schlongTipMotion.DescribesMotion() ? schlongTipMotion.trajectory.Vector() : sSchlong.Vector();
		const auto pAnalEnd = a_motionA.GetLatestMoment(NiMotion::pAnalEnd);
		auto vAnal = analEntryMotion.DescribesMotion() ? analEntryMotion.trajectory.Vector() : (pAnalEnd - pAnalStart);
		NiMath::EnsureParallelDirection(vAnal, vSchlong);

		const float distanceScore = distance / Settings::fDistanceCrotch;
		const float velocityScore = avgVelocity / Settings::fMinSpeedPenetration;

		auto descriptor = NiDescriptor<NiType::Anal>();
		AddAngleScores(descriptor, vAnal, vSchlong);
		AddBasicPairedScores(descriptor, analEntryMotion, schlongTipMotion);
		descriptor.AddValue(INiDescriptor::Feature::Distance, distanceScore);
		descriptor.AddValue(INiDescriptor::Feature::Velocity, velocityScore);

		result.descriptor = std::make_unique<NiDescriptor<NiType::Anal>>(std::move(descriptor));
		result.velocity = avgVelocity;

		return result;
	}

	NiInteraction EvaluateOral(const NiMotion& a_motionA, const NiMotion& a_motionB)
	{
		NiInteraction result{};
		assert(a_motionA.HasSufficientData() && a_motionB.HasSufficientData());
		const auto headBound = a_motionA.GetLatestHeadBound();
		if (!a_motionB.HasMomentData(NiMotion::pSchlongTip) || !headBound.IsValid()) {
			return result;
		}

		// a_motionA: receiving actor (head)
		// a_motionB: penetrating actor (with schlong)
		const auto headEntryMotion = a_motionA.DescribeMotion(NiMotion::pHead);
		const auto schlongBaseMotion = a_motionB.DescribeMotion(NiMotion::pSchlongBase);
		if (!headEntryMotion.DescribesMotion() && !schlongBaseMotion.DescribesMotion()) {
			return result;
		}

		const auto pSchlongStart = a_motionB.GetLatestMoment(NiMotion::pSchlongBase);
		const auto pSchlongEnd = a_motionB.GetLatestMoment(NiMotion::pSchlongTip);
		const NiMath::Segment sSchlong{ pSchlongStart, pSchlongEnd };
		const auto pMouth = a_motionA.GetLatestMoment(NiMotion::pMouth);

		const auto distance = sSchlong.ShortestSegmentTo(pMouth).Length();
		const auto distanceLimit = headBound.boundMax.y * Settings::fCloseToHeadRatio;
		if (distance > distanceLimit * 2.0f) {
			return result;
		}
		const auto avgVelocity = 0.5f * (headEntryMotion.avgSpeed + schlongBaseMotion.avgSpeed);

		const auto vHead = a_motionA.GetLatestMoment(NiMotion::vHeadY);
		auto vSchlong = schlongBaseMotion.DescribesMotion() ? schlongBaseMotion.trajectory.Vector() : sSchlong.Vector();
		NiMath::EnsureAntiParallelDirection(vSchlong, vHead);

		const float distanceScore = distance / distanceLimit;
		const float velocityScore = avgVelocity / Settings::fMinSpeedPenetration;

		auto descriptor = NiDescriptor<NiType::Oral>();
		AddAngleScores(descriptor, vHead, vSchlong);
		AddBasicPairedScores(descriptor, headEntryMotion, schlongBaseMotion);
		descriptor.AddValue(INiDescriptor::Feature::Distance, distanceScore);
		descriptor.AddValue(INiDescriptor::Feature::Velocity, velocityScore);

		result.descriptor = std::make_unique<NiDescriptor<NiType::Oral>>(std::move(descriptor));
		result.velocity = avgVelocity;

		return result;
	}

	NiInteraction EvaluateGrinding(const NiMotion& a_motionA, const NiMotion& a_motionB)
	{
		NiInteraction result{};
		assert(a_motionA.HasSufficientData() && a_motionB.HasSufficientData());
		if (!a_motionB.HasMomentData(NiMotion::pSchlongTip)) {
			return result;
		}

		// a_motionA: receiving actor (crotch)
		// a_motionB: penetrating actor (with schlong)
		const auto crotchEntryMotion = a_motionA.DescribeMotion(NiMotion::pPelvis);
		const auto schlongTipMotion = a_motionB.DescribeMotion(NiMotion::pSchlongTip);
		if (!crotchEntryMotion.DescribesMotion() && !schlongTipMotion.DescribesMotion()) {
			return result;
		}

		const auto pPelvis = a_motionA.GetLatestMoment(NiMotion::pPelvis);
		const auto pSchlongStart = a_motionB.GetLatestMoment(NiMotion::pSchlongBase);
		const auto pSchlongEnd = a_motionB.GetLatestMoment(NiMotion::pSchlongTip);
		const NiMath::Segment sSchlong{ pSchlongStart, pSchlongEnd };

		const auto sDistance = sSchlong.ShortestSegmentTo(pPelvis);
		const auto distance = sDistance.Length();
		if (distance > Settings::fDistanceCrotch * 2.0f) {
			return result;
		}

		const float avgVelocity = 0.5f * (crotchEntryMotion.avgSpeed + schlongTipMotion.avgSpeed);
		const auto vSchlong = schlongTipMotion.DescribesMotion() ? schlongTipMotion.trajectory.Vector() : sSchlong.Vector();
		const auto pSpineLower = a_motionA.GetLatestMoment(NiMotion::pSpineLower);
		auto vCrotch = crotchEntryMotion.DescribesMotion() ? crotchEntryMotion.trajectory.Vector() : (pSpineLower - pPelvis);
		NiMath::EnsureParallelDirection(vCrotch, vSchlong);

		const float distanceScore = distance / Settings::fDistanceCrotch;
		const float velocityScore = avgVelocity / Settings::fMinSpeedPenetration;

		auto descriptor = NiDescriptor<NiType::Grinding>();
		AddAngleScores(descriptor, vCrotch, vSchlong);
		AddBasicPairedScores(descriptor, crotchEntryMotion, schlongTipMotion);
		descriptor.AddValue(INiDescriptor::Feature::Distance, distanceScore);
		descriptor.AddValue(INiDescriptor::Feature::Velocity, velocityScore);

		result.descriptor = std::make_unique<NiDescriptor<NiType::Grinding>>(std::move(descriptor));
		result.velocity = avgVelocity;

		return result;
	}

	NiInteraction EvaluateDeepthroat(const NiMotion& a_motionA, const NiMotion& a_motionB)
	{
		NiInteraction result{};
		assert(a_motionA.HasSufficientData() && a_motionB.HasSufficientData());
		const auto headBound = a_motionA.GetLatestHeadBound();
		if (!a_motionB.HasMomentData(NiMotion::pSchlongBase) || !headBound.IsValid()) {
			return result;
		}

		// a_motionA: receiving actor (head)
		// a_motionB: penetrating actor (with schlong)
		const auto headEntryMotion = a_motionA.DescribeMotion(NiMotion::pHead);
		const auto schlongBaseMotion = a_motionB.DescribeMotion(NiMotion::pSchlongBase);
		if (!headEntryMotion.DescribesMotion() && !schlongBaseMotion.DescribesMotion()) {
			return result;
		}

		const auto pSchlongStart = a_motionB.GetLatestMoment(NiMotion::pSchlongBase);
		const auto pSchlongEnd = a_motionB.GetLatestMoment(NiMotion::pSchlongTip);
		const NiMath::Segment sSchlong{ pSchlongStart, pSchlongEnd };
		const auto pHead = a_motionA.GetLatestMoment(NiMotion::pHead);

		const auto pPelvis = a_motionA.GetLatestMoment(NiMotion::pPelvis);
		const float distanceLimit = headBound.boundMax.y * Settings::fThroatToleranceRadius;
		const float distance = headBound.IsPointInside(pPelvis) ? 0.1f : pHead.GetDistance(pSchlongEnd);
		if (distance > distanceLimit * 2.0f) {
			return result;
		}

		const auto avgVelocity = 0.5f * (headEntryMotion.avgSpeed + schlongBaseMotion.avgSpeed);
		const auto vHead = a_motionA.GetLatestMoment(NiMotion::vHeadY);
		auto vSchlong = schlongBaseMotion.DescribesMotion() ? schlongBaseMotion.trajectory.Vector() : sSchlong.Vector();
		NiMath::EnsureAntiParallelDirection(vSchlong, vHead);

		const float distanceScore = distance / distanceLimit;
		const float velocityScore = avgVelocity / Settings::fMinSpeedPenetration;

		auto descriptor = NiDescriptor<NiType::Deepthroat>();
		AddAngleScores(descriptor, vHead, vSchlong);
		AddBasicPairedScores(descriptor, headEntryMotion, schlongBaseMotion);
		descriptor.AddValue(INiDescriptor::Feature::Distance, distanceScore);
		descriptor.AddValue(INiDescriptor::Feature::Velocity, velocityScore);

		result.descriptor = std::make_unique<NiDescriptor<NiType::Deepthroat>>(std::move(descriptor));
		result.velocity = avgVelocity;

		return result;
	}

	NiInteraction EvaluateSkullfuck(const NiMotion& a_motionA, const NiMotion& a_motionB)
	{
		NiInteraction result{};
		assert(a_motionA.HasSufficientData() && a_motionB.HasSufficientData());
		const auto headBound = a_motionA.GetLatestHeadBound();
		if (!a_motionB.HasMomentData(NiMotion::pSchlongBase) || !headBound.IsValid()) {
			return result;
		}

		// a_motionA: receiving actor (head)
		// a_motionB: penetrating actor (with schlong)
		const auto headEntryMotion = a_motionA.DescribeMotion(NiMotion::pHead);
		const auto schlongBaseMotion = a_motionB.DescribeMotion(NiMotion::pSchlongBase);
		if (!headEntryMotion.DescribesMotion() && !schlongBaseMotion.DescribesMotion()) {
			return result;
		}

		const auto pSchlongStart = a_motionB.GetLatestMoment(NiMotion::pSchlongBase);
		const auto pSchlongEnd = a_motionB.GetLatestMoment(NiMotion::pSchlongTip);
		const NiMath::Segment sSchlong{ pSchlongStart, pSchlongEnd };
		const auto pHead = a_motionA.GetLatestMoment(NiMotion::pHead);

		const auto distance = pSchlongEnd.GetDistance(pHead);
		const auto distanceLimit = headBound.boundMax.y * Settings::fCloseToHeadRatio;
		if (distance > distanceLimit * 2.0f) {
			return result;
		}
		const auto avgVelocity = 0.5f * (headEntryMotion.avgSpeed + schlongBaseMotion.avgSpeed);

		const auto vHead = a_motionA.GetLatestMoment(NiMotion::vHeadY);
		auto vSchlong = schlongBaseMotion.DescribesMotion() ? schlongBaseMotion.trajectory.Vector() : sSchlong.Vector();
		NiMath::EnsureAntiParallelDirection(vSchlong, vHead);

		const float distanceScore = distance / distanceLimit;
		const float velocityScore = avgVelocity / Settings::fMinSpeedPenetration;

		auto descriptor = NiDescriptor<NiType::Skullfuck>();
		AddAngleScores(descriptor, vHead, vSchlong);
		AddBasicPairedScores(descriptor, headEntryMotion, schlongBaseMotion);
		descriptor.AddValue(INiDescriptor::Feature::Distance, distanceScore);
		descriptor.AddValue(INiDescriptor::Feature::Velocity, velocityScore);

		result.descriptor = std::make_unique<NiDescriptor<NiType::Skullfuck>>(std::move(descriptor));
		result.velocity = avgVelocity;

		return result;
	}

	NiInteraction EvaluateLickingShaft(const NiMotion& a_motionA, const NiMotion& a_motionB)
	{
		NiInteraction result{};
		assert(a_motionA.HasSufficientData() && a_motionB.HasSufficientData());
		const auto headBound = a_motionA.GetLatestHeadBound();
		if (!a_motionB.HasMomentData(NiMotion::pSchlongBase) || !headBound.IsValid()) {
			return result;
		}

		// a_motionA: receiving actor (head)
		// a_motionB: penetrating actor (with schlong)
		const auto headEntryMotion = a_motionA.DescribeMotion(NiMotion::pHead);
		const auto schlongBaseMotion = a_motionB.DescribeMotion(NiMotion::pSchlongBase);
		if (!headEntryMotion.DescribesMotion() && !schlongBaseMotion.DescribesMotion()) {
			return result;
		}

		const auto pSchlongStart = a_motionB.GetLatestMoment(NiMotion::pSchlongBase);
		const auto pSchlongEnd = a_motionB.GetLatestMoment(NiMotion::pSchlongTip);
		const NiMath::Segment sSchlong{ pSchlongStart, pSchlongEnd };
		const auto pMouth = a_motionA.GetLatestMoment(NiMotion::pMouth);

		const float distanceLimit = headBound.boundMax.y * Settings::fThroatToleranceRadius;
		const float distance = sSchlong.ShortestSegmentTo(pMouth).Length();
		if (distance > distanceLimit * 2.0f) {
			return result;
		}

		const auto avgVelocity = 0.5f * (headEntryMotion.avgSpeed + schlongBaseMotion.avgSpeed);
		const auto vHead = a_motionA.GetLatestMoment(NiMotion::vHeadX);
		auto vSchlong = schlongBaseMotion.DescribesMotion() ? schlongBaseMotion.trajectory.Vector() : sSchlong.Vector();
		NiMath::EnsureAntiParallelDirection(vSchlong, vHead);

		const float distanceScore = distance / distanceLimit;
		const float velocityScore = avgVelocity / Settings::fMinSpeedPenetration;

		auto descriptor = NiDescriptor<NiType::LickingShaft>();
		AddAngleScores(descriptor, vHead, vSchlong);
		AddBasicPairedScores(descriptor, headEntryMotion, schlongBaseMotion);
		descriptor.AddValue(INiDescriptor::Feature::Distance, distanceScore);
		descriptor.AddValue(INiDescriptor::Feature::Velocity, velocityScore);

		result.descriptor = std::make_unique<NiDescriptor<NiType::LickingShaft>>(std::move(descriptor));
		result.velocity = avgVelocity;

		return result;
	}

	NiInteraction EvaluateKissing(const NiMotion& a_motionA, const NiMotion& a_motionB)
	{
		NiInteraction result{};
		assert(a_motionA.HasSufficientData() && a_motionB.HasSufficientData());

		const auto mouthA = a_motionA.DescribeMotion(NiMotion::pMouth);
		const auto mouthB = a_motionB.DescribeMotion(NiMotion::pMouth);
		const float mouthDistance = mouthA.Mean().GetDistance(mouthB.Mean());
		if (mouthDistance > Settings::fDistanceMouth * 2.0f) {
			return result;
		}

		const float avgVelocity = 0.5f * (mouthA.avgSpeed + mouthB.avgSpeed);
		const auto vHeadYA = a_motionA.GetLatestMoment(NiMotion::vHeadY);
		const auto vHeadYB = a_motionB.GetLatestMoment(NiMotion::vHeadY);

		const float distanceScore = mouthDistance / Settings::fDistanceMouth;
		const float velocityScore = avgVelocity / Settings::fMaxKissSpeed;

		auto descriptor = NiDescriptor<NiType::Kissing>();
		AddAngleScores(descriptor, vHeadYA, vHeadYB);
		AddBasicPairedScores(descriptor, mouthA, mouthB);
		descriptor.AddValue(INiDescriptor::Feature::Distance, distanceScore);
		descriptor.AddValue(INiDescriptor::Feature::Velocity, velocityScore);

		result.descriptor = std::make_unique<NiDescriptor<NiType::Kissing>>(std::move(descriptor));
		result.velocity = avgVelocity;

		return result;
	}

}  // namespace Thread::NiNode
