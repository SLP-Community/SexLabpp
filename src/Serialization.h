#pragma once

#include "Papyrus/sslLibrary/Serialize.h"
#include "Registry/Stats.h"

namespace Serialization
{
    inline std::string GetTypeName(uint32_t a_type)
    {
        constexpr auto size = sizeof(uint32_t);
        std::string ret{};
        ret.resize(size);
        const char* it = reinterpret_cast<char*>(&a_type);
        for (size_t i = 0, j = size - 1; i < size; i++, j--)
            ret[j] = std::isprint(it[i]) ? it[i] : '_';

        return ret;
    }

    class Serialize final :
      public Singleton<Serialize>
    {
      public:
        enum : std::uint32_t
        {
            _Version = 1,

            _Statistics = 'stcs',
            _Tracking = 'trcn'
        };

        static void SaveCallback(SKSE::SerializationInterface* a_intfc)
        {
#define SAVE(type, func)                                   \
    if (!a_intfc->OpenRecord(type, _Version)) {            \
        std::string insert = #type##s;                     \
        logger::error("Failed to open record {}", insert); \
    } else {                                               \
        func;                                              \
    }
            SAVE(_Statistics, Registry::Statistics::StatisticsData::GetSingleton()->Save(a_intfc))
            SAVE(_Tracking, Papyrus::Tracking::GetSingleton()->Save(a_intfc))

#undef SAVE
        }

        static void LoadCallback(SKSE::SerializationInterface* a_intfc)
        {
            const auto v = static_cast<uint32_t>(_Version);
            uint32_t type, version, length;
            while (a_intfc->GetNextRecordInfo(type, version, length)) {
                if (version != v) {
                    logger::info("Invalid Version for loaded Data of Type = {}. Expected = {}; Got = {}", GetTypeName(type), v, version);
                    continue;
                }
                logger::info("Loading record {}", GetTypeName(type));
                switch (type) {
                case _Statistics:
                    Registry::Statistics::StatisticsData::GetSingleton()->Load(a_intfc);
                    break;
                case _Tracking:
                    Papyrus::Tracking::GetSingleton()->Load(a_intfc);
                    break;
                default:
                    break;
                }
            }

            logger::info("Finished loading data from cosave");
        }

        static void RevertCallback(SKSE::SerializationInterface* a_intfc)
        {
            Registry::Statistics::StatisticsData::GetSingleton()->Revert(a_intfc);
            Papyrus::Tracking::GetSingleton()->Revert(a_intfc);
        }

        static void FormDeleteCallback(RE::VMHandle)
        {
        }

    };  // class Serialize

}  // namespace Serialization
