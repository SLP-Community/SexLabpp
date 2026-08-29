#include "ActorState.h"

namespace Thread::Interaction::NiSurface
{
    namespace
    {
        float DistanceToOpening(const GeometryMath::Segment& a_segment, const OpeningShape& a_opening, float a_startRadius = 0.0f, float a_endRadius = 0.0f)
        {
            const auto vector = a_segment.Vector();
            const auto lengthSq = vector.SqrLength();
            if (lengthSq <= FLT_EPSILON) {
                const auto offset = a_segment.start - a_opening.center;
                const auto planeDistance = offset.Dot(a_opening.axis);
                const auto radialDistance = (offset - a_opening.axis * planeDistance).Length();
                const auto radialMiss = std::max(radialDistance - a_opening.radius - a_startRadius, 0.0f);
                return std::sqrt(radialMiss * radialMiss + planeDistance * planeDistance);
            }

            const auto planeDenominator = vector.Dot(a_opening.axis);
            const auto toCenter = a_opening.center - a_segment.start;
            const auto t = std::clamp(
                std::abs(planeDenominator) > FLT_EPSILON ? toCenter.Dot(a_opening.axis) / planeDenominator : toCenter.Dot(vector) / lengthSq,
                0.0f,
                1.0f);
            const auto offset = a_segment.start + vector * t - a_opening.center;
            const auto planeDistance = offset.Dot(a_opening.axis);
            const auto radialDistance = (offset - a_opening.axis * planeDistance).Length();
            const auto shaftRadius = a_startRadius + (a_endRadius - a_startRadius) * t;
            const auto radialMiss = std::max(radialDistance - a_opening.radius - shaftRadius, 0.0f);
            return std::sqrt(radialMiss * radialMiss + planeDistance * planeDistance);
        }

        float DistanceToOpening(const ShaftShape& a_shaft, const OpeningShape& a_opening)
        {
            if (a_shaft.sections.size() < 2) {
                return std::numeric_limits<float>::max();
            }
            // Before entry, only the physical tip can approach the opening; after entry, test the inserted capsule chain.
            if ((a_shaft.tip - a_opening.center).Dot(a_opening.axis) < 0.0f) {
                return DistanceToOpening(GeometryMath::Segment{ a_shaft.tip }, a_opening);
            }
            float distance = std::numeric_limits<float>::max();
            for (std::size_t i = 1; i < a_shaft.sections.size(); ++i) {
                distance = std::min(distance, DistanceToOpening({ a_shaft.sections[i - 1].center, a_shaft.sections[i].center }, a_opening,
                                                  a_shaft.sections[i - 1].radius, a_shaft.sections[i].radius));
            }
            return std::min(distance, DistanceToOpening({ a_shaft.sections.back().center, a_shaft.tip }, a_opening, a_shaft.sections.back().radius, 0.0f));
        }

        RE::NiPoint3 GetShaftTipDirection(const ShaftShape& a_shaft)
        {
            if (a_shaft.sections.empty()) {
                return {};
            }
            auto direction = a_shaft.tip - a_shaft.sections.back().center;
            if (direction.SqrLength() <= FLT_EPSILON && a_shaft.sections.size() >= 2) {
                direction = a_shaft.sections.back().center - a_shaft.sections[a_shaft.sections.size() - 2].center;
            }
            return direction;
        }

        float GetNormalizedPositionAlongShaft(const GeometryMath::Segment& a_shaft, const RE::NiPoint3& a_point)
        {
            const auto shaft = a_shaft.Vector();
            const auto lengthSq = shaft.SqrLength();
            return lengthSq > FLT_EPSILON ? std::clamp((a_point - a_shaft.start).Dot(shaft) / lengthSq, 0.0f, 1.0f) : 0.0f;
        }
    }

