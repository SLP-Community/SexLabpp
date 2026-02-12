#pragma once

#include <SimpleIni.h>

#include "Util/StringUtil.h"

namespace Thread::NiNode
{
	namespace NiType
	{
		enum class Cluster
		{
			None = 0,
			Crotch,
			Head,
			Kissing,
		};
		constexpr static inline size_t NUM_CLUSTERS = magic_enum::enum_count<Cluster>();

		enum class Type
		{
			None = 0,
#define NI_TYPE(name, cluster) name,

#include "NiType.def"

#undef NI_TYPE
		};
		constexpr static inline size_t NUM_TYPES = magic_enum::enum_count<Type>();

		inline Cluster GetClusterForType(Type type)
		{
#define NI_TYPE(name, cluster)   \
	if (type == Type::name) {    \
		return Cluster::cluster; \
	}
#include "NiType.def"
#undef NI_TYPE
			return Cluster::None;
		}

		inline std::vector<Type> GetTypesForCluster(Cluster a_cluster)
		{
			std::vector<Type> types;
#define NI_TYPE(name, cluster)         \
	if (a_cluster == Cluster::cluster) { \
		types.push_back(Type::name);   \
	}
#include "NiType.def"
#undef NI_TYPE
			return types;
		}
	}  // namespace NiType

	class INiDescriptor
	{
	  public:
		enum class Feature : uint8_t
		{
			Distance01,
			Time01,
			Velocity01,
			Oscillation01,
			Impulse01,
			Stability01,
			StabilityVariance01,

			Distance02,
			Time02,
			Velocity02,
			Oscillation02,
			Impulse02,
			Stability02,
			StabilityVariance02,

			Distance03,
			Time03,
			Velocity03,
			Oscillation03,
			Impulse03,
			Stability03,
			StabilityVariance03,

			Distance04,

			Angle01,
			Angle02,
			Angle03,
		};
		constexpr static inline size_t NUM_FEATURES = magic_enum::enum_count<Feature>();

	  public:
		virtual ~INiDescriptor() = default;

		virtual float Predict() const = 0;
		virtual std::string CsvRow() const = 0;
		virtual NiType::Type GetType() const = 0;

	  public:
		static std::string CreateCsvHeader(std::vector<INiDescriptor*> descriptors)
		{
			std::ranges::sort(descriptors, [](const INiDescriptor* a, const INiDescriptor* b) {
				assert(a && b);
				return a->GetType() < b->GetType();
			});
			std::string header = "";
			const auto featureNames = magic_enum::enum_names<Feature>();
			for (const auto& descriptor : descriptors) {
				assert(descriptor);
				const auto dType = descriptor->GetType();
				const auto dTypeName = magic_enum::enum_name(dType);
				header += std::format("{}_{},", "Id", dTypeName);
				for (const auto& featureName : featureNames) {
					header += std::format("{}_{},", dTypeName, featureName);
				}
				header += std::format("{}_Prediction", dTypeName);
				if (descriptor != descriptors.back()) {
					header += ",";
				}
			}
			return header;
		}

		static std::string CreateCsvRow(std::vector<INiDescriptor*> descriptors)
		{
			std::ranges::sort(descriptors, [](const INiDescriptor* a, const INiDescriptor* b) {
				assert(a && b);
				return a->GetType() < b->GetType();
			});
			std::string row = "";
			for (const auto& descriptor : descriptors) {
				row += descriptor->CsvRow();
				if (descriptor != descriptors.back()) {
					row += ",";
				}
			}
			return row;
		}
	};

	template <NiType::Type Id = NiType::Type::None>
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
			std::string row = std::format("{},", magic_enum::enum_name(Id));
			const auto allFeatures = magic_enum::enum_values<Feature>();
			for (const auto& feature : allFeatures) {
				const auto featureValue = features[static_cast<size_t>(feature)];
				row += std::to_string(featureValue) + ",";
			}
			return std::format("{}{}", row, prediction);
		}

		NiType::Type GetType() const override
		{
			return Id;
		}

	  public:
		static void Initialize(CSimpleIniA& inifile)
		{
			constexpr auto NaN = std::numeric_limits<float>::quiet_NaN();
			std::string section{ magic_enum::enum_name<NiType::Type>(Id) };
			bias = static_cast<float>(inifile.GetDoubleValue(section.c_str(), "bias", NaN));
			if (std::isnan(bias)) {
				const auto err = std::format("Descriptor '{}': Missing bias value", section);
				throw std::runtime_error(err);
			}
			const auto features = magic_enum::enum_entries<Feature>();
			for (const auto& [feature, name] : features) {
				const auto lowerName = StringUtil::CastLower(std::string { name });
				const auto value = static_cast<float>(inifile.GetDoubleValue(section.c_str(), lowerName.c_str(), NaN));
				if (std::isnan(value)) {
					const auto err = std::format("Descriptor '{}': Missing value for feature '{}'", section, name);
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
