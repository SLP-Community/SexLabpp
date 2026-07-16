ScriptName sslThreadModel extends SexLabThread Hidden
{
	Internal class for primary scene management. 
	Implements SexLabThread, builds and controls scene-flow and keeps track of scene actors

	To start a scene, please check the functions provided in the main API (SexLabFramework.psc)
	To access, read and write scene related data see sslThreadController.psc
}

int Function GetThreadID()
	return tid
EndFunction

String Function GetActiveScene() native
String Function GetActiveStage() native
; Scene set of the active scene
String[] Function GetPlayingScenes() native

Function StopAnimation()
	EndAnimation()
EndFunction

float Function GetTime()
	return StartedAt
endfunction
float Function GetTimeTotal()
	return TotalTime
EndFunction

String[] Function GetStageHistory()
	return PapyrusUtil.RemoveString(_StageHistory, "")
EndFunction
int Function GetStageHistoryLength()
	return _StageHistory.Length
EndFunction
int Function GetLegacyStageNum()
	return (SexlabRegistry.GetAllStages(GetActiveScene()).Find(GetActiveStage()) + 1)
EndFunction
int Function GetLegacyStagesCount()
	return SexlabRegistry.GetNumStages(GetActiveScene())
EndFunction

; ------------------------------------------------------- ;
; --- Position Access                                 --- ;
; ------------------------------------------------------- ;

; Current positions ordered by active scene
Actor[] Function GetPositions() native

bool Function HasPlayer()
	return HasPlayer
EndFunction
bool Function HasActor(Actor ActorRef)
	return _Positions.Find(ActorRef) != -1
EndFunction

int Function GetPositionIdx(Actor akActor)
	return _Positions.Find(akActor)
EndFunction
Actor Function GetIdxPosition(int n)
	return _Positions[n]
EndFunction

int Function GetActorSex(Actor akActor)
	return GetNthPositionSex(GetPositionIdx(akActor))
EndFunction

int Function GetNthPositionSex(int n)
	If (n < 0 || n >= _Positions.Length)
		return 0
	EndIf
	return ActorAlias[n].GetSex()
EndFunction

int[] Function GetPositionSexes()
	int[] ret = Utility.CreateIntArray(_Positions.Length)
	int i = 0
	While (i < ret.Length)
		ret[i] = ActorAlias[i].GetSex()
		i += 1
	EndWhile
	return ret
EndFunction

Function SetCustomStrip(Actor akActor, int aiSlots, bool abWeapon, bool abApplyNow)
	sslActorAlias it = ActorAlias(akActor)
	If (!it)
		return
	EndIf
	it.SetStripping(aiSlots, abWeapon, abApplyNow)
EndFunction
Function ResetCustomStrip(Actor akActor)
	sslActorAlias it = ActorAlias(akActor)
	If (!it)
		return
	EndIf
	it.DeleteCustomStripping()
EndFunction

bool Function IsUndressAnimationAllowed(Actor akActor)
	sslActorAlias it = ActorAlias(akActor)
	return it && it.DoUndress
EndFunction
Function SetIsUndressAnimationAllowed(Actor akActor, bool abAllowed)
	sslActorAlias it = ActorAlias(akActor)
	If (!it)
		return
	EndIf
	it.DisableStripAnimation(!abAllowed)
EndFunction

bool Function IsRedressAllowed(Actor akActor)
	sslActorAlias it = ActorAlias(akActor)
	return it && it.DoRedress
EndFunction
Function SetIsRedressAllowed(Actor akActor, bool abAllowed)
	sslActorAlias it = ActorAlias(akActor)
	If (!it)
		return
	EndIf
	it.SetAllowRedress(abAllowed)
EndFunction

Function SetPathingFlag(Actor akActor, int aiPathingFlag)
	sslActorAlias it = ActorAlias(akActor)
	If (!it)
		return
	EndIf
	it.SetPathing(aiPathingFlag)
EndFunction

; Voice
Function SetActorVoice(Actor akActor, String asVoice, bool abForceSilent)
	sslActorAlias ref = ActorAlias(akActor)
	If (!ref)
		return
	EndIf
	ref.SetActorVoice(asVoice, abForceSilent)
EndFunction
String Function GetActorVoice(Actor akActor)
	sslActorAlias ref = ActorAlias(akActor)
	If (!ref)
		return none
	EndIf
	return ref.GetActorVoice()
EndFunction

; Expressions
String Function GetActorExpression(Actor akActor)
	sslActorAlias ref = ActorAlias(akActor)
	If (!ref)
		return none
	EndIf
	return ref.GetActorExpression()
EndFunction

Function SetActorExpression(Actor akActor, String asExpression)
	sslActorAlias ref = ActorAlias(akActor)
	If (!ref)
		return
	EndIf
	ref.SetActorExpression(asExpression)
EndFunction

Function SetActorMouthForcedOpen(Actor akActor, bool abForceOpen)
	sslActorAlias ref = ActorAlias(akActor)
	If (!ref)
		return
	EndIf
	ref.SetMouthForcedOpen(abForceOpen)
EndFunction

; Enjoyment
int Function GetEnjoyment(Actor ActorRef)
	sslActorAlias ref = ActorAlias(ActorRef)
	If (!ref)
		return 0
	EndIf
	return ref.GetEnjoyment()
EndFunction

Function SetEnjoyment(Actor ActorRef, int aiSet)
	sslActorAlias ref = ActorAlias(ActorRef)
	If (!ref)
		return
	EndIf
	return ref.SetEnjoyment(aiSet)
EndFunction

Function AdjustEnjoyment(Actor ActorRef, int AdjustBy)
	sslActorAlias ref = ActorAlias(ActorRef)
	If (!ref)
		return
	EndIf
	return ref.AdjustEnjoyment(AdjustBy)
EndFunction

Function ModEnjoymentMult(Actor ActorRef, float afSet, bool bAdjust = False)
	sslActorAlias ref = ActorAlias(ActorRef)
	If (!ref)
		return
	EndIf
	return ref.ModEnjoymentMult(afSet, bAdjust)
EndFunction

; Orgasms
Function DisableOrgasm(Actor ActorRef, bool OrgasmDisabled = true)
	sslActorAlias ref = ActorAlias(ActorRef)
	If (!ref)
		return
	EndIf
	return ref.DisableOrgasm(OrgasmDisabled)
EndFunction

bool Function IsOrgasmAllowed(Actor ActorRef)
	sslActorAlias ref = ActorAlias(ActorRef)
	If (!ref)
		return false
	EndIf
	return ref.IsOrgasmAllowed()
EndFunction

Function ForceOrgasm(Actor ActorRef)
	sslActorAlias ref = ActorAlias(ActorRef)
	If (!ref)
		return none
	EndIf
	return ref.DoOrgasm(true)
EndFunction

int Function GetOrgasmCount(Actor ActorRef)
	sslActorAlias ref = ActorAlias(ActorRef)
	If (!ref)
		return 0
	EndIf
	return ref.GetOrgasmCount()
EndFunction

Actor[] Function CanBeImpregnated(Actor akActor,  bool abAllowFutaImpregnation, bool abFutaCanPregnate, bool abCreatureCanPregnate)
	Actor[] ret
	sslActorAlias ref = ActorAlias(akActor)
	If (!ref)
		return ret
	EndIf
	int refsex = ref.GetSex()
	If !(refsex == 1 || abAllowFutaImpregnation && refsex == 2)
		return ret
	EndIf
	ret = new Actor[5]
	String[] orgasmStages = SexLabRegistry.GetClimaxStages(GetActiveScene())
	int i = 0
	While (i < orgasmStages.Length)
		If (_StageHistory.Find(orgasmStages[i]) > -1 && SexLabRegistry.IsStageTag(GetActiveScene(), orgasmStages[i], "~Grinding, ~Vaginal, Penetration"))
			int[] orgP = SexLabRegistry.GetClimaxingActors(GetActiveScene(), orgasmStages[i])
			int n = 0
			While (n < orgP.Length)
				If (_Positions[n] != akActor && ActorAlias[n].IsOrgasmAllowed())
					int orgSex = ActorAlias[n].GetSex()
					If (orgSex == 0 || (abFutaCanPregnate && orgSex == 2) || (abCreatureCanPregnate && orgSex == 3))
						ret[n] = _Positions[n]
					EndIf
				EndIf
				n += 1
			EndWhile
		EndIf
		i += 1
	EndWhile
	return PapyrusUtil.RemoveActor(ret, none)
EndFunction

; Actor Strapons
bool Function IsUsingStrapon(Actor ActorRef)
	return ActorAlias(ActorRef).IsUsingStrapon()
EndFunction

Function SetStrapon(Actor ActorRef, Form ToStrapon)
	ActorAlias(ActorRef).SetStrapon(ToStrapon)
endfunction

Form Function GetStrapon(Actor ActorRef)
	return ActorAlias(ActorRef).GetStrapon()
endfunction

; ------------------------------------------------------- ;
; --- Submission                                      --- ;
; ------------------------------------------------------- ;
;/
	Functions for consent interpretation and to view and manipulate the submissive flag for individual actors
/;

bool Function IsConsent()
	return !HasContext("Aggressive")
EndFunction

Function SetConsent(bool abIsConsent)
	If (abIsConsent)
		RemoveContext("Aggressive")
	Else
		AddContext("Aggressive")
	EndIf
EndFunction

Actor[] Function GetSubmissives()
	Actor[] ret = new Actor[5]
	int i = 0
	While(i < _Positions.Length)
		If(ActorAlias[i].IsVictim())
			ret[i] = _Positions[i]
		EndIf
		i += 1
	EndWhile
	return PapyrusUtil.RemoveActor(ret, none)
EndFunction

Function SetIsSubmissive(Actor akActor, bool abIsSubmissive)
	sslActorAlias it = ActorAlias(akActor)
	If (!it)
		return
	EndIf
	it.SetVictim(abIsSubmissive)
EndFunction

bool Function GetSubmissive(Actor akActor)
	sslActorAlias it = ActorAlias(akActor)
	return it && it.IsVictim()
EndFunction
bool Function IsVictim(Actor ActorRef)
	return GetSubmissive(ActorRef)
EndFunction

bool Function IsAggressor(Actor ActorRef)
	sslActorAlias agr = ActorAlias(ActorRef)
	return GetSubmissives().Length && agr && agr.IsAggressor()
EndFunction

; ------------------------------------------------------- ;
; --- Tagging System                                  --- ;
; ------------------------------------------------------- ;

bool Function HasTag(String Tag)
	return _ThreadTags.Length && _ThreadTags.Find(Tag) > -1
EndFunction

bool Function HasSceneTag(String Tag)
	return SexLabRegistry.IsSceneTag(GetActiveScene(), Tag)
EndFunction
bool Function IsVaginal()
	return HasSceneTag("Vaginal")
EndFunction
bool Function IsAnal()
	return HasSceneTag("Anal")
EndFunction
bool Function IsOral()
	return HasSceneTag("Oral")
EndFunction

bool Function HasStageTag(String Tag)
	return SexLabRegistry.IsStageTag(GetActiveScene(), GetActiveStage(), Tag)
EndFunction

String[] Function GetTags()
	return PapyrusUtil.ClearEmpty(_ThreadTags)
EndFunction

bool Function HasContext(String asTag)
	return _ContextTags.Length && _ContextTags.Find(asTag) > -1
EndFunction

Function AddContext(String asContext)
	If (HasContext(asContext))
		return
	EndIf
	_ContextTags = PapyrusUtil.PushString(_ContextTags, asContext)
EndFunction
Function RemoveContext(String asContext)
	_ContextTags = PapyrusUtil.RemoveString(_ContextTags, asContext)
EndFunction

String[] Function AddContextExImpl(String[] asOldContext, String asContext) native
Function AddContextEx(String asContext)
	_ContextTags = AddContextExImpl(_ContextTags, asContext)
EndFunction

bool Function IsLeadIn()
	return LeadIn
EndFunction

; ------------------------------------------------------- ;
; --- Physics                                         --- ;
; ------------------------------------------------------- ;

bool Function IsInteractionRegistered()
	return IsCollisionRegistered()
EndFunction

int[] Function GetInteractionTypes(Actor akPosition, Actor akPartner)
	return GetCollisionActions(akPosition, akPartner)
EndFunction

bool Function HasInteractionType(int aiType, Actor akPosition, Actor akPartner)
	return HasCollisionAction(aiType, akPosition, akPartner)
EndFunction

Actor Function GetPartnerByType(Actor akPosition, int aiType)
	return GetPartnerByAction(akPosition, aiType)
EndFUnction
Actor[] Function GetPartnersByType(Actor akPosition, int aiType)
	return GetPartnersByAction(akPosition, aiType)
EndFUnction
Actor Function GetPartnerByTypeRev(Actor akPartner, int aiType)
	return GetPartnerByActionRev(akPartner, aiType)
EndFunction
Actor[] Function GetPartnersByTypeRev(Actor akPartner, int aiType)
	return GetPartnersByActionRev(akPartner, aiType)
EndFunction

float Function GetVelocity(Actor akPosition, Actor akPartner, int aiType)
	return GetActionVelocity(akPosition, akPartner, aiType)
EndFunction

; ------------------------------------------------------- ;
; --- Interactions Info                               --- ;
; ------------------------------------------------------- ;

bool[] Function GetCurrentInteractionFlags(Actor akPosition)
	sslActorAlias ref = ActorAlias(akPosition)
	If (!ref)
		return Utility.CreateBoolArray(SUPPORTED_INTER_COUNT, False)
	EndIf
	return ref.GetCurrentInteractionFlags()
EndFunction

bool Function HasCurrentInteractionFlag(Actor akPosition, int InterType)
	If (InterType < 0 || InterType > 27)
		return False
	EndIf
	bool[] curFlags = GetCurrentInteractionFlags(akPosition)
	return (curFlags[InterType])
EndFunction

bool Function HasCurrentInteractionFlagsAll(Actor akPosition, int[] InterTypes)
	If (InterTypes.Length == 0)
		return False
	EndIf
	int i = 0
	While (i < InterTypes.Length)
		If !(HasCurrentInteractionFlag(akPosition, InterTypes[i]))
			return False
		EndIf
		i += 1
	EndWhile
	return True
EndFunction

bool Function HasCurrentInteractionFlagsAny(Actor akPosition, int[] InterTypes)
	If (InterTypes.Length == 0)
		return False
	EndIf
	int i = 0
	While (i < InterTypes.Length)
		If (HasCurrentInteractionFlag(akPosition, InterTypes[i]))
			return True
		EndIf
		i += 1
	EndWhile
	return False
EndFunction

string Function GetCurrentInteractionString(Actor akPosition)
	bool[] curFlags = GetCurrentInteractionFlags(akPosition)
	string[] interTypes = Config.NameAllInteractions
	int len = interTypes.Length
	string ret = ""
	int i = 0
	While (i < len)
		If (curFlags[i])
			If ret != ""
				ret += ","
			EndIf
			ret += interTypes[i]
		EndIf
		i += 1
	EndWhile
	return ret
EndFunction

string[] Function GetCurrentInteractionStringA(Actor akPosition)
	bool[] curFlags = GetCurrentInteractionFlags(akPosition)
	string[] interTypes = Config.NameAllInteractions
	int len = interTypes.Length
	int activeCount = 0
	int i = 0
	While (i < len)
		If (curFlags[i])
			activeCount += 1
		EndIf
		i += 1
	EndWhile
	string[] ret = Utility.CreateStringArray(activeCount)
	int retIdx = 0
	i = 0
	While (i < len)
		If (curFlags[i])
			ret[retIdx] = interTypes[i]
			retIdx += 1
		EndIf
		i += 1
	EndWhile
	return ret
