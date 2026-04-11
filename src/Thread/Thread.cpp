#include "Thread.h"

#include "Registry/Library.h"
#include "Registry/Util/Scale.h"
#include "Thread/Interface/SceneMenu.h"
#include "Thread/Interface/SLToolsMenu.h"
#include "Registry/Util/RayCast/Offsets.h"
#include "Util/Script.h"

namespace Thread
{
	void Instance::CreateInstance(RE::TESQuest* a_linkedQst, const std::vector<RE::Actor*> a_submissives, const SceneMapping& a_scenes, FurniturePreference a_furniturePreference)
	{
		if (GetInstance(a_linkedQst)) {
			logger::warn("Thread instance already exists for quest {:X}.", a_linkedQst->formID);
			DispatchContinueSetup(a_linkedQst, false);
			return;
		}
		try {
			auto instance = std::make_unique<Instance>(a_linkedQst, a_submissives, a_scenes, a_furniturePreference);
			std::unique_lock lock{_mInstances};
			if (instance->pendingQst != nullptr) {
				pendingInstances.emplace_back(std::move(instance));
				// DispatchContinueSetup() called by Instance::FinalizeCenterRefSelection()
			} else {
				instances.emplace_back(std::move(instance));
				DispatchContinueSetup(a_linkedQst, true);
			}
			return;
		} catch (const std::exception& e) {
			logger::error("Failed to create thread instance: {}", e.what());
			DispatchContinueSetup(a_linkedQst, false);
			return;
		}
	}

	void Instance::DestroyInstance(RE::TESQuest* a_linkedQst)
	{
		std::unique_lock lock{_mInstances};
		std::erase_if(instances, [&](const auto& instance) { return instance->linkedQst == a_linkedQst; });
	}

	Instance* Instance::GetInstance(RE::TESQuest* a_linkedQst)
	{
		std::shared_lock lock{_mInstances};
		for (auto&& instance : instances) {
			if (instance->linkedQst == a_linkedQst) {
				return instance.get();
			}
		}
		return nullptr;
	}

	Instance* Instance::GetPendingInstance(RE::TESQuest* a_linkedQst)
	{
		std::shared_lock lock{ _mInstances };
		for (auto&& instance : pendingInstances) {
			if (instance->linkedQst == a_linkedQst) {
				return instance.get();
			}
		}
		return nullptr;
	}

	void Instance::DispatchContinueSetup(RE::TESQuest* a_linkedQst, bool a_result)
	{
		const auto handle = Script::GetScriptObject(a_linkedQst, "sslThreadModel");
		Script::CallbackPtr callbackPtr{};
		Script::DispatchMethodCall(handle, "ContinueSetup", callbackPtr, std::move(a_result));
	}

	void Instance::Center::SetReference(RE::TESObjectREFR* a_ref, Registry::FurnitureOffset a_offset)
	{
		assert(alias && a_ref);
		alias->ForceRefTo(a_ref);
		offset = a_offset;
		details = Registry::Library::GetSingleton()->GetFurnitureDetails(a_ref);
	}

	bool Instance::ControlsMenu()
	{
		return Interface::SceneMenu::IsInstance(this);
	}

	bool Instance::TryOpenMenu()
	{
		if (Interface::SceneMenu::IsOpen()) return false;
		Interface::SceneMenu::Show(this);
		return true;
	}

	bool Instance::TryCloseMenu()
	{
		if (!Interface::SceneMenu::IsOpen()) return false;
		Interface::SceneMenu::Hide();
		return true;
	}

	void Instance::UpdateTimer(float a_timer)
	{
		if (!ControlsMenu()) return;
		Interface::SceneMenu::UpdateTimer(a_timer);
	}

	void Instance::AdvanceScene(const Registry::Stage* a_nextStage)
	{
		assert(activeScene && activeScene->GetStageNodeType(a_nextStage) != Registry::Scene::NodeType::None);
		if (niInstance == nullptr) {
			niInstance = NiNode::NiUpdate::Register(linkedQst->formID, *activeAssignment, activeScene);
		}
		activeStage = a_nextStage;
		const auto scaling = Registry::Scale::GetSingleton();
		for (size_t i = 0; i < activeAssignment->size(); i++) {
			const auto& actor = activeAssignment->at(i);
			const auto& position = a_nextStage->positions[i];
			const auto& coordinate = position.offset.ApplyReturn(baseCoordinates);
			const auto& positionInfo = activeScene->GetNthPosition(i);
			const auto& animationEvent = activeScene->GetNthAnimationEvent(a_nextStage, i);

			scaling->SetScale(actor, positionInfo->data.GetRace(), positionInfo->data.GetScale());
			actor->SetAngle({ 0.0f, 0.0f, coordinate.rotation });
			actor->SetPosition(coordinate.AsNiPoint(), true);
			actor->Update3DPosition(true);
			actor->NotifyAnimationGraph(animationEvent);
		}
		if (ControlsMenu()) {
			Interface::SceneMenu::UpdateStageInfo();
		}
	}

