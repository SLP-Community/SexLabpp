#include "NiInteraction.h"

#include "NiMath.h"

namespace Thread::NiNode
{

    bool NiInteractionCluster::IncludesType(NiType::Type type) const
    {
        return std::ranges::any_of(interactions, [type](const NiInteraction& interaction) {
            return interaction.GetType() == type;
        });
    }

    NiType::Cluster NiInteractionCluster::GetClusterType() const
    {
        if (interactions.empty()) {
            return NiType::Cluster::None;
        }
        return NiType::GetClusterForType(interactions.front().GetType());
    }

    NiInteraction* NiInteractionCluster::ApplySoftmax()
    {
        if (interactions.empty()) {
            return nullptr;
        } else if (interactions.size() == 1) {
            return &interactions.front();
        }

        std::vector<float> logits;
        logits.reserve(interactions.size());
        for (auto& interaction : interactions) {
            if (!interaction.descriptor)
                logits.push_back(-std::numeric_limits<float>::infinity());
            else
                logits.push_back(interaction.descriptor->Predict());
        }
        float maxLogit = *std::max_element(logits.begin(), logits.end());

        std::vector<float> expVals;
        expVals.reserve(logits.size());

        float sum = 0.0f;
        for (float z : logits) {
            float e = std::exp(z - maxLogit);
            expVals.push_back(e);
            sum += e;
        }
        for (float& e : expVals)
            e /= sum;

        int bestIndex = 0;
        float bestProb = expVals[0];
        for (int i = 1; i < expVals.size(); ++i) {
            if (expVals[i] > bestProb) {
                bestProb = expVals[i];
                bestIndex = i;
            }
        }

        if (bestProb < Settings::fEnterThresholdSoftmax) {
            return nullptr;
        }

        return &interactions[bestIndex];
    }

    namespace
    {
        template <typename NiDescriptorType>
        void AddBasicPairedScores01(NiDescriptorType* descriptor, const MotionDescriptor& motionA, const MotionDescriptor& motionB)
        {
            const float duration = std::min(motionA.duration, motionB.duration);
            const float timeScore = duration / Settings::fMinTypeDuration;
            const float oscillationScore = 0.5f * (motionA.oscillation + motionB.oscillation);
            const float impulseScore = 0.5f * (motionA.impulse + motionB.impulse);
            const float stabilityScore = 0.5f * (motionA.positionalVariance + motionB.positionalVariance);
            const float stabilityVarScore = 0.5f * (motionA.directionalVariance + motionB.directionalVariance);

            descriptor->AddValue(INiDescriptor::Feature::Time01, timeScore);
            descriptor->AddValue(INiDescriptor::Feature::Oscillation01, oscillationScore);
            descriptor->AddValue(INiDescriptor::Feature::Impulse01, impulseScore);
            descriptor->AddValue(INiDescriptor::Feature::Stability01, stabilityScore);
            descriptor->AddValue(INiDescriptor::Feature::StabilityVariance01, stabilityVarScore);
        }

        template <typename NiDescriptorType>
        void AddBasicPairedScores02(NiDescriptorType* descriptor, const MotionDescriptor& motionA, const MotionDescriptor& motionB)
        {
            const float duration = std::min(motionA.duration, motionB.duration);
            const float timeScore = duration / Settings::fMinTypeDuration;
            const float oscillationScore = 0.5f * (motionA.oscillation + motionB.oscillation);
            const float impulseScore = 0.5f * (motionA.impulse + motionB.impulse);
            const float stabilityScore = 0.5f * (motionA.positionalVariance + motionB.positionalVariance);
            const float stabilityVarScore = 0.5f * (motionA.directionalVariance + motionB.directionalVariance);

            descriptor->AddValue(INiDescriptor::Feature::Time02, timeScore);
            descriptor->AddValue(INiDescriptor::Feature::Oscillation02, oscillationScore);
            descriptor->AddValue(INiDescriptor::Feature::Impulse02, impulseScore);
            descriptor->AddValue(INiDescriptor::Feature::Stability02, stabilityScore);
            descriptor->AddValue(INiDescriptor::Feature::StabilityVariance02, stabilityVarScore);
        }