EndFunction

; ------------------------------------------------------- ;
; --- Specific Detections                             --- ;
; ------------------------------------------------------- ;

bool Function IsVaginalComplex(Actor akPosition)
	sslActorAlias ref = ActorAlias(akPosition)
	If (!ref)
		return False
	EndIf
	return ref.IsVaginalComplex()
EndFunction

bool Function IsAnalComplex(Actor akPosition)
	sslActorAlias ref = ActorAlias(akPosition)
	If (!ref)
		return False
	EndIf
	return ref.IsAnalComplex()
EndFunction

bool Function IsOralComplex(Actor akPosition)
	sslActorAlias ref = ActorAlias(akPosition)
	If (!ref)
		return False
	EndIf
	return ref.IsOralComplex()
EndFunction

; ------------------------------------------------------- ;
; --- Event Hooks                                     --- ;
; ------------------------------------------------------- ;

Function SetHook(string AddHooks)
	string[] newHooks = PapyrusUtil.StringSplit(AddHooks)
	_Hooks = PapyrusUtil.MergeStringArray(_Hooks, newHooks, true)
EndFunction

Function RemoveHook(string DelHooks)
	string[] remove = PapyrusUtil.StringSplit(DelHooks)
	int i = 0
	While (i < remove.Length)
		int where = _Hooks.Find(remove[i])
		If(where > -1)
			_Hooks[where] = ""
		EndIf
		i += 1
	EndWhile
	_Hooks = PapyrusUtil.ClearEmpty(_Hooks)
EndFunction

string[] Function GetHooks()
	return PapyrusUtil.ClearEmpty(_Hooks)
EndFunction

; ------------------------------------------------------- ;
; --- VRIK Configs + Init                             --- ;
; ------------------------------------------------------- ;

Function SetConfigsVRIK(bool abEnabled=true, bool abOverrideConfig=false, int aiPOVMode=-1, \
	int abLockHeight=-1, float afHeightAdjSpeed=-1.0, int abTrackHead=-1, int aiTrackHands=-1, \
	float afDistHideHead=-1.0, float afDistNearClip=-1.0, int aiLockHmdToBody=-1, \
	float afLockHmdDistance=-1.0, float afLockHmdTolerance=-1.0, float afLockHmdSpeed=-1.0)
	If (abEnabled)
		Config.RefreshConfigsVRIK(abOverrideConfig, aiPOVMode, abLockHeight, afHeightAdjSpeed, \
			abTrackHead, aiTrackHands, afDistHideHead, afDistNearClip, aiLockHmdToBody, \
			afLockHmdDistance, afLockHmdTolerance, afLockHmdSpeed)
	EndIf
	Config.ApplyConfigsVRIK(abEnabled)
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

int thread_id
int Property tid hidden
	int Function get()
		return thread_id
	EndFunction
EndProperty

Actor Property PlayerRef Auto
bool Property HasPlayer Hidden
	bool Function Get()
		return _Positions.Find(PlayerRef) > -1
	EndFunction
EndProperty

sslSystemConfig Property Config Auto
Message Property InvalidCenterMsg Auto	; Invalid new cewnter -> [0: Keep Old Center, 1: End Scene]

; Constants
int Property POSITION_COUNT_MAX = 5 AutoReadOnly

String Property STATE_IDLE 		= "Unlocked" AutoReadOnly
String Property STATE_SETUP 	= "Making" AutoReadOnly
String Property STATE_SETUP_M	= "Making_M" AutoReadOnly
String Property STATE_PLAYING   = "Animating" AutoReadOnly
String Property STATE_END 		= "Ending" AutoReadOnly

; ------------------------------------------------------- ;
; --- Thread Status                                   --- ;
; ------------------------------------------------------- ;

bool Property IsLocked hidden
	bool Function get()
		return GetStatus() != STATUS_IDLE
	EndFunction
EndProperty

; Every valid state will oerwrite this
; Should this ever be called, then the Thread was in an unspecified state and will be reset
int Function GetStatus()
	Fatal("Undefined Status. Resetting thread...")
	return STATUS_UNDEF
EndFunction

; ------------------------------------------------------- ;
; --- Thread Data                                     --- ;
; ------------------------------------------------------- ;

sslActorAlias[] Property ActorAlias Auto
Actor[] _Positions

String[] _StageHistory
int Property Stage Hidden
	int Function Get()
		return _StageHistory.Length
	EndFunction
	Function Set(int aSet)
		return GoToStage(aSet)
	EndFunction
EndProperty

int Property FURNI_DISALLOW = 0 AutoReadOnly
int Property FURNI_ALLOW 		= 1 AutoReadOnly
int Property FURNI_PREFER 	= 2 AutoReadOnly
int _furniStatus

ReferenceAlias Property CenterAlias Auto	; the alias referencing _center
ObjectReference Property CenterRef Hidden	; shorthand for CenterAlias
	ObjectReference Function Get()
		return CenterAlias.GetReference()
	EndFunction
	Function Set(ObjectReference akNewCenter)
		CenterOnObject(akNewCenter)
	EndFunction
EndProperty

float Property StartedAt Auto Hidden
float Property TotalTime Hidden
	float Function get()
		return SexLabUtil.GetCurrentGameRealTime() - StartedAt
	EndFunction
EndProperty

bool Property AutoAdvance Auto Hidden
bool Property LeadIn Auto Hidden

String[] _ThreadTags
String[] _ContextTags
String[] _Hooks

; ------------------------------------------------------- ;
; --- Thread IDLE                                     --- ;
; ------------------------------------------------------- ;
;/
	An idle state from which the thread can be started
	Upone calling "Make" the thread will leap into the making state

	Every animation begins and ends in this state
/;
Auto State Unlocked
	sslThreadModel Function Make()
		GoToState(STATE_SETUP)
		return self
	EndFunction

	int Function GetStatus()
		return STATUS_IDLE
	EndFunction
EndState

sslThreadModel Function Make()
	Log("Thread is not idling", "Make()")
	return none
EndFunction

; ------------------------------------------------------- ;
; --- Thread SETUP                                    --- ;
; ------------------------------------------------------- ;
;/
	This State is being entered upon requesting the thread
	It marks the thread as blocked and allows functions to add/remove actors and configure the scene in various other ways
	it is also responsible for making sure an animation exists and sorts the actors appropriately

	Upon completion, this state will switch into the "Aniamting" State
/;

int _instanceCreationWaitLock
int _prepareAsyncCount
int _lockAsyncCount
String[] _CustomScenes
String[] _PrimaryScenes
String[] _LeadInScenes

State Making
	Event OnBeginState()
		Log("Entering Setup State")
		_CustomScenes = Utility.CreateStringArray(0)
		_PrimaryScenes = Utility.CreateStringArray(0)
		_LeadInScenes = Utility.CreateStringArray(0)
		RegisterForSingleUpdate(30.0)
	EndEvent
	Event OnUpdate()
		Fatal("Thread has timed out during setup. Resetting thread...")
		If (_QuickResetScenes)
			_QuickResetScenes = false
			GoToState(STATE_END)
		EndIf
		Initialize()
	EndEvent

	int Function AddActor(Actor ActorRef, bool IsVictim = false, sslBaseVoice Voice = none, bool ForceSilent = false)
		If(!ActorRef)
			Fatal("Failed to add actor -- Actor is a figment of your imagination", "AddActor(NONE)")
			return -1
		ElseIf(_Positions.Length >= POSITION_COUNT_MAX)
			Fatal("Failed to add actor -- Thread has reached actor limit", "AddActor(" + ActorRef.GetLeveledActorBase().GetName() + ")")
			return -1
		ElseIf(_Positions.Find(ActorRef) != -1)
			Fatal("Failed to add actor -- They have been already added to this thread", "AddActor(" + ActorRef.GetLeveledActorBase().GetName() + ")")
			return -1
		EndIf
		int ERRC = sslActorLibrary.ValidateActorImpl(ActorRef)
		If(ERRC < 0)
			Fatal("Failed to add actor -- They are not a valid target for animation | Error Code: " + ERRC, "AddActor(" + ActorRef.GetLeveledActorBase().GetName() + ")")
			return -1
		EndIf
		int i = _Positions.Length	; Index of the new actor in array after pushing
		If(!ActorAlias[i].SetActor(ActorRef))
			Fatal("Failed to add actor -- They were unable to fill an actor alias", "AddActor(" + ActorRef.GetLeveledActorBase().GetName() + ")")
			return -1
		EndIf
		ActorAlias[i].SetVictim(IsVictim)
		ActorAlias[i].SetVoice(Voice, ForceSilent)
		_Positions = PapyrusUtil.PushActor(_Positions, ActorRef)
		return _Positions.Find(ActorRef)
	EndFunction
	bool Function AddActors(Actor[] ActorList, Actor VictimActor = none)
		int i = 0
		While(i < ActorList.Length)
			If(AddActor(ActorList[i], ActorList[i] == VictimActor) == -1)
				return false
			EndIf
			i += 1
		EndWhile
		Log("Added " + SexLabUtil.ActorNames(ActorList) + " to thread", "AddActors()")
		return true
	EndFunction
	bool Function AddActorsA(Actor[] ActorList, Actor[] akVictims)
		int i = 0
		While(i < ActorList.Length)
			If(AddActor(ActorList[i], akVictims.Find(ActorList[i]) > -1) == -1)
				return false
			EndIf
			i += 1
		EndWhile
		Log("Added " + SexLabUtil.ActorNames(ActorList) + " to thread", "AddActorsA()")
		return true
	EndFunction

	Function SetScenes(String[] asScenes)
		_PrimaryScenes = SexLabRegistry.SceneExistA(asScenes)
	EndFunction
	Function ClearScenes()
		_PrimaryScenes = Utility.CreateStringArray(0)
	EndFunction
	Function SetForcedScenes(String[] asScenes)
		_CustomScenes = SexLabRegistry.SceneExistA(asScenes)
	EndFunction
	Function ClearForcedScenes()
		_CustomScenes = Utility.CreateStringArray(0)
	EndFunction
	Function SetLeadInScenes(String[] asScenes)
		_LeadInScenes = SexLabRegistry.SceneExistA(asScenes)
		LeadIn = _LeadInScenes.Length > 0
	EndFunction
	Function ClearLeadInScenes()
		_LeadInScenes = Utility.CreateStringArray(0)
	LeadIn = false
	EndFunction
	Function AddScene(String asSceneID)
		If (!asSceneID || !SexLabRegistry.SceneExists(asSceneID))
			return
		EndIf
		If(_CustomScenes.Length > 0)
			_CustomScenes = PapyrusUtil.PushString(_CustomScenes, asSceneID)
		ElseIf(LeadIn)
			_LeadInScenes = PapyrusUtil.PushString(_LeadInScenes, asSceneID)
		Else
			_PrimaryScenes = PapyrusUtil.PushString(_PrimaryScenes, asSceneID)
		EndIf
	EndFunction

	Function DisableLeadIn(bool disabling = true)
		LeadIn = !disabling
	EndFunction
	Function SetFurnitureStatus(int aiStatus)
		_furniStatus = PapyrusUtil.ClampInt(aiStatus, FURNI_DISALLOW, FURNI_PREFER)
	EndFunction

	Function CenterOnObject(ObjectReference CenterOn, bool resync = true)
		If (CenterOn)
			If (CenterOn as Actor && _Positions.Find(CenterOn as Actor) == -1)
				Form xMarker = Game.GetForm(0x3B)
				CenterOn = CenterOn.PlaceAtMe(xMarker, 1, false)
			EndIf
			CenterAlias.ForceRefTo(CenterOn)
		Else
			CenterAlias.Clear()
		EndIf
	EndFunction

	sslThreadController Function StartThread()
		UnregisterForUpdate()
		_Positions = PapyrusUtil.RemoveActor(_Positions, none)
		If(_Positions.Length <= 0 || _Positions.Length > POSITION_COUNT_MAX)
			Fatal("Failed to start Thread: Thread has reached actor limit or no actors were added", "StartThread()")
			Initialize()
			return none
		EndIf
		RunHook(Config.HOOKID_STARTING)
		; Validates scenes, finds a center & selects active scene. Returns false if the thread is invalid
		If (!LeadIn)
			ClearLeadInScenes()
		EndIf
		Actor[] submissives = GetSubmissives()
		_instanceCreationWaitLock = -1
		CreateInstance(submissives, _PrimaryScenes, _LeadInScenes, _CustomScenes, _furniStatus)
		While (_instanceCreationWaitLock < 0)
			Utility.Wait(0.05)
		EndWhile
		If (_instanceCreationWaitLock == 0)
			Fatal("Failed to start Thread: Unable to create thread instance. See 'Documents/My Games/Skyrim Special Edition/SKSE/SexLabUtil.log' for details", "StartThread()")
			Initialize()
			return none
		EndIf
		If (!_QuickResetScenes)
			GoToState(STATE_SETUP_M)
		EndIf
	return self as sslThreadController
	EndFunction
	; Called after CreateInstance() terminates (maybe async due to center selection)
	Function ContinueSetup(bool abContinue)
		Log("ContinueSetup called with " + abContinue)
		_instanceCreationWaitLock = abContinue as int
	EndFunction
	
	Function EndAnimation(bool Quickly = false)
		Initialize()
	EndFunction

	int Function GetStatus()
		return STATUS_SETUP
	EndFunction
EndState

; An immediate state to disallow setting additional data while aliases process setup
State Making_M
	Event OnBeginState()
		; Event to all active aliases, resync via PrepareDone() to continue startup
		_prepareAsyncCount = 0
		CenterRef.SendModEvent("SSL_PREPARE_Thread" + tid)
		_LeadInScenes = GetLeadInScenes()
		_PrimaryScenes = GetPrimaryScenes()
		_CustomScenes = GetCustomScenes()
		SortAliasesToPositions()
		PrepareDone()
		If (_CustomScenes.Length)
			_ThreadTags = SexLabRegistry.GetCommonTags(_CustomScenes)
		Else
			_ThreadTags = SexLabRegistry.GetCommonTags(_PrimaryScenes)
		EndIf
	EndEvent

	; Invoked n times by Aliases and once by StartThread, then continue to next state
	Function PrepareDone()
		_prepareAsyncCount += 1
		Log("PrepareDone() called " + _prepareAsyncCount + "/" + (_Positions.Length + 1) + " times")
		If (_prepareAsyncCount < (_Positions.Length + 1))
			return
		EndIf
		_lockAsyncCount = 0
		CenterRef.SendModEvent("SSL_LOCK_Thread" + tid)
		String activeScene = GetActiveScene()
		LeadIn = LeadIn && _LeadInScenes.Find(activeScene) > -1
		Log("Thread validated, playing animation: " + activeScene + ", " + SexLabRegistry.GetSceneName(activeScene), "StartThread()")
		SendThreadEvent("AnimationStarting")
		AliasLockDone()
	EndFunction

	Function AliasLockDone()
		_lockAsyncCount += 1
		Log("AliasLockDone() called " + _lockAsyncCount + "/" + (_Positions.Length + 1) + " times")
		If (_lockAsyncCount < (_Positions.Length + 1))
			return
		EndIf
		If (Config.HideHUD)
			SexLabUtil.HideElementsGameHUD(true)
		EndIf
		If (!UndressAndStripActors())
			EndAnimation()
			return
		EndIf
		GoToState(STATE_PLAYING)
	EndFunction

	bool Function UndressAndStripActors()
		bool WaitForUndress = false
		int i = 0
		While (i < _Positions.Length)
			WaitForUndress= (ActorAlias[i].InitiateUndressing() || WaitForUndress)
			i += 1
		EndWhile
		If (WaitForUndress)
			Utility.Wait(2.5)
		EndIf
		If (HasPlayer)
			If (sslSystemConfig.GetSettingInt("iUseFade") > 0)
				Config.ApplyFade()
			EndIf
		Else
			If (Config.ShowInMap && PlayerRef.GetDistance(CenterRef) > 750)
				SetObjectiveDisplayed(0, true)
			EndIf
		EndIf
		string activeScene = GetActiveScene()
		int[] strips_ = SexLabRegistry.GetStripDataA(activeScene, "")
		int[] sex_ = SexLabRegistry.GetPositionSexA(activeScene)
		int j = 0
		While (j < _Positions.Length)
			If (!ActorAlias[j].ReadyActor(strips_[j], sex_[j]))
				return false
			EndIf
			j += 1
		EndWhile
		If (WaitForUndress)
			Utility.Wait(1.5)
		EndIf
		return true
	EndFunction
	
	Function EndAnimation(bool Quickly = false)
		_prepareAsyncCount = -2147483648
		GoToState(STATE_END)
	EndFunction

	int Function GetStatus()
		return STATUS_SETUP
	EndFunction
