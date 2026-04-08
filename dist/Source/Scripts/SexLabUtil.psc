ScriptName SexLabUtil Hidden
{
	Public Misc Utility and Convenience
}

; ------------------------------------------------------- ;
; --- SexLab Accessors                                --- ;
; ------------------------------------------------------- ;

int Function GetVersion() global
	return SKSE.GetPluginVersion("SexLabUtil")
EndFunction

String function GetStringVer() global
	int[] pack = GetVersionPack()
	return pack[0] + "." + pack[1] + "." + pack[2] + "." + pack[3]
EndFunction

SexLabFramework function GetAPI() global
	return Game.GetFormFromFile(0xD62, "SexLab.esm") as SexLabFramework
endFunction

bool function SexLabIsActive() global
	return GetAPI().IsRunning
endFunction

bool function SexLabIsReady() global
	return GetAPI().Enabled
endFunction

; ------------------------------------------------------- ;
; --- Animation Starters                              --- ;
; ------------------------------------------------------- ;

SexLabThread Function StartScene(Actor[] akPositions, String asTags, Actor akSubmissive = none, ObjectReference akCenter = none, \
																		int aiFurniture = 1, String asHook = "") global
	return GetAPI().StartScene(akPositions, asTags, akSubmissive, akCenter, aiFurniture, asHook)
EndFunction

SexLabThread Function StartSceneEx(Actor[] akPositions, String[] asAnims, Actor akSubmissive = none, String asContext = "", ObjectReference akCenter = none, \
																		int aiFurniture = 1, String asHook = "") global
	return GetAPI().StartSceneEx(akPositions, asAnims, akSubmissive, asContext, akCenter, aiFurniture, asHook)
EndFunction

SexLabThread Function StartSceneQuick(Actor akActor1, Actor akActor2 = none, Actor akActor3 = none, Actor akActor4 = none, Actor akActor5 = none, \
                                        Actor akSubmissive = none, String asTags = "", String asHook = "") global
	return GetAPI().StartSceneQuick(akActor1, akActor2, akActor3, akActor4, akActor5, akSubmissive, asTags, asHook)
EndFunction

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

sslSystemConfig function GetConfig() global
	return Game.GetFormFromFile(0xD62, "SexLab.esm") as sslSystemConfig
endFunction

int[] Function GetVersionPack() global
	int v = GetVersion()
	int[] ret = new int[4]
	If (v == -1)
		return ret
	EndIf
	ret[0] = Math.LogicalAnd(Math.RightShift(v, 24), 0xFFF)
	ret[1] = Math.LogicalAnd(Math.RightShift(v, 16), 0x0FF)
	ret[2] = Math.LogicalAnd(Math.RightShift(v, 4), 0xFFF)
	ret[3] = Math.LogicalAnd(Math.RightShift(v, 0), 0x00F)
	return ret
EndFunction

; ------------------------------------------------------- ;
; --- Developer Utilities                             --- ;
; ------------------------------------------------------- ;

bool function HasKeywordSub(form ObjRef, string LookFor) global native
function PrintConsole(string output) global native
Actor[] function MakeActorArray(Actor Actor1 = none, Actor Actor2 = none, Actor Actor3 = none, Actor Actor4 = none, Actor Actor5 = none) global native
float function GetCurrentGameRealTime() global native
bool Function IsGodModeEnabled() global native
String Function GetTranslation(String asStr) global native
String[] Function ShuffleStringArray(String[] asArray, String asSetFirst = "", int aiMaxLen = 128) native global
Function HideElementsGameHUD(bool abHide = true) native global

String[] Function MergeSplitTags(String asTags, String asTagsSuppress, bool abRequireAll) global
  String[] ret1 = PapyrusUtil.ClearEmpty(PapyrusUtil.StringSplit(asTags, ","))
  String[] ret2 = PapyrusUtil.ClearEmpty(PapyrusUtil.StringSplit(asTagsSuppress, ","))
  If (ret1.Length + ret2.Length == 0)
    return Utility.CreateStringArray(0)
  EndIf
  If (!abRequireAll)
    int i = 0
    While (i < ret1.Length)
      ret1[i] = "~" + ret1[i]
      i += 1
    EndWhile
  EndIf
  int n = 0
  While (n < ret2.Length)
    ret2[n] = "-" + ret2[n]
    n += 1
  EndWhile
  If (ret1.Length && ret2.Length)
    return PapyrusUtil.MergeStringArray(ret1, ret2, true)
  ElseIf(ret1.Length)
    return ret1
  Else
    return ret2
  EndIf
EndFunction

string function ActorName(Actor ActorRef) global
	return ActorRef.GetLeveledActorBase().GetName()
