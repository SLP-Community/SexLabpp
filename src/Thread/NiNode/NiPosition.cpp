#include "NiPosition.h"

#include "NiMath.h"
#include "Util/Premutation.h"
#include "Registry/Util/RayCast/ObjectBound.h"

// TODO: delete

namespace Thread::NiNode
{
	// bool NiPosition::Snapshot::GetHeadFootInteractions(const Snapshot& a_partner)
	// {
	// 	if (!bHead.IsValid())
	// 		return false;
	// 	const auto footL = a_partner.position.nodes.toe_left;
	// 	const auto footR = a_partner.position.nodes.toe_right;
	// 	if (!footL || !footR)
	// 		return false;
	// 	const auto mouth = GetMouthStartPoint();
	// 	if (!mouth)
	// 		return false;
	// 	const auto distanceLeft = footL->world.translate.GetDistance(*mouth);
	// 	const auto distanceRight = footR->world.translate.GetDistance(*mouth);
	// 	if (distanceLeft > Settings::fDistanceFootMouth && distanceRight > Settings::fDistanceFootMouth)
	// 		return false;
	// 	interactions.emplace_back(a_partner.position.actor, Interaction::Action::ToeSucking, std::min(distanceLeft, distanceRight));
	// 	return true;
	// }

	// bool NiPosition::Snapshot::GetHeadPenisInteractions(const Snapshot& a_partner, std::shared_ptr<Node::NodeData::Schlong> a_schlong)
	// {
	// 	if (!bHead.IsValid()) {
	// 		return false;
	// 	}
	// 	assert(position.nodes.head);
	// 	const auto& headworld = position.nodes.head->world;
	// 	const auto sSchlong = a_schlong->GetReferenceSegment();
	// 	const auto dCenter = [&]() {
	// 		auto res = NiMath::ClosestSegmentBetweenSegments({ headworld.translate }, sSchlong);
	// 		return res.Length();
	// 	}();
	// 	if (dCenter > bHead.boundMax.y * Settings::fCloseToHeadRatio) {
	// 		return false;
	// 	}
	// 	const auto& partnernodes = a_partner.position.nodes;
	// 	const auto baseNode = a_schlong->GetBaseReferenceNode();
	// 	const auto vHead = headworld.rotate.GetVectorY();

	// 	const auto [angleToHead, angleToMouth, angleToBase] = [&]() {
	// 		const auto vBaseToHead = headworld.translate - sSchlong.first;
	// 		const auto vPartnerDir = partnernodes.GetCrotchSegment().Vector();
	// 		const auto proj1 = NiMath::ProjectedComponent(vPartnerDir, vHead);
	// 		const auto proj2 = NiMath::ProjectedComponent(vBaseToHead, vHead);
	// 		return std::make_tuple(
	// 			NiMath::GetAngleDegree(proj1, proj2),
	// 			NiMath::GetAngleDegree(proj1, -vHead),
	// 			NiMath::GetAngleDegree(proj2, vHead));
	// 	}();

	// 	const auto aiming_at_head = std::abs(angleToHead - angleToMouth) < Settings::fAngleToHeadTolerance;
	// 	const auto at_side_of_head = std::abs(angleToBase - 90) < Settings::fAngleToHeadSidewaysTolerance;
	// 	const auto in_front_of_head = std::abs(angleToBase - 180) < Settings::fAngleToHeadFrontalTolerance;
	// 	const auto penetrating_skull = dCenter < (at_side_of_head ? bHead.boundMax.x : bHead.boundMax.y);
	// 	const auto vertical_to_shaft = [&]() {
	// 		const auto vSchlong = sSchlong.Vector();
	// 		const auto aSchlongToMouth = NiMath::GetAngleDegree(vSchlong, vHead);
	// 		return std::abs(aSchlongToMouth - 90) < 30.0f;
	// 	}();
	// 	const auto close_to_mouth = [&]() {
	// 		const auto pMouth = GetMouthStartPoint();
	// 		assert(pMouth);
	// 		const auto seg = NiMath::ClosestSegmentBetweenSegments({ *pMouth }, sSchlong);
	// 		const auto d = seg.Length();
	// 		return d < bHead.boundMax.x && d < dCenter;
	// 	}();

