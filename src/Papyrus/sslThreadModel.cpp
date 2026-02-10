#include "sslThreadModel.h"

#include "Registry/Library.h"
#include "Registry/Stats.h"
#include "Registry/Util/RayCast.h"
#include "Registry/Util/RayCast/ObjectBound.h"
#include "Registry/Util/Scale.h"
#include "Thread/Collision/CollisionHandler.h"
#include "Thread/NiNode/Node.h"
#include "Thread/Thread.h"
#include "UserData/StripData.h"
#include "Util/Script.h"
#include "Util/StringUtil.h"

using Offset = Registry::CoordinateType;

namespace Papyrus::ThreadModel
{
#define GET_INSTANCE(ret)                                         \
	auto instance = Thread::Instance::GetInstance(a_qst);         \
	if (!instance) {                                              \
		a_vm->TraceStack("Thread instance not found", a_stackID); \
		return ret;                                               \
	}

	namespace ActorAlias
	{
#define GET_POSITION(ret)                                                                     \
	const auto actor = a_alias->GetActorReference();                                          \
	if (!actor) {                                                                             \
		a_vm->TraceStack("ReferenceAlias must be filled with an actor reference", a_stackID); \
		return ret;                                                                           \
	}                                                                                         \
	const auto a_qst = a_alias->owningQuest;                                                  \
	GET_INSTANCE(ret);                                                                        \
	auto position = instance->GetPosition(actor);                                             \
	if (!position) {                                                                          \
		a_vm->TraceStack("Position not found", a_stackID);                                    \
		return ret;                                                                           \
	}

		RE::BSFixedString GetActorVoice(ALIASARGS)
		{
			GET_POSITION(RE::BSFixedString{});
			const auto& voice = position->voice;
			return voice ? voice->GetId() : RE::BSFixedString{};
		}

		RE::BSFixedString GetActorExpression(ALIASARGS)
		{
			GET_POSITION(RE::BSFixedString{});
			const auto& expression = position->expression;
			return expression ? expression->GetId() : RE::BSFixedString{};
		}

		void SetActorVoiceImpl(ALIASARGS, RE::BSFixedString a_voice)
		{
			GET_POSITION();
			position->voice = Registry::Library::GetSingleton()->GetVoiceById(a_voice);
		}

		void SetActorExpressionImpl(ALIASARGS, RE::BSFixedString a_expression)
		{
			GET_POSITION();
			position->expression = Registry::Library::GetSingleton()->GetExpressionById(a_expression);
		}

		void LockActorImpl(ALIASARGS)
		{
			const auto actor = a_alias->GetActorReference();
			if (!actor) {
				a_vm->TraceStack("Reference is empty or not an actor", a_stackID);
				return;
			}
			if (actor->IsPlayerRef()) {
				RE::PlayerCharacter::GetSingleton()->SetAIDriven(true);
				const auto ui = RE::UI::GetSingleton();
				const auto interfacestr = RE::InterfaceStrings::GetSingleton();
				if (ui->IsMenuOpen(interfacestr->dialogueMenu)) {
					if (auto view = ui->GetMovieView(interfacestr->dialogueMenu)) {
						RE::GFxValue arg{ interfacestr->dialogueMenu };
						view->InvokeNoReturn("_global.skse.CloseMenu", &arg, 1);
					}
				}
				actor->AsActorState()->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kAlive;
			} else {
				switch (actor->AsActorState()->actorState1.lifeState) {
				case RE::ACTOR_LIFE_STATE::kUnconcious:
					actor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable05, STATUS05::Unconscious);
					break;
				case RE::ACTOR_LIFE_STATE::kDying:
				case RE::ACTOR_LIFE_STATE::kDead:
					actor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable05, STATUS05::Dying);
					actor->Resurrect(false, true);
					break;
				}
				actor->AsActorState()->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kRestrained;
			}

