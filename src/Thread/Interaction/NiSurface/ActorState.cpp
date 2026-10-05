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

        struct ShaftContact
        {
            float distance{ std::numeric_limits<float>::max() };
            RE::NiPoint3 direction{};
        };

        ShaftContact GetShaftContact(const ShaftShape& a_shaft, const RE::NiPoint3& a_point)
        {
            ShaftContact result;
            const auto visitSegment = [&](const RE::NiPoint3& a_start, const RE::NiPoint3& a_end, float a_startRadius, float a_endRadius) {
                const auto direction = a_end - a_start;
                const auto lengthSq = direction.SqrLength();
                const auto offset = a_point - a_start;
                const auto radiusDelta = a_endRadius - a_startRadius;
                float t = a_endRadius > a_startRadius ? 1.0f : 0.0f;
                if (lengthSq > radiusDelta * radiusDelta) {
                    const auto length = std::sqrt(lengthSq);
                    const auto along = offset.Dot(direction) / length;
                    const auto radialSq = std::max(offset.SqrLength() - along * along, 0.0f);
                    // Minimize distance to the linearly varying radius, rather than just to its centerline.
                    const auto shift = radiusDelta * std::sqrt(radialSq / (lengthSq - radiusDelta * radiusDelta));
                    t = std::clamp((along + shift) / length, 0.0f, 1.0f);
                }
                const auto radius = a_startRadius + radiusDelta * t;
                const auto distance = (offset - direction * t).Length() - radius;
                if (distance < result.distance) {
                    result.distance = distance;
                    result.direction = lengthSq > FLT_EPSILON ? direction : GetShaftTipDirection(a_shaft);
                }
            };
            for (std::size_t i = 1; i < a_shaft.sections.size(); ++i) {
                visitSegment(a_shaft.sections[i - 1].center, a_shaft.sections[i].center, a_shaft.sections[i - 1].radius, a_shaft.sections[i].radius);
            }
            if (!a_shaft.sections.empty()) {
                visitSegment(a_shaft.sections.back().center, a_shaft.tip, a_shaft.sections.back().radius, 0.0f);
            }
            return result;
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

    void ActorState::ResetGeometry(RE::NiAVObject* a_root)
    {
        geometry = {};
        root.reset(a_root);
        if (!root) {
            return;
        }
        try {
            geometry = Geometry::ActorGeometry{ actor.get() };
        } catch (const std::exception& e) {
            // Keep the attempted root so an unsupported replacement is not rescanned every frame.
            logger::warn("NiSurface Interaction: Failed to rebind actor {:X}: {}", actor->GetFormID(), e.what());
        }
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
        const auto* shaftShape = a_shaft.GetCollisionShape();
        if (!shaftShape || shaftShape->sections.size() < 2 || !headBounds.IsValid()) {
            return false;
        }
        assert(state.geometry.head);
        const auto& headWorld = state.geometry.head->world;
        const auto shaftSegment = a_shaft.GetReferenceSegment();
        const auto shaftTip = shaftShape->tip;
        const auto headContact = GetShaftContact(*shaftShape, headWorld.translate);
        const auto headDistance = std::max(headContact.distance, 0.0f);
        if (headDistance > headBounds.boundMax.y * Settings::fCloseToHeadRatio) {
            return false;
        }
        const auto baseNode = a_shaft.GetBaseReferenceNode();
        const auto vHead = headWorld.rotate.GetVectorY();
        const auto basePosition = baseNode ? baseNode->world.translate : shaftShape->sections.front().center;
        const auto vBaseToHead = headWorld.translate - basePosition;
        const auto angleToBase = GeometryMath::GetAngleDegree(vBaseToHead, vHead);
        const auto aimingAtHead = GeometryMath::GetAngleDegree(headContact.direction, vBaseToHead) < Settings::fAngleToHeadTolerance;
        const auto atSideOfHead = std::abs(angleToBase - 90) < Settings::fAngleToHeadSidewaysTolerance;
        const auto inFrontOfHead = std::abs(angleToBase - 180) < Settings::fAngleToHeadFrontalTolerance;
        const auto penetratingSkull = headDistance < (atSideOfHead ? headBounds.boundMax.x : headBounds.boundMax.y);
        const auto mouthDistance = mouthOpening ? DistanceToOpening(*shaftShape, *mouthOpening) : std::numeric_limits<float>::max();
        const auto mouthContact = mouthOpening ? GetShaftContact(*shaftShape, mouthOpening->center) : ShaftContact{};
        const auto lickingDistance = mouthOpening ? std::max(mouthContact.distance - mouthOpening->radius, 0.0f) : std::numeric_limits<float>::max();
        const auto contactingMouth = mouthOpening && mouthDistance <= Settings::fDistanceMouth;
        const auto aimingAtMouth = mouthOpening && GeometryMath::GetAngleDegree(GetShaftTipDirection(*shaftShape), mouthOpening->axis) < Settings::fAngleToHeadTolerance;
        const auto verticalToShaft = std::abs(GeometryMath::GetAngleDegree(mouthContact.direction, vHead) - 90.0f) < 30.0f;
        const auto closeToMouth = mouthOpening && lickingDistance <= Settings::fDistanceMouth && lickingDistance < headDistance;

        if (inFrontOfHead && verticalToShaft && closeToMouth) {
            const auto mouth = GetMouthCenter();
            assert(mouth);
            interactions.emplace_back(a_partner.state.actor, Interaction::Action::LickingShaft, lickingDistance,
                RE::NiPoint3{ GetNormalizedPositionAlongShaft(shaftSegment, *mouth), 0.0f, 0.0f }, shaftSegment.Length());
            return true;
        } else if (contactingMouth && inFrontOfHead && aimingAtMouth) {
            const auto throat = GetThroatPoint(), mouth = GetMouthCenter();
            assert(throat && mouth);
            if (!baseNode || RotateNode(baseNode, shaftSegment, *throat, Settings::fAdjustSchlongLimit)) {
                RotateNode(state.geometry.head, { *mouth, *throat }, shaftSegment.start, Settings::fAdjustHeadLimit);
                interactions.emplace_back(a_partner.state.actor, Interaction::Action::Oral, mouthDistance, shaftTip - *mouth);
                const auto throatDistance = shaftTip.GetDistance(*throat);
                const auto tipAtThroat = throatDistance < headBounds.boundMax.y * Settings::fThroatToleranceRadius;
                const auto baseAtHead = baseNode && headBounds.IsPointInside(baseNode->world.translate);
                if (tipAtThroat || baseAtHead) {
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
        const auto* shaftShape = a_shaft.GetCollisionShape();
        if (!shaftShape || shaftShape->sections.size() < 2 || (!vaginalOpening && !analOpening)) {
            return false;
        }
        const auto shaftSegment = a_shaft.GetReferenceSegment();
        const auto shaftTip = shaftShape->tip;
        const auto shaftNode = a_shaft.GetBaseReferenceNode();
        const auto dVaginal = vaginalOpening ? DistanceToOpening(*shaftShape, *vaginalOpening) : std::numeric_limits<float>::max();
        const auto dAnal = analOpening ? DistanceToOpening(*shaftShape, *analOpening) : std::numeric_limits<float>::max();
        const auto previous = std::ranges::find_if(state.interactions, [&](const Interaction& it) {
            return it.partner == a_partner.state.actor && (it.action == Interaction::Action::Vaginal || it.action == Interaction::Action::Anal);
        });
        auto tolerance = Settings::fPenetrationVaginalTolerance;
        if (previous != state.interactions.end()) {
            tolerance = previous->action == Interaction::Action::Vaginal ? Settings::fPenetrationVaginalToleranceRepeat : -Settings::fPenetrationAnalToleranceRepeat;
        }
        // Retain the preference and repeat tolerance when both openings are available.
        const bool preferVaginal = vaginalOpening && (!analOpening || dVaginal - dAnal < tolerance);
        const auto detectOpening = [&](const std::optional<OpeningShape>& a_opening, Interaction::Action a_type, float a_distance) {
            if (!a_opening || a_distance > Settings::fDistanceCrotch) {
                return false;
            }
            const auto angle = GeometryMath::GetAngleDegree(a_opening->axis, GetShaftTipDirection(*shaftShape));
            if (angle > Settings::fAnglePenetration || (shaftNode && !RotateNode(shaftNode, shaftSegment, a_opening->deep, Settings::fAdjustSchlongVaginalLimit))) {
                return false;
            }
            interactions.emplace_back(a_partner.state.actor, a_type, a_distance, shaftTip - a_opening->center);
            return true;
        };
        if (preferVaginal) {
            if (detectOpening(vaginalOpening, Interaction::Action::Vaginal, dVaginal) || detectOpening(analOpening, Interaction::Action::Anal, dAnal)) {
                return true;
            }
        } else if (detectOpening(analOpening, Interaction::Action::Anal, dAnal) || detectOpening(vaginalOpening, Interaction::Action::Vaginal, dVaginal)) {
            return true;
        }
        if (vaginalOpening && analOpening && std::min(dVaginal, dAnal) <= Settings::fDistanceCrotch) {
            const auto& opening = dVaginal < dAnal ? *vaginalOpening : *analOpening;
            const auto contact = GetShaftContact(*shaftShape, opening.center);
            const auto crotchDirection = vaginalOpening->center - analOpening->center;
            const auto angle = GeometryMath::GetAngleDegree(crotchDirection, contact.direction);
            if (std::abs(angle - 180.0f) <= Settings::fAngleGrinding) {
                interactions.emplace_back(a_partner.state.actor, Interaction::Action::Grinding, std::min(dVaginal, dAnal), shaftTip - opening.center);
                return true;
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
        return std::nullopt;
    }

    std::optional<RE::NiPoint3> ActorState::Frame::GetThroatPoint() const
    {
        if (mouthOpening) {
            return mouthOpening->deep;
        }
        return std::nullopt;
    }

}  // namespace Thread::Interaction::NiSurface