endFunction

string[] function ActorNames(Actor[] ActorRefs) global
    string[] ret = PapyrusUtil.StringArray(ActorRefs.Length)
    int i = 0
    while (i < ActorRefs.Length)
        ret[i] = ActorName(ActorRefs[i])
        i += 1
    endwhile
    return ret
EndFunction

float Function CalcPathingTargetDistance(int k) global
    If (k==2||k==3||k==9||k==11||k==12||k==32||k==31||k==34||k==35||k==40||k==41||k==47)
        return 300.0
    ElseIf (k==18||k==24||k==27||k==38||k==46)
        return 400.0
    ElseIf (k==14||k==26||k==36)
        return 800.0
    EndIf
    return 128.0
EndFunction

; ------------------------------------------------------- ;
; --- Threading Utilities                             --- ;
; ------------------------------------------------------- ;

Function ToggleFreeCamera(int aiForceState = -1) global
	;[-1:Toggle, 0:TFC_STAYS_OFF, 1:TFC_STAYS_ON]
	bool bVRMode = GetConfig().HasVRIK
	If (Game.GetCameraState() == 3)
		If (aiForceState != 1)
			If (!bVRMode)
				MiscUtil.ToggleFreeCamera()
			Else
				Utility.SetIniBool("bDisablePlayerCollision:Havok", false)
				SetActorMovement(Game.GetPlayer(), 2) ;MOVEMENT_LOCK
			EndIf
		EndIf
		return
	Else
		If (aiForceState != 0)
			ForceThirdPerson()
			If (!bVRMode)
				MiscUtil.SetFreeCameraSpeed(GetConfig().AutoSUCSM)
				MiscUtil.ToggleFreeCamera()
			Else
				Utility.SetIniBool("bDisablePlayerCollision:Havok", true)
				SetActorMovement(Game.GetPlayer(), 2) ;MOVEMENT_LOCK (bVRTPP==true)
			EndIf
		EndIf
	EndIf
EndFunction

Function ForceThirdPerson() global
	bool bVRMode = GetConfig().HasVRIK
	bool bVRTPP = bVRMode && (GetConfig().POVModeVR == 2) ;VRIK_TPP_FREE
	If (bVRTPP)
		return
	ElseIf (bVRMode)
		GetConfig().SetPOVModeVRIK(2, abForced=true) ;VRIK_TPP_FREE
		return
	Else
		While (Game.GetCameraState() == 0)
			Game.ForceThirdPerson()
		EndWhile
	EndIf
EndFunction

Function SetActorMovement(Actor akActor, int aiMovement) global
	;[0:MOVEMENT_RELEASE, 1:MOVEMENT_UNLOCK, 2:MOVEMENT_LOCK]
	If ((!akActor) || (aiMovement < 0) || (aiMovement > 2))
		return
	EndIf
	If (akActor != Game.GetPlayer())
		If (aiMovement == 0) ;RELEASE
			akActor.SetDontMove(false)
			akActor.SetRestrained(false)
		Else
			akActor.SetDontMove(true)
			akActor.SetRestrained(true)
		EndIf
		return
	EndIf
	bool bVRMode = GetConfig().HasVRIK
	While (!bVRMode && Game.GetCameraState()==0)
		Game.ForceThirdPerson()
	EndWhile
	If (aiMovement == 2) ;LOCK
		bool bVRTPP = bVRMode && (GetConfig().POVModeVR == 2) ;VRIK_TPP_FREE
		Game.SetPlayerAIDriven(!bVRTPP)
		Game.DisablePlayerControls(abMovement=!bVRTPP, abCamSwitch=true, abSneaking=true, abMenu=false)
	Else
		Game.SetPlayerAIDriven(false)
		If (aiMovement == 1) ;UNLOCK
			Game.EnablePlayerControls(abFighting=false, abCamSwitch=false, abSneaking=false, abActivate=false)
		Else ;RELEASE
			Game.EnablePlayerControls()
		EndIf
	EndIf
EndFunction

Function UpdateAnimatingActorMovement(Actor akActor) global
	If (!akActor)
		return
	EndIf
	akActor.EvaluatePackage()
	int aiFactionRank = akActor.GetFactionRank(GetConfig().AnimatingFaction)
	int aiMovement = -1
	If (aiFactionRank < 0) ; OnAliasClear / NotAnimating / OnExtThreadRelease -> MOVEMENT_RELEASE
		aiMovement = 0
	ElseIf (aiFactionRank == 0 || aiFactionRank == 2) ; OnActorUnlocked / OnPathing -> MOVEMENT_UNLOCK
		aiMovement = 1
	ElseIf (aiFactionRank == 1) ; OnSetActor / OnActorLocked / OnStateAnimating / OnExtThreadControl -> MOVEMENT_LOCK
		aiMovement = 2
	EndIf
	SetActorMovement(akActor, aiMovement)
