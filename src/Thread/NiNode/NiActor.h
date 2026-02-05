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

	  public:
		RE::ActorPtr actor;
		Node::NodeData nodes;
		REX::EnumSet<Registry::Sex> sex;
		NiMotion<> motion;
	};

}  // namespace Thread::NiNode