    bool RotateNode(RE::NiPointer<RE::NiNode> a_node, const GeometryMath::Segment& a_segment, const RE::NiPoint3& a_target, float a_maxAngle)
    {
        const auto targetVector = a_target - a_segment.start;
        if (!a_node || targetVector.SqrLength() <= FLT_EPSILON || a_segment.Vector().SqrLength() <= FLT_EPSILON) {
            return false;
        }
        const Eigen::Vector3f segment = GeometryMath::ToEigen(a_segment.Vector()).normalized();
        const Eigen::Vector3f target = GeometryMath::ToEigen(targetVector).normalized();
        float angle = std::acos(std::clamp(segment.dot(target), -1.0f, 1.0f));
        if (angle < FLT_EPSILON) {
            return true;
        }
        a_maxAngle = glm::radians(a_maxAngle);
        if (angle > a_maxAngle) {
            return false;
        }
        if (!Settings::bAdjustNodes) {
            return true;
        }
        auto& local = a_node->local.rotate;
        const Eigen::Quaternionf worldQuat(GeometryMath::ToEigen(a_node->world.rotate));
        const Eigen::Quaternionf localQuat(GeometryMath::ToEigen(local));
        auto adjustedLocal = worldQuat.conjugate() * localQuat;

        auto rotationAxis = segment.cross(target);
        if (rotationAxis.norm() > FLT_EPSILON) {
            rotationAxis.normalize();
            angle = std::min(angle, a_maxAngle);
            const auto rotation = Eigen::AngleAxisf{ angle, rotationAxis };
            const Eigen::Quaternionf rotationQuaternion{ rotation.inverse() };
            adjustedLocal = rotationQuaternion * adjustedLocal;
        }

        const Eigen::Quaternionf result = worldQuat * adjustedLocal;
        local = GeometryMath::ToNiMatrix(result.toRotationMatrix());

        RE::NiUpdateData update{ 0.5f, RE::NiUpdateData::Flag::kNone };
        a_node->Update(update);
        return true;
    }

    ActorState::Frame::Frame(ActorState& a_state) :
      state(a_state),
      headBounds([&]() {
          auto* head = a_state.geometry.head.get();
          if (!head)
              return ObjectBound{};
          const auto bounds = ObjectBound::MakeBoundingBox(head);
          return bounds ? *bounds : ObjectBound{};
      }()),
      mouthOpening(a_state.geometry.GetMouthOpening()),
      vaginalOpening(a_state.geometry.GetVaginalOpening()),
      analOpening(a_state.geometry.GetAnalOpening())
    {
        a_state.geometry.UpdateShafts();
    }

    bool ActorState::Frame::DetectKissing(const Frame& a_partner)
    {
        const auto mouthCenter = GetMouthCenter();
        const auto partnerMouthCenter = a_partner.GetMouthCenter();
        if (!mouthCenter || !partnerMouthCenter)
            return false;
        const auto distance = mouthCenter->GetDistance(*partnerMouthCenter);
        if (distance > Settings::fDistanceMouth)
            return false;
        const auto vMyHead = *mouthCenter - state.geometry.head->world.translate;
        const auto vPartnerHead = *partnerMouthCenter - a_partner.state.geometry.head->world.translate;
        auto angle = GeometryMath::GetAngleDegree(vMyHead, vPartnerHead);
        if (std::abs(angle - 180) > Settings::fAngleKissing) {
            return false;
        }
        interactions.emplace_back(a_partner.state.actor, Interaction::Action::Kissing, distance, *mouthCenter - *partnerMouthCenter);
        return true;
    }

    bool ActorState::Frame::DetectToeSucking(const Frame& a_partner)
    {
        if (!headBounds.IsValid())
            return false;
        const auto footL = a_partner.state.geometry.leftToe;
        const auto footR = a_partner.state.geometry.rightToe;
        if (!footL || !footR)
            return false;
        const auto mouth = GetMouthCenter();
        if (!mouth)
            return false;
        const auto distanceLeft = footL->world.translate.GetDistance(*mouth);
        const auto distanceRight = footR->world.translate.GetDistance(*mouth);
        if (distanceLeft > Settings::fDistanceFootMouth && distanceRight > Settings::fDistanceFootMouth)
            return false;
        const bool useLeft = distanceLeft < distanceRight;
        const auto toePosition = useLeft ? footL->world.translate : footR->world.translate;
        interactions.emplace_back(a_partner.state.actor, Interaction::Action::ToeSucking, std::min(distanceLeft, distanceRight), *mouth - toePosition,
            1.0f, useLeft ? 1 : 2);
        return true;
    }