EndFunction

; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;
; ----------------------------------------------------------------------------- ;
;				██╗     ███████╗ ██████╗  █████╗  ██████╗██╗   ██╗				;
;				██║     ██╔════╝██╔════╝ ██╔══██╗██╔════╝╚██╗ ██╔╝				;
;				██║     █████╗  ██║  ███╗███████║██║      ╚████╔╝ 				;
;				██║     ██╔══╝  ██║   ██║██╔══██║██║       ╚██╔╝  				;
;				███████╗███████╗╚██████╔╝██║  ██║╚██████╗   ██║   				;
;				╚══════╝╚══════╝ ╚═════╝ ╚═╝  ╚═╝ ╚═════╝   ╚═╝   				;
; ----------------------------------------------------------------------------- ;
; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;

int function StartSex(actor[] sexActors, sslBaseAnimation[] anims, actor victim = none, ObjectReference centerOn = none, bool allowBed = true, string hook = "") global
	SexLabFramework SexLab = GetAPI()
	if !SexLab
		return -1
	endIf
	return SexLab.StartSex(sexActors, anims, victim, centerOn, allowBed, hook)
endFunction

sslThreadModel function NewThread(float timeout = 30.0) global
	SexLabFramework SexLab = GetAPI()
	if !SexLab
		return none
	endIf
	return SexLab.NewThread(timeout)
endFunction

sslThreadController function QuickStart(actor a1, actor a2 = none, actor a3 = none, actor a4 = none, actor a5 = none, actor victim = none, string hook = "", string animationTags = "") global
	SexLabFramework SexLab = GetAPI()
	if !SexLab
		return none
	endIf
	return SexLab.QuickStart(a1, a2, a3, a4, a5, victim, hook, animationTags)
endFunction

int Function GetSex(Actor akActor) global
	return SexLabRegistry.GetSex(akActor, false)
EndFunction

bool function IsActorActive(Actor ActorRef) global
	return ActorRef.IsInFaction(GetConfig().AnimatingFaction)
endFunction

bool function IsValidActor(Actor ActorRef) global
	return GetAPI().IsValidActor(ActorRef)
endFunction

bool function HasCreature(Actor ActorRef) global
	return sslCreatureAnimationSlots.HasCreatureType(ActorRef)
endFunction

bool function HasRace(Race RaceRef) global
	return sslCreatureAnimationSlots.HasRaceType(RaceRef)
endFunction

function VehicleFixMode(int mode) global
	; No longer used
	; This function used disable the player ref scale beig forced to 1.0 upon entering vehicle
	; SLP+ no longer relies on SetScale() uses Node Scaling
EndFunction

float function FloatIfElse(bool isTrue, float returnTrue, float returnFalse = 0.0) global
	If (isTrue)
		return returnTrue
	EndIf
	return returnFalse
EndFunction
int function IntIfElse(bool isTrue, int returnTrue, int returnFalse = 0) global
	If (isTrue)
		return returnTrue
	EndIf
	return returnFalse
EndFunction
string function StringIfElse(bool isTrue, string returnTrue, string returnFalse = "") global
	If (isTrue)
		return returnTrue
	EndIf
	return returnFalse
EndFunction
Form function FormIfElse(bool isTrue, Form returnTrue, Form returnFalse = none) global
	If (isTrue)
		return returnTrue
	EndIf
	return returnFalse
EndFunction
Actor function ActorIfElse(bool isTrue, Actor returnTrue, Actor returnFalse = none) global
	If (isTrue)
		return returnTrue
	EndIf
	return returnFalse
EndFunction
ObjectReference function ObjectIfElse(bool isTrue, ObjectReference returnTrue, ObjectReference returnFalse = none) global
	If (isTrue)
		return returnTrue
	EndIf
	return returnFalse
EndFunction
ReferenceAlias function AliasIfElse(bool isTrue, ReferenceAlias returnTrue, ReferenceAlias returnFalse = none) global
	If (isTrue)
		return returnTrue
	EndIf
	return returnFalse
EndFunction

