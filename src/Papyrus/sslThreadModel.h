#pragma once

#include "Registry/Define/Animation.h"

namespace Papyrus::ThreadModel
{
	enum FurniStatus
	{
		Disallow = 0,
		Allow = 1,
		Prefer = 2,
	};

	namespace ActorAlias
	{
		enum STATUS05
		{
			Unconscious = -5,
			Dying = -10,
		};

		RE::BSFixedString GetActorVoice(ALIASARGS);
		RE::BSFixedString GetActorExpression(ALIASARGS);
		void SetActorVoiceImpl(ALIASARGS, RE::BSFixedString a_voice);
		void SetActorExpressionImpl(ALIASARGS, RE::BSFixedString a_expression);

		void StartSetActorInterrupts(ALIASARGS);
		void EndSetActorInterrupts(ALIASARGS);
		void SetActorCollisions(ALIASARGS, bool a_enable);

		std::vector<RE::TESForm*> StripByData(ALIASARGS, int32_t a_stripdata, std::vector<uint32_t> a_defaults, std::vector<uint32_t> a_overwrite);
		std::vector<RE::TESForm*> StripByDataEx(ALIASARGS, int32_t a_stripdata, std::vector<uint32_t> a_defaults, std::vector<uint32_t> a_overwrite, std::vector<RE::TESForm*> a_mergewith);

		void UpdateEnjoyment(ALIASARGS, float a_enjoyment);

		inline bool Register(VM* a_vm)
		{
			REGISTERFUNC(GetActorVoice, "sslActorAlias", false);
			REGISTERFUNC(GetActorExpression, "sslActorAlias", false);
			REGISTERFUNC(SetActorVoiceImpl, "sslActorAlias", false);
			REGISTERFUNC(SetActorExpressionImpl, "sslActorAlias", false);

			REGISTERFUNC(StartSetActorInterrupts, "sslActorAlias", false);
			REGISTERFUNC(EndSetActorInterrupts, "sslActorAlias", false);
			REGISTERFUNC(SetActorCollisions, "sslActorAlias", false);

			REGISTERFUNC(StripByData, "sslActorAlias", false);
			REGISTERFUNC(StripByDataEx, "sslActorAlias", false);

			REGISTERFUNC(UpdateEnjoyment, "sslActorAlias", false);

			return true;
		}
	}	 // namespace ActorAlias

	RE::BSFixedString GetActiveScene(QUESTARGS);
	RE::BSFixedString GetActiveStage(QUESTARGS);
	std::vector<RE::BSFixedString> GetPlayingScenes(QUESTARGS);
	std::vector<RE::Actor*> GetPositions(QUESTARGS);
	std::vector<RE::BSFixedString> AddContextExImpl(RE::TESQuest*, std::vector<RE::BSFixedString> a_oldcontext, std::string a_newcontext);

	void CreateInstance(QUESTARGS, std::vector<RE::Actor*> a_submissives, std::vector<RE::BSFixedString> a_scenesPrimary, std::vector<RE::BSFixedString> a_scenesLeadIn, std::vector<RE::BSFixedString> a_scenesCustom, int a_furniturepref);
	void DestroyInstance(RE::TESQuest* a_qst);
	std::vector<RE::BSFixedString> GetLeadInScenes(QUESTARGS);
	std::vector<RE::BSFixedString> GetPrimaryScenes(QUESTARGS);
	std::vector<RE::BSFixedString> GetCustomScenes(QUESTARGS);
	std::vector<RE::BSFixedString> AdvanceScene(QUESTARGS, std::vector<RE::BSFixedString> a_history, RE::BSFixedString a_nextStage);
	int SelectNextStage(QUESTARGS, std::vector<RE::BSFixedString> a_tags);
	bool SetActiveScene(QUESTARGS, RE::BSFixedString a_sceneid);
	bool ReassignCenter(QUESTARGS, RE::TESObjectREFR* a_centeron);
	bool SetNextPermutation(QUESTARGS, RE::Actor* a_position);
	void UpdatePlacement(QUESTARGS, RE::Actor* a_position);

