#include "SceneMenu.h"

#include "Interface.h"
#include "Registry/Library.h"
#include "Util/StringUtil.h"

namespace Thread::Interface
{
	SceneMenu::SceneMenu() :
		RE::IMenu()
	{
		this->inputContext = Context::kNone;
		this->depthPriority = DEPTH_PRIORITY;
		this->menuFlags.set(
			Flag::kAlwaysOpen,
			Flag::kCustomRendering,
			Flag::kAssignCursorToRenderer);

		auto scaleform = RE::BSScaleformManager::GetSingleton();
		[[maybe_unused]] bool success = scaleform->LoadMovieEx(this, FILEPATH, [](RE::GFxMovieDef* a_def) -> void {
			a_def->SetState(
				RE::GFxState::StateType::kLog,
				RE::make_gptr<FlashLogger<SceneMenu>>().get());
		});
		assert(success);

		auto view = this->uiMovie;
		view->SetMouseCursorCount(0);
		FunctionManager::AttachSKSEFunctions(view);
		AttachSexLabAPIFunctions(view);

		auto hud = RE::UI::GetSingleton()->GetMovieView(RE::HUDMenu::MENU_NAME);
		RE::GFxValue hudMain, showMessage;
		success = hud->GetVariable(&hudMain, "_root.HUDMovieBaseInstance");
		assert(success);
		success = hudMain.GetMember("ShowMessage", &showMessage);
		assert(success);
		success = hudMain.SetMember("ShowMessage_SEXLABREROUTE", showMessage);
		assert(success);
		FunctionManager::AttachFunction<HUDMenu_ShowMessageEx>(hud, hudMain, "ShowMessage");
	}

