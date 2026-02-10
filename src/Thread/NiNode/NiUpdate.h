#pragma once

#include "NiInstance.h"

namespace Thread::NiNode
{
	class NiUpdate
	{
		struct MLTrainingState
		{
			NiType::Type type{ NiType::None };
			std::vector<std::string> recordedData{};
			bool enabled{ false };
			size_t frameInterval{ 20 };
			size_t frameCount{ 0 };
		};

	  public:
		static void Install();
		static float GetDeltaTime();

		static std::shared_ptr<NiInstance> Register(RE::FormID a_id, std::vector<RE::Actor*> a_positions, const Registry::Scene* a_scene) noexcept;
		static void Unregister(RE::FormID a_id) noexcept;

		static void UpdateMLTrainingState(NiType::Type a_type, bool enabled);
		static void SetMLTrainingFrameInterval(size_t interval);
		static void ClearMLTrainingData();
		static bool IsMLTrainingEnabled();
		static MLTrainingState GetMLTrainingState();

	  private:
		static bool InitializeDescriptors();

		static void OnFrameUpdate(RE::PlayerCharacter* a_this);
		static inline REL::Relocation<decltype(OnFrameUpdate)> _OnFrameUpdate;

		static inline float time = 0.0f;
		static inline std::mutex _m{}, _mlMutex{};
		static inline std::vector<std::pair<RE::FormID, std::shared_ptr<NiInstance>>> _instances{};
		static inline MLTrainingState mlTrainingState{};
	};

}  // namespace Thread::NiNode