EndState

sslThreadController Function StartThread()
	Log("Cannot start thread outside of setup phase", "StartThread()")
	return none
EndFunction
int Function AddActor(Actor ActorRef, bool IsVictim = false, sslBaseVoice Voice = none, bool ForceSilent = false)
	Log("Cannot add an actor to a locked thread", "AddActor()")
	return -1
EndFunction
bool Function AddActors(Actor[] ActorList, Actor VictimActor = none)
	Log("Cannot add a list of actors to a locked thread", "AddActors()")
	return false
EndFunction
bool Function AddActorsA(Actor[] akActors, Actor[] akVictims)
	Log("Cannot add a list of actors to a locked thread", "AddActorsA()")
	return false
EndFunction
Function SetScenes(String[] asScenes)
	Log("Primary scenes can only be set during setup", "SetScenes()")
EndFunction
Function ClearScenes()
	Log("Primary scenes can only be cleared during setup", "SetScenes()")
EndFunction
Function SetForcedScenes(String[] asScenes)
	Log("Forced animations can only be set during setup", "SetForcedScenes()")
EndFunction
Function ClearForcedScenes()
	Log("Forced animations can only be cleared during setup", "SetForcedScenes()")
EndFunction
Function SetLeadInScenes(String[] asScenes)
	Log("LeadIn animations can only be set during setup", "SetLeadInScenes()")
EndFunction
Function ClearLeadInScenes()
	Log("LeadIn animations can only be cleared during setup", "SetLeadInScenes()")
EndFunction
Function AddScene(String asSceneID)
	Log("Cannot add a scene to a locked thread", "AddScene()")
EndFunction
Function SetStartingScene(String asFirstAnimation)
	Log("Start animations can only be set during setup", "SetStartingScene()")
EndFunction
Function DisableLeadIn(bool disabling = true)
	Log("Lead in status can only be set during setup", "DisableLeadIn()")
EndFunction
Function SetFurnitureStatus(int aiStatus)
	Log("Furniture status can only be set during setup", "SetFurnitureStatus()")
EndFunction
Function ContinueSetup(bool abContinue)
	Log("ContinueSetup() can only be called during setup", "ContinueSetup()")
EndFunction
bool Function UndressAndStripActors()
	Log("Actors can be undressed only during setup", "UndressAndStripActors()")
	return false
EndFunction

Function CreateInstance(Actor[] akSubmissives, String[] asPrimaryScenes, String[] asLeadInScenes, String[] asCustomScenes, int aiFurnitureStatus) native
String[] Function GetLeadInScenes() native
String[] Function GetPrimaryScenes() native
String[] Function GetCustomScenes() native

; ------------------------------------------------------- ;
; --- Thread PLAYING                                  --- ;
; ------------------------------------------------------- ;
;/
	The state manages actors and the animation itself from start to finish
	By this time, most Scene information is read only
/;

float Property ANIMATING_UPDATE_INTERVAL = 0.5 AutoReadOnly
int _animationSyncCount
int _initialRealignTicks	; Placement is only asserted once per stage; if the first assert races with a busy
							; actor (furniture exit, get-up, pathing) it silently fails until the next stage.
							; Counts down OnUpdate ticks after AnimationStart to re-assert placement (see RealignActors)

bool _QuickResetScenes		; reinits thread without actor/center changes (e.g. to get new playing scenes)
bool _ForceAdvance		; Force fully auto advance (set by timed stages)
float _StageTimer			; timer for the current stage
float _StageDuration		; duration of the current stage
bool _TimerPaused		; whether timer drain for current stage should be paused
float _SFXTimer				; so long until new SFX effect
float[] _CustomTimers	; Custom set of timers to use for this animation
float[] Property Timers hidden
	{In use timer set of the active scene}
	float[] Function Get()
		If (_CustomTimers.Length)
			return _CustomTimers
		EndIf
		return Config.StageTimer
	EndFunction
	Function Set(float[] value)
		_CustomTimers = value
	EndFunction
EndProperty

State Animating
	Event OnBeginState()
		SetFurnitureIgnored(true)
		_SFXTimer = Config.SFXDelay
		If (!_QuickResetScenes)
			_animationSyncCount = 0
			SendModEvent("SSL_READY_Thread" + tid)
			AnimationStart()
		EndIf
	EndEvent
	Function AnimationStart()
		_animationSyncCount += 1
		Log("AnimationStart called " + _animationSyncCount + "/" + (_Positions.Length + 1) + " times")
		If (_animationSyncCount < (_Positions.Length + 1))
			return
		EndIf
		Log("AnimationStart fully setup, begin animating")
		If (HasPlayer)
			If(IsVictim(PlayerRef) && Config.DisablePlayer)
				AutoAdvance = true
			Else
				AutoAdvance = Config.AutoAdvance
				Config.GetThreadControl(self as sslThreadController)
				(self as sslThreadController).TogglePrismaMenu(1)
			EndIf
		Else
			AutoAdvance = true
		EndIf
		StartedAt = SexLabUtil.GetCurrentGameRealTime()
		_initialRealignTicks = 4
		StartStage(Utility.CreateStringArray(0), "")
		SendThreadEvent("AnimationStart")
		If(LeadIn)
			SendThreadEvent("LeadInStart")
		EndIf
	EndFunction

	bool Function ResetScene(String asNewScene)
		UnregisterForUpdate()
		String currentScene = GetActiveScene()
		AddExperience(_Positions, currentScene, _StageHistory)
		If (asNewScene != currentScene)
			If (!SetActiveScene(asNewScene))
				Log("Unable to reset scene. New scene is invalid for this thread")
				return false
			EndIf
			SortAliasesToPositions()
		EndIf
		int[] strips_ = SexLabRegistry.GetStripDataA(currentScene, "")
		int[] sex_ = SexLabRegistry.GetPositionSexA(currentScene)
		int i = 0
		While (i < _Positions.Length)
			ActorAlias[i].TryLockAndUnpause()
			ActorAlias[i].ResetPosition(strips_[i], sex_[i])
			i += 1
		EndWhile
		StartStage(Utility.CreateStringArray(0), "")
		return true
	EndFunction

	Function PlayNext(int aiNextBranch)
		UnregisterForUpdate()
		SendThreadEvent("StageEnd")
		RunHook(Config.HOOKID_STAGEEND)
		PlayNextImpl(SexLabRegistry.BranchTo(GetActiveScene(), GetActiveStage(), aiNextBranch))
	EndFunction
	Function PlayNextImpl(String asNewStage)
		If (!asNewStage)
			Log("Invalid branch or previous stage is sink, ending scene")
			If(LeadIn)
				EndLeadIn()
			Else
				EndAnimation()
			EndIf
			return
		ElseIf(!Leadin)
			int ctype = sslSystemConfig.GetSettingInt("iClimaxType")
			If (ctype == Config.CLIMAXTYPE_LEGACY)
				If (SexLabRegistry.GetNodeType(GetActiveScene(), asNewStage) == 2)
					SendThreadEvent("OrgasmStart")
					TriggerOrgasm()
				EndIf
			ElseIf ((ctype == Config.CLIMAXTYPE_SCENE) || (!HasPlayer))
				int[] cactors = SexLabRegistry.GetClimaxingActors(GetActiveScene(), asNewStage)
				If (cactors.Length > 0)
					SendThreadEvent("OrgasmStart")
					int i = 0
					While (i < cactors.Length)
						ActorAlias[cactors[i]].DoOrgasm()
						i += 1
					EndWhile
				EndIf
			EndIf
		EndIf
		int[] strips = SexLabRegistry.GetStripDataA(GetActiveScene(), asNewStage)
		int i = 0
		While (i < _Positions.Length)
			ActorAlias[i].TryLockAndUnpause()
			ActorAlias[i].UpdateNext(strips[i])
			i += 1
		EndWhile
		StartStage(_StageHistory, asNewStage)
	EndFunction
	Function TriggerOrgasm()
		SendModEvent("SSL_ORGASM_Thread" + tid)
	EndFunction

	Function ResetStage()
		GoToStage(Stage)
	EndFunction

	Function StartStage(String[] asHistory, String asNextStageId)
		Log("Starting stage " + asNextStageId + " with history: " + asHistory, "StartStage()")
		SendThreadEvent("StageStart")
		RunHook(Config.HOOKID_STAGESTART)
		_StageHistory = AdvanceScene(asHistory, asNextStageId)
		UpdateOffsetSlidersDisplay()
		ReStartTimer()
	EndFunction

	; NOTE: This here counts from 1 instead of 0
	Function GoToStage(int ToStage)
		If (ToStage <= 1)
			ResetScene(GetActiveScene())
		ElseIf (ToStage > Stage)
			int idx = SelectNextStage(_ThreadTags)
			PlayNext(idx)
		ElseIf (ToStage == Stage)
			ReStartTimer()
		Else	; Skip stripping for already played stages
			int i = 0
			While (i < _Positions.Length)
				ActorAlias[i].TryLockAndUnpause()
				i += 1
			EndWhile
			StartStage(Utility.ResizeStringArray(_StageHistory, ToStage - 2), _StageHistory[ToStage - 1])
		EndIf
	EndFunction
	Function BranchTo(int aiNextBranch)
		PlayNext(aiNextBranch)
	EndFunction
	Function SkipTo(String asNextStage)
		PlayNextImpl(asNextStage)
	EndFunction

	Function ReStartTimer()
		_ForceAdvance = false
		_StageDuration = GetTimer()
		_StageTimer = _StageDuration
		If (!_ForceAdvance && !AutoAdvance)
			UpdateMenuTimerDisplay(0.0, 0.0)
		Else
			UpdateMenuTimerDisplay(_StageDuration, _StageTimer)
		EndIf
		RegisterForSingleUpdate(ANIMATING_UPDATE_INTERVAL)
	EndFunction

	Function UpdateTimer(float AddSeconds = 0.0)
		_StageTimer += AddSeconds
		_ForceAdvance = true
		UpdateMenuTimerDisplay(_StageDuration, _StageTimer)
	EndFunction

	Function PauseTimer(bool abEnable)
		_TimerPaused = abEnable
	EndFunction

	Function SetTimers(float[] SetTimers)
		If (!SetTimers.Length)
			Log("SetTimers() - Empty timers given.", "ERROR")
			return
		EndIf
		Timers = SetTimers
	EndFunction

	float Function GetTimer()
		float timer = SexLabRegistry.GetFixedLength(GetActiveScene(), GetActiveStage())
		If (!timer)
			return GetStageTimer(0)
		EndIf
		Log("GetTimer() - Fixed timer: " + timer)
		_ForceAdvance = true
		return timer
	EndFunction

	float Function GetStageTimer(int maxstage)
		int[] c = SexLabRegistry.GetClimaxingActors(GetActiveScene(), GetActiveStage())
		bool isClimaxStage = c.Length > 0
		If (isClimaxStage)
			return Timers[Timers.Length - 1]
		EndIf
		int lastTimerIdx = Timers.Length - 2
		If (_StageHistory.Length < lastTimerIdx)
			return Timers[_StageHistory.Length]
		EndIf
		return Timers[lastTimerIdx]
	Endfunction
	
	Event OnUpdate()
		If (_initialRealignTicks > 0)
			_initialRealignTicks -= 1
			If (_initialRealignTicks == 3 || _initialRealignTicks == 0)
				RealignActors()
			EndIf
		EndIf
		If (!_TimerPaused && (AutoAdvance || _ForceAdvance))
			_StageTimer -= ANIMATING_UPDATE_INTERVAL
			UpdateMenuTimerDisplay(_StageDuration, _StageTimer)
			If (_StageTimer <= 0)
				If !ThreadWaitsForOrgasm()
					GoToStage(_StageHistory.Length + 1)
					return
				Else
					string[] NewSceneStage = FindSimilarSceneStage()
					ResetScene(NewSceneStage[0])
					int NewStageNum = SexlabRegistry.GetAllStages(NewSceneStage[0]).Find(NewSceneStage[1])
					GoToStage(NewStageNum)
					Log("Skipped scene to " + SexlabRegistry.GetSceneName(NewSceneStage[0]) + " (Stage: " + NewStageNum + ")")
					return
				EndIf
			EndIf
		EndIf
		If (_SFXTimer > 0)
			_SFXTimer -= ANIMATING_UPDATE_INTERVAL
		Else
			bool[] interFlags = ListDetectedInteractionsInternal(None, None)
			bool penetration = interFlags[pVaginal] || interFlags[aVaginal] || interFlags[pAnal] || interFlags[aAnal]
			bool oral = interFlags[pOral] || interFlags[aOral] || interFlags[pDeepthroat] || interFlags[aDeepthroat]
			If Config.DebugMode
				Log("SFX Testing; penetration = " + penetration + " / oral = " + oral)
			EndIf
			If (oral && penetration)
				Config.SexMixedFX.Play(CenterRef)
			ElseIf (oral)
				Config.SuckingFX.Play(CenterRef)
			Else
				Config.SquishingFX.Play(CenterRef)
			EndIf
			_SFXTimer = Utility.RandomFloat(0.9, 1.3) * Config.SFXDelay
			If (_SFXTimer < 0.8)
				_SFXTimer = 0.8
			EndIf
		EndIf
		UpdateAnimationSpeed()
		RegisterForSingleUpdate(ANIMATING_UPDATE_INTERVAL)
	EndEvent

	Function CenterOnObject(ObjectReference CenterOn, bool resync = true)
		If (!CenterOn)
			return
		ElseIf (CenterOn as Actor && _Positions.Find(CenterOn as Actor) == -1)
			Form xMarker = Game.GetForm(0x3B)
			CenterOn = CenterOn.PlaceAtMe(xMarker, 1, false)
		EndIf
		SetFurnitureIgnored(false)
		If (!ReassignCenter(CenterOn))
			If (Config.HasThreadControl(Self) && InvalidCenterMsg.Show() == 1)
				Log("Cannot relocate center, end scene by player choice", "CenterOnObject")
				EndAnimation()
				return
			Else
				Log("Cannot relocate center, cancel relocation", "CenterOnObject")
			EndIf
		Else
			SendThreadEvent("ActorsRelocated")
		EndIf
		SetFurnitureIgnored(true)
	EndFunction

	Function RealignActors()
		AdvanceScene(_StageHistory, GetActiveStage())
	EndFunction
	
	Function ChangeActors(Actor[] NewPositions)
		SendThreadEvent("ActorChangeStart")
		Actor[] submissives = GetSubmissives()
		Actor[] argSub = PapyrusUtil.ActorArray(NewPositions.Length)
		int i = 0
		int ii = 0
		While (i < submissives.Length)
			If (NewPositions.Find(submissives[i]) > -1)
				argSub[ii] = submissives[i]
				ii += 1
			EndIf
			i += 1
		EndWhile
		If (ResetAnimation(NewPositions, argSub, none))
			SendThreadEvent("ActorChangeEnd")
		Else
			Log("Unable to change actors", "ChangeActorsEx()")
		EndIf
	EndFunction

	function EndLeadIn()
		If (!LeadIn)
			return
		EndIf
		LeadIn = false
		UnregisterForUpdate()
		SendThreadEvent("LeadInEnd")
		String[] nextSceneSet = GetCustomScenes()
		If (!nextSceneSet.Length)
			nextSceneSet = GetPrimaryScenes()
		EndIf
		If (!ResetScene(nextSceneSet[Utility.RandomInt(0, nextSceneSet.Length - 1)]))
			EndAnimation()
		EndIf
	endFunction

	Function Initialize()
		EndAnimation()
	EndFunction
	Function EndAnimation(bool Quickly = false)
		UnregisterForUpdate()
		If ((sslSystemConfig.GetSettingInt("iClimaxType") == Config.CLIMAXTYPE_LEGACY) && (!_QuickResetScenes))
			If (SexLabRegistry.GetNodeType(GetActiveScene(), GetActiveStage()) == 2)
				SendThreadEvent("OrgasmEnd")
			EndIf
		EndIf
		GoToState(STATE_END)
	EndFunction

	bool Function ResetAnimation(Actor[] akNewPositions, Actor[] akSubmissives, ObjectReference akCenter)
		EndAnimation()
		return ResetAnimation(akNewPositions, akSubmissives, akCenter)
	EndFunction

	bool Function ResetAnimationQuick(String asTagString = "")
		UnregisterForUpdate()
		String[] validScenes = SexLabRegistry.LookupScenesA(_Positions, asTagString, GetSubmissives(), _furniStatus, CenterRef)
		If ((validScenes.Length < 1) || (validScenes[0] == ""))
			ReStartTimer()
			return false
		EndIf
		_QuickResetScenes = true
		EndAnimation()
		return ResetAnimationQuick(asTagString)
	EndFunction

	Function OnSceneResetBySearch(String asQuerry)
		bool aiNewScenes = ResetAnimationQuick(asQuerry)
		If (!aiNewScenes)
			string asSelectedScene = SexLabRegistry.GetSceneByName(asQuerry)
			If (asSelectedScene)
				ResetScene(asSelectedScene)
			EndIf
		EndIf
	EndFunction

	int Function GetStatus()
		return STATUS_INSCENE
	EndFunction

	Event OnEndState()
		UnregisterForUpdate()
		SetFurnitureIgnored(false)
	EndEvent