			if (actor->AsActorState()->IsWeaponDrawn()) {
				const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
				if (const auto script = factory ? factory->Create() : nullptr) {
					script->SetCommand("rae weaponsheathe");
					script->CompileAndRun(actor);
					if (!actor->IsPlayerRef() && actor->IsSneaking()) {
						script->SetCommand("setforcesneak 0");
						script->CompileAndRun(actor);
					}
					delete script;
				}
			}

			Thread::Collision::CollisionHandler::GetSingleton()->AddActor(actor->GetFormID());

			actor->StopCombat();
			actor->EndDialogue();
			actor->InterruptCast(false);
			actor->StopInteractingQuick(true);
			// actor->SetCollision(false);

			if (const auto process = actor->GetActorRuntimeData().currentProcess) {
				process->ClearMuzzleFlashes();
			}
			actor->StopMoving(1.0f);
		}

		void UnlockActorImpl(ALIASARGS)
		{
			const auto actor = a_alias->GetActorReference();
			if (!actor) {
				a_vm->TraceStack("Reference is empty or not an actor", a_stackID);
				return;
			}
			Registry::Scale::GetSingleton()->RemoveScale(actor);
			switch (static_cast<int32_t>(actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kVariable05))) {
			case STATUS05::Unconscious:
				actor->AsActorState()->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kUnconcious;
				break;
			default:
				actor->AsActorState()->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kAlive;
				break;
			}
			if (actor->IsPlayerRef()) {
				RE::PlayerCharacter::GetSingleton()->SetAIDriven(false);
			} else {
				actor->AsActorValueOwner()->SetActorValue(RE::ActorValue::kVariable05, 0.0f);
			}
			Thread::Collision::CollisionHandler::GetSingleton()->RemoveActor(actor->GetFormID());
			// actor->SetCollision(true);
		}

		std::vector<RE::TESForm*> StripByData(ALIASARGS, int32_t a_stripdata, std::vector<uint32_t> a_defaults, std::vector<uint32_t> a_overwrite)
		{
			return StripByDataEx(a_vm, a_stackID, a_alias, a_stripdata, a_defaults, a_overwrite, {});
		}

		std::vector<RE::TESForm*> StripByDataEx(ALIASARGS,
		  int32_t a_stripdata,
		  std::vector<uint32_t> a_defaults,		  // use if a_stripData == default
		  std::vector<uint32_t> a_overwrite,	  // use if exists
		  std::vector<RE::TESForm*> a_mergewith)  // [HighHeelSpell, WeaponRight, WeaponLeft, Armor...]
		{
			using Strip = Registry::Position::StripData;
			using SlotMask = RE::BIPED_MODEL::BipedObjectSlot;

			enum MergeIDX
			{
				Spell = 0,
				Right = 1,
				Left = 2,
			};

			if (!a_alias) {
				a_vm->TraceStack("Cannot call StripByDataEx on a none alias", a_stackID);
				return a_mergewith;
			}
			const auto actor = a_alias->GetActorReference();
			if (!actor) {
				a_vm->TraceStack("ReferenceAlias must be filled with an actor reference", a_stackID);
				return a_mergewith;
			}
			if (!actor->IsHumanoid()) {
				return a_mergewith;
			}
			if (a_mergewith.size() < 3) {
				a_mergewith.resize(3, nullptr);
			}
			REX::EnumSet<Strip> stripnum(static_cast<Strip>(a_stripdata));
			if (stripnum == Strip::None) {
				logger::info("Using stripping policy: None");
				return a_mergewith;
			}
			uint32_t slots;
			bool weapon;
			if (a_overwrite.size() >= 2) {
				slots = a_overwrite[0];
				weapon = a_overwrite[1];
			} else if (stripnum.all(Strip::All)) {
				slots = static_cast<uint32_t>(-1);
				weapon = true;
			} else {
				if (stripnum.all(Strip::Default) && a_defaults.size() >= 2) {
					slots = a_defaults[0];
					weapon = a_defaults[1];
				} else {
					slots = 0;
					weapon = 0;
				}
				if (stripnum.all(Strip::Boots)) {
					slots |= static_cast<uint32_t>(SlotMask::kFeet);
				}
				if (stripnum.all(Strip::Gloves)) {
					slots |= static_cast<uint32_t>(SlotMask::kHands);
				}
				if (stripnum.all(Strip::Helmet)) {
					slots |= static_cast<uint32_t>(SlotMask::kHead);
				}
			}
			const auto stripconfig = UserData::StripData::GetSingleton();
			const auto manager = RE::ActorEquipManager::GetSingleton();
			for (const auto& [form, data] : actor->GetInventory()) {
				if (!data.second->IsWorn()) {
					continue;
				}
				switch (stripconfig->CheckStrip(form)) {
				case UserData::Strip::NoStrip:
					continue;
				case UserData::Strip::Always:
					break;
				case UserData::Strip::None:
					if (form->IsWeapon() && !weapon) {
						continue;
					} else if (const auto biped = form->As<RE::BGSBipedObjectForm>()) {
						const auto biped_slots = static_cast<uint32_t>(biped->GetSlotMask());
						if ((biped_slots & slots) == 0) {
							continue;
						}
					}
					break;
				}
				if (form->IsWeapon() && actor->GetActorRuntimeData().currentProcess) {
					if (actor->GetActorRuntimeData().currentProcess->GetEquippedRightHand() == form)
						a_mergewith[Right] = form;
					else
						a_mergewith[Left] = form;
				} else {
					a_mergewith.push_back(form);
				}
				manager->UnequipObject(actor, form);
			}
			std::vector<RE::FormID> ids{};
			ids.reserve(a_mergewith.size());
			for (auto&& it : a_mergewith)
				ids.push_back(it ? it->formID : 0);
			logger::info("Stripping, Policy: [{:X}, {}], Stripped Equipment: [{}]", weapon, slots, [&] {
				if (ids.empty()) {
					return std::string("");
				}
				return std::accumulate(std::next(ids.begin()), ids.end(), std::format("{:X}", ids[0]), [](std::string a, auto b) {
					return std::move(a) + ", " + std::format("{:X}", b);
				});
			}());
			actor->Update3DModel();
			return a_mergewith;
		}

		void UpdateEnjoyment(ALIASARGS, float a_enjoyment)
		{
			const auto& a_qst = a_alias->owningQuest;
			GET_INSTANCE();
			instance->SetEnjoyment(a_alias->GetActorReference(), a_enjoyment);
		}