    bool ActorState::Frame::DetectShaftHead(const Frame& a_partner, const Geometry::Shaft& a_shaft)
    {
        if (!headBounds.IsValid()) {
            return false;
        }
        assert(state.geometry.head);
        const auto& headWorld = state.geometry.head->world;
        const auto shaftSegment = a_shaft.GetReferenceSegment();
        const auto shaftTip = shaftSegment.end;
        const auto* shaftShape = a_shaft.GetCollisionShape();
        const auto headDistance = [&]() {
            const auto closest = GeometryMath::ClosestSegmentBetweenSegments(GeometryMath::Segment{ headWorld.translate }, shaftSegment);
            return closest.Length();
        }();
        const auto mouthDistance = mouthOpening ? (shaftShape ? DistanceToOpening(*shaftShape, *mouthOpening) : DistanceToOpening(shaftSegment, *mouthOpening)) : headDistance;
        if (headDistance > headBounds.boundMax.y * Settings::fCloseToHeadRatio) {
            return false;
        }
        const auto& partnerGeometry = a_partner.state.geometry;
        const auto baseNode = a_shaft.GetBaseReferenceNode();
        const auto vHead = headWorld.rotate.GetVectorY();

        const auto [angleToHead, angleToMouth, angleToBase] = [&]() {
            const auto vBaseToHead = headWorld.translate - shaftSegment.start;
            const auto vPartnerDir = partnerGeometry.GetCrotchSegment().Vector();
            const auto proj1 = GeometryMath::ProjectedComponent(vPartnerDir, vHead);
            const auto proj2 = GeometryMath::ProjectedComponent(vBaseToHead, vHead);
            return std::make_tuple(
                GeometryMath::GetAngleDegree(proj1, proj2),
                GeometryMath::GetAngleDegree(proj1, -vHead),
                GeometryMath::GetAngleDegree(proj2, vHead));
        }();

        const auto aimingAtHead = std::abs(angleToHead - angleToMouth) < Settings::fAngleToHeadTolerance;
        const auto atSideOfHead = std::abs(angleToBase - 90) < Settings::fAngleToHeadSidewaysTolerance;
        const auto inFrontOfHead = std::abs(angleToBase - 180) < Settings::fAngleToHeadFrontalTolerance;
        const auto penetratingSkull = headDistance < (atSideOfHead ? headBounds.boundMax.x : headBounds.boundMax.y);
        const auto contactingMouth = mouthOpening ? mouthDistance <= Settings::fDistanceMouth : penetratingSkull;
        const auto verticalToShaft = [&]() {
            const auto shaftVector = shaftSegment.Vector();
            const auto shaftToMouthAngle = GeometryMath::GetAngleDegree(shaftVector, vHead);
            return std::abs(shaftToMouthAngle - 90) < 30.0f;
        }();
        const auto closeToMouth = [&]() {
            return mouthDistance < headBounds.boundMax.x && mouthDistance < headDistance;
        }();

        if (inFrontOfHead && verticalToShaft && closeToMouth) {
            const auto mouth = GetMouthCenter();
            assert(mouth);
            interactions.emplace_back(a_partner.state.actor, Interaction::Action::LickingShaft, mouthDistance,
                RE::NiPoint3{ GetNormalizedPositionAlongShaft(shaftSegment, *mouth), 0.0f, 0.0f }, shaftSegment.Length());
            return true;
        } else if (contactingMouth && inFrontOfHead && aimingAtHead) {
            const auto throat = GetThroatPoint(), mouth = GetMouthCenter();
            assert(throat && mouth);
            if (!baseNode || RotateNode(baseNode, shaftSegment, *throat, Settings::fAdjustSchlongLimit)) {
                RotateNode(state.geometry.head, { *mouth, *throat }, shaftSegment.start, Settings::fAdjustHeadLimit);
                interactions.emplace_back(a_partner.state.actor, Interaction::Action::Oral, mouthDistance, shaftTip - *mouth);
                assert(partnerGeometry.pelvis);
                const auto throatDistance = shaftShape ? shaftShape->tip.GetDistance(*throat) : GeometryMath::ClosestSegmentBetweenSegments(GeometryMath::Segment{ *throat }, shaftSegment).Length();
                const auto tipAtThroat = throatDistance < headBounds.boundMax.y * Settings::fThroatToleranceRadius;
                const auto pelvisAtHead = headBounds.IsPointInside(partnerGeometry.pelvis->world.translate);
                if (tipAtThroat || pelvisAtHead) {
                    interactions.emplace_back(a_partner.state.actor, Interaction::Action::Deepthroat, throatDistance, shaftTip - *throat);
                }
                return true;
            }
        } else if (penetratingSkull && aimingAtHead) {
            if (!baseNode || RotateNode(baseNode, shaftSegment, headWorld.translate, Settings::fAdjustSchlongLimit)) {
                interactions.emplace_back(a_partner.state.actor, Interaction::Action::Skullfuck, headDistance, shaftTip - headWorld.translate);
            }
            return true;
        } else if (inFrontOfHead && aimingAtHead) {
            interactions.emplace_back(a_partner.state.actor, Interaction::Action::Facial, headDistance, shaftTip - headWorld.translate);
            return true;
        }
        return false;
    }