EndState

Function RealignActors()
	Log("Cannot align actors outside the playing state", "RealignActors()")
EndFunction
Function ChangeActors(Actor[] NewPositions)
	Log("Cannot change actors outside the playing state", "ChangeActors()")
EndFunction
bool Function ResetScene(String asNewScene)
	Log("Cannot reset outside the playing state", "ResetScene()")
	return false
EndFunction
Function ResetStage()
	Log("Cannot reset outside the playing state", "ResetStage()")
EndFunction
Function StartStage(String[] asHistory, String asNextStageId)
	Log("Cannot reset outside the playing state", "StartStage()")
EndFunction
Function EndLeadIn()
	Log("Cannot end leadin outside the playing state", "EndLeadIn()")
EndFunction
Function PlayNext(int aiNextBranch)
	Log("Cannot play next branch outside the playing state", "PlayNext()")
EndFunction
Function PlayNextImpl(String asNewStage)
	Log("Cannot play next branch outside the playing state", "PlayNextImpl()")
EndFunction
Function GoToStage(int ToStage)
	Log("Cannot change playing branch outside the playing state", "GoToStage()")
EndFunction
Function TriggerOrgasm()
	Log("Cannot trigger orgasms outside the playing state", "TriggerOrgasm()")
EndFunction
Function ReStartTimer()
	Log("Cannot re/start timers outside of playing state", "ReStartTimer()")
EndFunction
Function UpdateTimer(float AddSeconds = 0.0)
	Log("Cannot update timers outside of playing state", "UpdateTimer()")
EndFunction
Function PauseTimer(bool abEnable)
	Log("Cannot pause timers outside of playing state", "PauseTimer()")
EndFunction
Function SetTimers(float[] SetTimers)
	Log("Cannot set timers outside of playing state", "SetTimers()")
EndFunction
float Function GetTimer()
	Log("timers are not defined outside of playing state", "GetTimer()")
	return 0.0
EndFunction
float Function GetStageTimer(int maxstage)
	Log("timers are not defined outside of playing state", "GetStageTimer()")
	return 0.0
Endfunction
Function BranchTo(int aiNextBranch)
	Log("Cannot branch to another stage while scene is not playing", "BranchTo()")
EndFunction
Function SkipTo(String asNextStage)
	Log("Cannot skip to another stage while scene is not playing", "SkipTo()")
EndFunction
Function OnSceneResetBySearch(String asQuerry)
	Log("Cannot reset scenes by search outside of playing state", "OnSceneResetBySearch()")
EndFunction

Function PlayStageAnimations()
	RealignActors()
EndFunction

; Set location for all _Positions on CenterAlias, incl offset, and play their respected animation. _Positions are assumed to be sorted by scene
String[] Function AdvanceScene(String[] asHistory, String asNextStageId) native
int Function SelectNextStage(String[] asThreadTags) native
bool Function SetActiveScene(String asScene) native
bool Function ReassignCenter(ObjectReference CenterOn) native
bool Function SetNextPermutation(Actor akActor) native
Function UpdatePlacement(Actor akActor) native
; Physics/SFX Related
bool Function IsCollisionRegistered() native
Function UnregisterCollision() native
int[] Function GetCollisionActions(Actor akPosition, Actor akPartner) native
bool Function HasCollisionAction(int aiType, Actor akPosition, Actor akPartner) native
Actor Function GetPartnerByAction(Actor akPosition, int aiType) native
Actor[] Function GetPartnersByAction(Actor akPosition, int aiType) native
Actor Function GetPartnerByActionRev(Actor akPartner, int aiType) native
Actor[] Function GetPartnersByActionRev(Actor akPartner, int aiType) native
float Function GetActionVelocity(Actor akPosition, Actor akPartner, int aiType) native

; ------------------------------------------------------- ;
; --- Thread END                                      --- ;
; ------------------------------------------------------- ;
;/
	The end state has 2 purposes:
	1) Reset all actors in the animation to their pre-animation status
	2) Reset the thread after a short buffer duration back to the idle state
/;

State Ending
	Event OnBeginState()
		If (_QuickResetScenes)
			return
		EndIf
		Config.DisableThreadControl(self as sslThreadController)
		(self as sslThreadController).TogglePrismaMenu(-1)
		SendModEvent("SSL_CLEAR_Thread" + tid, "", 1.0)
		MoveActorsAwayFromPlayer()
		UnregisterCollision()
		If(IsObjectiveDisplayed(0))
			SetObjectiveDisplayed(0, False)
		EndIf
		If (Config.HideHUD)
			SexLabUtil.HideElementsGameHUD(false)
		EndIf
		UpdateAllEncounters()
		int i = 0
		While (i < ActorAlias.Length)
			If (ActorAlias[i].GetState() == ActorAlias[i].STATE_IDLE)
				i += 1
			Else
				Utility.Wait(0.05)
			EndIf
		EndWhile
		SendThreadEvent("AnimationEnding")
		SendThreadEvent("AnimationEnd")
		RunHook(Config.HOOKID_END)
		; Cant use default OnUpdate() event as the previous state could leak a registration into this one here
		; any attempt to prevent this leak without artificially slowing down the code have failed
		; 0.1 gametime = 6ig minutes = 360 ig seconds = 360 / 20 rt seconds = 18 rt seconds with default timescale
		RegisterForSingleUpdateGameTime(0.1)
	EndEvent

	bool Function ResetAnimation(Actor[] akNewPositions, Actor[] akSubmissives, ObjectReference akCenter)
		UnregisterForUpdateGameTime()
		If (!akCenter)
			akCenter = CenterRef
		EndIf
		String[] validScenes
		If (akCenter == CenterRef)
			validScenes = SexLabRegistry.ValidateScenesA(GetPrimaryScenes(), akNewPositions, "", akSubmissives)
		EndIf
		If (validScenes.Length == 0)
			String threadTags = PapyrusUtil.StringJoin(_ThreadTags)
			validScenes = SexLabRegistry.LookupScenesA(akNewPositions, threadTags, akSubmissives, _furniStatus, akCenter)
			If (validScenes.Length == 0)
				Log("Unable to find a valid scene for the given actors", "ResetAnimation()")
				RegisterForSingleUpdateGameTime(0.1)
				return false
			EndIf
		EndIf
		int i = 0
		While(i < ActorAlias.Length)
			ActorAlias[i].Initialize()
			i += 1
		EndWhile
		_Positions = PapyrusUtil.ActorArray(0)
		DestroyInstance()
		GoToState(STATE_SETUP)
		SetScenes(validScenes)
		return AddActorsA(akNewPositions, akSubmissives) && StartThread()
	EndFunction

	bool Function ResetAnimationQuick(String asTagString = "")
		UnregisterForUpdateGameTime()
		(self as sslThreadController).TogglePrismaMenu(-1)
		String[] validScenes = SexLabRegistry.LookupScenesA(_Positions, asTagString, GetSubmissives(), _furniStatus, CenterRef)
		DestroyInstance()
		GoToState(STATE_SETUP)
		SetScenes(validScenes)
		If (!StartThread())
			_QuickResetScenes = false
			GoToState(STATE_END)
			return false
		EndIf
		GoToState(STATE_PLAYING)
		_PrimaryScenes = GetPrimaryScenes()
		_ThreadTags = SexLabRegistry.GetCommonTags(_PrimaryScenes)
		string activeScene = GetActiveScene()
		Log("Thread validated, playing animation: " + activeScene + ", " + SexLabRegistry.GetSceneName(activeScene), "StartThread()")
		(self as sslThreadController).TogglePrismaMenu(1)
		StartStage(Utility.CreateStringArray(0), "")
		_QuickResetScenes = false
		return true
	EndFunction

	Event OnUpdateGameTime()
		Initialize()
	EndEvent
	Event OnEndState()
		UnregisterForUpdateGameTime()
		Log("Returning to thread pool...")
	EndEvent

	int Function GetStatus()
		return STATUS_ENDING
	EndFunction
EndState

; ------------------------------------------------------- ;
; --- State Independent                               --- ;
; ------------------------------------------------------- ;
;/
	Functions whichs behavior is not dependent on the currently playing state
/;

sslActorAlias Function PickAlias(Actor ActorRef)
	int i
	while i < 5
		if ActorAlias[i].ForceRefIfEmpty(ActorRef)
			return ActorAlias[i]
		endIf
		i += 1
	endWhile
	return none
EndFunction

Function SetFurnitureIgnored(bool disabling = true)
	If (CenterRef as Actor)
		return
	EndIf
	CenterRef.SetDestroyed(disabling)
	CenterRef.BlockActivation(disabling)
	CenterRef.SetNoFavorAllowed(disabling)
EndFunction

int Function IndexTravelComplex(int curIdx, bool abReverse = false, Actor akSkip = None)
	int posLen = _Positions.Length
	If (posLen > 1)
		int step = 1
		If (abReverse)
			step = -1
		EndIf
		int newIdx = (curIdx + step)
		int i = 0
		While (i < posLen)
			If (newIdx >= posLen)
				newIdx = 0
			ElseIf (newIdx < 0)
				newIdx = (posLen - 1)
			EndIf
			If (newIdx != curIdx) && (akSkip == None || _Positions[newIdx] != akSkip)
				return newIdx
			EndIf
			newIdx += step
			i += 1
		EndWhile
	EndIf
	return curIdx
EndFunction

Function MoveActorsAwayFromPlayer(bool MovePlayer = false)
	float adjOffset = 35.0
	int moveDir = -1
	int i = 0
	while (i < _Positions.Length)
		Actor curActor = _Positions[i]
		If (curActor != PlayerRef)
			If (curActor.GetDistance(PlayerRef) < 50.0)
				If (MovePlayer)
					PlayerRef.SetPosition(PlayerRef.X + 70, PlayerRef.Y, PlayerRef.Z)
					return
				Else
					moveDir += 1
				EndIf
				float newX = curActor.X
				float newY = curActor.Y
				If (moveDir == 0)
					newY += adjOffset
				ElseIf (moveDir == 1)
					newX += adjOffset
				ElseIf (moveDir == 2)
					newY -= adjOffset
				ElseIf (moveDir == 3)
					newX -= adjOffset
				EndIf
				curActor.SetPosition(newX, newY, curActor.Z)
			EndIf
		EndIf
		i += 1
	EndWhile
EndFunction

; ------------------------------------------------------- ;
; --- Function Declarations                           --- ;
; ------------------------------------------------------- ;
;/
	Most functions used to manage animations have a unique behavior depending on the currently active state
	The below block defines such functions. All of these functions will be overwritten for every state where there
	is reason to implement them
/;

Function CenterOnObject(ObjectReference CenterOn, bool resync = true)
	Log("CenterOnObject(), Function called from invalid state: " + GetState())
EndFunction
Function EndAnimation(bool Quickly = false)
	Log("EndAnimation(), Function called from invalid state: " + GetState())
EndFunction
bool Function ResetAnimation(Actor[] akNewPositions, Actor[] akSubmissives, ObjectReference akCenter)
	Log("ResetAnimation(), Function called from invalid state: " + GetState())
	return false
EndFunction
bool Function ResetAnimationQuick(String asTagString = "")
	Log("ResetAnimationQuick(), Function called from invalid state: " + GetState())
	return false
EndFunction
Function PrepareDone()
	Log("PrepareDone(), Function called from invalid state: " + GetState())
EndFunction
Function AliasLockDone()
	Log("AliasLockDone(), Function called from invalid state: " + GetState())
EndFunction
Function AnimationStart()
	Log("AnimationStart(), Function called from invalid state: " + GetState())
EndFunction

; ------------------------------------------------------- ;
; --- Actor Alias                                     --- ;
; ------------------------------------------------------- ;
;/
	QoL accessors for the specified Actor
/;

int Function FindSlot(Actor ActorRef)
	return _Positions.Find(ActorRef)
EndFunction

sslActorAlias Function ActorAlias(Actor ActorRef)
	return PositionAlias(FindSlot(ActorRef))
