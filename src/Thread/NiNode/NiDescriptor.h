#pragma once

#include <SimpleIni.h>

#include "NiMath.h"
#include "Util/StringUtil.h"

namespace Thread::NiNode
{
	namespace NiType
	{
		enum Type
		{
			None = 0,
			Vaginal,
			Anal,
			Oral,
			Grinding,
			Deepthroat,
			Skullfuck,
			LickingShaft,
			FootJob,
			HandJob,
			Kissing,
			Facial,
			AnimObjFace,
			ToeSucking,
			Boobjob,
		};
		constexpr static inline size_t NUM_TYPES = magic_enum::enum_count<Type>();

		enum Area
		{
			None = 0,
			Pelvis,
			Mouth,

		};
		constexpr static inline size_t NUM_AREAS = magic_enum::enum_count<Area>();

	}

	class INiDescriptor
	{
	  public:
		enum class Feature : uint8_t
		{
			Distance,
			Time,
			Velocity,
			Oscillation,
			Impulse,
			Stability,
			StabilityVariance,
			AngleXY,
			AngleXZ,
			AngleYZ,
		};
		constexpr static inline size_t NUM_FEATURES = magic_enum::enum_count<Feature>();

	  public:
		virtual ~INiDescriptor() = default;

		virtual float Predict() const = 0;
		virtual std::string CsvRow() const = 0;
		virtual NiType::Type GetType() const = 0;

	  public:
		static std::string CsvHeader()
		{
			std::string header = "Id,";
			const auto allFeaturesNames = magic_enum::enum_names<Feature>();
			for (const auto& featureName : allFeaturesNames) {
				header += std::format("{},", featureName);
			}
			return std::format("{}Prediction,Prediction-Sigmoid", header);
		}
	};

	template <NiType::Type Id = NiType::None>
	class NiDescriptor :
	  public INiDescriptor
	{
	  public:
		void AddValue(Feature feature, float value)
		{
			features[static_cast<size_t>(feature)] = value;
		}

		float Predict() const override
		{
			return std::inner_product(coefficients.begin(), coefficients.end(), features.begin(), bias);
		}

		std::string CsvRow() const override
		{
			const auto prediction = Predict();
			const auto predSig = NiMath::Sigmoid(prediction);
			std::string row = std::format("{},", magic_enum::enum_name(Id));
			const auto allFeatures = magic_enum::enum_values<Feature>();
			for (const auto& feature : allFeatures) {
				const auto featureValue = features[static_cast<size_t>(feature)];
				row += std::to_string(featureValue) + ",";
			}
			return std::format("{}{},{}", row, prediction, predSig);
		}

		NiType::Type GetType() const override
		{
			return Id;
		}

	  public:
		static void Initialize(CSimpleIniA& inifile)
		{
			constexpr auto NaN = std::numeric_limits<float>::quiet_NaN();
			std::string section { magic_enum::enum_name<NiType::Type>(Id) };
			bias = static_cast<float>(inifile.GetDoubleValue(section.c_str(), "bias", NaN));
			if (std::isnan(bias)) {
				const auto err = std::format("Descriptor '{}': Missing bias value", section);
				throw std::runtime_error(err);
			}
			const auto features = magic_enum::enum_entries<Feature>();
			for (const auto& [feature, name] : features) {
				const auto lower = Util::CastLower(std::string{ name });
				const auto value = static_cast<float>(inifile.GetDoubleValue(section.c_str(), lower.c_str(), NaN));
				if (std::isnan(value)) {
					const auto err = std::format("Descriptor '{}': Missing value for feature '{}'", section, lower);
					throw std::runtime_error(err);
				}
				coefficients[static_cast<size_t>(feature)] = value;
			}
			logger::info("{}: Loaded {} coefficients, bias={:.3f}", section, coefficients, bias);
		}

	  private:
		std::array<float, NUM_FEATURES> features{ 0.0f };
		static inline std::array<float, NUM_FEATURES> coefficients{ 0.0f };
		static inline float bias{ 0.0f };
	};

}  // namespace Thread::NiNode