    bool ActorState::Frame::DetectShaftCrotch(const Frame& a_partner, const Geometry::Shaft& a_shaft)
    {
        const auto shaftSegment = a_shaft.GetReferenceSegment();
        const auto shaftTip = shaftSegment.end;
        const auto shaftNode = a_shaft.GetBaseReferenceNode();
        const auto* shaftShape = a_shaft.GetCollisionShape();
        if (vaginalOpening && analOpening) {
            const auto [type, segment, distance] = [&]() {
                enum
                {
                    tNone,
                    tVaginal,
                    tAnal
                };
                const auto tLast = [&] {
                    const auto where = std::ranges::find_if(state.interactions, [&](const Interaction& it) {
                        return it.partner == a_partner.state.actor && (it.action == Interaction::Action::Vaginal || it.action == Interaction::Action::Anal);
                    });
                    if (where == state.interactions.end()) {
                        return tNone;
                    } else if (where->action == Interaction::Action::Vaginal) {
                        return tVaginal;
                    } else {
                        return tAnal;
                    }
                }();
                const auto dVaginal = shaftShape ? DistanceToOpening(*shaftShape, *vaginalOpening) : DistanceToOpening(shaftSegment, *vaginalOpening);
                const auto dAnal = shaftShape ? DistanceToOpening(*shaftShape, *analOpening) : DistanceToOpening(shaftSegment, *analOpening);
                const auto dif = dVaginal - dAnal;
                bool branchVaginal = true;
                switch (tLast) {
                case tVaginal:
                    branchVaginal = dif < Settings::fPenetrationVaginalToleranceRepeat;
                    break;
                case tAnal:
                    branchVaginal = dif < -Settings::fPenetrationAnalToleranceRepeat;
                    break;
                default:
                    branchVaginal = dif < Settings::fPenetrationVaginalTolerance;
                    break;
                }
                // Prefer vaginal contact slightly when the two tracked openings are equally close.
                if (branchVaginal) {
                    return std::tuple{
                        Interaction::Action::Vaginal,
                        *vaginalOpening,
                        dVaginal
                    };
                } else {
                    return std::tuple{
                        Interaction::Action::Anal,
                        *analOpening,
                        dAnal
                    };
                }
            }();
            if (distance <= Settings::fDistanceCrotch) {
                const auto shaftDirection = shaftShape ? GetShaftTipDirection(*shaftShape) : shaftSegment.Vector();
                const auto aSegment = GeometryMath::GetAngleDegree(segment.axis, shaftDirection);
                if (aSegment <= Settings::fAnglePenetration && (!shaftNode || RotateNode(shaftNode, shaftSegment, segment.deep, Settings::fAdjustSchlongVaginalLimit))) {
                    interactions.emplace_back(a_partner.state.actor, type, distance, shaftTip - segment.center);
                    return true;
                }
                const auto crotchSegment = GeometryMath::Segment{ analOpening->center, vaginalOpening->center };
                const auto crotchAngle = GeometryMath::GetAngleDegree(crotchSegment.Vector(), shaftSegment.Vector());
                if (std::abs(crotchAngle - 180.0f) <= Settings::fAngleGrinding) {
                    interactions.emplace_back(a_partner.state.actor, Interaction::Action::Grinding, distance, shaftTip - segment.center);
                    return true;
                }
            }
        } else {
            const auto crotchSegment = state.geometry.GetCrotchSegment();
            const auto crotchDistance = GeometryMath::ClosestSegmentBetweenSegments(crotchSegment, shaftSegment).Length();
            if (crotchDistance <= Settings::fDistanceCrotch) {
                const auto vBaseToSpine = crotchSegment.start - shaftSegment.start;
                const auto crotchAngle = GeometryMath::GetAngleDegree(vBaseToSpine, shaftSegment.Vector());
                if (crotchAngle <= Settings::fAnglePenetration && (!shaftNode || RotateNode(shaftNode, shaftSegment, crotchSegment.start, Settings::fAdjustSchlongVaginalLimit))) {
                    interactions.emplace_back(a_partner.state.actor, Interaction::Action::Anal, crotchDistance, shaftTip - crotchSegment.start);
                    return true;
                } else if (std::abs(crotchAngle - 90.0f) <= Settings::fAngleGrinding) {
                    interactions.emplace_back(a_partner.state.actor, Interaction::Action::Anal, crotchDistance, shaftTip - crotchSegment.start);
                    return true;
                }
            }
        }
        return false;
    }

