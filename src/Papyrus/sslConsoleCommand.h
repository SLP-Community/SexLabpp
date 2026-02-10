#pragma once

#include "Thread/NiNode/NiUpdate.h"

namespace Papyrus::ConsoleCommand
{
    using NiType = Thread::NiNode::NiType::Type;

	std::string MLStart(STATICARGS, std::string a_type, bool a_enabled)
	{
		const auto activeType = Thread::NiNode::NiUpdate::GetMLTrainingState().type;
		const auto type = magic_enum::enum_cast<NiType>(a_type, magic_enum::case_insensitive).value_or(NiType::None);
        if (activeType != type && activeType != NiType::None) {
            return "ML training already in progress. Use 'MLStop' to stop the current training before starting a new one.";
        }
		Thread::NiNode::NiUpdate::UpdateMLTrainingState(type, a_enabled);
		const auto typeName = magic_enum::enum_name<NiType>(type);
		const auto enableStr = a_enabled ? "enabled" : "disabled";
		return std::format("ML training for {} {}", typeName, enableStr);
	}

    std::string MLSwitch(STATICARGS, int a_state)
    {
		const auto state = Thread::NiNode::NiUpdate::GetMLTrainingState();
		if (a_state == -1)
			a_state = 1 - state.enabled;
        if (a_state != 0 && a_state != 1) {
            return "Invalid state. Valid states are: 0 (disable), 1 (enable), -1 (toggle)";
        }
		Thread::NiNode::NiUpdate::UpdateMLTrainingState(state.type, a_state == 1);
        const auto typeName = magic_enum::enum_name<NiType>(state.type);
        const auto enableStr = a_state == 1 ? "enabled" : "disabled";
        return std::format("ML training for {} {}", typeName, enableStr);
    }

	std::string MLStop(STATICARGS)
	{
		Thread::NiNode::NiUpdate::UpdateMLTrainingState(NiType::None, false);
		return "ML training stopped";
	}

	std::string MLSetInterval(STATICARGS, int a_frameInterval)
	{
		if (a_frameInterval <= 0) {
			return "Invalid frame interval. Please provide a positive integer.";
		}
		Thread::NiNode::NiUpdate::SetMLTrainingFrameInterval(static_cast<size_t>(a_frameInterval));
		return std::format("ML training frame interval set to {} frames", a_frameInterval);
	}

	std::string MLDrop(STATICARGS)
	{
		Thread::NiNode::NiUpdate::ClearMLTrainingData();
		return "ML training data cleared";
	}

	inline bool Register(VM* a_vm)
	{
		REGISTERFUNC(MLStart, "sslConsoleCommands", true);
		REGISTERFUNC(MLSwitch, "sslConsoleCommands", true);
		REGISTERFUNC(MLStop, "sslConsoleCommands", true);
        REGISTERFUNC(MLSetInterval, "sslConsoleCommands", true);
        REGISTERFUNC(MLDrop, "sslConsoleCommands", true);

		return true;
	}

}  // namespace Papyrus::ConsoleCommand
