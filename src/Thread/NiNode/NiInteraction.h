#pragma once

#include "NiActor.h"
#include "NiDescriptor.h"

namespace Thread::NiNode
{
	struct NiInteraction
	{
		enum class Type
		{
			None = 0,
			Kissing,
		} type{ Type::None };
		float confidence{ 0.0f };  // 0..1
		float duration{ 0.0f };	   // ms? COMEBACK: check units
		float velocity{ 0.0f };
		bool active{ false };	   // whether the interaction is currently active
		std::string csvRow{ "" };  // to train ML models, contains raw feature values and predictions for each evaluated moment, in CSV format

	  public:
		NiInteraction() = default;
		NiInteraction(Type a_type) : type(a_type) {}
		~NiInteraction() = default;

		constexpr static inline size_t NUM_TYPES = magic_enum::enum_count<Type>();
	};

	NiInteraction EvaluateKissing(const NiMotion& a_motionA, const NiMotion& a_motionB);

}  // namespace Thread::NiNode
