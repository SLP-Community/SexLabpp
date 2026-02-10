#pragma once

#include "NiMotion.h"
#include "Node.h"
#include "Registry/Define/Sex.h"

namespace Thread::NiNode
{
	struct NiActor
	{
		NiActor(RE::Actor* a_owner, Registry::Sex a_sex) :
		  actor(a_owner), nodes(a_owner, a_sex != Registry::Sex::Female), sex(a_sex) {}
		~NiActor() = default;

		void CaptureSnapshot(float a_timeStamp) { motion.Push(nodes, a_timeStamp); }
		bool HasSufficientMotionData() const { return motion.HasSufficientData(); }

		RE::Actor* Actor() const { return actor.get(); }
		bool IsSex(Registry::Sex a_sex) const { return sex.any(a_sex); }
		const NiMotion& Motion() const { return motion; }

	  public:
		bool operator==(const NiActor& a_rhs) const { return this->actor == a_rhs.actor; }

	  public:
		RE::ActorPtr actor;
		Node::NodeData nodes;
		REX::EnumSet<Registry::Sex> sex;
		NiMotion motion;
	};

}  // namespace Thread::NiNode