	bool IsCollisionRegistered(QUESTARGS);
	void UnregisterCollision(QUESTARGS);
	std::vector<int> GetCollisionActions(QUESTARGS, RE::Actor* a_position, RE::Actor* a_partner);
	bool HasCollisionAction(QUESTARGS, int a_type, RE::Actor* a_position, RE::Actor* a_partner);
	RE::Actor* GetPartnerByAction(QUESTARGS, RE::Actor* a_position, int a_type);
	std::vector<RE::Actor*> GetPartnersByAction(QUESTARGS, RE::Actor* a_position, int a_type);
	RE::Actor* GetPartnerByTypeRev(QUESTARGS, RE::Actor* a_position, int a_type);
	std::vector<RE::Actor*> GetPartnersByTypeRev(QUESTARGS, RE::Actor* a_position, int a_type);
	float GetActionVelocity(QUESTARGS, RE::Actor* a_position, RE::Actor* a_partner, int a_type);

	void SetAnimationPlaybackSpeed(QUESTARGS, float a_playbackSpeed);

	void AddExperience(QUESTARGS, std::vector<RE::Actor*> a_positions, RE::BSFixedString a_scene, std::vector<RE::BSFixedString> a_playedstages);
	void UpdateStatistics(QUESTARGS, RE::Actor* a_actor, std::vector<RE::Actor*> a_positions, RE::BSFixedString a_scene, std::vector<RE::BSFixedString> a_playedstages, float a_time);

	bool IsOwningSceneMenu(QUESTARGS);
	bool TryOpenSceneMenu(QUESTARGS);
	bool TryCloseSceneMenu(QUESTARGS);
	void TryUpdateMenuTimer(QUESTARGS, float a_time);

	inline bool Register(VM* a_vm)
	{
		REGISTERFUNC(GetActiveScene, "sslThreadModel", true);
		REGISTERFUNC(GetActiveStage, "sslThreadModel", true);
		REGISTERFUNC(GetPlayingScenes, "sslThreadModel", true);
		REGISTERFUNC(GetPositions, "sslThreadModel", true);
		REGISTERFUNC(AddContextExImpl, "sslThreadModel", true);

		REGISTERFUNC(CreateInstance, "sslThreadModel", true);
		REGISTERFUNC(DestroyInstance, "sslThreadModel", true);
		REGISTERFUNC(GetLeadInScenes, "sslThreadModel", true);
		REGISTERFUNC(GetPrimaryScenes, "sslThreadModel", true);
		REGISTERFUNC(GetCustomScenes, "sslThreadModel", true);
		REGISTERFUNC(AdvanceScene, "sslThreadModel", false);
		REGISTERFUNC(SelectNextStage, "sslThreadModel", true);
		REGISTERFUNC(SetActiveScene, "sslThreadModel", false);
		REGISTERFUNC(ReassignCenter, "sslThreadModel", false);
		REGISTERFUNC(UpdatePlacement, "sslThreadModel", false);
		REGISTERFUNC(SetNextPermutation, "sslThreadModel", false);

		REGISTERFUNC(IsCollisionRegistered, "sslThreadModel", true);
		REGISTERFUNC(UnregisterCollision, "sslThreadModel", true);
		REGISTERFUNC(GetCollisionActions, "sslThreadModel", true);
		REGISTERFUNC(HasCollisionAction, "sslThreadModel", true);
		REGISTERFUNC(GetPartnerByAction, "sslThreadModel", true);
		REGISTERFUNC(GetPartnersByAction, "sslThreadModel", true);
		REGISTERFUNC(GetPartnerByTypeRev, "sslThreadModel", true);
		REGISTERFUNC(GetPartnersByTypeRev, "sslThreadModel", true);
		REGISTERFUNC(GetActionVelocity, "sslThreadModel", true);

		REGISTERFUNC(SetAnimationPlaybackSpeed, "sslThreadModel", false);

		REGISTERFUNC(AddExperience, "sslThreadModel", true);
		REGISTERFUNC(UpdateStatistics, "sslThreadModel", true);

		REGISTERFUNC(IsOwningSceneMenu, "sslThreadModel", true);
		REGISTERFUNC(TryOpenSceneMenu, "sslThreadModel", true);
		REGISTERFUNC(TryCloseSceneMenu, "sslThreadModel", true);
		REGISTERFUNC(TryUpdateMenuTimer, "sslThreadModel", true);

		return ActorAlias::Register(a_vm);
	}

}	 // namespace Papyrus::ThreadModel