	bool Instance::SetActiveScene(const Registry::Scene* a_scene)
	{
		assert(a_scene);
		if (!a_scene->IsCompatibleFurniture(center.offset.type)) {
			logger::warn("Scene {} is not compatible with center reference {}.", a_scene->id, center.GetRef()->GetFormID());
			return false;
		}
		const auto fragments = std::ranges::fold_left(positions, std::vector<Registry::ActorFragment>{}, [](auto&& acc, const auto& it) {
			return (acc.push_back(it.data), acc);
		});
		const auto newAssignments = a_scene->FindAssignments(fragments);
		if (newAssignments.empty()) {
			logger::warn("Scene {} has no valid assignments.", a_scene->id);
			return false;
		}
		assignments = newAssignments;
		activeScene = a_scene;
		std::map<RE::Actor*, uint8_t> positionCounts;
		for (const auto& permutation : assignments) {
			for (size_t i = 0; i < permutation.size(); i++) {
				positionCounts[permutation[i]]++;
			}
		}
		for (auto&& p : positions) {
			p.uniquePermutations = positionCounts[p.data.GetActor()];
		}
		baseCoordinates = center.offset.offset.ApplyReturn(center.GetRef());
		activeScene->furnitureOffset.Apply(baseCoordinates);
		activeAssignment = assignments.begin();
		if (ControlsMenu()) {
			Interface::SceneMenu::UpdateActiveScene();
		}
		return true;
	}

	std::vector<const Registry::Scene*> Instance::GetThreadScenes(SceneType a_type)
	{
		assert(a_type < SceneType::Total);
		return scenes[a_type];
	}

	std::vector<const Registry::Scene*> Instance::GetThreadScenes()
	{
		for (auto&& sceneVec : scenes) {
			if (std::ranges::contains(sceneVec, activeScene)) {
				return sceneVec;
			}
		}
		return {};
	}

	const std::vector<RE::Actor*>& Instance::GetActors()
	{
		assert(activeAssignment != assignments.end());
		return *activeAssignment;
	}

	Instance::Position* Instance::GetPosition(RE::Actor* a_actor)
	{
		assert(a_actor);
		for (auto& position : positions) {
			if (position.data.GetActor() == a_actor) {
				return &position;
			}
		}
		return nullptr;
	}

	const Registry::PositionInfo* Instance::GetPositionInfo(RE::Actor* a_actor)
	{
		assert(a_actor);
		const auto i = std::distance(activeAssignment->begin(), std::find(activeAssignment->begin(), activeAssignment->end(), a_actor));
		assert(i >= 0);
		if (static_cast<size_t>(i) >= activeAssignment->size()) {
			logger::warn("Actor {} is not part of the current scene.", a_actor->GetFormID());
			return nullptr;
		}
		return activeScene->GetNthPosition(i);
	}

	void Instance::UpdatePlacement(RE::Actor* a_actor)
	{
		assert(a_actor);
		const auto i = std::distance(activeAssignment->begin(), std::find(activeAssignment->begin(), activeAssignment->end(), a_actor));
		assert(i >= 0);
		if (static_cast<size_t>(i) >= activeAssignment->size()) {
			logger::warn("Actor {} is not part of the current scene.", a_actor->GetFormID());
			return;
		}
		const auto& position = activeStage->positions[i];
		const auto& coordinate = position.offset.ApplyReturn(baseCoordinates);
		a_actor->SetAngle({ 0.0f, 0.0f, coordinate.rotation });
		a_actor->SetPosition(coordinate.AsNiPoint(), true);
		a_actor->Update3DPosition(true);
	}

