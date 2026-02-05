#pragma once

#include "NiActor.h"

namespace Thread::NiNode
{
	struct NiInteraction
	{
		enum class Type
		{
			None = 0,
			Kissing,
			Choking,
		} type{ Type::None };
		float confidence{ 0.0f };  // 0..1
		float duration{ 0.0f };	   // ms? COMEBACK: check units
		float velocity{ 0.0f };
		bool active{ false };

		constexpr static inline size_t NUM_TYPES = magic_enum::enum_count<Type>();

	  public:
		NiInteraction() = default;
		NiInteraction(Type a_type) : type(a_type) {}
		~NiInteraction() = default;
	};

	void EvaluateKissing(NiInteraction& result, const NiActor& a_self, const NiActor& a_other);

}  // namespace Thread::NiNode