EndFunction

sslActorAlias Function PositionAlias(int Position)
	If(Position < 0 || Position >= _Positions.Length)
		return none
	EndIf
	return ActorAlias[Position]
EndFunction

Function SortAliasesToPositions()
	_Positions = GetPositions()
	int i = 0
	While (i < ActorAlias.Length)
		Actor position = ActorAlias[i].GetReference() as Actor
		If (position)
			int inActorArray = _Positions.Find(position)
			sslActorAlias tmp = ActorAlias[inActorArray]
			ActorAlias[inActorArray] = ActorAlias[i]
			ActorAlias[i] = tmp
		EndIf
		i += 1
	EndWhile
EndFunction

Sound Function GetAliasSound(sslActorAlias akThis, String asVoice, int aiStrength)
	return sslBaseVoice.GetSoundObject(asVoice, aiStrength, GetActiveScene(), ActorAlias.Find(akThis), akThis.OpenMouth)
EndFunction
Sound Function GetAliasOrgasmSound(sslActorAlias akThis, String asVoice)
	return sslBaseVoice.GetOrgasmSound(asVoice, GetActiveScene(), ActorAlias.Find(akThis), akThis.OpenMouth)
EndFunction

; ------------------------------------------------------- ;
; --- Statistics	                                    --- ;
; ------------------------------------------------------- ;
;/
	Statistics related functions
/;

; Called at the end of an active scene, shortly before its swapped or the thread ends
Function AddExperience(Actor[] akPositions, String asActiveStage, String[] asStageHistory) native
; Only call this once per actor, before _Positions are cleared. Only updates actors own statistics (no encounter updates)
Function UpdateStatistics(Actor akActor, Actor[] akPositions,  String asActiveScene, String[] asPlayedStages, float afTimeInThread) native
Function RequestStatisticUpdate(Actor akPosition, float afRegisteredAt)	; Called when one of the _Positions is cleared
	float timeregistered = SexLabUtil.GetCurrentGameRealTime() - afRegisteredAt
	UpdateStatistics(akPosition, _Positions, GetActiveScene(), _StageHistory, timeregistered)
EndFunction

; int Property ENC_Any 			  = 0	AutoReadOnly Hidden
; int Property ENC_Victim		  = 1	AutoReadOnly Hidden
; int Property ENC_Assault	  = 2	AutoReadOnly Hidden
; int Property ENC_Submissive	= 3	AutoReadOnly Hidden
; int Property ENC_Dominant	  = 4	AutoReadOnly Hidden

Function UpdateEncounters(Actor akActor, int i = 0)
	int consent = 2 * IsConsent() as int
	bool submissive = ActorAlias(akActor).IsVictim()
	While (i < _Positions.Length)
		If (_Positions[i] != akActor)
			bool subB = ActorAlias[i].IsVictim()
			int type
			If (subB == submissive)
				type = 0
			ElseIf (submissive)
				type = 1 + consent
			Else
				type = 2 + consent
			EndIf
			SexLabStatistics.AddEncounter(akActor, _Positions[i], type)
		EndIf
		i += 1
	EndWhile
EndFunction
Function UpdateAllEncounters()
	int i = 0
	While (i < _Positions.Length)
		UpdateEncounters(_Positions[i], i + 1)
		i += 1
	EndWhile
EndFunction

; -------------------------------------------------- ;
; --- PRISMA UI                                  --- ;
; -------------------------------------------------- ;

bool Property AdjustStage               Auto Hidden
float Property MenuScaleMult			Auto Hidden

bool Property GameHUD                   Auto Hidden
bool Property OverlayAnimSpeed          Auto Hidden
bool Property OverlayEnjBars            Auto Hidden
bool Property OverlayOffsetAdjust       Auto Hidden
bool Property OverlaySceneSelector      Auto Hidden
bool Property OverlayThreadConfig       Auto Hidden
bool Property OverlayVisibilityControl  Auto Hidden

int Property PRISMA_SCENE_MENU          = 0 AutoReadOnly
int Property OVERLAY_ANIM_SPEED         = 1 AutoReadOnly
int Property OVERLAY_ENJOYMENT_BARS     = 2 AutoReadOnly
int Property OVERLAY_OFFSET_ADJUST      = 3 AutoReadOnly
int Property OVERLAY_SCENE_SELECTOR     = 4 AutoReadOnly
int Property OVERLAY_THREAD_CONFIG      = 5 AutoReadOnly
int Property OVERLAY_VISIBILITY_CONTROL = 6 AutoReadOnly

Function TryPrismaOverlaysStart()
	If (GetStatus() != STATUS_INSCENE)
		Log("Cannot initialize Prisma overlays outside of playing state, TryPrismaOverlayInit()")
		return
	EndIf
	RefreshPrismaPropertiesState("Get")
	PrismaOverlayInitAndShow(PRISMA_SCENE_MENU)
	PrismaOverlayInitAndShow(OVERLAY_ANIM_SPEED)
	PrismaOverlayInitAndShow(OVERLAY_ENJOYMENT_BARS)
	PrismaOverlayInitAndShow(OVERLAY_OFFSET_ADJUST)
	PrismaOverlayInitAndShow(OVERLAY_SCENE_SELECTOR)
	PrismaOverlayInitAndShow(OVERLAY_THREAD_CONFIG)
	PrismaOverlayInitAndShow(OVERLAY_VISIBILITY_CONTROL)
EndFunction

Function TryPrismaOverlaysClose()
	PrismaOverlayDestroyImpl(PRISMA_SCENE_MENU)
	RefreshPrismaPropertiesState("Set")
EndFunction

Function PrismaOverlayInitAndShow(int aiOverlayIndex)
	bool abProceed = False
	If (aiOverlayIndex == PRISMA_SCENE_MENU)
		abProceed = True
	ElseIf (aiOverlayIndex == OVERLAY_ANIM_SPEED)
		abProceed = Config.OverlayAnimSpeed
	ElseIf (aiOverlayIndex == OVERLAY_ENJOYMENT_BARS)
		abProceed = (Config.OverlayEnjBars && (Config.ClimaxType == Config.CLIMAXTYPE_SLSO))
	ElseIf (aiOverlayIndex == OVERLAY_OFFSET_ADJUST)
		abProceed = Config.OverlayOffsetAdjust
	ElseIf (aiOverlayIndex == OVERLAY_SCENE_SELECTOR)
		abProceed = Config.OverlaySceneSelector
	ElseIf (aiOverlayIndex == OVERLAY_THREAD_CONFIG)
		abProceed = Config.OverlayThreadConfig
	ElseIf (aiOverlayIndex == OVERLAY_VISIBILITY_CONTROL)
		abProceed = Config.OverlayVisibilityControl		
	EndIf
	If (abProceed)
		PrismaOverlayInitImpl(aiOverlayIndex)
	EndIf
EndFunction

Function RefreshPrismaPropertiesState(string asMode)
	If (asMode == "Get")
		AdjustStage = Config.AdjustStage
		MenuScaleMult = Config.MenuScaleMult
		GameHUD = !Config.HideHUD
		OverlayAnimSpeed = Config.OverlayAnimSpeed
		OverlayEnjBars = Config.OverlayEnjBars
		OverlayOffsetAdjust = Config.OverlayOffsetAdjust
		OverlaySceneSelector = Config.OverlaySceneSelector
		OverlayThreadConfig = Config.OverlayThreadConfig
		OverlayVisibilityControl = Config.OverlayVisibilityControl
	ElseIf (asMode == "Set")
		Config.AdjustStage = AdjustStage
		Config.MenuScaleMult = MenuScaleMult
		Config.HideHUD = !GameHUD
		Config.OverlayAnimSpeed = OverlayAnimSpeed
		Config.OverlayEnjBars = OverlayEnjBars
		Config.OverlayOffsetAdjust = OverlayOffsetAdjust
		Config.OverlaySceneSelector = OverlaySceneSelector
		Config.OverlayThreadConfig = OverlayThreadConfig
		Config.OverlayVisibilityControl = OverlayVisibilityControl
	EndIf
EndFunction

Function PrismaOverlayInitImpl(int aiOverlayIndex) native
Function PrismaOverlayDestroyImpl(int aiOverlayIndex) native
Function TogglePrismaFocusImpl() native

Function UpdateMenuTimerDisplay(float afDuration, float afTime) native
Function UpdateOffsetSlidersDisplay() native ;call on stage change
Function EnjBarsChangeHighlightedPartner(Actor akActor) native

; -------------------------------------------------- ;
; --- Interactions Info - INTERNAL               --- ;
; -------------------------------------------------- ;

bool[] Function ListDetectedInteractionsInternal(Actor akPosition, Actor akPartner = None)
	If (IsInteractionRegistered())
		return ListDetectedPhysicsInteractionsInternal(akPosition, akPartner)
	EndIf
	;COMEBACK: Re-assess the need for the fallback with new typing update
	If (Config.FallbackToTagsForDetection && HasSceneTag("PosTagged"))
		return ListDetectedPosTagsInteractionsInternal(akPosition)
    EndIf
	;If all else fails, returns pAnal, which has the highest enj factor
	bool[] better_than_nothing = Utility.CreateBoolArray(SUPPORTED_INTER_COUNT, False)
	better_than_nothing[pAnal] = True
	return better_than_nothing 
EndFunction

bool[] Function ListDetectedPhysicsInteractionsInternal(Actor akPosition, Actor akPartner)
	bool[] phyActive = Utility.CreateBoolArray(SUPPORTED_INTER_COUNT, False)
	phyActive[aAnimObjFace] = HasCollisionAction(CTYPE_AnimObjFace, akPartner, akPosition)
	phyActive[pAnimObjFace] = HasCollisionAction(CTYPE_AnimObjFace, akPosition, akPartner)
	phyActive[bKissing] = HasCollisionAction(CTYPE_Kissing, akPosition, akPartner)
	phyActive[aSuckingToes] = HasCollisionAction(CTYPE_SuckingToes, akPosition, akPartner)
	phyActive[pSuckingToes] = HasCollisionAction(CTYPE_SuckingToes, akPartner, akPosition)
	phyActive[aFacial] = HasCollisionAction(CTYPE_Facial, akPartner, akPosition)
	phyActive[pFacial] = HasCollisionAction(CTYPE_Facial, akPosition, akPartner)
	phyActive[aGrinding] = HasCollisionAction(CTYPE_Grinding, akPartner, akPosition)
	phyActive[pGrinding] = HasCollisionAction(CTYPE_Grinding, akPosition, akPartner)
	phyActive[aHandJob] = HasCollisionAction(CTYPE_HandJob, akPosition, akPartner)
	phyActive[pHandJob] = HasCollisionAction(CTYPE_HandJob, akPartner, akPosition)
	phyActive[aFootJob] = HasCollisionAction(CTYPE_FootJob, akPosition, akPartner)
	phyActive[pFootJob] = HasCollisionAction(CTYPE_FootJob, akPartner, akPosition)
	;phyActive[aBoobJob] = False 	; awaiting support
	;phyActive[pBoobJob] = False	; awaiting support
	phyActive[aLickingShaft] = HasCollisionAction(CTYPE_LickingShaft, akPosition, akPartner)
	phyActive[pLickingShaft] = HasCollisionAction(CTYPE_LickingShaft, akPartner, akPosition)
	phyActive[aOral] = HasCollisionAction(CTYPE_Oral, akPosition, akPartner)
	phyActive[pOral] = HasCollisionAction(CTYPE_Oral, akPartner, akPosition)
	phyActive[aDeepthroat] = HasCollisionAction(CTYPE_Deepthroat, akPosition, akPartner)
	phyActive[pDeepthroat] = HasCollisionAction(CTYPE_Deepthroat, akPartner, akPosition)
	phyActive[aSkullfuck] = HasCollisionAction(CTYPE_Skullfuck, akPartner, akPosition)
	phyActive[pSkullfuck] = HasCollisionAction(CTYPE_Skullfuck, akPosition, akPartner)
	phyActive[aVaginal] = HasCollisionAction(CTYPE_Vaginal, akPartner, akPosition)
	phyActive[pVaginal] = HasCollisionAction(CTYPE_Vaginal, akPosition, akPartner)
	phyActive[aAnal] = HasCollisionAction(CTYPE_Anal, akPartner, akPosition)
	phyActive[pAnal] = HasCollisionAction(CTYPE_Anal, akPosition, akPartner)
	return phyActive
EndFunction

; --- Tags Fallback 
bool[] Function ListDetectedPosTagsInteractionsInternal(Actor akPosition)
	string[] posTags = SexLabRegistry.GetPositionTags(GetActiveScene(), GetActiveStage(), GetPositionIdx(akPosition))
	bool[] tagActive = Utility.CreateBoolArray(SUPPORTED_INTER_COUNT, False)
	string[] interTypes = Config.NameAllInteractions
	int i = 0
	int len = posTags.Length
	While (i < len)
		int tagIdx = interTypes.Find(posTags[i])
		If (tagIdx != -1)
            tagActive[tagIdx] = true
        EndIf
		i += 1
	EndWhile
	return tagActive
EndFunction

; ------------------------------------------------------- ;
; --- ORGASM FX                                       --- ;
; ------------------------------------------------------- ;

Function ApplyCumFX(Actor SourceRef)
	If (!Config.UseCum)
        return
    EndIf
	int i = 0
	While (i < _Positions.Length)
		Actor TargetRef = _Positions[i]
		; Skip the source's own slot and any not-yet-loaded target, but keep scanning the
		; rest of the positions - a bare "return" here would abort the whole loop and deny
		; cum FX (and the SexLabApplyCumFX event) to every position after this one.
		If (TargetRef != SourceRef && TargetRef.Is3DLoaded() && TargetRef.GetParentCell() && TargetRef.GetParentCell().IsAttached())
			bool[] interFlags = ListDetectedInteractionsInternal(SourceRef, TargetRef)
			;variable names are from SourceRef's (male/futa) perspective
			;bool pHandJob_ = interFlags[pHandJob]
			;bool pFootJob_ = interFlags[pFootJob]
			;bool pBoobJob_ = interFlags[pBoobJob]
			;bool aFacial_ = interFlags[pBoobJob]
			;bool aSkullfuck_ = interFlags[pBoobJob]
			bool pOral_ = interFlags[pOral]
			bool pDeepthroat_ = interFlags[pDeepthroat]
			bool pLickingShaft_ = interFlags[pLickingShaft]
			bool aVaginal_ = interFlags[aVaginal]
			bool aGrinding_ = interFlags[aGrinding]
			bool aAnal_ = interFlags[aAnal]
			bool any_oral = pOral_ || pDeepthroat_ || pLickingShaft_
			; Comeback: reasses need for fallback
			If (!any_oral && !aVaginal_ && !aGrinding_ && !aAnal_)
				any_oral = IsOral()
				aVaginal_ = IsVaginal()
				aAnal_ = IsAnal()
			EndIf
			Log("ApplyCumFX(): Source [" + SexLabUtil.ActorName(SourceRef) + "] Target [" + SexLabUtil.ActorName(TargetRef) + "] CumFX_Types [O: " + any_oral + ", V: " + (aVaginal_ || aGrinding_) + ", A: " + aAnal_ + "]")
			int aiType = -2
			If (aVaginal_ || aGrinding_)
				aiType = ActorLib.FX_VAGINAL
			ElseIf (aAnal_)
				aiType = ActorLib.FX_ANAL
			ElseIf (any_oral)
				aiType = ActorLib.FX_ORAL
			EndIf
			If (aiType != -2)
				ActorLib.AddCumFx(TargetRef, aiType)
				Int handle = ModEvent.Create("SexLabApplyCumFX")
				ModEvent.PushForm(handle, TargetRef)
				ModEvent.PushForm(handle, SourceRef)
				ModEvent.PushInt(handle, aiType)
				ModEvent.Send(handle)
			EndIf
		EndIf
		i += 1
	EndWhile