    bool ActorState::Frame::DetectShaftHand(const Frame& a_partner, const Geometry::Shaft& a_shaft)
    {
        const auto leftHand = state.geometry.leftHand;
        const auto rightHand = state.geometry.rightHand;
        const auto leftThumb = state.geometry.leftThumb;
        const auto rightThumb = state.geometry.rightThumb;
        if (!leftHand || !rightHand || !leftThumb || !rightThumb) {
            return false;
        }
        const auto shaftSegment = a_shaft.GetReferenceSegment();
        const auto leftPosition = leftHand->world.translate;
        const auto rightPosition = rightHand->world.translate;
        const auto leftDistance = GeometryMath::ClosestSegmentBetweenSegments(GeometryMath::Segment{ leftPosition }, shaftSegment).Length();
        const auto rightDistance = GeometryMath::ClosestSegmentBetweenSegments(GeometryMath::Segment{ rightPosition }, shaftSegment).Length();
        const auto closeToLeft = leftDistance < Settings::fDistanceHand;
        const auto closeToRight = rightDistance < Settings::fDistanceHand;
        bool pickLeft;
        if (!closeToRight && !closeToLeft) {
            return false;
        } else if (closeToRight && closeToLeft) {
            const auto shaftNode = a_shaft.GetBaseReferenceNode();
            pickLeft = shaftNode && shaftNode->world.translate.GetDistance(leftPosition) < shaftNode->world.translate.GetDistance(rightPosition);
        } else {
            pickLeft = closeToLeft;
        }
        const auto referencePoint = pickLeft ? (leftPosition + leftThumb->world.translate) / 2 : (rightPosition + rightThumb->world.translate) / 2;
        RotateNode(a_shaft.GetBaseReferenceNode(), shaftSegment, referencePoint, Settings::fAdjustSchlongLimit);
        interactions.emplace_back(a_partner.state.actor, Interaction::Action::HandJob, pickLeft ? leftDistance : rightDistance,
            RE::NiPoint3{ GetNormalizedPositionAlongShaft(shaftSegment, referencePoint), 0.0f, 0.0f }, shaftSegment.Length(), pickLeft ? 1 : 2);
        return true;
    }

    bool ActorState::Frame::DetectShaftFoot(const Frame& a_partner, const Geometry::Shaft& a_shaft)
    {
        const auto shaftSegment = a_shaft.GetReferenceSegment();
        const auto get = [&](const auto& foot, std::uint8_t source) {
            if (!foot)
                return false;
            const auto footPosition = foot->world.translate;
            const auto distance = GeometryMath::ClosestSegmentBetweenSegments(GeometryMath::Segment{ footPosition }, shaftSegment).Length();
            if (distance > Settings::fDistanceFoot)
                return false;
            interactions.emplace_back(a_partner.state.actor, Interaction::Action::FootJob, distance,
                RE::NiPoint3{ GetNormalizedPositionAlongShaft(shaftSegment, footPosition), 0.0f, 0.0f }, shaftSegment.Length(), source);
            return true;
        };
        return get(state.geometry.leftFoot, 1) || get(state.geometry.rightFoot, 2);
    }

    bool ActorState::Frame::DetectVaginalOral(const Frame& a_partner)
    {
        const auto mouthCenter = GetMouthCenter();
        if (!mouthCenter)
            return false;
        if (!a_partner.vaginalOpening)
            return false;
        const float distance = a_partner.vaginalOpening->center.GetDistance(*mouthCenter);
        if (distance > Settings::fDistanceMouth)
            return false;
        assert(state.geometry.head);
        const auto& headWorld = state.geometry.head->world;
        const auto vHead = headWorld.rotate.GetVectorY();
        const auto angle = GeometryMath::GetAngleDegree(a_partner.vaginalOpening->axis, vHead);
        if (angle > Settings::fAngleCunnilingus)
            return false;
        interactions.emplace_back(a_partner.state.actor, Interaction::Action::Oral, distance, *mouthCenter - a_partner.vaginalOpening->center);
        return true;
    }

