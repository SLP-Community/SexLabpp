#include "Node.h"

#include "Registry/Define/RaceKey.h"
#include "Registry/Define/Transform.h"

namespace Thread::NiNode::Node
{
	NodeData::NodeData(RE::Actor* a_actor, bool a_forceSchlong)
	{
		const auto obj = a_actor->Get3D();
		if (!obj) {
			const auto msg = std::format("Unable to retrieve 3D of actor {:X}", a_actor->GetFormID());
			throw std::exception(msg.c_str());
		}
		const auto racekey = Registry::RaceKey(a_actor);
		const auto racestr = racekey.IsValid() ? racekey.AsString() : "?";
		const auto get = [&](auto str, auto& target, bool log) {
			auto node = obj->GetObjectByName(str);
			auto ninode = node ? node->AsNode() : nullptr;
			if (!ninode) {
				if (log)
					logger::info("Actor {:X} (Race: {}) is missing Node {} (This may be expected)", a_actor->GetFormID(), racestr, str);
				return false;
			}
			target = RE::NiPointer{ ninode };
			return true;
		};
		if (!get(PELVIS, pelvis, true) || !get(SPINELOWER, spine_lower, true)) {
			throw std::exception("Missing mandatory 3d object (body)");
		}
		get(HEAD, head, true);
		get(HANDLEFTREF, hand_left, true);
		get(HANDRIGHTREF, hand_right, false);
		get(THUMBLEFT, thumb_left, false);
		get(THUMBRIGHT, thumb_right, false);
		get(FOOTLEFT, foot_left, true);
		get(FOOTRIGHT, foot_right, false);
		get(TOELEFT, toe_left, true);
		get(TOERIGHT, toe_right, true);
		get(CLITORIS, clitoris, true);
		get(VAGINADEEP, vaginadeep, true);
		get(VAGINAB, vaginab, false);
		get(VAGINALLEFT, vaginaleft, false);
		get(VAGINALRIGHT, vaginaright, false);
		get(ANALDEEP, analdeep, true);
		get(ANALLEFT, analleft, false);
		get(ANALRIGHT, analright, false);
		get(ANIMOBJECTA, animobj_a, false);
		get(ANIMOBJECTB, animobj_b, false);
		get(ANIMOBJECTL, animobj_l, false);
		get(ANIMOBJECTR, animobj_r, false);
		for (auto&& it : SCHLONG_NODES) {
			auto niavbase = obj->GetObjectByName(it.base);
			auto niobj = niavbase ? niavbase->AsNode() : nullptr;
			if (!niobj) {
				continue;
			}
			auto ptr = std::make_shared<SchlongData>(RE::NiPointer{ niobj }, it.rot);
			schlongs.push_back(ptr);
		}
		if (a_forceSchlong && schlongs.empty()) {

		}
	}

	std::optional<NiMath::Segment> NodeData::GetVaginalSegment() const
	{
		if (!vaginadeep || !vaginaleft || !vaginaright)
			return std::nullopt;

		const auto start = (vaginaleft->world.translate + vaginaright->world.translate) / 2;
		const auto end = vaginadeep->world.translate;
		return NiMath::Segment{ start, end };
	}

	std::optional<NiMath::Segment> NodeData::GetAnalSegment() const
	{
		if (!analdeep || !analleft || !analright)
			return std::nullopt;

		const auto start = (analleft->world.translate + analright->world.translate) / 2;
		const auto end = analdeep->world.translate;
		return NiMath::Segment{ start, end };
	}

	NiMath::Segment NodeData::GetCrotchSegment() const
	{
		assert(pelvis && spine_lower);
		return { spine_lower->world.translate, pelvis->world.translate };
	}

	std::optional<RE::NiPoint3> NodeData::GetToeVectorLeft() const
	{
		if (!foot_left || !toe_left)
			return std::nullopt;
		return toe_left->world.translate - foot_left->world.translate;
	}

	std::optional<RE::NiPoint3> NodeData::GetToeVectorRight() const
	{
		if (!foot_right || !toe_right)
			return std::nullopt;
		return toe_right->world.translate - foot_right->world.translate;
	}

	NiMath::Segment NodeData::FakeSchlong::GetReferenceSegment() const
	{
		return{ ApproximateBase(), ApproximateTip() };
	}