EndFunction

; ------------------------------------------------------- ;
; --- Thread Hooks & Events                           --- ;
; ------------------------------------------------------- ;
;/
	Interface to send blocking and non blocking hooks
/;

Function RunHook(int aiHookID)
	Config.RunHook(aiHookID, self)
EndFunction

Function SendThreadEvent(string HookEvent)
	Log(HookEvent, "Event Hook")
	SetupThreadEvent(HookEvent)
	int i = 0
	While (i < _Hooks.Length)
		SetupThreadEvent(HookEvent + "_" + _Hooks[i])
		i += 1
	EndWhile
EndFunction
Function SetupThreadEvent(string HookEvent)
	int eid = ModEvent.Create("Hook"+HookEvent)
	if eid
		ModEvent.PushInt(eid, thread_id)
		ModEvent.PushBool(eid, HasPlayer)
		ModEvent.Send(eid)
	endIf
	SendModEvent(HookEvent, thread_id)
EndFunction

; ------------------------------------------------------- ;
; --- Initialization                                  --- ;
; ------------------------------------------------------- ;
;/
	Functions for re/initialization
/;

; Only called on framework re/initialization through ThreadSlots.psc
Function SetTID(int id)
	thread_id = id
	Log(self, "Setup")
	int i = 0
	While(i < ActorAlias.Length)
		ActorAlias[i].Setup()
		i += 1
	EndWhile
	Initialize()
EndFunction

; Reset this thread to base status
Function Initialize()
	UnregisterForUpdate()
	UnregisterForUpdateGameTime()
	Config.DisableThreadControl(self as sslThreadController)
	int i = 0
	While(i < ActorAlias.Length)
		ActorAlias[i].Initialize()
		i += 1
	EndWhile
	CenterAlias.TryToClear()
	_Positions = PapyrusUtil.ActorArray(0)
	_StageHistory = Utility.CreateStringArray(0)
	_furniStatus = FURNI_ALLOW
	LeadIn = false
	_ThreadTags = Utility.CreateStringArray(0)
	_ContextTags = Utility.CreateStringArray(0)
	_Hooks = Utility.CreateStringArray(0)
	_AnimationSpeedBase = 1.0
	_TimerPaused = false
	_QuickResetScenes = false
	; Prisma state-awareness

	; Enter thread selection pool
	DestroyInstance()
	GoToState(STATE_IDLE)
EndFunction

Function DestroyInstance() native

; ------------------------------------------------------- ;
; --- Logging                                         --- ;
; ------------------------------------------------------- ;
;/
	Generic logging utility
/;

Function Log(string msg, string src = "")
	msg = "Thread[" + thread_id + "] " + msg
	sslLog.Log(msg)
EndFunction

Function Fatal(string msg, string src = "", bool halt = true)
	String errMsg = "Thread["+thread_id+"] - FATAL - "
	If (src != "")
		errMsg += src + ": "
	EndIf
	sslLog.Error(errMsg + msg, true)
	If (halt)
		Initialize()
	EndIf
EndFunction

; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;
; --------------------------------------------------------------------------------------- ;
;  ███████╗███╗   ██╗     ██╗ ██████╗ ██╗   ██╗███╗     ╔███╗███████╗███╗   ██╗████████╗  ;
;  ██╔════╝████╗  ██║     ██║██╔═══██╗╚██╗ ██╔╝████╗   ╔████║██╔════╝████╗  ██║╚══██╔══╝  ;
;  █████╗  ██╔██╗ ██║     ██║██║   ██║ ╚████╔╝ ██╔██╗ ╔██╔██║█████╗  ██╔██╗ ██║   ██║     ;
;  ██╔══╝  ██║╚██╗██║██   ██║██║   ██║  ╚██╔╝  ██║╚██ ██╔╝██║██╔══╝  ██║╚██╗██║   ██║     ;
;  ███████╗██║ ╚████║╚█████╔╝╚██████╔╝   ██║   ██║ ╚███╔╝ ██║███████╗██║ ╚████║   ██║     ;
;  ╚══════╝╚═╝  ╚═══╝ ╚════╝  ╚═════╝    ╚═╝   ╚═╝  ╚══╝  ╚═╝╚══════╝╚═╝  ╚═══╝   ╚═╝     ;
; --------------------------------------------------------------------------------------- ;
; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;
; preferably move these to a separate script sslEnjoymentUtils

Int Property CONSENT_CONNONSUB 		= 0 AutoReadOnly Hidden
Int Property CONSENT_NONCONNONSUB 	= 1 AutoReadOnly Hidden
Int Property CONSENT_CONSUB 		= 2 AutoReadOnly Hidden
Int Property CONSENT_NONCONSUB 		= 3 AutoReadOnly Hidden

AssociationType Property SpouseAssocation Auto
Faction Property PlayerMarriedFaction Auto

; -------------------------------------------------- ;
; --- Interactions Factors                       --- ;
; -------------------------------------------------- ;

float Function CalculateInteractionFactor(Actor akPosition, bool[] interActive)
	float factorTotal = 0.25
	float[] factorValues = sslSystemConfig.GetEnjoymentFactors()
	int len = interActive.Length
	int i = 0
	While (i < len)
		If (interActive[i])
			; velFactor: [Range: 1.0 to 2.0]
			; factorValue: [Default: 1 to 12] [Adjusted: 0.2 to 2.4]
			; factorType: [Result: 0.2 to 4.8]
			float velFactor = CalcInterVelocityFactor(akPosition, i)
			float adjustedFactor = 0.25 + (factorValues[i] / 5.0)
			factorTotal += (adjustedFactor * velFactor)
			;Log("InterFactor: TYPE: " + i + ", Value: " + factorValues[i] + ", Adjusted: " + adjustedFactor)
		EndIf
		i += 1
	EndWhile
	return factorTotal
EndFunction

float Function CalcInterVelocityFactor(Actor akActor, int interType)
	;Velocity is simply too unpredictable in its current implementation
	If (!IsInteractionRegistered())
		return 1.5
	EndIf
	int CType = 0
	If (interType == aVaginal || interType == pVaginal)
		CType = CTYPE_Vaginal
	ElseIf (interType == aAnal || interType == pAnal)
		CType = CTYPE_Anal
	ElseIf (interType == aOral || interType == pOral)
		CType = CTYPE_Oral
	ElseIf (interType == aGrinding || interType == pGrinding)
		CType = CTYPE_Grinding
	ElseIf (interType == aDeepthroat || interType == pDeepthroat)
		CType = CTYPE_Deepthroat
	ElseIf (interType == aSkullfuck || interType == pSkullfuck)
		CType = CTYPE_Skullfuck
	ElseIf (interType == aLickingShaft || interType == pLickingShaft)
		CType = CTYPE_LickingShaft
	ElseIf (interType == aFootJob || interType == pFootJob)
		CType = CTYPE_FootJob
	ElseIf (interType == aHandJob || interType == pHandJob)
		CType = CTYPE_HandJob
	ElseIf (interType == bKissing)
		CType = CTYPE_Kissing
	ElseIf (interType == aAnimObjFace || interType == pAnimObjFace)
		CType = CTYPE_AnimObjFace
	ElseIf (interType == aSuckingToes || interType == pSuckingToes)
		CType = CTYPE_SuckingToes
	EndIf
	;calculate velocity multiplier... have seen velActual upto 0.097075
	;after adjustments: 0.01-->1.11, 0.05-->1.55, 0.09-->1.99
	float velActual = Math.Abs(GetActionVelocity(akActor, None, CType))
	float velAdjusted = 1.0 + (velActual * 11.0)
	return velAdjusted
EndFunction

; -------------------------------------------------- ;
; --- Thread Info                                --- ;
; -------------------------------------------------- ;

int Function IdentifyConsentSubStatus()
	int ConSubStatus = CONSENT_CONNONSUB
	If GetSubmissives().Length == 0
		If !IsConsent()
			ConSubStatus = CONSENT_NONCONNONSUB
		EndIf
	Else
		If IsConsent()
			ConSubStatus = CONSENT_CONSUB
		Else
			ConSubStatus = CONSENT_NONCONSUB
		EndIf
	EndIf
	return ConSubStatus
EndFunction

bool[] Function CheckActiveHomoTypes()
	int PosCount = _Positions.Length
	int[] SexCount = sslActorLibrary.CountSexAll(_Positions)
	bool[] HomoTypes = Utility.CreateBoolArray(5, False)
	HomoTypes[0] = (PosCount == 1) ;solo
	If (PosCount > 1)
		HomoTypes[1] = (SexCount[0] + SexCount[3] == PosCount) ;males
		HomoTypes[2] = (SexCount[1] + SexCount[4] == PosCount) ;females
		HomoTypes[3] = (SexCount[2] == PosCount) ;futa
		HomoTypes[4] = (SexCount[0] + SexCount[2] + SexCount[3] == PosCount) ;futa with males
	EndIf
	return HomoTypes
EndFunction

bool Function CrtMaleHugePP()
	bool HugePP = False
	If sslActorLibrary.CountCrtMale(_Positions) > 0
		int CreMalePos = -1
		int i = 0
		While i < _Positions.Length
			If _Positions[i] != None
				int gender = GetNthPositionSex(i)
				If gender == 3
					CreMalePos = i
				EndIf
			EndIf
			i += 1
		EndWhile
		If CreMalePos > -1
			string CreRacekey = SexlabRegistry.GetRaceKey(_Positions[CreMalePos])
			If CreRacekey ==  "bears" || CreRacekey ==  "chaurus" || CreRacekey ==  "chaurushunters" || CreRacekey ==  "chaurusreapers" || CreRacekey ==  "dragons" || CreRacekey ==  "dwarvencenturions" || CreRacekey ==  "frostatronach" || CreRacekey ==  "gargoyles" || CreRacekey ==  "giants" || CreRacekey ==  "giantspiders" || CreRacekey ==  "horses" || CreRacekey ==  "largespiders" || CreRacekey ==  "lurkers" || CreRacekey ==  "mammoths" || CreRacekey ==  "sabrecats" || CreRacekey ==  "trolls" || CreRacekey ==  "werewolves"
				HugePP = true
			EndIf
		EndIf
	EndIf
	return HugePP
EndFunction

bool Function ThreadWaitsForOrgasm()
	If Config.InternalEnjoymentEnabled && (GetLegacyStagesCount() - GetLegacyStageNum() == 1)
		int i = 0
		While i < _Positions.Length
			If ActorAlias[i].WaitForOrgasm()
				return true
			EndIf
			i += 1
		EndWhile
	EndIf
	return false
EndFunction

string[] Function FindSimilarSceneStage()
	string[] ret = Utility.CreateStringArray(2, "")
	string PlayingScene = GetActiveScene()
	string PlayingStage = GetActiveStage()
	bool[] PlayingStageType = CheckSpecificStageTags(PlayingScene, PlayingStage)
	string[] AvailableScenes = GetPlayingScenes()
	int CountScenes = AvailableScenes.Length
	int i = 0	
	While (i < CountScenes)
		string AvailableScene = AvailableScenes[i]
		If AvailableScene != PlayingScene
			string[] AvailableStages = SexLabRegistry.GetAllStages(AvailableScene)
			int CountStages = AvailableStages.Length
			int n = 3 ;skip first two stages
			While (n < CountStages)
				string AvailableStage = AvailableStages[n]
				bool[] AvailableStageType = CheckSpecificStageTags(AvailableScene, AvailableStage)
				If (AvailableStageType == PlayingStageType)
					ret[0] = AvailableScene
					ret[1] = AvailableStage
					return ret
				EndIf
				n += 1
			EndWhile
		EndIf
		i += 1
	EndWhile
	return ret
EndFunction

bool[] Function CheckSpecificStageTags(string asScene, string asStage)
	bool[] ret = new bool[13]
	ret[0] = SexLabRegistry.IsStageTag(asScene, asStage, "Oral")
	ret[1] = SexLabRegistry.IsStageTag(asScene, asStage, "Vaginal")
	ret[2] = SexLabRegistry.IsStageTag(asScene, asStage, "Anal")
	ret[3] = SexLabRegistry.IsStageTag(asScene, asStage, "Furniture")
	ret[4] = SexLabRegistry.IsStageTag(asScene, asStage, "Toys")
	ret[5] = SexLabRegistry.IsStageTag(asScene, asStage, "Magic")
	ret[6] = SexLabRegistry.IsStageTag(asScene, asStage, "Lying")
	ret[7] = SexLabRegistry.IsStageTag(asScene, asStage, "Standing")
	ret[8] = SexLabRegistry.IsStageTag(asScene, asStage, "Forced")
	ret[9] = SexLabRegistry.IsStageTag(asScene, asStage, "Unconscious")
	ret[10] = SexLabRegistry.IsStageTag(asScene, asStage, "PosTagged")
	ret[11] = SexLabRegistry.IsStageTag(asScene, asStage, "PosFast")
	ret[12] = SexLabRegistry.IsStageTag(asScene, asStage, "PosSlow")
	return ret
EndFunction

; -------------------------------------------------- ;
; --- Best Relation                              --- ;
; -------------------------------------------------- ;

bool Function CheckLoverAssociation(Actor akPosition, Actor TargetRef)
	return ((akPosition == PlayerRef && TargetRef.IsInFaction(PlayerMarriedFaction)) \
	|| akPosition.HasAssociation(SpouseAssocation, TargetRef) \
	|| akPosition.GetRelationshipRank(TargetRef) >= 4)
EndFunction

bool Function ActorIsWithLover(Actor akPosition)
	If (_Positions.Length <= 1)
		return False
	EndIf
	int i = 0
	While (i < _Positions.Length)
		If (_Positions[i] != akPosition)
			If (CheckLoverAssociation(akPosition, _Positions[i]))
				return True
			EndIf
		EndIf
		i += 1
	EndWhile
	return False
EndFunction

; ---------------------------------------------- ;
; --- Enjoyment Game                         --- ;
; ---------------------------------------------- ;

Function GameRaiseEnjoyment(Actor akTarget)
	If (SexLabUtil.IsGodModeEnabled())
		AdjustEnjoyment(akTarget, Config.GameEnjAdjAmount)
		return
	ElseIf (PlayerRef.GetActorValue("Stamina") > Config.GameStaminaCost)
		PlayerRef.DamageActorValue("Stamina", Config.GameStaminaCost)
		AdjustEnjoyment(akTarget, Config.GameEnjAdjAmount)
	EndIf
EndFunction

Function GameHoldback(Actor akTarget)
	If (SexLabUtil.IsGodModeEnabled())
		AdjustEnjoyment(akTarget, -Config.GameEnjAdjAmount)
		return
	ElseIf (PlayerRef.GetActorValue("Magicka") > Config.GameMagickaCost)
		PlayerRef.DamageActorValue("Magicka", Config.GameMagickaCost)
		AdjustEnjoyment(akTarget, -Config.GameEnjAdjAmount)
	EndIf
EndFunction