	RE::UI_MESSAGE_RESULTS SceneMenu::ProcessMessage(RE::UIMessage& a_message)
	{
		using Type = RE::UI_MESSAGE_TYPE;
		using Result = RE::UI_MESSAGE_RESULTS;

		const auto input = RE::BSInputDeviceManager::GetSingleton();
		const auto controls = RE::ControlMap::GetSingleton();
		switch (*a_message.type) {
		case Type::kShow:
			logger::info("SceneMenu opened.");
			assert(threadInstance);
			UpdatePositions();
			UpdateActiveScene();
			input->AddEventSink<RE::InputEvent*>(this);
			controls->ToggleControls(RE::ControlMap::UEFlag::kActivate, false, false);
			controls->ToggleControls(RE::ControlMap::UEFlag::kMovement, false, false);
			if (Settings::bHideHUD) {
				RE::UIMessageQueue::GetSingleton()->AddMessage(RE::HUDMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
			}
			return Result::kHandled;
		case Type::kForceHide:
		case Type::kHide:
			logger::info("SceneMenu closed.");
			RE::UIMessageQueue::GetSingleton()->AddMessage(RE::HUDMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
			controls->ToggleControls(RE::ControlMap::UEFlag::kActivate, true, false);
			controls->ToggleControls(RE::ControlMap::UEFlag::kMovement, true, false);
			controls->AllowTextInput(false);
			input->RemoveEventSink(this);
			threadInstance = nullptr;
			return Result::kHandled;
		case Type::kUserEvent:
		case Type::kScaleformEvent:
			if (RE::ControlMap::GetSingleton()->GetRuntimeData().textEntryCount <= 0)
				return Result::kPassOn;
			__fallthrough;
		default:
			return RE::IMenu::ProcessMessage(a_message);
		}
	}

	void SceneMenu::UpdateSlider(RE::FormID a_actorId, float a_enjoyment)
	{
		SKSE::GetTaskInterface()->AddUITask([=]() {
			const auto view = RE::UI::GetSingleton()->GetMovieView(MENU_NAME);
			assert(view);
			std::vector<RE::GFxValue> args{};
			args.emplace_back(a_actorId);
			args.emplace_back(a_enjoyment);
			view->InvokeNoReturn("_root.main.updateSliderPct", args.data(), static_cast<uint32_t>(args.size()));
		});
	}

	void SceneMenu::SetSliderTo(RE::FormID a_actorId, float a_enjoyment)
	{
		SKSE::GetTaskInterface()->AddUITask([=]() {
			const auto view = RE::UI::GetSingleton()->GetMovieView(MENU_NAME);
			assert(view);
			std::vector<RE::GFxValue> args{};
			args.emplace_back(a_actorId);
			args.emplace_back(a_enjoyment);
			view->InvokeNoReturn("_root.main.setSliderPct", args.data(), static_cast<uint32_t>(args.size()));
		});
	}

	void SceneMenu::UpdatePositions()
	{
		assert(threadInstance);
		SKSE::GetTaskInterface()->AddUITask([]() {
			const auto positions = threadInstance->GetActors();
			const auto view = RE::UI::GetSingleton()->GetMovieView(MENU_NAME);
			assert(view);
			std::vector<RE::GFxValue> args{};
			args.reserve(positions.size());
			for (const auto& pos : positions) {
				assert(pos);
				const auto& pInfo = threadInstance->GetPosition(pos);
				assert(pInfo);
				RE::GFxValue arg;
				view->CreateObject(&arg);
				arg.SetMember("id", { pos->GetFormID() });
				arg.SetMember("name", { pos->GetName() });
				arg.SetMember("submissive", { pInfo->data.IsSubmissive() });
				args.push_back(arg);
			}
			view->InvokeNoReturn("_root.main.updatePositions", args.data(), static_cast<uint32_t>(args.size()));
		});
	}

	void SceneMenu::UpdateStageInfo()
	{
		SKSE::GetTaskInterface()->AddUITask([]() {
			const auto activeScene = threadInstance->GetActiveScene();
			const auto activeStage = threadInstance->GetActiveStage();
			const auto view = RE::UI::GetSingleton()->GetMovieView(MENU_NAME);
			assert(view && activeScene && activeStage);
			const auto& edges = activeScene->GetAdjacentStages(activeStage);
			std::vector<RE::GFxValue> args{};
			if (edges && !edges->empty()) {
				args.reserve(edges->size());
				for (const auto& edge : *edges) {
					RE::GFxValue arg;
					view->CreateObject(&arg);
					arg.SetMember("id", { edge->id.c_str() });
					arg.SetMember("name", { edge->navtext.c_str() });
					arg.SetMember("length", { edge->fixedlength > 0 });
					bool hasClimax = false;
					for (const auto& pos : edge->positions) {
						if (pos.climax) {
							hasClimax = true;
							break;
						}
					}
					arg.SetMember("climax", { hasClimax });
					bool endNode = activeScene->GetNumAdjacentStages(edge) == 0;
					arg.SetMember("end", { endNode });
					args.push_back(arg);
				}
			} else {
				RE::GFxValue arg;
				view->CreateObject(&arg);
				arg.SetMember("id", { ""sv });
				arg.SetMember("name", { "$SSL_EndScene"sv });
				args.push_back(arg);
			}
			view->InvokeNoReturn("_root.main.setStages", args.data(), static_cast<uint32_t>(args.size()));
		});
	}

	void SceneMenu::UpdateActiveScene()
	{
		SKSE::GetTaskInterface()->AddUITask([=]() {
			const auto view = RE::UI::GetSingleton()->GetMovieView(MENU_NAME);
			const auto activeScene = threadInstance->GetActiveScene();
			assert(view && activeScene);
			const auto activePackage = Registry::Library::GetSingleton()->GetPackageFromScene(activeScene);
			assert(activePackage);
			const auto tagVec = activeScene->tags.AsVector();
			const auto tagStr = Util::StringJoin(tagVec, ", ");
			const auto annotations = activeScene->tags.GetAnnotations();
			const auto annotationStr = Util::StringJoin(annotations, ", ");
			RE::GFxValue arg;
			view->CreateObject(&arg);
			arg.SetMember("name", { activeScene->name.c_str() });
			arg.SetMember("author", { activePackage->GetAuthor().c_str() });
			arg.SetMember("package", { activePackage->GetName().c_str() });
			arg.SetMember("tags", { tagStr.c_str() });
			arg.SetMember("annotations", { annotationStr.c_str() });
			view->InvokeNoReturn("_root.main.setActiveScene", &arg, 1);
		});
	}

	void SceneMenu::UpdateTimer(float a_time)
	{
		SKSE::GetTaskInterface()->AddUITask([=]() {
			const auto view = RE::UI::GetSingleton()->GetMovieView(MENU_NAME);
			assert(view);
			RE::GFxValue arg{ a_time };
			view->InvokeNoReturn("_root.main.setTimer", &arg, 1);
		});
	}

	void SceneMenu::DisableTimer()
	{
		UpdateTimer(0.0f);
	}

	RE::BSEventNotifyControl SceneMenu::ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*)
	{
		using EventResult = RE::BSEventNotifyControl;
		if (!a_event) {
			return EventResult::kContinue;
		}
		const auto ui = RE::UI::GetSingleton();
		if (ui->IsMenuOpen(RE::Console::MENU_NAME) || ui->GameIsPaused() || ui->IsApplicationMenuOpen())
			return EventResult::kContinue;
		auto navType = Settings::KeyType::None;
		bool mode = false;
		bool reset = false;
		for (const RE::InputEvent* input = *a_event; input; input = input->next) {
			const auto buttonEvent = input->AsButtonEvent();
			if (!buttonEvent)
				continue;
			const auto idEvent = input->AsIDEvent();
			if (RE::PlayerCamera::GetSingleton()->IsInFreeCameraMode()) {
				constexpr std::array directionEvents{
					"Forward"sv,
					"Back"sv,
					"Strafe Left"sv,
					"Strafe Right"sv
				};
				const auto& userEvent = idEvent->QUserEvent();
				const auto directionEvent = std::ranges::find_if(directionEvents, [&](const auto& e) { return userEvent == e; });
				if (directionEvent != directionEvents.end()) {
					continue;
				}
			}
			const auto dxCode = idEvent->GetIDCode();
			const auto type = Settings::GetKeyType(dxCode);
			switch (type) {
			case Settings::KeyType::None:
				break;
			case Settings::KeyType::Reset:
				if (buttonEvent->IsPressed()) reset = true;
				break;
			case Settings::KeyType::Modes:
				if (buttonEvent->IsPressed()) mode = true;
				break;
			default:
				if (buttonEvent->IsDown()) navType = type;
				break;
			}
		}
		if (navType == Settings::KeyType::None)
			return EventResult::kContinue;
		auto enumName = magic_enum::enum_name(navType);
		if (enumName.empty())
			return EventResult::kContinue;
		SKSE::GetTaskInterface()->AddUITask([=]() {
			std::string navEq{ enumName };
			Util::ToLower(navEq);
			std::vector<RE::GFxValue> args{
				RE::GFxValue(navEq.c_str()),
				RE::GFxValue(mode),
				RE::GFxValue(reset)
			};
			if (this->uiMovie) {
				this->uiMovie->InvokeNoReturn("_root.main.handleInputEx", args.data(), static_cast<uint32_t>(args.size()));
			}
		});
		return EventResult::kContinue;
	}

	void SceneMenu::HUDMenu_ShowMessageEx::Call(Params& a_args)
	{
		const auto ui = RE::UI::GetSingleton();
		if (SceneMenu::IsOpen() && !ui->IsMenuOpen(RE::HUDMenu::MENU_NAME)) {
			auto scene = ui->GetMovieView(SceneMenu::MENU_NAME);
			scene->InvokeNoReturn("_root.main.ShowMessage", a_args.args, a_args.argCount);
		} else {
			auto hud = ui->GetMovieView(RE::HUDMenu::MENU_NAME);
			hud->InvokeNoReturn("_root.HUDMovieBaseInstance.ShowMessage_SEXLABREROUTE", a_args.args, a_args.argCount);
		}
	}

	RE::Actor* SceneMenu::GFxFunctionHandlerWrapper::GetActorByReferenceId(Params& a_args, size_t argIdx)
	{
		assert(a_args.argCount > argIdx && a_args.args[argIdx].GetType() == RE::GFxValue::ValueType::kNumber);
		const auto formId = a_args.args[argIdx].GetUInt();
		const auto positions = threadInstance->GetActors();
		for (auto&& actor : positions) {
			if (actor->GetFormID() == formId) {
				return actor;
			}
		}
		logger::warn("GetActorByReferenceId: Invalid form ID {}", formId);
		return nullptr;
	}

	void SceneMenu::AttachSexLabAPIFunctions(RE::GPtr<RE::GFxMovieView> a_view)
	{
		auto sexLabAPI = FunctionManager::MakeFunctionObject(a_view, "SexLabAPI");
		if (!sexLabAPI) {
			logger::warn("Failed to create SexLabAPI function object");
			return;
		}
		FunctionManager::AttachFunction<SLAPI_GetHotkeyCombination>(a_view, *sexLabAPI, "GetHotkeyCombination");
		FunctionManager::AttachFunction<SLAPI_GetActiveFurnitureName>(a_view, *sexLabAPI, "GetActiveFurnitureName");
		FunctionManager::AttachFunction<SLAPI_GetOffset>(a_view, *sexLabAPI, "GetOffset");
		FunctionManager::AttachFunction<SLAPI_ResetOffsets>(a_view, *sexLabAPI, "ResetOffsets");
		FunctionManager::AttachFunction<SLAPI_GetOffsetStepSize>(a_view, *sexLabAPI, "GetOffsetStepSize");
		FunctionManager::AttachFunction<SLAPI_AdjustOffsetStepSize>(a_view, *sexLabAPI, "AdjustOffsetStepSize");
		FunctionManager::AttachFunction<SLAPI_GetAdjustStageOnly>(a_view, *sexLabAPI, "GetAdjustStageOnly");
		FunctionManager::AttachFunction<SLAPI_SetAdjustStageOnly>(a_view, *sexLabAPI, "SetAdjustStageOnly");
		FunctionManager::AttachFunction<SLAPI_GetAlternateScenes>(a_view, *sexLabAPI, "GetAlternateScenes");
		FunctionManager::AttachFunction<SLAPI_ToggleAutoPlay>(a_view, *sexLabAPI, "ToggleAutoPlay");
		FunctionManager::AttachFunction<SLAPI_IsAutoPlay>(a_view, *sexLabAPI, "IsAutoPlay");
		FunctionManager::AttachFunction<SLAPI_SetHideHUD>(a_view, *sexLabAPI, "SetHUDVisible");
		FunctionManager::AttachFunction<SLAPI_GetHideHUD>(a_view, *sexLabAPI, "GetHUDVisible");
		FunctionManager::AttachFunction<SLAPI_GetPermutationData>(a_view, *sexLabAPI, "GetPermutationData");
		FunctionManager::AttachFunction<SLAPI_SelectNextPermutation>(a_view, *sexLabAPI, "SelectNextPermutation");
		FunctionManager::AttachFunction<SLAPI_GetGhostMode>(a_view, *sexLabAPI, "GetGhostMode");
		FunctionManager::AttachFunction<SLAPI_SetGhostMode>(a_view, *sexLabAPI, "SetGhostMode");
		FunctionManager::AttachFunction<SLAPI_GetExpressionName>(a_view, *sexLabAPI, "GetExpressionName");
		FunctionManager::AttachFunction<SLAPI_SetExpression>(a_view, *sexLabAPI, "SetExpression");
		FunctionManager::AttachFunction<SLAPI_GetExpressions>(a_view, *sexLabAPI, "GetExpressions");
		FunctionManager::AttachFunction<SLAPI_GetVoiceName>(a_view, *sexLabAPI, "GetVoiceName");
		FunctionManager::AttachFunction<SLAPI_SetVoice>(a_view, *sexLabAPI, "SetVoice");
		FunctionManager::AttachFunction<SLAPI_GetVoices>(a_view, *sexLabAPI, "GetVoices");
	}

	void SceneMenu::SLAPI_GetHotkeyCombination::Call(Params& a_args)
	{
		assert(a_args.argCount == 1 && a_args.args->GetType() == RE::GFxValue::ValueType::kString);
		std::string idStr{ a_args.args->GetString() };
		const auto enumName = magic_enum::enum_cast<Settings::KeyType>(idStr, magic_enum::case_insensitive);
		const auto keyCode = Settings::GetKeyCode(enumName.value_or(Settings::KeyType::None));
		if (keyCode == 0) {
			logger::warn("GetHotkeyCombination: Invalid key type {}", idStr);
			return a_args.retVal->SetString("");
		}
		const auto nameStr = SKSE::InputMap::GetKeyName(keyCode);
		a_args.retVal->SetString(nameStr.c_str());
	}

	void SceneMenu::SLAPI_GetActiveFurnitureName::Call(Params& a_args)
	{
		const auto type = threadInstance->GetFurnitureType();
		const auto name = threadInstance->GetCenterRef()->GetDisplayFullName();
		const auto nameStr = std::format("{} ({})", name, type.ToString());
		a_args.retVal->SetString(nameStr.c_str());
	}

	void SceneMenu::SLAPI_GetOffset::Call(Params& a_args)
	{
		assert(a_args.argCount > 0 && a_args.args[0].GetType() == RE::GFxValue::ValueType::kString);
		const auto argStr = a_args.args[0].GetString();
		auto offsetIdx = magic_enum::enum_cast<Registry::CoordinateType>(argStr, magic_enum::case_insensitive);
		if (!offsetIdx.has_value()) {
			logger::warn("GetOffset: Invalid offset type {}", argStr);
			return a_args.retVal->SetNumber(0.0f);
		}
		const auto activeScene = threadInstance->GetActiveScene();
		const auto activeStage = threadInstance->GetActiveStage();
		auto offsets = &activeScene->furnitureOffset;
		if (a_args.argCount > 1 && a_args.args[1].GetType() == RE::GFxValue::ValueType::kNumber) {
			auto act = GetActorByReferenceId(a_args, 1);
			if (!act) {
				logger::warn("GetOffset: Invalid actor reference ID {}", a_args.args[1].GetUInt());
				return a_args.retVal->SetNumber(0.0f);
			}
			const auto threadAct = threadInstance->GetActors();
			const auto i = std::distance(threadAct.begin(), std::find(threadAct.begin(), threadAct.end(), act));
			assert(i >= 0);
			if (static_cast<size_t>(i) >= threadAct.size()) {
				logger::warn("GetOffset: Actor {} is not part of the current scene", act->GetFormID());
				return a_args.retVal->SetNumber(0.0f);
			}
			offsets = &activeStage->positions[i].offset;
		}
		const auto offset = offsets->GetOffset(offsetIdx.value());
		a_args.retVal->SetNumber(offset);
	}

	void SceneMenu::SLAPI_ResetOffsets::Call(Params& a_args)
	{
		const auto activeScene = threadInstance->GetActiveScene();
		const auto activeStage = threadInstance->GetActiveStage();
		Registry::Library::GetSingleton()->EditScene(activeScene, [&](Registry::Scene* scene) {
			if (a_args.argCount > 0 && a_args.args[0].GetType() == RE::GFxValue::ValueType::kNumber) {
				auto act = GetActorByReferenceId(a_args, 0);
				if (!act) {
					logger::warn("GetOffset: Invalid actor reference ID {}", a_args.args[1].GetUInt());
					return;
				}
				const auto threadAct = threadInstance->GetActors();
				const auto i = std::find(threadAct.begin(), threadAct.end(), act) - threadAct.begin();
				if (static_cast<size_t>(i) >= threadAct.size()) {
					logger::warn("GetOffset: Actor {} is not part of the current scene", act->GetFormID());
					return;
				}
				const auto stage = scene->GetStageByID(activeStage->id);
				assert(stage);
				stage->positions[i].offset.ResetOffset();
			} else {
				scene->furnitureOffset.ResetOffset();
			}
		});
		threadInstance->AdvanceScene(activeStage);
	}

	void SceneMenu::SLAPI_GetOffsetStepSize::Call(Params& a_args)
	{
		a_args.retVal->SetNumber(Settings::fAdjustStepSize);
	}

	void SceneMenu::SLAPI_AdjustOffsetStepSize::Call(Params& a_args)
	{
		assert(a_args.argCount > 0 && a_args.args->GetType() == RE::GFxValue::ValueType::kBoolean);
		bool upward = a_args.args->GetBool();
		if (upward) {
			Settings::fAdjustStepSize += Settings::fAdjustStepSizeIncrement;
		} else {
			Settings::fAdjustStepSize -= Settings::fAdjustStepSizeIncrement;
		}
		a_args.retVal->SetNumber(Settings::fAdjustStepSize);
	}

	void SceneMenu::SLAPI_GetAdjustStageOnly::Call(Params& a_args)
	{
		a_args.retVal->SetBoolean(Settings::bAdjustStage);
	}

	void SceneMenu::SLAPI_SetAdjustStageOnly::Call(Params& a_args)
	{
		assert(a_args.argCount > 0 && a_args.args->GetType() == RE::GFxValue::ValueType::kBoolean);
		Settings::bAdjustStage = a_args.args->GetBool();
	}

	void SceneMenu::SLAPI_GetAlternateScenes::Call(Params& a_args)
	{
		a_args.movie->CreateArray(a_args.retVal);
		const auto scenes = threadInstance->GetThreadScenes();
		for (const auto& scene : scenes) {
			RE::GFxValue object;
			a_args.movie->CreateObject(&object);
			object.SetMember("name", { std::string_view{ scene->name } });
			object.SetMember("id", { std::string_view{ scene->id } });
			a_args.retVal->PushBack(object);
		}
	}

	void SceneMenu::SLAPI_ToggleAutoPlay::Call(Params& a_args)
	{
		assert(a_args.argCount == 1 && a_args.args[0].GetType() == RE::GFxValue::ValueType::kBoolean);
		bool enable = a_args.args->GetBool();
		threadInstance->SetAutoplayEnabled(enable);
	}

	void SceneMenu::SLAPI_IsAutoPlay::Call(Params& a_args)
	{
		a_args.retVal->SetBoolean(threadInstance->GetAutoplayEnabled());
	}

	void SceneMenu::SLAPI_SetHideHUD::Call(Params& a_args)
	{
		assert(a_args.argCount == 1 && a_args.args[0].GetType() == RE::GFxValue::ValueType::kBoolean);
		const bool visible = a_args.args[0].GetBool();
		RE::UIMessageQueue::GetSingleton()->AddMessage(
			RE::HUDMenu::MENU_NAME,
			visible ? RE::UI_MESSAGE_TYPE::kShow : RE::UI_MESSAGE_TYPE::kHide,
			nullptr);
	}

	void SceneMenu::SLAPI_GetHideHUD::Call(Params& a_args)
	{
		const bool open = RE::UI::GetSingleton()->IsMenuOpen(RE::HUDMenu::MENU_NAME);
		a_args.retVal->SetBoolean(open);
	}

	void SceneMenu::SLAPI_GetPermutationData::Call(Params& a_args)
	{
		assert(a_args.argCount == 1);
		const auto actor = GetActorByReferenceId(a_args, 0);
		if (!actor) {
			logger::warn("GetPermutationData: Invalid actor reference ID");
			a_args.retVal->SetString("ERROR");
			return;
		}
		const auto totalPermutations = threadInstance->GetUniquePermutations(actor);
		const auto currentPermutation = threadInstance->GetCurrentPermutation(actor);
		const auto value = std::format("{} / {}", currentPermutation, totalPermutations);
		a_args.retVal->SetString(value);
	}

	void SceneMenu::SLAPI_SelectNextPermutation::Call(Params& a_args)
	{
		assert(a_args.argCount == 1);
		const auto actor = GetActorByReferenceId(a_args, 0);
		if (!actor) {
			logger::warn("SelectNextPermutation: Invalid actor reference ID");
			return;
		}
		threadInstance->SetNextPermutation(actor);
		const auto totalPermutations = threadInstance->GetUniquePermutations(actor);
		const auto currentPermutation = threadInstance->GetCurrentPermutation(actor);
		const auto value = std::format("{} / {}", currentPermutation, totalPermutations);
		a_args.retVal->SetString(value);
	}

	void SceneMenu::SLAPI_GetGhostMode::Call(Params& a_args)
	{
		assert(a_args.argCount == 1);
		const auto actor = GetActorByReferenceId(a_args, 0);
		if (!actor) {
			logger::warn("GetGhostMode: Invalid actor reference ID");
			return;
		}
		const bool ghostMode = threadInstance->IsGhostMode(actor);
		a_args.retVal->SetBoolean(ghostMode);
	}

	void SceneMenu::SLAPI_SetGhostMode::Call(Params& a_args)
	{
		assert(a_args.argCount == 2 && a_args.args[1].GetType() == RE::GFxValue::ValueType::kBoolean);
		const auto actor = GetActorByReferenceId(a_args, 0);
		if (!actor) {
			logger::warn("SetGhostMode: Invalid actor reference ID");
			return;
		}
		const bool ghostMode = a_args.args[1].GetBool();
		threadInstance->SetGhostMode(actor, ghostMode);
	}

	void SceneMenu::SLAPI_GetExpressionName::Call(Params& a_args)
	{
		assert(a_args.argCount == 1);
		const auto actor = GetActorByReferenceId(a_args, 0);
		if (!actor) {
			logger::warn("GetExpressionName: Invalid actor reference ID");
			return;
		}
		const auto expression = threadInstance->GetExpression(actor);
		if (!expression) {
			logger::warn("GetExpressionName: No expression found for actor {}", actor->GetDisplayFullName());
			return;
		}
		a_args.retVal->SetString(expression->id.c_str());
	}

	void SceneMenu::SLAPI_SetExpression::Call(Params& a_args)
	{
		assert(a_args.argCount == 2 && a_args.args[1].GetType() == RE::GFxValue::ValueType::kString);
		const auto actor = GetActorByReferenceId(a_args, 0);
		if (!actor) {
			logger::warn("SetExpression: Invalid actor reference ID");
			return;
		}
		const auto expressionId = a_args.args[1].GetString();
		const auto expression = Registry::Library::GetSingleton()->GetExpressionById(expressionId);
		if (!expression) {
			logger::warn("SetExpression: No such expression registered: {}", expressionId);
			return;
		}
		threadInstance->SetExpression(actor, expression);
	}

	void SceneMenu::SLAPI_GetExpressions::Call(Params& a_args)
	{
		assert(a_args.argCount == 1);
		a_args.movie->CreateArray(a_args.retVal);
		const auto actor = GetActorByReferenceId(a_args, 0);
		if (!actor) {
			logger::warn("SetExpression: Invalid actor reference ID");
			a_args.retVal->SetArraySize(0);
		} else if (!Registry::RaceKey(actor).Is(Registry::RaceKey::Value::Human)) {
			logger::info("SetExpression: Actor {} is not a human", actor->GetDisplayFullName());
			a_args.retVal->SetArraySize(0);
		} else {
			const auto lib = Registry::Library::GetSingleton();
			lib->ForEachExpression([&](const auto& expression) {
				if (!expression.enabled) {
					return false;
				}
				RE::GFxValue object;
				a_args.movie->CreateObject(&object);
				object.SetMember("id", { std::string_view{ expression.GetId() } });
				object.SetMember("name", { std::string_view{ expression.GetId() } });
				a_args.retVal->PushBack(object);
				return false;
			});
		}
	}

	void SceneMenu::SLAPI_GetVoiceName::Call(Params& a_args)
	{
		assert(a_args.argCount == 1);
		const auto actor = GetActorByReferenceId(a_args, 0);
		if (!actor) {
			logger::warn("GetVoiceName: Invalid actor reference ID");
			return;
		}
		const auto voice = threadInstance->GetVoice(actor);
		if (!voice) {
			logger::warn("GetVoiceName: No voice found for actor {}", actor->GetDisplayFullName());
			return;
		}
		const auto name = voice->GetDisplayName();
		a_args.retVal->SetString(name);
	}

	void SceneMenu::SLAPI_SetVoice::Call(Params& a_args)
	{
		assert(a_args.argCount == 2 && a_args.args[1].GetType() == RE::GFxValue::ValueType::kString);
		const auto actor = GetActorByReferenceId(a_args, 0);
		if (!actor) {
			logger::warn("SetVoice: Invalid actor reference ID");
			return;
		}
		const auto voiceId = a_args.args[1].GetString();
		const auto voice = Registry::Library::GetSingleton()->GetVoiceById(voiceId);
		if (!voice) {
			logger::warn("SetVoice: No such voice registered: {}", voiceId);
			return;
		}
		threadInstance->SetVoice(actor, voice);
	}

	void SceneMenu::SLAPI_GetVoices::Call(Params& a_args)
	{
		assert(a_args.argCount == 1);
		a_args.movie->CreateArray(a_args.retVal);
		const auto actor = GetActorByReferenceId(a_args, 0);
		if (!actor) {
			logger::warn("SetVoice: Invalid actor reference ID");
			a_args.retVal->SetArraySize(0);
			return;
		}
		const auto actRace = Registry::RaceKey{ actor };
		Registry::Library::GetSingleton()->ForEachVoice([&](const auto& voice) {
			if (!voice.HasRace(actRace)) {
				return false;
			}
			RE::GFxValue object;
			a_args.movie->CreateObject(&object);
			object.SetMember("id", { std::string_view{ voice.GetId() } });
			object.SetMember("name", { std::string_view{ voice.GetDisplayName() } });
			a_args.retVal->PushBack(object);
			return false;
		});
	}
}