	// 	if (in_front_of_head && vertical_to_shaft && close_to_mouth) {
	// 		interactions.emplace_back(a_partner.position.actor, Interaction::Action::LickingShaft, dCenter);
	// 		return true;
	// 	} else if (penetrating_skull && in_front_of_head && aiming_at_head) {
	// 		const auto throat = GetThroatPoint(), mouth = GetMouthStartPoint();
	// 		assert(throat && mouth);
	// 		if (!baseNode || RotateNode(baseNode, sSchlong, *throat, Settings::fAdjustSchlongLimit)) {
	// 			RotateNode(position.nodes.head, { *mouth, *throat }, sSchlong.first, Settings::fAdjustHeadLimit);
	// 			interactions.emplace_back(a_partner.position.actor, Interaction::Action::Oral, dCenter);
	// 			assert(partnernodes.pelvis);
	// 			const auto tip_at_throat = dCenter < bHead.boundMax.y * Settings::fThroatToleranceRadius;
	// 			const auto pelvis_at_head = bHead.IsPointInside(partnernodes.pelvis->world.translate);
	// 			if (tip_at_throat || pelvis_at_head) {
	// 				interactions.emplace_back(a_partner.position.actor, Interaction::Action::Deepthroat, dCenter);
	// 			}
	// 			return true;
	// 		}
	// 	} else if (penetrating_skull && aiming_at_head) {
	// 		if (!baseNode || RotateNode(baseNode, sSchlong, headworld.translate, Settings::fAdjustSchlongLimit)) {
	// 			interactions.emplace_back(a_partner.position.actor, Interaction::Action::Skullfuck, dCenter);
	// 		}
	// 		return true;
	// 	} else if (in_front_of_head && aiming_at_head) {
	// 		interactions.emplace_back(a_partner.position.actor, Interaction::Action::Facial, dCenter);
	// 		return true;
	// 	}
	// 	return false;
	// }

	// bool NiPosition::Snapshot::GetHandPenisInteractions(const Snapshot& a_partner, std::shared_ptr<Node::NodeData::Schlong> a_schlong)
	// {
	// 	const auto lHand = position.nodes.hand_left;
	// 	const auto rHand = position.nodes.hand_right;
	// 	const auto lThumb = position.nodes.thumb_left;
	// 	const auto rThumb = position.nodes.thumb_right;
	// 	if (!lHand || !rHand || !lThumb || !rThumb) {
	// 		return false;
	// 	}
	// 	const auto sSchlong = a_schlong->GetReferenceSegment();
	// 	const auto pLeft = lHand->world.translate;
	// 	const auto pRight = rHand->world.translate;
	// 	const auto lDist = NiMath::ClosestSegmentBetweenSegments(pLeft, sSchlong).Length();
	// 	const auto rDist = NiMath::ClosestSegmentBetweenSegments(pRight, sSchlong).Length();
	// 	const auto closeToL = lDist < Settings::fDistanceHand;
	// 	const auto closeToR = rDist < Settings::fDistanceHand;
	// 	bool pickLeft;
	// 	if (!closeToR && !closeToL) {
	// 		return false;
	// 	} else if (closeToR && closeToL) { // Both hands are close, pick the closest to the base
	// 		const auto nSchlong = a_schlong->GetBaseReferenceNode();
	// 		pickLeft = nSchlong && nSchlong->world.translate.GetDistance(pLeft) < nSchlong->world.translate.GetDistance(pRight);
	// 	} else {
	// 		pickLeft = closeToL;
	// 	}
	// 	auto referencePoint = pickLeft ? (pLeft + lThumb->world.translate) / 2 : (pRight + rThumb->world.translate) / 2;
	// 	RotateNode(a_schlong->GetBaseReferenceNode(), sSchlong, referencePoint, Settings::fAdjustSchlongLimit);
	// 	interactions.emplace_back(a_partner.position.actor, Interaction::Action::HandJob, pickLeft ? lDist : rDist);
	// 	return true;
	// }

	// bool NiPosition::Snapshot::GetFootPenisInteractions(const Snapshot& a_partner, std::shared_ptr<Node::NodeData::Schlong> a_schlong)
	// {
	// 	const auto nSchlong = a_schlong->GetBaseReferenceNode();
	// 	const auto sSchlong = a_schlong->GetReferenceSegment();
	// 	const auto get = [&](const auto& foot) {
	// 		if (!foot)
	// 			return false;
	// 		const auto pFoot = foot->world.translate;
	// 		const auto d = NiMath::ClosestSegmentBetweenSegments(pFoot, sSchlong).Length();
	// 		if (d > Settings::fDistanceFoot)
	// 			return false;
	// 		interactions.emplace_back(a_partner.position.actor, Interaction::Action::FootJob, d);
	// 		return true;
	// 	};
	// 	return get(position.nodes.foot_left) || get(position.nodes.foot_right);
	// }