Function ProcessEnjGameArg(String arg, Actor akPartner, bool abAdjustTarget)
	Actor akTarget = PlayerRef ;change self/player enj
	If (abAdjustTarget)
		akTarget = akPartner ;change partner enj
	EndIf
	If (arg == "Magicka")
		GameHoldback(akTarget)
	ElseIf (arg == "Stamina")
		If ((Config.GameRequiredOnHighEnj) && (GetEnjoyment(PlayerRef) > 80))
			ActorAlias[GetPositionIdx(PlayerRef)].InitRaiseEnjAttempt()
		Else
			GameRaiseEnjoyment(akTarget)
		EndIf
	EndIf
EndFunction

Function OnRaiseEnjAttemptResult(int aiSuccess)
	sslActorAlias ref = ActorAlias(PlayerRef)
	If (!ref)
		return
	EndIf ; Called by Thread::PrismaUI::EnjoymentBars
	return ref.OnRaiseEnjAttemptResult(aiSuccess as bool)
EndFunction

Function EnjBasedSkipToLastStage(bool abSkip)
	if !abSkip
		return
	EndIf
	bool NotEndStageScenario = (GetLegacyStageNum() < GetLegacyStagesCount())
	bool SoloDuoScenario = (_Positions.Length == 1 || _Positions.Length == 2) 
	If (NotEndStageScenario && SoloDuoScenario)
		Log("Skipping to sink stage, EnjBasedSkipToLastStage()")
		SkipTo(SexLabRegistry.GetEndingStages(GetActiveScene())[0])
		_StageTimer -= (GetTimer() / 2)
		UpdateBaseSpeed(0.8)
	EndIf
EndFunction

; -------------------------------------------------- ;
; --- AnimSpeed                                  --- ;
; -------------------------------------------------- ;

float _AnimationSpeedBase

Function UpdateBaseSpeed(float afBaseSpeed)
	_AnimationSpeedBase = afBaseSpeed
EndFunction

float Function CalcInstThreadAnimSpeed()
	float animSpeed = 0.0
	If (Config.SetAnimSpeedByEnjoyment)
		int i = 0
		While (i < _Positions.Length)
			float actorSpeed = _AnimationSpeedBase * PapyrusUtil.ClampFloat(GetEnjoyment(_Positions[i]) as float / 90, 0.8, 1.2)
			If (actorSpeed > animSpeed)
				animSpeed = actorSpeed
			EndIf
			i += 1
		EndWhile
	Else
		animSpeed = _AnimationSpeedBase
	EndIf
	return animSpeed
EndFunction

Function UpdateAnimationSpeed()
	float animSpeed = CalcInstThreadAnimSpeed()
	SetAnimationPlaybackSpeed(animSpeed)
EndFunction

Function SetAnimationPlaybackSpeed(float afAnimationSpeed) native
Function AnimSpeedOverlayInit() native
Function AnimSpeedOverlayClose() native

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

Package Property DoNothingPackage Auto	; used previously in the alias scripts

sslThreadLibrary Property ThreadLib Hidden
	sslThreadLibrary Function Get()
		return Game.GetFormFromFile(0xD62, "SexLab.esm") as sslThreadLibrary
	EndFunction
EndProperty
sslAnimationSlots Property AnimSlots Hidden
	sslAnimationSlots Function Get()
		return Game.GetFormFromFile(0x639DF, "SexLab.esm") as sslAnimationSlots
	EndFunction
EndProperty
sslCreatureAnimationSlots Property CreatureSlots Hidden
	sslCreatureAnimationSlots Function Get()
		return Game.GetFormFromFile(0x664FB, "SexLab.esm") as sslCreatureAnimationSlots
	EndFunction
EndProperty
sslActorLibrary property ActorLib Hidden
  sslActorLibrary Function Get()
    return Game.GetFormFromFile(0xD62, "SexLab.esm") as sslActorLibrary
  EndFunction
EndProperty

Actor[] Property Positions Hidden
	Actor[] Function Get()
		return PapyrusUtil.RemoveActor(_Positions, none)
	EndFunction
EndProperty
int Property ActorCount Hidden
	int Function Get()
		return _Positions.Length
	EndFunction
EndProperty
Actor[] property Victims Hidden
	Actor[] Function Get()
		GetAllVictims()
	EndFunction
EndProperty

String[] Property Scenes Hidden
	String[] Function get()
		return GetPlayingScenes()
	EndFunction
EndProperty

int[] Property Genders Hidden
	int[] Function Get()
		int[] g = ActorLib.GetGendersAll(_Positions)
		int[] ret = new int[4]
		ret[0] = PapyrusUtil.CountInt(g, 0)
		ret[1] = PapyrusUtil.CountInt(g, 1)
		ret[2] = PapyrusUtil.CountInt(g, 2)
		ret[3] = PapyrusUtil.CountInt(g, 3)
		return ret
	endFunction
	Function Set(int[] aSet)
	EndFunction
EndProperty
int Property Males hidden
	int Function get()
		return Genders[0]
	EndFunction
EndProperty
int Property Females hidden
	int Function get()
		return Genders[1]
	EndFunction
EndProperty

bool Property HasCreature hidden
	bool Function get()
		return Creatures > 0
	EndFunction
EndProperty
int Property Creatures hidden
	int Function get()
		return Genders[2] + Genders[3]
	EndFunction
EndProperty
int Property MaleCreatures hidden
	int Function get()
		return Genders[2]
	EndFunction
EndProperty
int Property FemaleCreatures hidden
	int Function get()
		return Genders[3]
	EndFunction
EndProperty

string[] Property AnimEvents Hidden
	String[] Function Get()
		return SexLabRegistry.GetAnimationEventA(GetActiveScene(), GetActiveStage())
	EndFunction
EndProperty

string Property AdjustKey Hidden
	String Function Get()
		return "Global"
	EndFunction
EndProperty

bool[] Property IsType Hidden	; [0] IsAggressive, [1] IsVaginal, [2] IsAnal, [3] IsOral, [4] IsLoving, [5] IsDirty, [6] HadVaginal, [7] HadAnal, [8] HadOral
	bool[] Function Get()
		bool[] ret = new bool [9]
		ret[0] = IsAggressive
		ret[1] = IsVaginal
		ret[2] = IsAnal
		ret[3] = IsOral
		ret[4] = IsLoving
		ret[5] = IsDirty
		int i = 0
		While (i < _StageHistory.Length - 1)
			ret[6] = ret[6] || SexlabRegistry.IsStageTag(GetActiveScene(), _StageHistory[i], "Vaginal")
			ret[7] = ret[7] || SexlabRegistry.IsStageTag(GetActiveScene(), _StageHistory[i], "Anal")
			ret[8] = ret[8] || SexlabRegistry.IsStageTag(GetActiveScene(), _StageHistory[i], "Oral")
			i += 1
		EndWhile
		return ret
	EndFUnction
	Function Set(bool[] aSet)
	EndFunction
EndProperty
bool Property IsAggressive hidden
	bool Function get()
		return !IsConsent()
	endfunction
	Function set(bool value)
		SetConsent(value)
	EndFunction
EndProperty
bool Property IsVaginal hidden
	bool Function get()
		return SexlabRegistry.IsSceneTag(GetActiveScene(), "Vaginal")
	endfunction
	Function set(bool value)
	EndFunction
EndProperty
bool Property IsAnal hidden
	bool Function get()
		return SexlabRegistry.IsSceneTag(GetActiveScene(), "Anal")
	endfunction
	Function set(bool value)
	EndFunction
EndProperty
bool Property IsOral hidden
	bool Function get()
		return SexlabRegistry.IsSceneTag(GetActiveScene(), "Oral")
	endfunction
	Function set(bool value)
	EndFunction
EndProperty
bool Property IsLoving hidden
	bool Function get()
		return SexlabRegistry.IsSceneTag(GetActiveScene(), "Loving")
	endfunction
	Function set(bool value)
	EndFunction
EndProperty
bool Property IsDirty hidden
	bool Function get()
		return SexlabRegistry.IsSceneTag(GetActiveScene(), "Dirty") || SexlabRegistry.IsSceneTag(GetActiveScene(), "Forced")
	endfunction
	Function set(bool value)
	EndFunction
EndProperty

int[] Property BedStatus Hidden
	int[] Function Get()
		int[] ret = new int[2]
		ret[0] = _furniStatus - 1
		ret[1] = BedTypeID
	EndFunction
	Function Set(int[] aSet)
	EndFunction
EndProperty
ObjectReference Property BedRef Hidden
	ObjectReference Function Get()
		If (sslThreadLibrary.IsBed(CenterRef))
			return CenterRef
		EndIf
		return none
	EndFunction
	Function Set(ObjectReference aSet)
	EndFunction
EndProperty
int Property BedTypeID hidden
	int Function get()
		return sslThreadLibrary.GetBedTypeImpl(CenterRef)
	EndFunction
EndProperty
bool Property UsingBed hidden
	bool Function get()
		return BedRef != none
	EndFunction
EndProperty
bool Property UsingBedRoll hidden
	bool Function get()
		return BedTypeID == 1
	EndFunction
EndProperty
bool Property UsingSingleBed hidden
	bool Function get()
		return BedTypeID == 2
	EndFunction
EndProperty
bool Property UsingDoubleBed hidden
	bool Function get()
		return BedTypeID == 3
	EndFunction
EndProperty
bool Property UseNPCBed hidden
	bool Function get()
		int NPCBed = Config.NPCBed
		return NPCBed == 2 || (NPCBed == 1 && (Utility.RandomInt(0, 1) as bool))
	EndFunction
EndProperty

Actor property VictimRef hidden
	Actor Function Get()
		Actor[] vics = GetAllVictims()
		If(vics.Length)
			return vics[0]
		EndIf
		return none
	EndFunction
	Function Set(Actor ActorRef)
		sslActorAlias vic = ActorAlias(ActorRef)
		If(!vic)
			return
		EndIf
		vic.SetVictim(true)
	EndFunction
EndProperty
Actor[] Function GetAllVictims()
	return GetSubmissives()
EndFunction
Function SetVictim(Actor ActorRef, bool Victimize = true)
	SetIsSubmissive(ActorRef, Victimize)
EndFunction

float[] Property CenterLocation Hidden
	float[] Function Get()
		float[] ret = new float[6]
		ret[0] = CenterRef.GetPositionX()
		ret[1] = CenterRef.GetPositionY()
		ret[2] = CenterRef.GetPositionZ()
		ret[3] = CenterRef.GetAngleX()
		ret[4] = CenterRef.GetAngleY()
		ret[5] = CenterRef.GetAngleZ()
		return ret
	EndFunction
	Function Set(float[] aSet)
	EndFunction
EndProperty

sslBaseAnimation Function GetSetAnimationLegacyCast(String asScene)
	int[] sexes = SexLabRegistry.GetPositionSexA(asScene)
	If (sexes.Find(3) > -1 || sexes.Find(4) > -1)
		return CreatureSlots.GetSetAnimation(asScene)
	Else
		return AnimSlots.GetSetAnimation(asScene)
	EndIf
EndFunction
sslBaseAnimation Property Animation Hidden
	sslBaseAnimation Function Get()
		return GetSetAnimationLegacyCast(GetActiveScene())
	EndFunction
	Function Set(sslBaseAnimation aSet)
		SetAnimationImpl(aSet)
	EndFunction
EndProperty
sslBaseAnimation Property StartingAnimation Hidden
	sslBaseAnimation Function Get()
		return GetSetAnimationLegacyCast(GetActiveScene())
	EndFunction
	Function Set(sslBaseAnimation aSet)
		SetStartingAnimation(aSet)
	EndFunction
EndProperty
sslBaseAnimation[] Property Animations hidden
	sslBaseAnimation[] Function get()
		return GetAnimationsLegacyCast(Scenes)
	EndFunction
EndProperty

Function LogConsole(String asReport)
	String msg = "Thread[" + thread_id + "] - " + asReport
	sslLog.Log(msg, true)
EndFunction
Function LogRedundant(String asFunction)
	Debug.MessageBox("[SexLab]\nState '" + GetState() + "'; Function '" + asFunction + "' is an internal function that is no longer used.\nThis is most likely due to a file conflict. Ensure no other mod overwrites SexLab files.\nSee Papyrus Log for more information.")
	Debug.TraceStack("[SexLab] Redundant function called: " + asFunction)
EndFunction

Function AddAnimation(sslBaseAnimation AddAnimation, bool ForceTo = false)
	If(!AddAnimation)
		return
	EndIf
	AddScene(AddAnimation.Registry)
EndFunction
Function SetAnimation(int aid = -1)
	if aid < 0 || aid >= Animations.Length
		aid = Utility.RandomInt(0, (Animations.Length - 1))
	endIf
	SetAnimationImpl(Animations[aid])
EndFunction
Function SetAnimationImpl(sslBaseAnimation akAnimation)
	ResetScene(akAnimation.Registry)
EndFunction

Function SetVoice(Actor ActorRef, sslBaseVoice Voice, bool ForceSilent = false)
	sslActorAlias ref = ActorAlias(ActorRef)
	If (!ref)
		return
	EndIf
	ref.SetVoice(Voice, ForceSilent)
EndFunction
sslBaseVoice Function GetVoice(Actor ActorRef)
	sslActorAlias ref = ActorAlias(ActorRef)
	If (!ref)
		return none
	EndIf
	return ref.GetVoice()
EndFunction

Function SetExpression(Actor ActorRef, sslBaseExpression Expression)
	sslActorAlias ref = ActorAlias(ActorRef)
	If (!ref)
		return
	EndIf
	ref.SetExpression(Expression)
EndFunction
sslBaseExpression Function GetExpression(Actor ActorRef)
	sslActorAlias ref = ActorAlias(ActorRef)
	If (!ref)
		return none
	EndIf
	return ref.GetExpression()
EndFunction

bool function AddTag(string Tag)
	return false
endFunction
bool function RemoveTag(string Tag)
	return false
endFunction
bool function ToggleTag(string Tag)
	return false
endFunction
bool function AddTagConditional(string Tag, bool AddTag)
	return false
endFunction
bool Function CheckTags(String[] CheckTags, bool RequireAll = true, bool Suppress = false)
	int i = 0
	While (i < CheckTags.Length)
		If (HasTag(CheckTags[i]))
			If (!RequireAll || Suppress)
				return !Suppress
			EndIf
		EndIf
		i += 1
	EndWhile
	return !Suppress
EndFunction
String[] Function AddString(string[] ArrayValues, string ToAdd, bool RemoveDupes = true)
	if ToAdd != ""
		string[] Output = ArrayValues
		if !RemoveDupes || Output.length < 1
			return PapyrusUtil.PushString(Output, ToAdd)
		elseIf Output.Find(ToAdd) == -1
			int i = Output.Find("")
			if i != -1
				Output[i] = ToAdd
			else
				Output = PapyrusUtil.PushString(Output, ToAdd)
			endIf
		endIf
		return Output
	endIf
	return ArrayValues
EndFunction

Sound Property SoundFX Hidden
	Sound Function Get()
		return none
	EndFunction
	Function Set(Sound aSet)
	EndFunction
EndProperty

function SyncEvent(int id, float WaitTime)
endFunction
function SyncEventDone(int id)
endFunction
Function SyncDone()
EndFunction
Function RefreshDone()
EndFunction
Function ResetDone()
EndFunction
Function StripDone()
EndFunction
Function OrgasmDone()
EndFunction
Function StartupDone()
EndFunction

