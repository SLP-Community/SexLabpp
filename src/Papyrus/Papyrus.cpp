#include "Papyrus.h"

#include "SexLabRegistry.h"
#include "SexLabUtil.h"
#include "sslActorStats.h"
#include "sslLibrary/sslActorLibrary.h"
#include "sslLibrary/sslThreadLibrary.h"
#include "sslObject/sslAnimationSlots.h"
#include "sslObject/sslCreatureAnimationSlots.h"
#include "sslObject/sslExpressionSlots.h"
#include "sslObject/sslVoiceSlots.h"
#include "sslSystemConfig.h"
#include "sslThreadModel.h"
#include "sslConsoleCommand.h"

namespace Papyrus
{
	bool Register()
	{
		const auto papyrus = SKSE::GetPapyrusInterface();
		if (!papyrus) {
			logger::critical("Failed to get papyurs interface");
			return false;
		}
		papyrus->Register(Papyrus::SexLabRegistry::Register);
		papyrus->Register(Papyrus::SexLabUtil::Register);
		papyrus->Register(Papyrus::ActorLibrary::Register);
		papyrus->Register(Papyrus::ActorStats::Register);
		papyrus->Register(Papyrus::AnimationSlots::Register);
		papyrus->Register(Papyrus::CreatureAnimationSlots::Register);
		papyrus->Register(Papyrus::ExpressionSlots::Register);
		papyrus->Register(Papyrus::SystemConfig::Register);
		papyrus->Register(Papyrus::ThreadLibrary::Register);
		papyrus->Register(Papyrus::ThreadModel::Register);
		papyrus->Register(Papyrus::VoiceSlots::Register);
		papyrus->Register(Papyrus::ConsoleCommand::Register);

		return true;
	}
}	 // namespace Papyrus