	// bool NiPosition::Snapshot::GetHeadVaginaInteractions(const Snapshot& a_partner)
	// {
	// 	const auto mouthstart = GetMouthStartPoint();
	// 	if (!mouthstart)
	// 		return false;
	// 	const auto& nClitoris = a_partner.position.nodes.clitoris;
	// 	const auto sVaginal = a_partner.position.nodes.GetVaginalSegment();
	// 	if (!sVaginal || !nClitoris)
	// 		return false;
	// 	float distance = nClitoris->world.translate.GetDistance(*mouthstart);
	// 	if (distance > Settings::fDistanceMouth)
	// 		return false;
	// 	assert(position.nodes.head);
	// 	const auto& headworld = position.nodes.head->world;
	// 	const auto vHead = headworld.rotate.GetVectorY();
	// 	const auto angle = NiMath::GetAngleDegree(sVaginal->Vector(), vHead);
	// 	if (angle > Settings::fAngleCunnilingus)
	// 		return false;
	// 	interactions.emplace_back(a_partner.position.actor, Interaction::Action::Oral, distance);
	// 	return true;
	// }

	// bool NiPosition::Snapshot::GetVaginaVaginaInteractions(const Snapshot& a_partner)
	// {
	// 	const auto &c1 = position.nodes.clitoris, &c2 = a_partner.position.nodes.clitoris;
	// 	if (!c1 || !c2)
	// 		return false;
	// 	const auto distance = c1->world.translate.GetDistance(c2->world.translate);
	// 	if (distance > Settings::fDistanceCrotch)
	// 		return false;
	// 	const auto sVaginal = position.nodes.GetVaginalSegment();
	// 	const auto sVaginalPartner = a_partner.position.nodes.GetVaginalSegment();
	// 	if (!sVaginal || !sVaginalPartner)
	// 		return false;
	// 	const auto angle = NiMath::GetAngleDegree(sVaginal->Vector(), sVaginalPartner->Vector());
	// 	if (std::abs(angle - 180) > Settings::fAngleGrindingFF)
	// 		return false;
	// 	interactions.emplace_back(a_partner.position.actor, Interaction::Action::Grinding, distance);
	// 	return true;
	// }
	
	// bool NiPosition::Snapshot::GetVaginaLimbInteractions(const Snapshot& a_partner)
	// {
	// 	const auto& nClitoris = a_partner.position.nodes.clitoris;
	// 	if (!nClitoris)
	// 		return false;
	// 	const auto get = [&](const auto& limb, auto type, float maxDist) {
	// 		if (!limb)
	// 			return false;
	// 		const auto pLimb = limb->world.translate;
	// 		const auto d = pLimb.GetDistance(nClitoris->world.translate);
	// 		if (d > maxDist)
	// 			return false;
	// 		interactions.emplace_back(position.actor, type, d);
	// 		return true;
	// 	};
	// 	const auto lHand = position.nodes.hand_left;
	// 	const auto rHand = position.nodes.hand_right;
	// 	const auto lFoot = position.nodes.foot_left;
	// 	const auto rFoot = position.nodes.foot_right;
	// 	return get(lHand, Interaction::Action::HandJob, Settings::fDistanceHand) ||
	// 				 get(rHand, Interaction::Action::HandJob, Settings::fDistanceHand) ||
	// 				 get(lFoot, Interaction::Action::FootJob, Settings::fDistanceFoot) ||
	// 				 get(rFoot, Interaction::Action::FootJob, Settings::fDistanceFoot);
	// }

	// bool NiPosition::Snapshot::GetHeadAnimObjInteractions(const Snapshot& a_partner)
	// {
	// 	bool bAnimObjectLoaded;
	// 	a_partner.position.actor->GetGraphVariableBool("bAnimObjectLoaded", bAnimObjectLoaded);
	// 	if (!bAnimObjectLoaded)
	// 		return false;
	// 	const auto pMouth = GetMouthStartPoint();
	// 	if (!pMouth)
	// 		return false;
	// 	const auto get = [&](const auto& animObj) {
	// 		if (!animObj)
	// 			return false;
	// 		const auto pAnimObj = animObj->world.translate;
	// 		const auto d = pAnimObj.GetDistance(*pMouth);
	// 		if (d > Settings::fAnimObjDist)
	// 			return false;
	// 		interactions.emplace_back(a_partner.position.actor, Interaction::Action::AnimObjFace, d);
	// 		return true;
	// 	};
	// 	const auto& n = a_partner.position.nodes;
	// 	return get(n.animobj_a) || get(n.animobj_b) || get(n.animobj_r) || get(n.animobj_l);
	// }

}	 // namespace Thread::NiNode