    bool ActorState::Frame::DetectVaginalContact(const Frame& a_partner)
    {
        if (!vaginalOpening || !a_partner.vaginalOpening)
            return false;
        const auto distance = vaginalOpening->center.GetDistance(a_partner.vaginalOpening->center);
        if (distance > Settings::fDistanceCrotch)
            return false;
        const auto angle = GeometryMath::GetAngleDegree(vaginalOpening->axis, a_partner.vaginalOpening->axis);
        if (std::abs(angle - 180) > Settings::fAngleGrindingFF)
            return false;
        interactions.emplace_back(a_partner.state.actor, Interaction::Action::Grinding, distance, vaginalOpening->center - a_partner.vaginalOpening->center);
        return true;
    }

    bool ActorState::Frame::DetectVaginalLimb(const Frame& a_partner)
    {
        if (!a_partner.vaginalOpening)
            return false;
        const auto get = [&](const auto& limb, auto type, float maxDist, std::uint8_t source) {
            if (!limb)
                return false;
            const auto limbPosition = limb->world.translate;
            const auto distance = limbPosition.GetDistance(a_partner.vaginalOpening->center);
            if (distance > maxDist)
                return false;
            interactions.emplace_back(a_partner.state.actor, type, distance, limbPosition - a_partner.vaginalOpening->center, 1.0f, source);
            return true;
        };
        const auto lHand = state.geometry.leftHand;
        const auto rHand = state.geometry.rightHand;
        const auto lFoot = state.geometry.leftFoot;
        const auto rFoot = state.geometry.rightFoot;
        return get(lHand, Interaction::Action::HandJob, Settings::fDistanceHand, 1) ||
               get(rHand, Interaction::Action::HandJob, Settings::fDistanceHand, 2) ||
               get(lFoot, Interaction::Action::FootJob, Settings::fDistanceFoot, 1) ||
               get(rFoot, Interaction::Action::FootJob, Settings::fDistanceFoot, 2);
    }

    bool ActorState::Frame::DetectAnimObjectFace(const Frame& a_partner)
    {
        bool bAnimObjectLoaded;
        a_partner.state.actor->GetGraphVariableBool("bAnimObjectLoaded", bAnimObjectLoaded);
        if (!bAnimObjectLoaded)
            return false;
        const auto pMouth = GetMouthCenter();
        if (!pMouth)
            return false;
        const auto get = [&](const auto& animObj, std::uint8_t source) {
            if (!animObj)
                return false;
            const auto animObjectPosition = animObj->world.translate;
            const auto d = animObjectPosition.GetDistance(*pMouth);
            if (d > Settings::fDistanceAnimObj)
                return false;
            interactions.emplace_back(a_partner.state.actor, Interaction::Action::AnimObjFace, d, animObjectPosition - *pMouth, 1.0f, source);
            return true;
        };
        const auto& partnerGeometry = a_partner.state.geometry;
        return get(partnerGeometry.animObjectA, 1) || get(partnerGeometry.animObjectB, 2) || get(partnerGeometry.animObjectRight, 3) ||
               get(partnerGeometry.animObjectLeft, 4);
    }

    std::optional<RE::NiPoint3> ActorState::Frame::GetMouthCenter() const
    {
        if (mouthOpening) {
            return mouthOpening->center;
        }
        const auto throat = GetThroatPoint();
        if (!throat) {
            return std::nullopt;
        }
        const auto& head = state.geometry.head;
        assert(head);
        const auto forwardDistance = headBounds.boundMax.y * 0.88f;
        return head->world.rotate.GetVectorY() * forwardDistance + *throat;
    }

    std::optional<RE::NiPoint3> ActorState::Frame::GetThroatPoint() const
    {
        if (!headBounds.IsValid()) {
            return std::nullopt;
        }
        const auto& head = state.geometry.head;
        assert(head);
        const auto downwardDistance = headBounds.boundMin.z * 0.17f;
        return head->world.rotate.GetVectorZ() * downwardDistance + head->world.translate;
    }

}  // namespace Thread::Interaction::NiSurface
