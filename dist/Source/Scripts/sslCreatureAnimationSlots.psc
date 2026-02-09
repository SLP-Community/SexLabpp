ScriptName sslCreatureAnimationSlots extends sslAnimationSlots
{
  Legacy Animation Registry Creature Script
  Use SexLabRegistry.psc instead
}

; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;
; ----------------------------------------------------------------------------- ;
;               ██╗     ███████╗ ██████╗  █████╗  ██████╗██╗   ██╗              ;
;               ██║     ██╔════╝██╔════╝ ██╔══██╗██╔════╝╚██╗ ██╔╝              ;
;               ██║     █████╗  ██║  ███╗███████║██║      ╚████╔╝               ;
;               ██║     ██╔══╝  ██║   ██║██╔══██║██║       ╚██╔╝                ;
;               ███████╗███████╗╚██████╔╝██║  ██║╚██████╗   ██║                 ;
;               ╚══════╝╚══════╝ ╚═════╝ ╚═╝  ╚═╝ ╚═════╝   ╚═╝                 ;
; ----------------------------------------------------------------------------- ;
; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;

; ------------------------------------------------------- ;
; --- Creature aniamtion support                      --- ;
; ------------------------------------------------------- ;

string function GetRaceKey(Race RaceRef) global
  String ret = SexLabRegistry.GetRaceKeyByRace(RaceRef)
  If (ret == "humans")
    return ""
  EndIf
  return ret
EndFunction

Race Function GetRaceByEditorID(String asEditorID) global native
string function GetRaceKeyByID(string RaceID) global
  Race r = GetRaceByEditorID(RaceID)
  If (!r)
    return ""
  EndIf
  return GetRaceKey(r)
EndFunction
bool function HasRaceIDType(string RaceID) global
  return GetRaceKeyByID(RaceID) != ""
EndFunction

function AddRaceID(string RaceKey, string RaceID) global
EndFunction

bool function HasRaceID(string RaceKey, string RaceID) global
  return SexLabRegistry.MapRaceKeyToID(RaceKey) > 0
EndFunction
bool function HasRaceKey(string RaceKey) global
  return GetAllRaceKeys().Find(RaceKey) > 0
EndFunction
bool function ClearRaceKey(string RaceKey) global
  return true
EndFunction
bool function HasCreatureType(Actor ActorRef) global
  return SexLabRegistry.GetRaceID(ActorRef) > 0
EndFunction
bool function HasRaceType(Race RaceRef) global
  String racekey = SexLabRegistry.GetRaceKeyByRace(RaceRef)
  return SexLabRegistry.MapRaceKeyToID(racekey) > 0
EndFunction

string[] function GetAllRaceKeys(Race RaceRef = none) global
  String[] ret
  If (!RaceRef)
    ret = SexLabRegistry.GetAllRaceKeys(false)
  Else
    ret = SexLabRegistry.GetRaceKeyByRaceA(RaceRef)
  EndIf
  return PapyrusUtil.RemoveString(ret, "humans")
EndFunction

string[] function GetAllRaceIDs(string RaceKey) global native
Race[] function GetAllRaces(string RaceKey) global native

; ------------------------------------------------------- ;
; --- Lookup creature aniamtions                      --- ;
; ------------------------------------------------------- ;

String[] Function GetByRaceKeyTagsImpl(int aiActorCount, String asRaceKey, String[] asTags) native
String[] Function GetByCreatureActorsTagsImpl(int aiActorCount, Actor[] akCreatures, String[] asTags) native
String[] Function GetByRaceGendersTagsImpl(int aiActorCount, String asRaceKey, int aiMaleCrt, int aiFemaleCrt, String[] asTags) native

sslBaseAnimation[] function GetByRace(int ActorCount, Race RaceRef)
  return GetByRaceTags(ActorCount, RaceRef, "")
endFunction
sslBaseAnimation[] function GetByRaceTags(int ActorCount, Race RaceRef, string Tags, string TagsSuppressed = "", bool RequireAll = true)
  return GetByRaceKeyTags(ActorCount, SexlabRegistry.GetRaceKeyByRace(RaceRef), Tags, TagsSuppressed, RequireAll)
endFunction
sslBaseAnimation[] function GetByRaceKey(int ActorCount, string RaceKey)
  return GetByRaceKeyTags(ActorCount, RaceKey, "")
endFunction
sslBaseAnimation[] function GetByRaceKeyTags(int ActorCount, string RaceKey, string Tags, string TagsSuppressed = "", bool RequireAll = true)
  return AsBaseAnimation(GetByRaceKeyTagsImpl(ActorCount, RaceKey, SexLabUtil.MergeSplitTags(Tags, TagsSuppressed, RequireAll)))
EndFunction

sslBaseAnimation[] function GetByCreatureActors(int ActorCount, Actor[] Positions)
  return GetByCreatureActorsTags(ActorCount, Positions, "")
endFunction
sslBaseAnimation[] function GetByCreatureActorsTags(int ActorCount, Actor[] Positions, string Tags, string TagsSuppressed = "", bool RequireAll = true)
  return AsBaseAnimation(GetByCreatureActorsTagsImpl(ActorCount, Positions, SexLabUtil.MergeSplitTags(Tags, TagsSuppressed, RequireAll)))
