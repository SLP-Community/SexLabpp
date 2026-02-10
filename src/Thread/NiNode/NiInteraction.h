#pragma once

#include "NiActor.h"
#include "NiDescriptor.h"

namespace Thread::NiNode
{
	struct NiInteraction
	{
		NiInteraction() = default;
		NiInteraction(std::unique_ptr<INiDescriptor> a_descriptor) :
		  descriptor(std::move(a_descriptor)) {}
		~NiInteraction() = default;

		// Explicitly define move semantics
		NiInteraction(NiInteraction&&) = default;
		NiInteraction& operator=(NiInteraction&&) = default;

		// Delete copy semantics (unique_ptr is move-only)
		NiInteraction(const NiInteraction&) = delete;
		NiInteraction& operator=(const NiInteraction&) = delete;

		NiType::Type GetType() const { return descriptor ? descriptor->GetType() : NiType::None; }

	public:
		std::unique_ptr<INiDescriptor> descriptor{ nullptr };
		float timeActive{ 0.0f };
		float velocity{ 0.0f };
		bool active{ false };
	};

	NiInteraction EvaluateVaginal(const NiMotion& a_motionA, const NiMotion& a_motionB);
	NiInteraction EvaluateAnal(const NiMotion& a_motionA, const NiMotion& a_motionB);
	NiInteraction EvaluateOral(const NiMotion& a_motionA, const NiMotion& a_motionB);
	NiInteraction EvaluateGrinding(const NiMotion& a_motionA, const NiMotion& a_motionB);
	NiInteraction EvaluateDeepthroat(const NiMotion& a_motionA, const NiMotion& a_motionB);
	NiInteraction EvaluateSkullfuck(const NiMotion& a_motionA, const NiMotion& a_motionB);
	NiInteraction EvaluateLickingShaft(const NiMotion& a_motionA, const NiMotion& a_motionB);
	NiInteraction EvaluateFootJob(const NiMotion& a_motionA, const NiMotion& a_motionB);
	NiInteraction EvaluateHandJob(const NiMotion& a_motionA, const NiMotion& a_motionB);
	NiInteraction EvaluateKissing(const NiMotion& a_motionA, const NiMotion& a_motionB);
	NiInteraction EvaluateFacial(const NiMotion& a_motionA, const NiMotion& a_motionB);
	NiInteraction EvaluateAnimObjFace(const NiMotion& a_motionA, const NiMotion& a_motionB);
	NiInteraction EvaluateToeSucking(const NiMotion& a_motionA, const NiMotion& a_motionB);
	NiInteraction EvaluateBoobjob(const NiMotion& a_motionA, const NiMotion& a_motionB);

}  // namespace Thread::NiNode