function Log(string msg, string source, string type = "NOTICE", string display = "trace", bool minimal = true) global
	if StringUtil.Find(display, "trace") != -1
		if minimal
			Debug.Trace("-- SexLab "+type+"-- "+source+": "+msg)
		else
			Debug.Trace("--- SexLab "+source+" --------------------------------")
			Debug.Trace(" "+type+":")
			Debug.Trace("   "+msg)
			Debug.Trace("-----------------------------------------------------------")
		endIf
	endIf
	if StringUtil.Find(display, "box") != -1
		Debug.MessageBox(type+" "+source+": "+msg)
	endIf
	if StringUtil.Find(display, "notif") != -1
		Debug.Notification(type+": "+msg)
	endIf
	if StringUtil.Find(display, "stack") != -1
		Debug.TraceStack("-- SexLab "+type+"-- "+source+": "+msg)
	endIf
	if StringUtil.Find(display, "console") != -1
		SexLabUtil.PrintConsole(type+" SexLab "+source+": "+msg)
	endIf
endFunction

function DebugLog(string Log, string Type = "NOTICE", bool DebugMode = false) global
	Log = Type+": "+Log
	if DebugMode
		SexLabUtil.PrintConsole(Log)
	endIf
	if Type == "FATAL" || Type == "ERROR" || Type == "DEPRECATED"
		Debug.TraceStack("SEXLAB - "+Log)
	else
		Debug.Trace("SEXLAB - "+Log)
	endIf
endFunction

int Function GetPluginVersion() global
	return SKSE.GetPluginVersion("SexLabUtil")
EndFunction
string function RemoveSubString(string InputString, string RemoveString) global native
int function IntMinMaxValue(int[] searchArray, bool findHighestValue = true) global native
int function IntMinMaxIndex(int[] searchArray, bool findHighestValue = true) global native
float function FloatMinMaxValue(float[] searchArray, bool findHighestValue = true) global native
int function FloatMinMaxIndex(float[] searchArray, bool findHighestValue = true) global native

float function GetCurrentGameTimeHours() global
	return Utility.GetCurrentGameTime() * 24.0
endFunction

float function GetCurrentGameTimeMinutes() global
	return Utility.GetCurrentGameTime() * 1440.0
endFunction

float function GetCurrentGameTimeSeconds() global
	return Utility.GetCurrentGameTime() * 86400.0
endFunction

function Wait(float seconds) global
	float timer = Utility.GetCurrentRealTime() + seconds
	while Utility.GetCurrentRealTime() < timer
		Utility.Wait(0.50)
	endWhile
endFunction

float function Timer(float Timestamp, string Log) global
	float i = Utility.GetCurrentRealTime()
	DebugLog(Log, "["+(i - Timestamp)+"]", true)
	return i
endFunction

int function GetGender(Actor ActorRef) global
	return GetAPI().GetGender(ActorRef)
endFunction

string function MakeGenderTag(Actor[] Positions) global
	int[] Genders = GetAPI().GenderCount(Positions)
	return GetGenderTag(Genders[1], Genders[0], Genders[2] + Genders[3])
endFunction

bool function IsImportant(Actor ActorRef, bool Strict = false) global
	if ActorRef == Game.GetPlayer()
		return true
	elseIf !ActorRef || ActorRef.IsDead() || ActorRef.IsDeleted() || ActorRef.IsChild()
		return false
	elseIf !Strict
		return true
	endIf
	; Strict check
	ActorBase BaseRef = ActorRef.GetLeveledActorBase()
	return BaseRef.IsUnique() || BaseRef.IsEssential() || BaseRef.IsInvulnerable() || BaseRef.IsProtected() || ActorRef.IsGuard() || ActorRef.IsPlayerTeammate() || ActorRef.Is3DLoaded()
endFunction

string function GetGenderTag(int Females = 0, int Males = 0, int Creatures = 0) global
	string Tag
	while Females > 0
		Females -= 1
		Tag += "F"
	endWhile
	while Males > 0
		Males -= 1
		Tag += "M"
	endWhile
	while Creatures > 0
		Creatures -= 1
		Tag += "C"
	endWhile
	return Tag
endFunction

string function GetReverseGenderTag(int Females = 0, int Males = 0, int Creatures = 0) global
	string Tag
	while Creatures > 0
		Creatures -= 1
		Tag += "C"
	endWhile
	while Males > 0
		Males -= 1
		Tag += "M"
	endWhile
	while Females > 0
		Females -= 1
		Tag += "F"
	endWhile
	return Tag
endFunction

bool function IsActor(Form FormRef) global
	if FormRef
		int Type = FormRef.GetType()
		return Type == 43 || Type == 44 || Type == 62 ; kNPC = 43 kLeveledCharacter = 44 kCharacter = 62
	endIf
	return false
endFunction

function EnableFreeCamera(bool Enabling = true, float sucsm = 5.0) global
	return ToggleFreeCamera(Enabling as int)
endFunction
