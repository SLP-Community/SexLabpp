#pragma once

#include <SimpleIni.h>

#include "Util/StringUtil.h"

namespace Thread::NiNode
{
	enum class Feature : uint8_t
	{
		Distance,
		Facing,
		Time,
		Velocity,
		Stability,
	};
	constexpr static inline size_t NUM_FEATURES = magic_enum::enum_count<Feature>();

	template <int Id = 0, Feature... Fs>
	class InteractionDescriptor
	{
		constexpr static inline auto NaN = std::numeric_limits<float>::signaling_NaN();

	  public:
		void AddValue(Feature feature, float value)
		{
			assert(((feature == Fs) || ...) && "Feature not in descriptor");
			features[static_cast<size_t>(feature)] = value;
		}

		float Predict() const
		{
			float ret = bias;
			for (const auto& feature : { Fs... }) {
				const auto coeff = coefficients[static_cast<size_t>(feature)];
				const auto value = features[static_cast<size_t>(feature)];
				assert(!std::isnan(coeff));
				assert(!std::isnan(value));
				ret += coeff * value;
			}
			return ret;
		}

		std::string ToString() const
		{
			std::string row = std::format("{},", Id);
			const auto allFeatures = magic_enum::enum_values<Feature>();
			for (const auto& feature : allFeatures) {
				row += std::to_string(features[static_cast<size_t>(feature)]) + ",";
			}
			row += std::to_string(Predict());
			return row;
		}

	  public:
		static constexpr std::string CsvHeader()
		{
			std::string header = "Id,";
			const auto allFeaturesNames = magic_enum::enum_names<Feature>();
			for (const auto& featureName : allFeaturesNames) {
				header += std::format("{}," , featureName);
			}
			header += "Prediction";
			return header;
		}

		static void Initialize(const char* section, CSimpleIniA& inifile)
		{
			bias = static_cast<float>(inifile.GetDoubleValue(section, "bias", NaN));
			if (std::isnan(bias)) {
				logger::error("Descriptor bias not initialized (NaN).");
				assert(false && "Descriptor bias not initialized");
				throw std::runtime_error("Descriptor bias not initialized");
			}
			for (const auto& feature : { Fs... }) {
				const auto name = Util::CastLower(std::string{ magic_enum::enum_name(feature) });
				const auto value = static_cast<float>(inifile.GetDoubleValue(section, name.c_str(), NaN));
				if (std::isnan(value)) {
					logger::error("Descriptor '{}': Missing value for feature '{}'", section, name);
					assert(false && "Missing descriptor feature value");
					throw std::runtime_error("Missing descriptor feature value");
				}
				coefficients[static_cast<size_t>(feature)] = value;
			}
			logger::info("{}: Loaded {} coefficients, bias={:.3f}", section, coefficients.size(), bias);
		}

	  private:
		std::array<float, NUM_FEATURES> features{ NaN };
		static inline std::array<float, NUM_FEATURES> coefficients{ NaN };
		static inline float bias{ NaN };
	};

	using KissingDescriptor = InteractionDescriptor<1, Feature::Distance, Feature::Facing, Feature::Time, Feature::Velocity, Feature::Stability>;

}  // namespace Thread::NiNode
