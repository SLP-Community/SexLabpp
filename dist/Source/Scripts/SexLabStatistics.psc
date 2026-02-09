ScriptName SexLabStatistics Hidden
{
  Global Script for Statistics Access
}

; Returns an array of Actors containing of all currently tracked actors
Actor[] Function GetAllTrackedActors() native global
; Same as above, but the list is trimmed of any non-unique actors and lexicographically sorted on their display name
Actor[] Function GetAllTrackedUniqueActorsSorted() native global

int Property LastUpdate_GameTime  = 0   AutoReadOnly
int Property SecondsInScene       = 1   AutoReadOnly
int Property XP_Vaginal           = 2   AutoReadOnly
int Property XP_Anal              = 3   AutoReadOnly
int Property XP_Oral              = 4   AutoReadOnly
int Property PartnersMale         = 5   AutoReadOnly
int Property PartnersFemale       = 6   AutoReadOnly
int Property PartnersFuta         = 7   AutoReadOnly
int Property PartnersCreature     = 8   AutoReadOnly
int Property TimesOral            = 9   AutoReadOnly
int Property TimesVaginal         = 10  AutoReadOnly
int Property TimesAnal            = 11  AutoReadOnly
int Property TimesMasturbated     = 12  AutoReadOnly
int Property TimesSubmissive      = 13  AutoReadOnly
int Property TimesDominant        = 14  AutoReadOnly
int Property TimesTotal           = 15  AutoReadOnly
int Property Sexuality            = 16  AutoReadOnly
int Property Arousal              = 17  AutoReadOnly

; Obtain a statistic value, the id must be one of the above
float Function GetStatistic(Actor akActor, int id) native global

int Property Sexuality_Hetero = 0 AutoReadOnly
int Property Sexuality_Homo   = 1 AutoReadOnly
int Property Sexuality_Bi     = 2 AutoReadOnly

; Get the sexuality mapping (see above) for the given actor
int Function GetSexuality(Actor akActor) native global
int Function MapSexuality(float aiSexualityStat) native global

; --- Custom Statistics

; Set some custom statistic for this actor (and this actor only). The stat has to be unique across all types
; If the stat does not already exist, it will be created
Function SetCustomStatFlt(Actor akActor, String asStat, float afValue) native global
Function SetCustomStatStr(Actor akActor, String asStat, String asValue) native global

; Retrieve a custom statistic from this actor, returning 'default' if it does not exist or the type does not match
float Function GetCustomStatFlt(Actor akActor, String asStat, float afDefault = 0.0) native global
String Function GetCustomStatStr(Actor akActor, String asStat, String asDefault = "") native global

; Check if the given actor has a custom statistic assigned to them
bool Function HasCustomStat(Actor akActor, String asStat) native global
; Delete a custom statistic from this actor, freeing up its memory
Function DeleteCustomStat(Actor akActor, String asStat) native global

; --- Encounter Statistics
int Property ENC_Any 			  = 0	AutoReadOnly Hidden
int Property ENC_Victim		  = 1	AutoReadOnly Hidden
int Property ENC_Assault	  = 2	AutoReadOnly Hidden
int Property ENC_Submissive	= 3	AutoReadOnly Hidden
int Property ENC_Dominant	  = 4	AutoReadOnly Hidden

; Return a list of all encounters with this actor, sorted and beginning with the least recent one
Actor[] Function GetAllEncounters(Actor akActor) native global
; Return an array of all actors that this actor assaulted
Actor[] Function GetAllEncounteredVictims(Actor akActor) native global
Actor[] Function GetAllEncounteredAssailants(Actor akActor) native global
; Return the most recent encounter of the given type
Actor Function GetMostRecentEncounter(Actor akActor, int aiEncounterType) native global
; Retrieve the time stamp at which these two actors met, in GameDaysPassed
float Function GetLastEncounterTime(Actor akActor, Actor akPartner) native global
; How often the two actors have met
int Function GetTimesMet(Actor akActor, Actor akPartner) native global
int Function GetTimesVictimzed(Actor akActor, Actor akAssailant) native global
int Function GetTimesAssaulted(Actor akActor, Actor akVictim) native global
int Function GetTimesSubmissive(Actor akActor, Actor akPartner) native global
int Function GetTimesDominant(Actor akActor, Actor akPartner) native global

; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;
; ----------------------------------------------------------------------------- ;
;        ██╗███╗   ██╗████████╗███████╗██████╗ ███╗   ██╗ █████╗ ██╗            ;
;        ██║████╗  ██║╚══██╔══╝██╔════╝██╔══██╗████╗  ██║██╔══██╗██║            ;
;        ██║██╔██╗ ██║   ██║   █████╗  ██████╔╝██╔██╗ ██║███████║██║            ;
;        ██║██║╚██╗██║   ██║   ██╔══╝  ██╔══██╗██║╚██╗██║██╔══██║██║            ;
;        ██║██║ ╚████║   ██║   ███████╗██║  ██║██║ ╚████║██║  ██║███████╗       ;
;        ╚═╝╚═╝  ╚═══╝   ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚══════╝       ;
; ----------------------------------------------------------------------------- ;
; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;

String[] Function GetAllCustomStatIDs(Actor akActor) native global
Function SetStatistic(Actor akActor, int id, float afValue) native global
Function SetSexuality(Actor akActor, int mapping) native global
Function AddEncounter(Actor akActor, Actor akPartner, int aiEncounterType) native global
Function ResetStatistics(Actor akActor) native global