#undef GET_POSITION
	}  // namespace ActorAlias

	RE::BSFixedString GetActiveScene(QUESTARGS)
	{
		GET_INSTANCE("");
		if (const auto& scene = instance->GetActiveScene()) {
			return scene->id;
		}
		a_vm->TraceStack("No active scene", a_stackID);
		return RE::BSFixedString{};
	}

	RE::BSFixedString GetActiveStage(QUESTARGS)
	{
		GET_INSTANCE("");
		if (const auto& stage = instance->GetActiveStage()) {
			return stage->id;
		}
		a_vm->TraceStack("No active stage", a_stackID);
		return RE::BSFixedString{};
	}

	std::vector<RE::BSFixedString> GetPlayingScenes(QUESTARGS)
	{
		GET_INSTANCE({});
		return std::ranges::fold_left(instance->GetThreadScenes(), std::vector<RE::BSFixedString>{}, [](auto&& acc, const auto& it) {
			return (acc.push_back(it->id), acc);
		});
	}

	std::vector<RE::Actor*> GetPositions(QUESTARGS)
	{
		GET_INSTANCE({});
		return instance->GetActors();
	}

	std::vector<RE::BSFixedString> AddContextExImpl(RE::TESQuest*, std::vector<RE::BSFixedString> a_oldcontext, std::string a_newcontext)
	{
		const auto list = Util::StringSplit(a_newcontext, ",");
		for (auto&& tag : list) {
			if (!tag.starts_with('!'))
				continue;
			const auto context = RE::BSFixedString(std::string(tag.substr(1)));
			if (std::find(a_oldcontext.begin(), a_oldcontext.end(), context) != a_oldcontext.end())
				continue;
			a_oldcontext.push_back(context);
		}
		return a_oldcontext;
	}

	void CreateInstance(QUESTARGS,
	  std::vector<RE::Actor*> a_submissives,
	  std::vector<RE::BSFixedString> a_scenesPrimary,
	  std::vector<RE::BSFixedString> a_scenesLeadIn,
	  std::vector<RE::BSFixedString> a_scenesCustom,
	  int a_furniturepref)
	{
		const auto library = Registry::Library::GetSingleton();
		const auto toVector = [&](const auto& a_list) {
			return std::ranges::fold_left(a_list, std::vector<const Registry::Scene*>{}, [&](auto&& acc, const auto& it) {
				const auto scene = library->GetSceneById(it);
				if (!scene) {
					const auto err = std::format("Invalid scene id {}", it);
					a_vm->TraceStack(err.c_str(), a_stackID);
					return acc;
				}
				return (acc.push_back(scene), acc);
			});
		};
		Thread::Instance::FurniturePreference preference{ a_furniturepref };
		Thread::Instance::SceneMapping scenes{
			toVector(a_scenesPrimary),
			toVector(a_scenesLeadIn),
			toVector(a_scenesCustom)
		};
		std::thread([=]() {
			bool result = Thread::Instance::CreateInstance(a_qst, a_submissives, scenes, preference);
			auto handle = Script::GetScriptObject(a_qst, "sslThreadModel");
			Script::CallbackPtr callbackPtr{};
			Script::DispatchMethodCall(handle, "ContinueSetup", callbackPtr, std::move(result));
		}).detach();
	}

	void DestroyInstance(RE::TESQuest* a_qst)
	{
		Thread::Instance::DestroyInstance(a_qst);
	}

	std::vector<RE::BSFixedString> GetLeadInScenes(QUESTARGS)
	{
		GET_INSTANCE({});
		const auto sceneList = instance->GetThreadScenes(Thread::Instance::SceneType::LeadIn);
		return std::ranges::fold_left(sceneList, std::vector<RE::BSFixedString>{}, [](auto&& acc, const auto& it) {
			return (acc.push_back(it->id), acc);
		});
	}

	std::vector<RE::BSFixedString> GetPrimaryScenes(QUESTARGS)
	{
		GET_INSTANCE({});
		const auto sceneList = instance->GetThreadScenes(Thread::Instance::SceneType::Primary);
		return std::ranges::fold_left(sceneList, std::vector<RE::BSFixedString>{}, [](auto&& acc, const auto& it) {
			return (acc.push_back(it->id), acc);
		});
	}

	std::vector<RE::BSFixedString> GetCustomScenes(QUESTARGS)
	{
		GET_INSTANCE({});
		const auto sceneList = instance->GetThreadScenes(Thread::Instance::SceneType::Custom);
		return std::ranges::fold_left(sceneList, std::vector<RE::BSFixedString>{}, [](auto&& acc, const auto& it) {
			return (acc.push_back(it->id), acc);
		});
	}

	std::vector<RE::BSFixedString> AdvanceScene(QUESTARGS, std::vector<RE::BSFixedString> a_history, RE::BSFixedString a_nextStage)
	{
		GET_INSTANCE(a_history);
		auto stage = instance->GetActiveScene()->GetStageByID(a_nextStage);
		if (!stage) {
			a_vm->TraceStack("Invalid stage id", a_stackID);
			return a_history;
		}
		instance->AdvanceScene(stage);
		a_history.push_back(a_nextStage);
		return a_history;
	}

	int SelectNextStage(QUESTARGS, std::vector<RE::BSFixedString> a_tags)
	{
		GET_INSTANCE(0);
		const auto& scene = instance->GetActiveScene();
		const auto& stage = instance->GetActiveStage();
		if (!scene || !stage) {
			a_vm->TraceStack("No active scene or stage", a_stackID);
			return 0;
		}
		const auto& adj = scene->GetAdjacentStages(stage);
		if (!adj || adj->empty()) return 0;
		Registry::TagData tags{ a_tags };
		std::vector<int> weights{};
		int n = 0;
		for (auto&& i : *adj) {
			auto c = i->tags.CountTags(tags);
			weights.resize(weights.size() + c + 1, n++);
		}
		return Random::draw(weights);
	}

	bool SetActiveScene(QUESTARGS, RE::BSFixedString a_sceneid)
	{
		GET_INSTANCE(false);
		const auto scene = Registry::Library::GetSingleton()->GetSceneById(a_sceneid);
		if (!scene) {
			a_vm->TraceStack("Invalid scene id", a_stackID);
			return false;
		}
		return instance->SetActiveScene(scene);
	}

	bool ReassignCenter(QUESTARGS, RE::TESObjectREFR* a_centeron)
	{
		GET_INSTANCE(false);
		if (!a_centeron) {
			a_vm->TraceStack("Cannot reassign a none reference center", a_stackID);
			return false;
		}
		return instance->ReplaceCenterRef(a_centeron);
	}

	void UpdatePlacement(QUESTARGS, RE::Actor* a_position)
	{
		GET_INSTANCE();
		instance->UpdatePlacement(a_position);
	}

	bool GetIsCompatiblecenter(QUESTARGS, RE::BSFixedString a_sceneid, RE::TESObjectREFR* a_center)
	{
		if (!a_center) {
			a_vm->TraceStack("Cannot validate a none reference center", a_stackID);
			return false;
		}
		const auto scene = Registry::Library::GetSingleton()->GetSceneById(a_sceneid);
		if (!scene) {
			a_vm->TraceStack("Invalid scene id", a_stackID);
			return false;
		}
		return scene->IsCompatibleFurniture(a_center);
	}

	bool IsCollisionRegistered(QUESTARGS)
	{
		GET_INSTANCE(false);
		return instance->HasNiInstance();
	}

	void UnregisterCollision(QUESTARGS)
	{
		GET_INSTANCE();
		instance->UnregisterNiInstance();
	}

	std::vector<int> GetCollisionActions(QUESTARGS, RE::Actor* a_position, RE::Actor* a_partner)
	{
		GET_INSTANCE({});
		auto niInstance = instance->GetNiInstance();
		if (!niInstance) {
			a_vm->TraceStack("Not registered", a_stackID);
			return {};
		}
		const auto idxA = a_position ? a_position->formID : 0;
		const auto idxB = a_partner ? a_partner->formID : 0;
		const auto interactions = niInstance->GetInteractions(idxA, idxB, Thread::NiNode::NiType::Type::None);
		const auto ret = std::ranges::fold_left(interactions, std::vector<int>{}, [](auto&& acc, const auto& it) {
			return (acc.push_back(static_cast<int>(it->GetType())), acc);
		});
		return ret;
	}

	bool HasCollisionAction(QUESTARGS, int a_type, RE::Actor* a_position, RE::Actor* a_partner)
	{
		GET_INSTANCE({});
		auto niInstance = instance->GetNiInstance();
		if (!niInstance) {
			a_vm->TraceStack("Not registered", a_stackID);
			return false;
		}
		const auto idxA = a_position ? a_position->formID : 0;
		const auto idxB = a_partner ? a_partner->formID : 0;
		const auto interactions = niInstance->GetInteractions(idxA, idxB, Thread::NiNode::NiType::Type(a_type));
		return !interactions.empty();
	}

	RE::Actor* GetPartnerByAction(QUESTARGS, RE::Actor* a_position, int a_type)
	{
		if (!a_position) {
			a_vm->TraceStack("Actor is none", a_stackID);
			return nullptr;
		}
		const auto ret = GetPartnersByAction(a_vm, a_stackID, a_qst, a_position, a_type);
		return ret.empty() ? nullptr : ret.front();
	}

	std::vector<RE::Actor*> GetPartnersByAction(QUESTARGS, RE::Actor* a_position, int a_type)
	{
		GET_INSTANCE({});
		auto niInstance = instance->GetNiInstance();
		if (!niInstance) {
			a_vm->TraceStack("Not registered", a_stackID);
			return {};
		}
		const auto idxA = a_position ? a_position->formID : 0;
		return niInstance->GetInteractionPartners(idxA, Thread::NiNode::NiType::Type(a_type));
	}

	RE::Actor* GetPartnerByTypeRev(QUESTARGS, RE::Actor* a_position, int a_type)
	{
		if (!a_position) {
			a_vm->TraceStack("Actor is none", a_stackID);
			return nullptr;
		}
		const auto ret = GetPartnersByTypeRev(a_vm, a_stackID, a_qst, a_position, a_type);
		return ret.empty() ? nullptr : ret.front();
	}

	std::vector<RE::Actor*> GetPartnersByTypeRev(QUESTARGS, RE::Actor* a_position, int a_type)
	{
		GET_INSTANCE({});
		auto niInstance = instance->GetNiInstance();
		if (!niInstance) {
			a_vm->TraceStack("Not registered", a_stackID);
			return {};
		}
		const auto idxB = a_position ? a_position->formID : 0;
		return niInstance->GetInteractionPartnersRev(idxB, Thread::NiNode::NiType::Type(a_type));
	}

	float GetActionVelocity(QUESTARGS, RE::Actor* a_position, RE::Actor* a_partner, int a_type)
	{
		if (!a_position) {
			a_vm->TraceStack("Actor is none", a_stackID);
			return 0.0f;
		}
		if (a_type == 0) {
			a_vm->TraceStack("Type cant be 'any'", a_stackID);
			return 0.0f;
		}
		GET_INSTANCE({});
		auto niInstance = instance->GetNiInstance();
		if (!niInstance) {
			a_vm->TraceStack("Not registered", a_stackID);
			return 0.0f;
		}
		float ret = 0.0f;
		const auto idxA = a_position->formID;
		const auto idxB = a_partner ? a_partner->formID : 0;
		const auto interactions = niInstance->GetInteractions(idxA, idxB, Thread::NiNode::NiType::Type(a_type));
		if (!interactions.empty()) {
			ret = interactions.front()->velocity;
		} else {
			a_vm->TraceStack("No such interaction found", a_stackID);
		}
		return ret;
	}

	void AddExperience(QUESTARGS, std::vector<RE::Actor*> a_positions, RE::BSFixedString a_scene, std::vector<RE::BSFixedString> a_playedstages)
	{
		const auto scene = Registry::Library::GetSingleton()->GetSceneById(a_scene);
		if (!scene) {
			a_vm->TraceStack("Invalid scene id", a_stackID);
			return;
		} else if (scene->CountPositions() != a_positions.size()) {
			a_vm->TraceStack("Position cound does not match scene position count", a_stackID);
			return;
		}
		int vaginal = 0, anal = 0, oral = 0;
		for (auto&& it : a_playedstages) {
			auto stage = scene->GetStageByID(it);
			if (!stage)
				continue;

			vaginal += stage->tags.HasTag(Registry::Tag::Vaginal);
			anal += stage->tags.HasTag(Registry::Tag::Anal);
			oral += stage->tags.HasTag(Registry::Tag::Oral);
		}
		const auto statdata = Registry::Statistics::StatisticsData::GetSingleton();
		for (auto&& p : a_positions) {
			if (!p)
				continue;

			auto& stats = statdata->GetStatistics(p);
			stats.AddStatistic(stats.XP_Vaginal, vaginal * 1.25f);
			stats.AddStatistic(stats.XP_Anal, anal * 1.25f);
			stats.AddStatistic(stats.XP_Oral, oral * 1.25f);
		}
	}

	void UpdateStatistics(QUESTARGS, RE::Actor* a_actor, std::vector<RE::Actor*> a_positions, RE::BSFixedString a_scene, std::vector<RE::BSFixedString> a_playedstages, float a_time)
	{
		if (!a_actor) {
			a_vm->TraceStack("Actor is none", a_stackID);
			return;
		}
		const auto scene = Registry::Library::GetSingleton()->GetSceneById(a_scene);
		if (!scene) {
			a_vm->TraceStack("Invalid scene id", a_stackID);
			return;
		} else if (scene->CountPositions() != a_positions.size()) {
			a_vm->TraceStack("Position cound does not match scene position count", a_stackID);
			return;
		}
		auto& stats = Registry::Statistics::StatisticsData::GetSingleton()->GetStatistics(a_actor);
		stats.SetStatistic(stats.LastUpdate_GameTime, RE::Calendar::GetSingleton()->GetCurrentGameTime());
		stats.AddStatistic(stats.SecondsInScene, a_time);
		stats.AddStatistic(stats.TimesTotal, 1);
		if (scene->CountPositions() == 1) {
			stats.AddStatistic(stats.TimesMasturbated, 1);
			if (scene->CountSubmissives() == 1) {
				stats.AddStatistic(stats.TimesSubmissive, 1);
			}
		} else {
			int sub = 0;
			for (size_t i = 0; i < a_positions.size(); i++) {
				if (!a_positions[i])
					continue;
				if (a_positions[i] == a_actor) {
					const auto& p = scene->GetNthPosition(i);
					sub = p->IsSubmissive();
					continue;
				}
				if (sub != 1 && scene->GetNthPosition(i)->IsSubmissive()) {
					sub = -1;
				}
				if (a_positions[i]->IsHumanoid()) {
					switch (Registry::GetSex(a_positions[i])) {
					case Registry::Sex::Male:
						stats.AddStatistic(stats.PartnersMale, 1);
						break;
					case Registry::Sex::Female:
						stats.AddStatistic(stats.PartnersFemale, 1);
						break;
					case Registry::Sex::Futa:
						stats.AddStatistic(stats.PartnersFuta, 1);
						break;
					}
				} else {
					stats.AddStatistic(stats.PartnersCreature, 1);
				}
			}
			switch (sub) {
			case -1:
				stats.AddStatistic(stats.TimesDominant, 1);
				break;
			case 1:
				stats.AddStatistic(stats.TimesSubmissive, 1);
				break;
			}
		}
		int vaginal = 0, anal = 0, oral = 0;
		for (auto&& it : a_playedstages) {
			auto stage = scene->GetStageByID(it);
			if (!stage)
				continue;

			vaginal += stage->tags.HasTag(Registry::Tag::Vaginal);
			anal += stage->tags.HasTag(Registry::Tag::Anal);
			oral += stage->tags.HasTag(Registry::Tag::Oral);
		}
		if (vaginal) {
			stats.AddStatistic(stats.TimesVaginal, 1);
			stats.AddStatistic(stats.XP_Vaginal, vaginal * 1.25f);
		}
		if (anal) {
			stats.AddStatistic(stats.TimesAnal, 1);
			stats.AddStatistic(stats.XP_Anal, anal * 1.25f);
		}
		if (oral) {
			stats.AddStatistic(stats.TimesOral, 1);
			stats.AddStatistic(stats.XP_Oral, oral * 1.25f);
		}
	}

	bool IsOwningSceneMenu(QUESTARGS)
	{
		GET_INSTANCE(false);
		return instance->ControlsMenu();
	}

	bool TryOpenSceneMenu(QUESTARGS)
	{
		GET_INSTANCE(false);
		return instance->TryOpenMenu();
	}

	bool TryCloseSceneMenu(QUESTARGS)
	{
		GET_INSTANCE(false);
		return instance->TryCloseMenu();
	}

	void TryUpdateMenuTimer(QUESTARGS, float a_time)
	{
		GET_INSTANCE();
		instance->UpdateTimer(a_time);
	}

}  // namespace Papyrus::ThreadModel
