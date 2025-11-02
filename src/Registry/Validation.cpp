#include "Validation.h"

#include "Define/RaceKey.h"
#include "Util/Scale.h"

bool Registry::IsValidActor(RE::Actor* a_actor)
{
	return IsValidActorImpl(a_actor) > 0;
}

int32_t Registry::IsValidActorImpl(RE::Actor* a_actor)
{
	if (!a_actor->Is3DLoaded())
		return -12;
	else if (a_actor->IsDisabled() || !a_actor->IsAIEnabled())
		return -14;

	const auto lifestate = a_actor->AsActorState()->GetLifeState();
	if (!Settings::bAllowDead && (lifestate == RE::ACTOR_LIFE_STATE::kDead || lifestate == RE::ACTOR_LIFE_STATE::kDying))
		return -13;
	else if (a_actor->AsActorState()->IsFlying())
		return -15;
	else if (a_actor->IsOnMount() || a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kVariable05) > 0)
		return -16;

	auto validfaction = 1;
	a_actor->VisitFactions([&validfaction](RE::TESFaction* a_faction, int8_t a_rank) {
		if (!a_faction || a_rank < 0)
			return false;
		if (a_faction == GameForms::AnimatingFaction) {
			validfaction = -10;
			return true;
		}
		if (a_faction == GameForms::ForbiddenFaction) {
			validfaction = -11;
			return true;
		}
		return false;
	});
	if (validfaction != 1)
		return validfaction;


	const RaceKey race{ a_actor };
	if (!Settings::bAllowCreatures && !race.Is(RaceKey::Human)) {
		return -17;
	}
	switch (race) {
	case RaceKey::Human:
		{
			if (a_actor->IsChild())
				return -11;
			if (Scale::GetSingleton()->GetScale(a_actor) < Settings::fMinScale)
				return -11;

			/* below might be interesting to investigate if GetScale() isnt working reliably
			 The function calculates the height difference between left foot and head,
			 but may be unreliable if there are mods changing proportions or if an animation puts head close to foot
			*/
			// const auto foot = a_actor->GetNodeByName("CME L Foot [Lft ]");
			// const auto head = a_actor->GetNodeByName("NPC Head [Head]");
			// if (!foot || !head)
			// 	return true;
			// const auto& footZ = foot->world.translate.z;
			// const auto& headZ = head->world.translate.z;
			// const auto& difference = headZ - footZ;
			// if (difference < 95) {
			// 	return false;
			// }
		}
		return true;
	case RaceKey::AshHopper:
		return Settings::bAshHopper;
	case RaceKey::Bear:
		return Settings::bBear;
	case RaceKey::BoarAny:
		return Settings::bBoar;
	case RaceKey::BoarMounted:
		return Settings::bBoarMounted;
	case RaceKey::BoarSingle:
		return Settings::bBoarSingle;
	case RaceKey::Canine:
		return Settings::bCanine;
	case RaceKey::Chaurus:
		return Settings::bChaurus;
	case RaceKey::ChaurusHunter:
		return Settings::bChaurusHunter;
	case RaceKey::ChaurusReaper:
		return Settings::bChaurusReaper;
	case RaceKey::Chicken:
		return Settings::bChicken;
	case RaceKey::Cow:
		return Settings::bCow;
	case RaceKey::Deer:
		return Settings::bDeer;
	case RaceKey::Dog:
		return Settings::bDog;
	case RaceKey::Dragon:
		return Settings::bDragon;
	case RaceKey::DragonPriest:
		return Settings::bDragonPriest;
	case RaceKey::Draugr:
		return Settings::bDraugr;
	case RaceKey::DwarvenBallista:
		return Settings::bDwarvenBallista;
	case RaceKey::DwarvenCenturion:
		return Settings::bDwarvenCenturion;
	case RaceKey::DwarvenSphere:
		return Settings::bDwarvenSphere;
	case RaceKey::DwarvenSpider:
		return Settings::bDwarvenSpider;
	case RaceKey::Falmer:
		return Settings::bFalmer;
	case RaceKey::FlameAtronach:
		return Settings::bFlameAtronach;
	case RaceKey::Fox:
		return Settings::bFox;
	case RaceKey::FrostAtronach:
		return Settings::bFrostAtronach;
	case RaceKey::Gargoyle:
		return Settings::bGargoyle;
	case RaceKey::Giant:
		return Settings::bGiant;
	case RaceKey::GiantSpider:
		return Settings::bGiantSpider;
	case RaceKey::Goat:
		return Settings::bGoat;
	case RaceKey::Hagraven:
		return Settings::bHagraven;
	case RaceKey::Hare:
		return Settings::bHare;
	case RaceKey::Horker:
		return Settings::bHorker;
	case RaceKey::Horse:
		return Settings::bHorse;
	case RaceKey::IceWraith:
		return Settings::bIceWraith;
	case RaceKey::LargeSpider:
		return Settings::bLargeSpider;
	case RaceKey::Lurker:
		return Settings::bLurker;
	case RaceKey::Mammoth:
		return Settings::bMammoth;
	case RaceKey::Mudcrab:
		return Settings::bMudcrab;
	case RaceKey::Netch:
		return Settings::bNetch;
	case RaceKey::Riekling:
		return Settings::bRiekling;
	case RaceKey::Sabrecat:
		return Settings::bSabrecat;
	case RaceKey::Seeker:
		return Settings::bSeeker;
	case RaceKey::Skeever:
		return Settings::bSkeever;
	case RaceKey::Slaughterfish:
		return Settings::bSlaughterfish;
	case RaceKey::Spider:
		return Settings::bSpider;
	case RaceKey::Spriggan:
		return Settings::bSpriggan;
	case RaceKey::StormAtronach:
		return Settings::bStormAtronach;
	case RaceKey::Troll:
		return Settings::bTroll;
	case RaceKey::VampireLord:
		return Settings::bVampireLord;
	case RaceKey::Werewolf:
		return Settings::bWerewolf;
	case RaceKey::Wisp:
		return Settings::bWisp;
	case RaceKey::Wispmother:
		return Settings::bWispmother;
	case RaceKey::Wolf:
		return Settings::bWolf;
	default:
		return -18;
	}
}