sslBaseAnimation[] Function GetAnimationsLegacyCast(String[] asScenes)
	sslBaseAnimation[] ret = AnimSlots.AsBaseAnimation(asScenes)
	If (!ret.Length)
		ret = CreatureSlots.AsBaseAnimation(asScenes)
	EndIf
	return ret
EndFunction
sslBaseAnimation[] Function GetForcedAnimations()
	return GetAnimationsLegacyCast(GetCustomScenes())
EndFunction
sslBaseAnimation[] Function GetAnimations()
	return GetAnimationsLegacyCast(GetPrimaryScenes())
EndFunction
sslBaseAnimation[] Function GetLeadAnimations()
	return GetAnimationsLegacyCast(GetLeadInScenes())
EndFunction

int Function GetHighestPresentRelationshipRank(Actor ActorRef)
	if _Positions.Length <= 1
		If(ActorRef == _Positions[0])
			return 0
		Else
			return ActorRef.GetRelationshipRank(_Positions[0])
		EndIf
	endIf
	int out = -4 ; lowest possible
	int i = _Positions.Length
	while i > 0 && out < 4
		i -= 1
		if _Positions[i] != ActorRef
			int rank = ActorRef.GetRelationshipRank(_Positions[i])
			if rank > out
				out = rank
			endIf
		endIf
	endWhile
	return out
EndFunction

int Function GetLowestPresentRelationshipRank(Actor ActorRef)
	if _Positions.Length <= 1
		If(ActorRef == _Positions[0])
			return 0
		Else
			return ActorRef.GetRelationshipRank(_Positions[0])
		EndIf
	endIf
	int out = 4 ; highest possible
	int i = _Positions.Length
	while i > 0 && out > -4
		i -= 1
		if _Positions[i] != ActorRef
			int rank = ActorRef.GetRelationshipRank(_Positions[i])
			if rank < out
				out = rank
			endIf
		endIf
	endWhile
	return out
EndFunction

string Function GetHook()
	If (_Hooks.Length)
		return _Hooks[0]
	EndIf
	return ""
EndFunction

Function Action(string FireState)
endfunction
Function FireAction()
EndFunction
Function EndAction()
EndFunction

Function InitShares()
EndFunction

int Function FilterAnimations()
	LogRedundant("FilterAnimations")
	return 0
EndFunction

Function HookAnimationStarting()
EndFunction
Function HookStageStart()
EndFunction
Function HookStageEnd()
EndFunction
Function HookAnimationEnd()
EndFunction

Function SendTrackedEvent(Actor ActorRef, string Hook = "")
	sslThreadLibrary.SendTrackingEvents(ActorRef, Hook, thread_id)
EndFunction
Function SetupActorEvent(Actor ActorRef, string Callback)
	sslThreadLibrary.MakeTrackingEvent(ActorRef, Callback, thread_id)
EndFunction

Function UpdateAdjustKey()
EndFunction

String Function Key(string Callback)
	return ""	; "SSL_" + thread_id + "_" + Callback
EndFunction
Function QuickEvent(string Callback)
	; ModEvent.Send(ModEvent.Create(Key(Callback)))
endfunction

Race Property CreatureRef Hidden
	Race Function Get()
		Keyword npc = Keyword.GetKeyword("ActorTypeNPC")
		int i = 0
		While(i < _Positions.Length)
			If(!_Positions[i].HasKeyword(npc))
				return _Positions[i].GetRace()
			EndIf
			i += 1
		EndWhile
		return none
	EndFunction
	Function Set(Race aSet)
	EndFunction
EndProperty

float[] Property RealTime Hidden
	float[] Function Get()
		float[] ret = new float[1]
		ret[0] = SexLabUtil.GetCurrentGameRealTime()
		return ret
	EndFunction
	Function Set(float[] aSet)
	EndFunction
EndProperty

bool Property FastEnd auto hidden

Actor Function GetPlayer()
	return PlayerRef
EndFunction
Actor Function GetVictim()
	return VictimRef
EndFunction

Function RemoveFade()
	if HasPlayer
		Config.RemoveFade()
	endIf
EndFunction
Function ApplyFade()
	if HasPlayer
		Config.ApplyFade()
	endIf
EndFunction

bool Function IsPlayerActor(Actor ActorRef)
	return ActorRef == PlayerRef
EndFunction
bool Function IsPlayerPosition(int Position)
	return Position == _Positions.Find(PlayerRef)
EndFunction
int Function GetPosition(Actor ActorRef)
	return _Positions.Find(ActorRef)
EndFunction
int Function GetPlayerPosition()
	return _Positions.Find(PlayerRef)
EndFunction

Function DisableRagdollEnd(Actor ActorRef = none, bool disabling = true)
EndFunction

Function SetStartAnimationEvent(Actor ActorRef, string EventName = "IdleForceDefaultState", float PlayTime = 0.1)
EndFunction
Function SetEndAnimationEvent(Actor ActorRef, string EventName = "IdleForceDefaultState")
EndFunction

bool Function CenterOnBed(bool AskPlayer = true, float Radius = 750.0)
	bool InStart = GetStatus() == STATUS_SETUP
	If (_furniStatus == FURNI_DISALLOW)
		return false
	ElseIf (InStart && !HasPlayer && Config.NPCBed == 0 || HasPlayer && Config.AskBed == 0)
		return false
	EndIf
	int i = 0
	While (i < _Positions.Length)
		ObjectReference furni = _Positions[i].GetFurnitureReference()
		If (furni)
			int BedType = sslThreadLibrary.GetBedTypeImpl(furni)
			If (BedType > 0 && (_Positions.Length < 4 || BedType != 2))
				CenterOnObject(furni)
				return true
			EndIf
		EndIf
		i += 1
	EndWhile
 	ObjectReference FoundBed
	Radius *= _furniStatus	; Double radius is preferring a furniture
	If (HasPlayer)
		If (!InStart || Config.AskBed == 1 || (Config.AskBed == 2 && (!IsVictim(PlayerRef) || UseNPCBed)))
			FoundBed = ThreadLib.GetNearestUnusedBed(PlayerRef, Radius)
			AskPlayer = AskPlayer && (!InStart || !(Config.AskBed == 2 && IsVictim(PlayerRef)))
		EndIf
	ElseIf (UseNPCBed)
		FoundBed = ThreadLib.GetNearestUnusedBed(_Positions[0], Radius)
	EndIf
	; Found a bed AND EITHER forced use OR don't care about players choice OR or player approved
	if FoundBed && (_furniStatus == FURNI_PREFER || (!AskPlayer || (AskPlayer && (Config.UseBed.Show() as bool))))
		CenterOnObject(FoundBed)
		return true
	endIf
	return false
EndFunction

Function CenterOnCoords(float LocX = 0.0, float LocY = 0.0, float LocZ = 0.0, float RotX = 0.0, float RotY = 0.0, float RotZ = 0.0, bool resync = true)
	Form xMarker = Game.GetForm(0x3B)
	ObjectReference new_center = CenterRef.PlaceAtMe(xMarker)
	new_center.SetAngle(RotX, RotY, RotZ)
	new_center.SetPosition(LocX, LocY, LocZ)
	CenterOnObject(new_center, resync)
EndFunction

int Function AreUsingFurniture(Actor[] ActorList)	
	int i = 0
	While(i < ActorList.Length)
		ObjectReference ref = ActorList[i].GetFurnitureReference()
		If(ref)
			return sslThreadLibrary.GetBedTypeImpl(ref)
		EndIf
		i += 1
	EndWhile
	return -1
EndFunction

; Function used to find and set the currently active Timer array
; Timers property now does this explicetly on each access
Function ResolveTimers()
EndFunction

Function SetStrip(Actor ActorRef, bool[] StripSlots)
	if StripSlots && StripSlots.Length == 33
		ActorAlias(ActorRef).OverrideStrip(StripSlots)
	else
		Log("Malformed StripSlots bool[] passed, must be 33 length bool array, "+StripSlots.Length+" given", "ERROR")
	endIf
EndFunction
Function SetNoStripping(Actor ActorRef)
	if ActorRef
		bool[] StripSlots = new bool[33]
		sslActorAlias Slot = ActorAlias(ActorRef)
		if Slot
			Slot.OverrideStrip(StripSlots)
			Slot.DoUndress = false
		endIf
	endIf
EndFunction

bool Function UseLimitedStrip()
	If (LeadIn)
		return true
	EndIf
	bool excplicit = HasTag("Penetration") || HasTag("DoublePenetration") || HasTag("TripplePenetration") || HasTag("Fingering") || HasTag("Fisting")
	excplicit = excplicit || HasTag("Tribadism") || HasTag("Grinding") || HasTag("Boobjob") || HasTag("Buttjob")
	return Config.LimitedStrip && !excplicit
EndFunction

Function DisableUndressAnimation(Actor ActorRef = none, bool disabling = true)
	if ActorRef && _Positions.Find(ActorRef) != -1
		ActorAlias(ActorRef).DoUndress = !disabling
	else
		ActorAlias[0].DoUndress = !disabling
		ActorAlias[1].DoUndress = !disabling
		ActorAlias[2].DoUndress = !disabling
		ActorAlias[3].DoUndress = !disabling
		ActorAlias[4].DoUndress = !disabling
	endIf
EndFunction
Function DisableRedress(Actor ActorRef = none, bool disabling = true)
	if ActorRef && _Positions.Find(ActorRef) != -1
		ActorAlias(ActorRef).DoRedress = !disabling
	else
		ActorAlias[0].DoRedress = !disabling
		ActorAlias[1].DoRedress = !disabling
		ActorAlias[2].DoRedress = !disabling
		ActorAlias[3].DoRedress = !disabling
		ActorAlias[4].DoRedress = !disabling
	endIf
EndFunction
Function DisablePathToCenter(Actor ActorRef = none, bool disabling = true)
	if ActorRef && _Positions.Find(ActorRef) != -1
		ActorAlias(ActorRef).DisablePathToCenter(disabling)
	else
		ActorAlias[0].DisablePathToCenter(disabling)
		ActorAlias[1].DisablePathToCenter(disabling)
		ActorAlias[2].DisablePathToCenter(disabling)
		ActorAlias[3].DisablePathToCenter(disabling)
		ActorAlias[4].DisablePathToCenter(disabling)
	endIf
EndFunction
Function ForcePathToCenter(Actor ActorRef = none, bool forced = true)
	if ActorRef && _Positions.Find(ActorRef) != -1
		ActorAlias(ActorRef).ForcePathToCenter(forced)
	else
		ActorAlias[0].ForcePathToCenter(forced)
		ActorAlias[1].ForcePathToCenter(forced)
		ActorAlias[2].ForcePathToCenter(forced)
		ActorAlias[3].ForcePathToCenter(forced)
		ActorAlias[4].ForcePathToCenter(forced)
	endIf
EndFunction

Function StopTranslations()
	int i = 0
	While (i < _Positions.Length)
		_Positions[i].StopTranslation()
		i += 1
	EndWhile
EndFunction

Function StartTranslations()
	int i = 0
	While (i < _Positions.Length)
		; Some creatures (such as horses or spiders) may tilt unnaturaly during scenes, 
		; forcing them in translation ensures they are angled correctly
		; translating only Y axis, as it does not have any visual effect on the actor
		If (ActorAlias[i].GetSex() > 2)
			float x = _Positions[i].X
			float y = _Positions[i].Y
			float z = _Positions[i].Z
			float ax = _Positions[i].GetAngleX()
			float ay = _Positions[i].GetAngleY() + 0.001
			float az = _Positions[i].GetAngleZ()
			_Positions[i].TranslateTo(x, y, z, ax, ay, az, 1.0, 0.0001)
		EndIf
		i += 1
	EndWhile
EndFunction

Function SetAnimations(sslBaseAnimation[] AnimationList)
	If (AnimationList.Length && AnimationList.Find(none) == -1)
		SetScenes(sslBaseAnimation.AsSceneIDs(AnimationList))
	EndIf
EndFunction
Function ClearAnimations()
	ClearScenes()
EndFunction
Function SetForcedAnimations(sslBaseAnimation[] AnimationList)
	If (AnimationList.Length && AnimationList.Find(none) == -1)
		SetForcedScenes(sslBaseAnimation.AsSceneIDs(AnimationList))
	EndIf
EndFunction
Function ClearForcedAnimations()
	ClearForcedScenes()
EndFunction
Function SetLeadAnimations(sslBaseAnimation[] AnimationList)
	if AnimationList.Length && AnimationList.Find(none) == -1
		SetLeadInScenes(sslBaseAnimation.AsSceneIDs(AnimationList))
	endIf
EndFunction
Function ClearLeadAnimations()
	ClearLeadInScenes()
EndFunction
Function SetStartingAnimation(sslBaseAnimation FirstAnimation)
	SetStartingScene(FirstAnimation.Registry)
EndFunction
Function DisableBedUse(bool disabling = true)
	SetFurnitureStatus((!disabling) as int)
EndFunction
Function SetBedFlag(int flag = 0)
	SetFurnitureStatus(flag + 1)	; New Status is [0, 2] instead of [-1, 1]
EndFunction
Function SetBedding(int flag = 0)
	SetBedFlag(flag)
EndFunction

int Function GetPain(Actor ActorRef)
	sslActorAlias ref = ActorAlias(ActorRef)
	If (!ref)
		return 0
	EndIf
	return ref.GetPain()
EndFunction

bool property DisableOrgasms hidden
	bool Function Get()
		bool ret = false
		int i = 0
		While (i < _Positions.Length && !ret)
			ret = !ActorAlias[i].IsOrgasmAllowed()
			i += 1
		EndWhile
		return ret
	EndFunction
	Function Set(bool abDisable)
		int i = 0
		While (i < _Positions.Length)
			ActorAlias[i].DisableOrgasm(abDisable)
			i += 1
		EndWhile
	EndFunction
EndProperty
Function DisableAllOrgasms(bool OrgasmsDisabled = true)
	DisableOrgasms = OrgasmsDisabled
EndFunction
bool Function NeedsOrgasm(Actor ActorRef)
	return ActorAlias(ActorRef).NeedsOrgasm()
EndFunction

; These are empty funcs on alias script. Correct equipping of strapons should be internal and function autonomous
Function EquipStrapon(Actor ActorRef)
	ActorAlias(ActorRef).EquipStrapon()
EndFunction
Function UnequipStrapon(Actor ActorRef)
	ActorAlias(ActorRef).UnequipStrapon()
EndFunction

bool Function PregnancyRisk(Actor ActorRef, bool AllowFemaleCum = false, bool AllowCreatureCum = false)
	return CanBeImpregnated(ActorRef, true, AllowFemaleCum, AllowCreatureCum)
EndFunction

float[] Property SkillBonus Hidden
	{[0] Foreplay, [1] Vaginal, [2] Anal, [3] Oral, [4] Pure, [5] Lewd}
	float[] Function Get()
		float[] ret = new float[6]
		return ret
	EndFunction
EndProperty
float[] Property SkillXP hidden
	{[0] Foreplay, [1] Vaginal, [2] Anal, [3] Oral, [4] Pure, [5] Lewd}
	float[] Function Get()
		float[] ret = new float[6]
		return ret
	EndFunction
EndProperty
Function SetBonuses()
EndFunction
Function RecordSkills()
	AddExperience(_Positions, GetActiveScene(), _StageHistory)
endfunction