	bool Instance::ReplaceCenterRef(RE::TESObjectREFR* a_ref)
	{
		if (((bool (*)(void))Offsets::NotOnGameThread.address())()) {
			logger::error("ReplaceCenterRef is not on valid thread, this should never happen and can cause random CTD/freezes");
		}
		assert(a_ref);
		if (a_ref == center.GetRef() && !a_ref->IsPlayerRef()) {
			return false;
		}
		const auto centerStr = center.offset.type.ToString();
		const auto& details = center.details = Registry::Library::GetSingleton()->GetFurnitureDetails(center.GetRef());
		if (!details) {
			if (!center.offset.type.IsNone()) {
				constexpr auto nonStr = Registry::FurnitureType::ToString<Registry::FurnitureType::None>();
				logger::warn("Mismatched furniture type. Expected {} but got {} for reference {:X}", centerStr, nonStr, a_ref->GetFormID());
				return false;
			}
			center.SetReference(a_ref, {});
		} else {
			const auto inBounds = details->GetClosestCoordinatesInBound(a_ref, center.offset.type.value, center.GetRef());
			if (inBounds.empty()) {
				logger::warn("Reference {:X} is not compatible with any scene.", a_ref->GetFormID());
				return false;
			}
			center.SetReference(a_ref, inBounds.front());
		}
		baseCoordinates = center.offset.offset.ApplyReturn(center.GetRef());
		activeScene->furnitureOffset.Apply(baseCoordinates);
		AdvanceScene(activeStage);
		return true;
	}

	bool Instance::GetAutoplayEnabled()
	{
		const auto scriptObj = Script::GetScriptObject(linkedQst, "sslThreadModel");
		return Script::GetTrivialProperty<bool>(scriptObj, "AutoAdvance");
	}

	void Instance::SetAutoplayEnabled(bool a_enabled)
	{
		const auto scriptObj = Script::GetScriptObject(linkedQst, "sslThreadModel");
		Script::SetProperty(scriptObj, "AutoAdvance", a_enabled);
	}

	void Instance::SetAnimationPlaybackSpeed(float playbackSpeed)
	{
		std::vector<std::pair<RE::BSAnimationGraphManagerPtr, std::unique_ptr<RE::BSSpinLockGuard>>> lockedGraphs;

		for (auto& position : positions) {
			const auto* actor = position.data.GetActor();
			if (!actor) {
				continue;
			}

			RE::BSAnimationGraphManagerPtr graphMgr;
			if (!actor->GetAnimationGraphManager(graphMgr) || !graphMgr) {
				continue;
			}

			auto& runtime = graphMgr->GetRuntimeData();
			lockedGraphs.emplace_back(
			  graphMgr,
			  std::make_unique<RE::BSSpinLockGuard>(runtime.updateLock));
		}

		for (auto& [graphMgr, lock] : lockedGraphs) {
			auto& runtime = graphMgr->GetRuntimeData();
			auto activeGraph = runtime.activeGraph;

			RE::BShkbAnimationGraph* animationGraph = graphMgr->graphs[activeGraph].get();
			if (!animationGraph || !animationGraph->behaviorGraph) {
				continue;
			}

			auto* activeNodes = *reinterpret_cast<RE::hkArray<RE::hkbNodeInfo>**>(
			  &animationGraph->behaviorGraph->activeNodes);
			if (!activeNodes) {
				continue;
			}

			for (const RE::hkbNodeInfo& activeNode : *activeNodes) {
				if (!activeNode.nodeClone) {
					continue;
				}
				if (auto* clip = skyrim_cast<RE::hkbClipGenerator*>(activeNode.nodeClone)) {
					clip->playbackSpeed = playbackSpeed;
					if (clip->animationControl) {
						clip->animationControl->playbackSpeed = playbackSpeed;
					}
				}
			}
		}
	}

	void Instance::SetEnjoyment(RE::Actor* a_position, float a_enjoyment)
	{
		// COMEBACK: If enjoyment is moved into backend, update this
		if (ControlsMenu()) {
			Interface::SceneMenu::UpdateSlider(a_position->GetFormID(), a_enjoyment);
		}
	}

	const Registry::Expression* Instance::GetExpression(RE::Actor* a_actor)
	{
		const auto position = GetPosition(a_actor);
		if (!position) {
			logger::warn("Actor {} is not part of the current scene.", a_actor->GetFormID());
			return nullptr;
		}
		return position->expression;
	}

	void Instance::SetExpression(RE::Actor* a_actor, const Registry::Expression* a_expression)
	{
		const auto position = GetPosition(a_actor);
		if (!position) {
			logger::warn("Actor {} is not part of the current scene.", a_actor->GetFormID());
			return;
		}
		position->expression = a_expression;
	}

	const Registry::Voice* Instance::GetVoice(RE::Actor* a_actor)
	{
		const auto position = GetPosition(a_actor);
		if (!position) {
			logger::warn("Actor {} is not part of the current scene.", a_actor->GetFormID());
			return nullptr;
		}
		return position->voice;
	}

