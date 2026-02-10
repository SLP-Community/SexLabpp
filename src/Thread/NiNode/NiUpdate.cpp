#include "NiUpdate.h"

#include <SimpleIni.h>

#include "NiDescriptor.h"

namespace Thread::NiNode
{
	void NiUpdate::Install()
	{
		if (!InitializeDescriptors()) {
			logger::critical("Failed to initialize descriptors. NiNode interactions will not work.");
			assert(false && "Failed to initialize descriptors");
			return;
		}

		auto& trampoline = SKSE::GetTrampoline();
		REL::Relocation<std::uintptr_t> update{ REL::RelocationID(35565, 36564), REL::VariantOffset(0x53, 0x6E, 0x68) };
		_OnFrameUpdate = trampoline.write_call<5>(update.address(), OnFrameUpdate);
	}

	float NiUpdate::GetDeltaTime()
	{
		static REL::Relocation<float*> deltaTime{ REL::VariantID(523660, 410199, 0x30C3A08) };
		return *deltaTime.get();
	}

	bool NiUpdate::InitializeDescriptors()
	{
		if (!fs::exists(MODELPATH)) {
			logger::error("Descriptors: Settings file not found at {}", MODELPATH);
			return false;
		}

		CSimpleIniA inifile{};
		inifile.SetUnicode();
		const auto ec = inifile.LoadFile(MODELPATH);
		if (ec < 0) {
			logger::error("Descriptors: Failed to read .ini file, Error: {}", ec);
			return false;
		}

		try {
			NiDescriptor<NiType::Kissing>::Initialize(inifile);

			logger::info("Descriptors: Model initialization complete");
			return true;
		} catch (const std::exception& e) {
			logger::error("Descriptors: Initialization failed - {}", e.what());
		}
		return false;
	}

	void NiUpdate::OnFrameUpdate(RE::PlayerCharacter* a_this)
	{
		_OnFrameUpdate(a_this);

		static auto calendar = RE::Calendar::GetSingleton();
		static auto lastGameHour = 0.0f;
		const auto currentGameHour = calendar->GetHour();
		if (currentGameHour == lastGameHour) {
			return;
		}
		lastGameHour = currentGameHour;

		std::scoped_lock mlLk{ _mlMutex };
		const bool isMLTraining = mlTrainingState.type != NiType::Type::None;
	
		std::scoped_lock lk{ _m };
		time += GetDeltaTime();
		for (auto&& [_, process] : _instances) {
			process->Update(time);
			if (!isMLTraining || !process->HasActor(a_this->GetFormID()))
				continue;
			if (++mlTrainingState.frameCount < mlTrainingState.frameInterval) {
				continue;
			}
			mlTrainingState.frameCount = 0;
			process->ForEachInteraction([&](RE::ActorPtr a, RE::ActorPtr b, const NiInteraction& interaction) {
				const auto csvRow = interaction.descriptor ? interaction.descriptor->CsvRow() : "";
				if (csvRow.empty() || !a->IsPlayerRef() && !b->IsPlayerRef()) {
					return;  // only log interactions involving the player & interaction has likelihood
				}
				const auto actorAId = a->GetFormID();
				const auto actorBId = b->GetFormID();
				const auto labelStr = mlTrainingState.enabled ? "1" : "0";
				const auto row = std::format("{:X},{:X},{},{}", actorAId, actorBId, csvRow, labelStr);
				mlTrainingState.recordedData.push_back(row);
			},
			  0, 0, mlTrainingState.type);
		}
	}