        template <typename NiDescriptorType>
        void AddBasicPairedScores03(NiDescriptorType* descriptor, const MotionDescriptor& motionA, const MotionDescriptor& motionB)
        {
            const float duration = std::min(motionA.duration, motionB.duration);
            const float timeScore = duration / Settings::fMinTypeDuration;
            const float oscillationScore = 0.5f * (motionA.oscillation + motionB.oscillation);
            const float impulseScore = 0.5f * (motionA.impulse + motionB.impulse);
            const float stabilityScore = 0.5f * (motionA.positionalVariance + motionB.positionalVariance);
            const float stabilityVarScore = 0.5f * (motionA.directionalVariance + motionB.directionalVariance);

            descriptor->AddValue(INiDescriptor::Feature::Time03, timeScore);
            descriptor->AddValue(INiDescriptor::Feature::Oscillation03, oscillationScore);
            descriptor->AddValue(INiDescriptor::Feature::Impulse03, impulseScore);
            descriptor->AddValue(INiDescriptor::Feature::Stability03, stabilityScore);
            descriptor->AddValue(INiDescriptor::Feature::StabilityVariance03, stabilityVarScore);
        }
    }

    NiInteractionCluster EvaluateCrotchInteractions(const NiMotion& a_motionA, const NiMotion& a_motionB)
    {
        // a_motionA: receiving actor (vagina/anal/grinding)
        // a_motionB: penetrating actor (with schlong)
        NiInteractionCluster result{};
        assert(a_motionA.HasSufficientData() && a_motionB.HasSufficientData());
        if (!a_motionB.HasMomentData(NiMotion::pSchlongTip)) {
            return result;
        }

        const auto schlongTipMotion = a_motionB.DescribeMotion(NiMotion::pSchlongTip);
        const auto pSchlongStart = a_motionB.GetLatestMoment(NiMotion::pSchlongBase);
        const auto pSchlongEnd = a_motionB.GetLatestMoment(NiMotion::pSchlongTip);
        const NiMath::Segment sSchlong{ pSchlongStart, pSchlongEnd };
        const auto vSchlong = schlongTipMotion.DescribesMotion() ? schlongTipMotion.trajectory.Vector() : sSchlong.Vector();

        auto descV = std::make_unique<NiDescriptor<NiType::Type::Vaginal>>();
        auto descA = std::make_unique<NiDescriptor<NiType::Type::Anal>>();
        auto descG = std::make_unique<NiDescriptor<NiType::Type::Grinding>>();
        auto descN = std::make_unique<NiDescriptor<NiType::Type::Crotch_NONE>>();

        const auto EvaluateInteraction = [&](NiMotion::Anchor motionAnchor, NiMotion::Anchor motionAnchorEnd)
            -> std::optional<std::tuple<const MotionDescriptor, RE::NiPoint3, float, float, float>> {
            if (!a_motionA.HasMomentData(motionAnchor)) {
                return std::nullopt;
            }
            const auto motion = a_motionA.DescribeMotion(motionAnchor);
            if (!motion.DescribesMotion() && !schlongTipMotion.DescribesMotion()) {
                return std::nullopt;
            }
            const auto pEntry = a_motionA.GetLatestMoment(motionAnchor);
            const auto sDistance = sSchlong.ShortestSegmentTo(pEntry);
            const auto distance = sDistance.Length();
            if (distance > Settings::fDistanceCrotch * 2.0f) {
                return std::nullopt;
            }
            const float avgVelocity = 0.5f * (motion.avgSpeed + schlongTipMotion.avgSpeed);
            const auto pEntryEnd = a_motionA.GetLatestMoment(motionAnchorEnd);
            auto vEntry = motion.DescribesMotion() ? motion.trajectory.Vector() : (pEntryEnd - pEntry);
            NiMath::EnsureParallelDirection(vEntry, vSchlong);

            float distanceScore = distance / Settings::fDistanceCrotch;
            float velocityScore = avgVelocity / Settings::fMinSpeedPenetration;
            return { { motion, vEntry, distanceScore, velocityScore, avgVelocity } };
        };

#define ADD_DATA(res, idx)                                                     \
    if (res) {                                                                 \
        const auto& [motion, vEntry, distanceScore, velocityScore, _] = *res;  \
        const auto cos = NiMath::GetAngleCos(vEntry, vSchlong);                \
        descV->AddValue(INiDescriptor::Feature::Angle##idx, cos);              \
        descA->AddValue(INiDescriptor::Feature::Angle##idx, cos);              \
        descG->AddValue(INiDescriptor::Feature::Angle##idx, cos);              \
        descN->AddValue(INiDescriptor::Feature::Angle##idx, cos);              \
        AddBasicPairedScores##idx(descV.get(), motion, schlongTipMotion);      \
        AddBasicPairedScores##idx(descA.get(), motion, schlongTipMotion);      \
        AddBasicPairedScores##idx(descG.get(), motion, schlongTipMotion);      \
        AddBasicPairedScores##idx(descN.get(), motion, schlongTipMotion);      \
        descV->AddValue(INiDescriptor::Feature::Distance##idx, distanceScore); \
        descV->AddValue(INiDescriptor::Feature::Velocity##idx, velocityScore); \
        descA->AddValue(INiDescriptor::Feature::Distance##idx, distanceScore); \
        descA->AddValue(INiDescriptor::Feature::Velocity##idx, velocityScore); \
        descG->AddValue(INiDescriptor::Feature::Distance##idx, distanceScore); \
        descG->AddValue(INiDescriptor::Feature::Velocity##idx, velocityScore); \
        descN->AddValue(INiDescriptor::Feature::Distance##idx, distanceScore); \
        descN->AddValue(INiDescriptor::Feature::Velocity##idx, velocityScore); \
    }

        const auto resV = EvaluateInteraction(NiMotion::pVaginalStart, NiMotion::pVaginalEnd);
        const auto resA = EvaluateInteraction(NiMotion::pAnalStart, NiMotion::pAnalEnd);
        const auto resG = EvaluateInteraction(NiMotion::pPelvis, NiMotion::pSpineLower);
        ADD_DATA(resV, 01)
        ADD_DATA(resA, 02)
        ADD_DATA(resG, 03)
#undef ADD_DATA

        result.interactions.emplace_back(std::move(descV), std::get<4>(*resV));
        result.interactions.emplace_back(std::move(descA), std::get<4>(*resA));
        result.interactions.emplace_back(std::move(descG), std::get<4>(*resG));
        result.interactions.emplace_back(std::move(descN), 0.0f);

        return result;
    }

    NiInteractionCluster EvaluateHeadInteractions(const NiMotion& a_motionA, const NiMotion& a_motionB)
    {
        // a_motionA: receiving actor
        // a_motionB: penetrating actor (with schlong)
        NiInteractionCluster result{};
        assert(a_motionA.HasSufficientData() && a_motionB.HasSufficientData());
        const auto headBound = a_motionA.GetLatestHeadBound();
        if (!a_motionB.HasMomentData(NiMotion::pSchlongBase) || !headBound.IsValid()) {
            return result;
        }

        const auto headEntryMotion = a_motionA.DescribeMotion(NiMotion::pHead);
        const auto schlongBaseMotion = a_motionB.DescribeMotion(NiMotion::pSchlongBase);
        if (!headEntryMotion.DescribesMotion() && !schlongBaseMotion.DescribesMotion()) {
            return result;
        }

        const auto pSchlongStart = a_motionB.GetLatestMoment(NiMotion::pSchlongBase);
        const auto pSchlongEnd = a_motionB.GetLatestMoment(NiMotion::pSchlongTip);
        const NiMath::Segment sSchlong{ pSchlongStart, pSchlongEnd };
        auto vSchlong = schlongBaseMotion.DescribesMotion() ? schlongBaseMotion.trajectory.Vector() : sSchlong.Vector();

        const auto pMouth = a_motionA.GetLatestMoment(NiMotion::pMouth);
        const auto pHead = a_motionA.GetLatestMoment(NiMotion::pHead);
        const auto pPelvis = a_motionA.GetLatestMoment(NiMotion::pPelvis);

        const float distanceClose = headBound.boundMax.y * Settings::fCloseToHeadRatio;
        const float distanceVeryClose = headBound.boundMax.y * Settings::fVeryCloseToHeadRatio;
        const float distanceOral = sSchlong.ShortestSegmentTo(pMouth).Length();
        const float distanceSkull = pSchlongEnd.GetDistance(pHead);
        const float distanceDP = headBound.IsPointInside(pPelvis) ? 0.1f : pHead.GetDistance(pSchlongEnd);
        if (distanceSkull > distanceClose * 3.0f) {
            return result;
        }

        const auto vHeadY = a_motionA.GetLatestMoment(NiMotion::vHeadY);
        const auto vHeadX = a_motionA.GetLatestMoment(NiMotion::vHeadX);
        const auto vHeadZ = a_motionA.GetLatestMoment(NiMotion::vHeadZ);
        NiMath::EnsureAntiParallelDirection(vSchlong, vHeadY);

        const auto cosY = NiMath::GetAngleCos(vHeadY, vSchlong);
        const auto cosX = NiMath::GetAngleCos(vHeadX, vSchlong);
        const auto cosZ = NiMath::GetAngleCos(vHeadZ, vSchlong);

        const auto avgVelocity = 0.5f * (headEntryMotion.avgSpeed + schlongBaseMotion.avgSpeed);
        const auto velocityScore = avgVelocity / Settings::fMinSpeedPenetration;
        const auto distanceScoreOral = distanceOral / distanceClose;
        const auto distanceScoreShaft = distanceOral / distanceVeryClose;
        const auto distanceScoreSkull = distanceSkull / distanceClose;
        const auto distanceScoreDP = distanceDP / distanceVeryClose;

        auto descO = std::make_unique<NiDescriptor<NiType::Type::Oral>>();
        auto descDP = std::make_unique<NiDescriptor<NiType::Type::Deepthroat>>();
        auto descSK = std::make_unique<NiDescriptor<NiType::Type::Skullfuck>>();
        auto descLS = std::make_unique<NiDescriptor<NiType::Type::LickingShaft>>();
        auto descN = std::make_unique<NiDescriptor<NiType::Type::Head_NONE>>();

#define ADD_DATA(descriptor)                                                      \
    descriptor->AddValue(INiDescriptor::Feature::Angle01, cosY);                  \
    descriptor->AddValue(INiDescriptor::Feature::Angle02, cosX);                  \
    descriptor->AddValue(INiDescriptor::Feature::Angle03, cosZ);                  \
    AddBasicPairedScores01(descriptor.get(), headEntryMotion, schlongBaseMotion); \
    descriptor->AddValue(INiDescriptor::Feature::Distance01, distanceScoreOral);  \
    descriptor->AddValue(INiDescriptor::Feature::Distance02, distanceScoreShaft); \
    descriptor->AddValue(INiDescriptor::Feature::Distance03, distanceScoreSkull); \
    descriptor->AddValue(INiDescriptor::Feature::Distance04, distanceScoreDP);    \
    descriptor->AddValue(INiDescriptor::Feature::Velocity01, velocityScore);      \
    result.interactions.emplace_back(std::move(descriptor), avgVelocity);

        ADD_DATA(descO)
        ADD_DATA(descDP)
        ADD_DATA(descSK)
        ADD_DATA(descLS)
        ADD_DATA(descN)

#undef ADD_DATA

        return result;
    }

    NiInteractionCluster EvaluateKissingCluster(const NiMotion& a_motionA, const NiMotion& a_motionB)
    {
        NiInteractionCluster result{};
        assert(a_motionA.HasSufficientData() && a_motionB.HasSufficientData());

        const auto mouthA = a_motionA.DescribeMotion(NiMotion::pMouth);
        const auto mouthB = a_motionB.DescribeMotion(NiMotion::pMouth);
        const float mouthDistance = mouthA.Mean().GetDistance(mouthB.Mean());
        if (mouthDistance > Settings::fDistanceMouth * 2.0f) {
            return result;
        }

        const float avgVelocity = 0.5f * (mouthA.avgSpeed + mouthB.avgSpeed);
        const auto vHeadYA = a_motionA.GetLatestMoment(NiMotion::vHeadY);
        const auto vHeadXA = a_motionA.GetLatestMoment(NiMotion::vHeadX);
        const auto vHeadZA = a_motionA.GetLatestMoment(NiMotion::vHeadZ);
        const auto vHeadYB = a_motionB.GetLatestMoment(NiMotion::vHeadY);
        const auto vHeadXB = a_motionB.GetLatestMoment(NiMotion::vHeadX);
        const auto vHeadZB = a_motionB.GetLatestMoment(NiMotion::vHeadZ);

        const float distanceScore = mouthDistance / Settings::fDistanceMouth;
        const float velocityScore = avgVelocity / Settings::fMaxKissSpeed;
        const float cosX = NiMath::GetAngleCos(vHeadYA, vHeadYB);
        const float cosY = NiMath::GetAngleCos(vHeadXA, vHeadXB);
        const float cosZ = NiMath::GetAngleCos(vHeadZA, vHeadZB);

        auto descriptor = std::make_unique<NiDescriptor<NiType::Type::Kissing>>();
        descriptor->AddValue(INiDescriptor::Feature::Angle01, cosX);
        descriptor->AddValue(INiDescriptor::Feature::Angle02, cosY);
        descriptor->AddValue(INiDescriptor::Feature::Angle03, cosZ);
        AddBasicPairedScores01(descriptor.get(), mouthA, mouthB);
        descriptor->AddValue(INiDescriptor::Feature::Distance01, distanceScore);
        descriptor->AddValue(INiDescriptor::Feature::Velocity01, velocityScore);

        result.interactions.emplace_back(std::move(descriptor), avgVelocity);
        return result;
    }

}  // namespace Thread::NiNode