endFunction

sslBaseAnimation[] function GetByRaceGenders(int ActorCount, Race RaceRef, int MaleCreatures = 0, int FemaleCreatures = 0, bool ForceUse = false)
  return GetByRaceGendersTags(ActorCount, RaceRef, MaleCreatures, FemaleCreatures, "")
endFunction
sslBaseAnimation[] function GetByRaceGendersTags(int ActorCount, Race RaceRef, int MaleCreatures = 0, int FemaleCreatures = 0, string Tags, string TagsSuppressed = "", bool RequireAll = true)
  String[] tags_ = SexLabUtil.MergeSplitTags(Tags, TagsSuppressed, RequireAll)
  return AsBaseAnimation(GetByRaceGendersTagsImpl(ActorCount, SexlabRegistry.GetRaceKeyByRace(RaceRef), MaleCreatures, FemaleCreatures, tags_))
endFunction

sslBaseAnimation[] Function FilterCreatureGenders(sslBaseAnimation[] Anims, int MaleCreatures = 0, int FemaleCreatures = 0)
  If (!Anims.Length)
    return Anims
  EndIf
  int[] pickup = Utility.CreateIntArray(Anims.Length, -1)
  int i = 0
  While (i < Anims.Length)
    ; Male = 0 | Female = 1 | Either = 2
    int[] count = new int[3]
    int p_count = Anims[i].ActorCount()
    int n = 0
    While (n < p_count)
      bool male = SexLabRegistry.GetIsMaleCreaturePositon(Anims[i].Registry, n)
      bool female = SexLabRegistry.GetIsFemaleCreaturePositon(Anims[i].Registry, n)
      If (male && female)
        count[2] = count[2] + 1
      ElseIf(female)
        count[1] = count[1] + 1
      ElseIf(male)
        count[0] = count[0] + 1
      EndIf  ; Else: humans
      n += 1
    EndWhile
    If (count[0] <= MaleCreatures && count[0] + count[2] >= MaleCreatures)
      count[2] = count[2] - MaleCreatures - count[0];
      If (count[1] + count[2] == FemaleCreatures)
        pickup[i] = i
      EndIf
    EndIf
    i += 1
  EndWhile
  int[] valids = PapyrusUtil.RemoveInt(pickup, -1)
  sslBaseAnimation[] ret = sslUtility.AnimationArray(valids.Length)
  int j = 0
  While (j < valids.Length)
    ret[j] = Anims[valids[j]]
    j += 1
  EndWhile
  return ret
EndFunction

bool function RaceHasAnimation(Race RaceRef, int ActorCount = -1, int Gender = -1)
  return RaceKeyHasAnimation(SexLabRegistry.GetRaceKeyByRace(RaceRef), ActorCount, Gender)
endFunction
bool Function RaceKeyHasAnimation(string RaceKey, int ActorCount = -1, int Gender = -1)
  sslBaseAnimation[] anims = GetByRaceKey(ActorCount, RaceKey)
  If (!anims.Length)
    return false
  ElseIf(Gender == -1)
    return true
  EndIf
  int i = 0
  While (i < anims.Length)
    If (anims[i].Genders[Gender] > 0)
      return true
    EndIf
    i += 1
  EndWhile
  return false
EndFunction

bool function HasCreature(Actor ActorRef)
  return sslCreatureAnimationSlots.HasCreatureType(ActorRef)
endFunction

bool function HasRace(Race RaceRef)
  return sslCreatureAnimationSlots.HasRaceType(RaceRef)
endFunction

bool function AllowedCreature(Race RaceRef)
  return Config.AllowCreatures && HasAnimation(RaceRef)
endFunction

bool Function AllowedCreatureCombination(Race RaceRef1, Race RaceRef2)
  if !Config.AllowCreatures || !RaceRef1 || !RaceRef2
    return false
  elseIf RaceRef1 == RaceRef2
    return true
  endIf
  String[] a1 = SexlabRegistry.GetRaceKeyByRaceA(RaceRef1)
  String[] a2 = SexlabRegistry.GetRaceKeyByRaceA(RaceRef2)
  return AllowedRaceKeyCombination(a1, a2)
EndFunction

bool Function AllowedRaceKeyCombination(string[] Keys1, string[] Keys2)
  if !Config.AllowCreatures || !Keys1.Length || !Keys2.Length
    return false
  ElseIf Keys1 == Keys2
    return true
  endIf
  int i = 0
  While (i < Keys1.Length)
    If (Keys2.Find(Keys1[i]) > -1)
      return true
    EndIf
    i += 1
  EndWhile
  return false
EndFunction

; Deprecated
bool function HasAnimation(Race RaceRef, int Gender = -1)
  return RaceHasAnimation(RaceRef, -1, Gender)
endFunction

; ------------------------------------------------------- ;
; --- System Use Only                                 --- ;
; ------------------------------------------------------- ;

int Function GetCrtSpecifier()
  return 1
EndFunction
function Setup()
  parent.Setup()
endfunction
function RegisterSlots()
endFunction
function RegisterRaces()
endFunction