	std::shared_ptr<NiInstance> NiUpdate::Register(RE::FormID a_id, std::vector<RE::Actor*> a_positions, const Registry::Scene* a_scene) noexcept
	{
		try {
			std::scoped_lock lk{ _m };
			const auto where = std::ranges::find(_instances, a_id, [](auto& it) { return it.first; });
			if (where != _instances.end()) {
				logger::info("Object with ID {:X} already registered. Resetting NiInstance.", a_id);
				std::swap(*where, _instances.back());
				_instances.pop_back();
			}
			auto process = std::make_shared<NiInstance>(a_positions, a_scene);
			return _instances.emplace_back(a_id, process).second;
		} catch (const std::exception& e) {
			logger::error("Failed to register NiInstance: {}", e.what());
			return nullptr;
		} catch (...) {
			logger::error("Failed to register NiInstance: Unknown error");
			return nullptr;
		}
	}

	void NiUpdate::Unregister(RE::FormID a_id) noexcept
	{
		std::scoped_lock lk{ _m };
		const auto where = std::ranges::find(_instances, a_id, [](auto& it) { return it.first; });
		if (where == _instances.end()) {
			logger::error("No object registered using ID {:X}", a_id);
			return;
		}
		_instances.erase(where);
	}

	void NiUpdate::UpdateMLTrainingState(NiType::Type a_type, bool enabled)
	{
		std::scoped_lock lk{ _mlMutex };
		if (mlTrainingState.type != a_type && mlTrainingState.recordedData.size() > 0) {
			// Clear recorded data when changing to a different interaction type
			const auto oldStateStr = magic_enum::enum_name(mlTrainingState.type);
			const auto newStateStr = magic_enum::enum_name(a_type);
			logger::info("ML Training State changing from {} to {}, clearing recorded data with {} rows", oldStateStr, newStateStr, mlTrainingState.recordedData.size());
			const auto headerStr = INiDescriptor::CsvHeader();
			const auto csvFile = std::ranges::fold_left(mlTrainingState.recordedData, std::format("ActorA,ActorB,{},Label", headerStr), [](std::string&& acc, const std::string& row) {
				return std::move(acc) + "\n" + row;
			});
			const auto folderPath = std::format("{}\\{}", MODELDATAPATH, oldStateStr);
			size_t uniqueFileId = 0;
			if (!fs::exists(folderPath)) {
				fs::create_directories(folderPath);
			} else {
				for (const auto& entry : fs::directory_iterator(folderPath)) {
					if (entry.is_regular_file() && entry.path().extension() == ".csv") {
						uniqueFileId++;
					}
				}
			}
			const auto finalPath = std::format("{}\\ML_TrainingData_{}.csv", folderPath, uniqueFileId);
			std::ofstream outFile(finalPath);
			if (outFile.is_open()) {
				outFile << csvFile;
				outFile.close();
				logger::info("Saved ML training data to {}", finalPath);
				Util::PrintConsole(std::format("Saved ML training data to {}", finalPath));
			} else {
				logger::error("Failed to save ML training data to {}", finalPath);
			}
			mlTrainingState.recordedData.clear();
		}
		mlTrainingState.type = a_type;
		mlTrainingState.enabled = enabled;
		mlTrainingState.frameCount = 0;  // reset frame count when changing state
		logger::info("ML Training State updated: Type={}, Enabled={}", magic_enum::enum_name(a_type), enabled);
	}

	void NiUpdate::SetMLTrainingFrameInterval(size_t interval)
	{
		std::scoped_lock lk{ _mlMutex };
		mlTrainingState.frameInterval = interval;
		logger::info("ML Training frame interval set to {} frames", interval);
	}

	void NiUpdate::ClearMLTrainingData()
	{
		std::scoped_lock lk{ _mlMutex };
		const auto dataSize = mlTrainingState.recordedData.size();
		mlTrainingState.recordedData.clear();
		logger::info("Cleared ML training data, removed {} rows", dataSize);
	}

	bool NiUpdate::IsMLTrainingEnabled()
	{
		std::scoped_lock lk{ _mlMutex };
		return mlTrainingState.type != NiType::Type::None;
	}

	NiUpdate::MLTrainingState NiUpdate::GetMLTrainingState()
	{	
		std::scoped_lock lk{ _mlMutex };
		return mlTrainingState;
	}

}  // namespace Thread::NiNode