	RE::NiPointer<RE::NiNode> NodeData::FakeSchlong::GetBaseReferenceNode() const
	{
		return { nullptr };
	}

	RE::NiPoint3 NodeData::FakeSchlong::ApproximateNode(float a_forward, float a_upward) const
	{
		assert(ownerNodes.pelvis);
		const auto pelvisWorld = ownerNodes.pelvis->world;
		Registry::Coordinate approx(std::vector{ a_forward, 0.0f, a_upward, 0.0f });
		RE::NiPoint3 angle;
		pelvisWorld.rotate.ToEulerAnglesXYZ(angle);
		Registry::Coordinate ret{ pelvisWorld.translate, angle.z };
		approx.Apply(ret);
		return ret.AsNiPoint();
	}

	RE::NiPoint3 NodeData::FakeSchlong::ApproximateTip() const
	{
		constexpr float forward = 20.0f;
		constexpr float upward = -4.0f;
		return ApproximateNode(forward, upward);
	}

	RE::NiPoint3 NodeData::FakeSchlong::ApproximateMid() const
	{
		constexpr float forward = 15.0f;
		constexpr float upward = -6.2f;
		return ApproximateNode(forward, upward);
	}

	RE::NiPoint3 NodeData::FakeSchlong::ApproximateBase() const
	{
		constexpr float forward = 10.0f;
		constexpr float upward = -5.0f;
		return ApproximateNode(forward, upward);
	}

	NodeData::SchlongData::SchlongData(RE::NiPointer<RE::NiNode> a_basenode, const glm::mat3& a_rot) :
		rot({ a_rot[0].x, a_rot[0].y, a_rot[0].z }, { a_rot[1].x, a_rot[1].y, a_rot[1].z }, { a_rot[2].x, a_rot[2].y, a_rot[2].z }),
		nodes({ a_basenode })
	{
		assert(a_basenode);
		do {
			auto& parent = nodes.back();
			auto& childs = parent->GetChildren();
			switch (childs.size()) {
			case 0:
				break;
			case 1:
				{
					auto& child = childs.front();
					auto niobj = child ? child->AsNode() : nullptr;
					if (niobj) {
						nodes.emplace_back(niobj);
					}
				}
				break;
			default:
				{
					auto v1 = nodes.size() < 2 ? parent->world.rotate.GetVectorY() : parent->world.translate - a_basenode->world.translate;
					if (v1.SqrLength() > FLT_EPSILON) {
						for (auto&& child : childs) {
							if (!child)
								continue;
							auto nichild = child->AsNode();
							if (!nichild)
								continue;
							auto v2 = nichild->world.translate - a_basenode->world.translate;
							auto angle = NiMath::GetAngleDegree(v1, v2);
							if (angle <= 90.0f) {
								nodes.emplace_back(nichild);
								break;
							}
						}
					} else {
						auto user = a_basenode->GetUserData();
						auto id = user ? user->GetFormID() : 0;
						logger::error("Ambiguous Skeleton Structure for user {:X} at node depth {}", id, nodes.size());
					}
				}
				break;
			}
			if (nodes.back() == parent)
				break;
		} while (true);
	}

	NiMath::Segment NodeData::SchlongData::GetReferenceSegment() const
	{
		switch (nodes.size()) {
		case 0:
			assert(false);
			throw std::invalid_argument("Schlong Data without any Nodes?");
		case 1:
			{
				auto translate = rot * nodes.front()->world.rotate;
				auto vforward = translate.GetVectorY() * MIN_SCHLONG_LEN;
				vforward.Unitize();
				auto s1 = nodes.front()->world.translate;
				auto s2 = (vforward * MIN_SCHLONG_LEN) + s1;
				return NiMath::Segment(s1, s2);
			}
		default:
			{
				std::vector<RE::NiPoint3> argV{};
				argV.reserve(nodes.size());
				for (auto&& node : nodes) {
					if (!node)
						continue;
					argV.push_back(node->world.translate);
				}
				return NiMath::BestFit(argV);
			}
		}
	}

	RE::NiPointer<RE::NiNode> NodeData::SchlongData::GetBaseReferenceNode() const
	{
		switch (nodes.size()) {
		case 0:
			assert(false);
			throw std::invalid_argument("Schlong Data without any Nodes?");
		default:
			return nodes.front();
		}
	}
}
