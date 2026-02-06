#pragma once

#include "NiActor.h"

namespace Thread::NiNode
{
	struct LinearModel
	{
		std::vector<float> coefficients;
		float bias{ 0.0f };

		float Predict(const std::vector<float>& features) const
		{
			assert(features.size() == coefficients.size());
			return std::inner_product(coefficients.begin(), coefficients.end(), features.begin(), bias);
		}
	};

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

	  public:
		NiInteraction() = default;
		NiInteraction(Type a_type) : type(a_type) {}
		~NiInteraction() = default;

		constexpr static inline size_t NUM_TYPES = magic_enum::enum_count<Type>();
		static std::array<LinearModel, NUM_TYPES> models;
	};

	NiInteraction EvaluateKissing(const NiMotion& a_motionA, const NiMotion& a_motionB);

}  // namespace Thread::NiNode