	void Instance::SetVoice(RE::Actor* a_actor, const Registry::Voice* a_voice)
	{
		const auto position = GetPosition(a_actor);
		if (!position) {
			logger::warn("Actor {} is not part of the current scene.", a_actor->GetFormID());
			return;
		}
		position->voice = a_voice;
	}

	bool Instance::IsGhostMode(RE::Actor* a_actor)
	{
		const auto position = GetPosition(a_actor);
		if (!position) {
			logger::warn("Actor {} is not part of the current scene.", a_actor->GetFormID());
			return false;
		}
		return position->ghostAlpha.has_value();
	}

	void Instance::SetGhostMode(RE::Actor* a_actor, bool a_ghostMode)
	{
		const auto position = GetPosition(a_actor);
		if (!position) {
			logger::warn("Actor {} is not part of the current scene.", a_actor->GetFormID());
			return;
		}
		if (a_ghostMode) {
			if (!position->ghostAlpha.has_value())
				position->ghostAlpha = a_actor->GetAlpha();
			a_actor->SetAlpha(Settings::fGhostModeAlpha);
		} else {
			if (position->ghostAlpha.has_value()) {
				a_actor->SetAlpha(position->ghostAlpha.value());
				position->ghostAlpha.reset();
			} else {
				logger::warn("Actor {} is not in ghost mode.", a_actor->GetFormID());
				a_actor->SetAlpha(1.0f);
			}
		}
	}

	int32_t Instance::GetUniquePermutations(RE::Actor* a_actor)
	{
		const auto position = GetPosition(a_actor);
		if (!position) {
			logger::warn("Actor {} is not part of the current scene.", a_actor->GetFormID());
			return 0;
		}
		return position->uniquePermutations;
	}

	int32_t Instance::GetCurrentPermutation(RE::Actor* a_actor)
	{
		const auto position = GetPosition(a_actor);
		if (!position) {
			logger::error("Actor {} is not part of the current scene.", a_actor->GetFormID());
			return 0;
		}
		std::set<ptrdiff_t> seenPermutations;
		for (auto it = assignments.begin(); it < assignments.end(); it++) {
			const auto idx = std::distance(it->begin(), std::find(it->begin(), it->end(), a_actor));
			seenPermutations.insert(idx);
			if (it == activeAssignment) {
				return static_cast<int32_t>(seenPermutations.size());
			}
		}
		throw std::runtime_error("Failed to find current permutation for actor.");
	}

	bool Instance::SetNextPermutation(RE::Actor* a_actor)
	{
		const auto position = GetPosition(a_actor);
		if (!position) {
			logger::error("Actor {} is not part of the current scene.", a_actor->GetFormID());
			return false;
		}
		if (position->uniquePermutations < 2) {
			logger::info("Actor {} has no alternative permutations.", a_actor->GetFormID());
			return false;
		}
		auto targetPermutation = GetCurrentPermutation(a_actor) + 1;
		if (targetPermutation > position->uniquePermutations) {
			targetPermutation = 1;
		}
		std::set<ptrdiff_t> seenPermutations;
		for (auto it = assignments.begin(); it < assignments.end(); it++) {
			const auto idx = std::distance(it->begin(), std::find(it->begin(), it->end(), a_actor));
			if (idx < 0 || static_cast<size_t>(idx) == it->size()) {
				logger::warn("Actor {} is not part of the current assignment.", a_actor->GetFormID());
				return false;
			} else if (!seenPermutations.contains(idx)) {
				seenPermutations.insert(idx);
				if (seenPermutations.size() == targetPermutation) {
					activeAssignment = it;
					AdvanceScene(activeStage);
					logger::info("Actor {} changed to permutation {}.", a_actor->GetFormID(), targetPermutation);
					return true;
				}
			}
		}
		logger::warn("Actor {} has no alternative permutations.", a_actor->GetFormID());
		return false;
	}

	void Instance::OpenSLToolsMenu()
	{
		std::vector<const Registry::Scene*> playingScenes = GetThreadScenes();
		std::vector<RE::BSFixedString> playingScenesNames;
		playingScenesNames.reserve(playingScenes.size());
		for (const Registry::Scene* playingScene : playingScenes) {
			playingScenesNames.push_back(playingScene->name);
		}
		Script::ObjectPtr scriptObj = Script::GetScriptObject(linkedQst, "sslThreadController");
		PrismaUI::SLToolsMenu::Open(scriptObj, activeScene->name, playingScenesNames);
	}

}	 // namespace Thread
