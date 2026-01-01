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

string Function GetInteractionString(Actor ActorRef)
	sslActorAlias ref = ActorAlias(ActorRef)
	If (!ref)
		return 0
	EndIf
	return ref.GetInteractionString()
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
; --- Physics					                                --- ;
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
Package Property DoNothingPackage Auto	; used in the alias scripts
Message Property InvalidCenterMsg Auto	; Invalid new cewnter -> [0: Keep Old Center, 1: End Scene]

; Constants
int Property POSITION_COUNT_MAX = 5 AutoReadOnly

String Property STATE_IDLE 		= "Unlocked" AutoReadOnly
String Property STATE_SETUP 	= "Making" AutoReadOnly
String Property STATE_SETUP_M	= "Making_M" AutoReadOnly
String Property STATE_PLAYING = "Animating" AutoReadOnly
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

bool Function IsOwningSceneMenu() native
bool Function TryOpenSceneMenu() native
bool Function TryCloseSceneMenu() native
Function TryUpdateMenuTimer(float afTime) native

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
    Log("Added " + ActorList + " to thread", "AddActors()")
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
    Log("Added " + ActorList + " to thread", "AddActorsA()")
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
			return none
		EndIf
		GoToState(STATE_SETUP_M)
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
		String activeScene = GetActiveScene()
		LeadIn = LeadIn && _LeadInScenes.Find(activeScene) > -1
		SortAliasesToPositions()
		PrepareDone()
		If (_CustomScenes.Length)
			_ThreadTags = SexLabRegistry.GetCommonTags(_CustomScenes)
		Else
			_ThreadTags = SexLabRegistry.GetCommonTags(_PrimaryScenes)
		EndIf
		Log("Thread validated, playing animation: " + activeScene + ", " + SexLabRegistry.GetSceneName(activeScene), "StartThread()")
		SendThreadEvent("AnimationStarting")
	EndEvent

	; Invoked n times by Aliases and once by StartThread, then continue to next state
	Function PrepareDone()
		If (_prepareAsyncCount < _Positions.Length)
			_prepareAsyncCount += 1
			Log("Prepare done called " + _prepareAsyncCount + "/" + (_Positions.Length + 1) + " times")
			return
		ElseIf (HasPlayer)
			If(IsVictim(PlayerRef) && Config.DisablePlayer)
				AutoAdvance = true
			Else
				AutoAdvance = Config.AutoAdvance
				Config.GetThreadControl(self as sslThreadController)
			EndIf
			If (sslSystemConfig.GetSettingInt("iUseFade") > 0)
				Config.ApplyFade()
			EndIf
		Else
			AutoAdvance = true
			If (Config.ShowInMap && PlayerRef.GetDistance(CenterRef) > 750)
				SetObjectiveDisplayed(0, True)
			EndIf
		EndIf
		GoToState(STATE_PLAYING)
	EndFunction
	
	Function EndAnimation(bool Quickly = false)
		_prepareAsyncCount = -2147483648
		Initialize()
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

bool _ForceAdvance		; Force fully auto advance (set by timed stages)
float _StageTimer			; timer for the current stage
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
		String activeScene = GetActiveScene()
		int[] strips_ = SexLabRegistry.GetStripDataA(activeScene, "")
		int[] sex_ = SexLabRegistry.GetPositionSexA(activeScene)
		int i = 0
		While (i < _Positions.Length)
			ActorAlias[i].ReadyActor(strips_[i], sex_[i])
			i += 1
		EndWhile
		_SFXTimer = Config.SFXDelay
		_animationSyncCount = 0
		SendModEvent("SSL_READY_Thread" + tid)
		StartedAt = SexLabUtil.GetCurrentGameRealTime()
		AnimationStart()
	EndEvent
	Function AnimationStart()
		If (_animationSyncCount < _Positions.Length)
			_animationSyncCount += 1
			Log("AnimationStart called " + _animationSyncCount + "/" + (_Positions.Length + 1) + " times")
			return
		EndIf
		Log("AnimationStart fully setup, begin animating")
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
			ActorAlias[i].TryLock()
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
			If (SexLabRegistry.GetNodeType(GetActiveScene(), asNewStage) == 2)
				SendThreadEvent("OrgasmStart")
				TriggerOrgasm()
			EndIf
			int ctype = sslSystemConfig.GetSettingInt("iClimaxType")
			If (ctype == Config.CLIMAXTYPE_SCENE)
				int[] cactors = SexLabRegistry.GetClimaxingActors(GetActiveScene(), asNewStage)
				int i = 0
				While (i < cactors.Length)
					ActorAlias[cactors[i]].DoOrgasm()
					i += 1
				EndWhile
			EndIf
		EndIf
		int[] strips = SexLabRegistry.GetStripDataA(GetActiveScene(), "")
		int i = 0
		While (i < _Positions.Length)
			ActorAlias[i].TryLock()
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
		StopTranslations()
		_StageHistory = AdvanceScene(asHistory, asNextStageId)
		StartTranslations()
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
				ActorAlias[i].TryLock()
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
		_StageTimer = GetTimer()
		If (!_ForceAdvance && !AutoAdvance)
			TryUpdateMenuTimer(0.0)
		Else
			TryUpdateMenuTimer(_StageTimer)
		EndIf
		RegisterForSingleUpdate(ANIMATING_UPDATE_INTERVAL)
	EndFunction

	Function UpdateTimer(float AddSeconds = 0.0)
		_StageTimer += AddSeconds
		_ForceAdvance = true
		TryUpdateMenuTimer(_StageTimer)
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
		If (AutoAdvance || _ForceAdvance)
			_StageTimer -= ANIMATING_UPDATE_INTERVAL
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
			bool penetration = HasCollisionAction(CTYPE_Vaginal, none, none) || HasCollisionAction(CTYPE_Anal, none, none)
			bool oral = HasCollisionAction(CTYPE_Oral, none, none)
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
		StopTranslations()
		AdvanceScene(_StageHistory, GetActiveStage())
		StartTranslations()
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
		If(SexLabRegistry.GetNodeType(GetActiveScene(), GetActiveStage()) == 2)
			SendThreadEvent("OrgasmEnd")
		EndIF
		GoToState(STATE_END)
	EndFunction

	bool Function ResetAnimation(Actor[] akNewPositions, Actor[] akSubmissives, ObjectReference akCenter)
		EndAnimation()
		return ResetAnimation(akNewPositions, akSubmissives, akCenter)
	EndFunction

	int Function GetStatus()
		return STATUS_INSCENE
	EndFunction

	Event OnEndState()
		UnregisterForUpdate()
		UnregisterCollision()
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
	Log("Cannot upate timers outside of playing state", "UpdateTimer()")
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

Function PlayStageAnimations()
	RealignActors()
EndFunction

; Set location for all _Positions on CenterAlias, incl offset, and play their respected animation. _Positions are assumed to be sorted by scene
String[] Function AdvanceScene(String[] asHistory, String asNextStageId) native
int Function SelectNextStage(String[] asThreadTags) native
bool Function SetActiveScene(String asScene) native
bool Function ReassignCenter(ObjectReference CenterOn) native
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
		Config.DisableThreadControl(self as sslThreadController)
		SendModEvent("SSL_CLEAR_Thread" + tid, "", 1.0)
		If(IsObjectiveDisplayed(0))
			SetObjectiveDisplayed(0, False)
		EndIf
		UpdateAllEncounters()
		int i = 0
		While (i < ActorAlias.Length)
			If (ActorAlias[i].GetState() == ActorAlias[i].STATE_IDLE)
				i += 1
			Else
				Utility.Wait(0.1)
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
		GoToState(STATE_SETUP)
		SetScenes(validScenes)
		return AddActorsA(akNewPositions, akSubmissives) && StartThread()
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

; ------------------------------------------------------- ;
; --- Function Declarations                           --- ;
; ------------------------------------------------------- ;
;/
	Most functions used to manage animations have a unique behavior depending on the currently active state
	The below block defines such functions. All of these functions will be overwritten for every state where there
	is reason to implement them
/;

Function CenterOnObject(ObjectReference CenterOn, bool resync = true)
	Log("Invalid State", "CenterOnObject()")
EndFunction
Function EndAnimation(bool Quickly = false)
	Log("Invalid state", "EndAnimation()")
EndFunction
bool Function ResetAnimation(Actor[] akNewPositions, Actor[] akSubmissives, ObjectReference akCenter)
	Log("Invalid state", "ResetAnimation()")
	return false
EndFunction
Function PrepareDone()
	Log("Invalid state", "PrepareDone()")
EndFunction
Function AnimationStart()
	Log("Invalid state", "AnimationStart()")
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

; ------------------------------------------------------- ;
; --- ORGASM FX                                       --- ;
; ------------------------------------------------------- ;

bool Function IsVaginalComplex(Actor ActorRef)
	If (StringUtil.Find(GetInteractionString(ActorRef), "Vaginal") != -1)
		return True
	EndIf
	return False
EndFunction

bool Function IsAnalComplex(Actor ActorRef)
	If (StringUtil.Find(GetInteractionString(ActorRef), "Anal") != -1)
		return True
	EndIf
	return False
EndFunction

bool Function IsOralComplex(Actor ActorRef)
	string interString = GetInteractionString(ActorRef)
	If ((StringUtil.Find(interString, "Oral") != -1) \
	|| (StringUtil.Find(interString, "LickingShaft") != -1) \
	|| (StringUtil.Find(interString, "Deepthroat") != -1))
		return True
	EndIf
	return False
EndFunction

Function ApplyCumFX(Actor SourceRef)
	; TODO: If there is no schlong, consider failing silently
	If (!Config.UseCum)
		return
	EndIf
	int i = 0
	while i < _Positions.Length
		int otherSex = GetNthPositionSex(i)

		Log("Checking for cum FX: " + i + " " + otherSex)

		if (i != GetPosition(SourceRef)) && (otherSex == 1 || otherSex == 2 || otherSex == 4 || (SameSexThread() && (otherSex == 0 || otherSex == 3)))
			ActorAlias[i].ApplyCum()
		endIf
		i += 1
	endWhile
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

Int Property ACTORINT_NONPART 		= 0 AutoReadOnly Hidden
Int Property ACTORINT_PASSIVE 		= 1 AutoReadOnly Hidden
Int Property ACTORINT_ACTIVE 		= 2 AutoReadOnly Hidden

;--> actor is getting non-penile stimulation
int Property pStimulation			= 0  AutoReadOnly Hidden	;pos_crotch is being fingered, fisted, or toys_inserted
int Property aAnimObjFace			= 1  AutoReadOnly Hidden	;pos_anim_obj is in front of partner's face
int Property pAnimObjFace			= 2  AutoReadOnly Hidden	;pos_face is in front of partner's anim_obj
;--> actor's body is receiving/doing something
int Property pSuckingToes			= 3  AutoReadOnly Hidden	;pos_toes are closer to partner's mouth
int Property pGrinding				= 4  AutoReadOnly Hidden	;pos_body is being grinded against by partner's crotch
int Property pSkullfuck				= 5  AutoReadOnly Hidden	;pos_head is being penetrated by partner's pp
int Property aHandJob				= 6  AutoReadOnly Hidden	;pos_hand is moving around partner's pp
int Property aFootJob				= 7  AutoReadOnly Hidden	;pos_foot is moving around partner's pp
int Property aBoobJob				= 8  AutoReadOnly Hidden	;pos_boob is moving around partner's pp
;--> actor's mouth is doing something
int Property bKissing				= 9  AutoReadOnly Hidden	;pos_face is in front of partner's face
int Property aSuckingToes			= 10  AutoReadOnly Hidden	;pos_face is in front of partner's toes
int Property pFacial				= 11  AutoReadOnly Hidden	;pos_face is in front of partner's pp
int Property aOral					= 12  AutoReadOnly Hidden	;pos_mouth is licking/sucking partner's crotch
int Property aLickingShaft			= 13  AutoReadOnly Hidden	;pos_mouth is licking shaft of partner's pp
int Property aDeepthroat			= 14  AutoReadOnly Hidden	;pos_mouth is deep-throating partner's pp
;--> actor is getting penile penetration
int Property pVaginal				= 15  AutoReadOnly Hidden	;pos_vag is being penetrated by partner's pp
int Property pAnal					= 16  AutoReadOnly Hidden	;pos_anus is being penetrated by partner's pp
;--> actor's pp is doing something
int Property aFacial				= 17  AutoReadOnly Hidden	;pos_pp is in front of partner's face
int Property aGrinding				= 18  AutoReadOnly Hidden	;pos_crotch is grinding against partner's body
int Property pHandJob				= 19  AutoReadOnly Hidden	;pos_pp is being pleasured by partner's hands
int Property pFootJob				= 20  AutoReadOnly Hidden	;pos_pp is being pleasured by partner's feet
int Property pBoobJob				= 21  AutoReadOnly Hidden	;pos_pp is being pleasured by partner's boobs
int Property pLickingShaft			= 22  AutoReadOnly Hidden	;pos_pp's shaft is being licked by partner's tongue
int Property pOral					= 23  AutoReadOnly Hidden	;pos_crotch is being licked/sucked by partner's mouth
int Property pDeepthroat			= 24  AutoReadOnly Hidden	;pos_pp is deep inside partner's mouth
int Property aSkullfuck				= 25  AutoReadOnly Hidden	;pos_pp is penetrating partner's head
int Property aVaginal				= 26  AutoReadOnly Hidden	;pos_pp is penetrating partner's vagina
int Property aAnal					= 27  AutoReadOnly Hidden	;pos_pp is penetrating partner's anus

AssociationType Property SpouseAssocation Auto
Faction Property PlayerMarriedFaction Auto

sslActorStats Property Stats Hidden
  sslActorStats Function Get()
	  return Game.GetFormFromFile(0xD62, "SexLab.esm") as sslActorStats
  EndFunction
EndProperty

; -------------------------------------------------- ;
; --- Interactions Detections                    --- ;
; -------------------------------------------------- ;

string[] Function NameAllInteractions() global
    string[] interTypes = new string[28]
    interTypes[0] = "pStimulation"
    interTypes[1] = "aAnimObjFace"
    interTypes[2] = "pAnimObjFace"
    interTypes[3] = "pSuckingToes"
    interTypes[4] = "pGrinding"
    interTypes[5] = "pSkullfuck"
    interTypes[6] = "aHandJob"
    interTypes[7] = "aFootJob"
    interTypes[8] = "aBoobJob"
    interTypes[9] = "bKissing"
    interTypes[10] = "aSuckingToes"
    interTypes[11] = "pFacial"
    interTypes[12] = "aOral"
    interTypes[13] = "aLickingShaft"
    interTypes[14] = "aDeepthroat"
    interTypes[15] = "pVaginal"
    interTypes[16] = "pAnal"
    interTypes[17] = "aFacial"
    interTypes[18] = "aGrinding"
    interTypes[19] = "pHandJob"
    interTypes[20] = "pFootJob"
    interTypes[21] = "pBoobJob"
    interTypes[22] = "pLickingShaft"
    interTypes[23] = "pOral"
    interTypes[24] = "pDeepthroat"
    interTypes[25] = "aSkullfuck"
    interTypes[26] = "aVaginal"
    interTypes[27] = "aAnal"
    return interTypes
EndFunction

bool[] Function ListDetectedPhysicsInter(Actor akPosition)
	;not relying on GetCollisionActions() to avoid slow papyrus loops and else-ifs
	If !IsInteractionRegistered()
        return Utility.CreateBoolArray(1, False)
    EndIf
	bool[] phyActive = Utility.CreateBoolArray(28, False)
	phyActive[aAnimObjFace] = HasCollisionAction(CTYPE_AnimObjFace, None, akPosition)
	phyActive[pAnimObjFace] = HasCollisionAction(CTYPE_AnimObjFace, akPosition, None)
	phyActive[bKissing] = HasCollisionAction(CTYPE_Kissing, akPosition, None)
	phyActive[aSuckingToes] = HasCollisionAction(CTYPE_SuckingToes, akPosition, None)
	phyActive[pSuckingToes] = HasCollisionAction(CTYPE_SuckingToes, None, akPosition)
	phyActive[aFacial] = HasCollisionAction(CTYPE_Facial, None, akPosition)
	phyActive[pFacial] = HasCollisionAction(CTYPE_Facial, akPosition, None)
	phyActive[aGrinding] = HasCollisionAction(CTYPE_Grinding, None, akPosition)
	phyActive[pGrinding] = HasCollisionAction(CTYPE_Grinding, akPosition, None)
	phyActive[aHandJob] = HasCollisionAction(CTYPE_HandJob, akPosition, None)
	phyActive[pHandJob] = HasCollisionAction(CTYPE_HandJob, None, akPosition)
	phyActive[aFootJob] = HasCollisionAction(CTYPE_FootJob, akPosition, None)
	phyActive[pFootJob] = HasCollisionAction(CTYPE_FootJob, None, akPosition)
	;phyActive[aBoobJob] = False 	; awaiting support
	;phyActive[pBoobJob] = False	; awaiting support
	phyActive[aLickingShaft] = HasCollisionAction(CTYPE_LickingShaft, akPosition, None)
	phyActive[pLickingShaft] = HasCollisionAction(CTYPE_LickingShaft, None, akPosition)
	phyActive[aOral] = HasCollisionAction(CTYPE_Oral, akPosition, None)
	phyActive[pOral] = HasCollisionAction(CTYPE_Oral, None, akPosition)
	phyActive[aDeepthroat] = HasCollisionAction(CTYPE_Deepthroat, akPosition, None)
	phyActive[pDeepthroat] = HasCollisionAction(CTYPE_Deepthroat, None, akPosition)
	phyActive[aSkullfuck] = HasCollisionAction(CTYPE_Skullfuck, None, akPosition)
	phyActive[pSkullfuck] = HasCollisionAction(CTYPE_Skullfuck, akPosition, None)
	phyActive[aVaginal] = HasCollisionAction(CTYPE_Vaginal, None, akPosition)
	phyActive[pVaginal] = HasCollisionAction(CTYPE_Vaginal, akPosition, None)
	phyActive[aAnal] = HasCollisionAction(CTYPE_Anal, None, akPosition)
	phyActive[pAnal] = HasCollisionAction(CTYPE_Anal, akPosition, None)
	return phyActive
EndFunction

bool[] Function ListDetectedRimTagsInter(Actor akPosition)
    If !HasSceneTag("RimTagged") ;hentairim
        return Utility.CreateBoolArray(1, False)
    EndIf
	string[] rimTags = SexLabRegistry.GetPositionTags(GetActiveScene(), GetActiveStage(), GetPositionIdx(akPosition))
	bool[] rimActive = Utility.CreateBoolArray(28, False)
	int i = 0
	int len = rimTags.Length
	While (i < len)
		string tag = rimTags[i]
		If (tag=="kis")
			rimActive[bKissing] = true
		ElseIf (tag=="sst" || tag=="fst" || tag=="bst")
			rimActive[pStimulation] = true
		ElseIf (tag=="ssb" || tag=="fsb")
			rimActive[aOral] = true
		ElseIf (tag=="smf" || tag=="fmf" || tag=="cun")
			rimActive[pOral] = true
		ElseIf (tag=="shj" || tag=="fhj")
			rimActive[pHandJob] = true
		ElseIf (tag=="sfj" || tag=="ffj")
			rimActive[pFootJob] = true
		ElseIf (tag=="stf" || tag=="ftf")
			rimActive[pBoobJob] = true
		ElseIf tag=="sdv" || tag=="fdv"
			rimActive[aVaginal] = true
		ElseIf tag=="sda" || tag=="fda"
			rimActive[aAnal] = true
		ElseIf (tag=="svp" || tag=="fvp" || tag=="sdp" || tag=="fdp" || tag=="scg" || tag=="fcg")
			rimActive[pVaginal] = true
		ElseIf (tag=="sap" || tag=="fap" || tag=="sdp" || tag=="fdp" || tag=="sac" || tag=="fac")
			rimActive[pAnal] = true
		EndIf
		i += 1
	EndWhile
	return rimActive
EndFunction

bool[] Function ListDetectedASLTagsInter(int ActorInterInfo)
	If HasStageTag("RimTagged") || (ActorInterInfo == ACTORINT_NONPART) 
		return Utility.CreateBoolArray(1, False)
	EndIf
	bool stageHJ = HasStageTag("Masturbation") || HasStageTag("HandJob") || HasStageTag("Fingering")
	bool stageFJ = HasStageTag("FootJob") || HasStageTag("Feet")
	bool stageBJ = HasStageTag("BoobJob")
	bool stageGR = HasStageTag("Grinding")
	bool stageOR = HasStageTag("Oral")
	bool stageVG = HasStageTag("Vaginal")
	bool stageAN = HasStageTag("Anal")
	;actor assumed passive/receiving overall is active for some action
	bool isActive = (ActorInterInfo == ACTORINT_ACTIVE)
	bool isPassive = (ActorInterInfo == ACTORINT_PASSIVE)
	If (stageOR||stageHJ||stageFJ||stageBJ) 
		If isPassive
			isPassive = False
			isActive = true
		ElseIf isActive
			isActive = False
			isPassive = true
		EndIf
	EndIf
	bool[] aslActive = Utility.CreateBoolArray(28, False)
	aslActive[bKissing] = HasStageTag("Kissing")
	aslActive[aGrinding] = stageGR && isActive
	aslActive[pGrinding] = stageGR && isPassive
	aslActive[aHandJob] = stageHJ && isActive
	aslActive[pHandJob] = stageHJ && isPassive
	aslActive[aFootJob] = stageFJ && isActive
	aslActive[pFootJob] = stageFJ && isPassive
	aslActive[aBoobJob] = stageBJ && isActive
	aslActive[pBoobJob] = stageBJ && isPassive
	aslActive[aOral] = stageOR && isActive
	aslActive[pOral] = stageOR && isPassive
	aslActive[aVaginal] = stageVG && isActive
	aslActive[pVaginal] = stageVG && isPassive
	aslActive[aAnal] = stageAN && isActive
	aslActive[pAnal] = stageAN && isPassive
	return aslActive
EndFunction

; -------------------------------------------------- ;
; --- Interactions Factors                       --- ;
; -------------------------------------------------- ;

string Function CreateInteractionString(Actor akPosition, int ActorInterInfo, int iStrength = -1)
    If (iStrength == -1)
        iStrength = Config.InterDetectionStrength
    EndIf
	If (iStrength < 0 || iStrength > 4)
		iStrength = 4
	EndIf
	string ret = "..."
	string[] interTypes = NameAllInteractions()
	If iStrength == 1
		return InterateForInteractionString(interTypes, ListDetectedASLTagsInter(ActorInterInfo))
	ElseIf iStrength == 2
		return InterateForInteractionString(interTypes, ListDetectedRimTagsInter(akPosition))
	ElseIf iStrength == 3
		return InterateForInteractionString(interTypes, ListDetectedPhysicsInter(akPosition))
	ElseIf iStrength == 4
		ret = InterateForInteractionString(interTypes, ListDetectedPhysicsInter(akPosition))
		If !ret
			ret = InterateForInteractionString(interTypes, ListDetectedRimTagsInter(akPosition))
			If !ret
				ret = InterateForInteractionString(interTypes, ListDetectedASLTagsInter(ActorInterInfo))
			EndIf
		EndIf
		return ret
	Else
		return ret
	EndIf
EndFunction

float Function CalculateInteractionFactor(Actor akPosition, string InteractionString)
	float factorPhysic = 0.0
	string[] InterActive = StringUtil.Split(InteractionString, ",")
	int i = 0
	int len = InterActive.Length
	While (i < len)
		If InterActive[i]
			float value = sslSystemConfig.GetEnjoymentFactor(InterActive[i])
			float velocityMult = CalcInterVelocityMultiplier(akPosition, InterActive[i])
			factorPhysic += (value / 20) * velocityMult
		EndIf
		i += 1
	EndWhile
	return factorPhysic
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

bool Function SameSexThread()
	bool SameSexThread = False
	int MaleCount = sslActorLibrary.CountMale(_Positions)
	int FemCount = sslActorLibrary.CountFemale(_Positions)
	int FutaCount = sslActorLibrary.CountFuta(_Positions)
	int CrtMaleCount = sslActorLibrary.CountCrtMale(_Positions)
	int CrtFemaleCount = sslActorLibrary.CountCrtFemale(_Positions)
	If (_Positions.Length != 1 && ((MaleCount + CrtMaleCount == _Positions.Length) || (FemCount + CrtFemaleCount == _Positions.Length) || (FutaCount == _Positions.Length)))
		SameSexThread = true ; returns False for solo scenes
	EndIf
	return SameSexThread
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
		While i < Positions.Length
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
	ret[10] = SexLabRegistry.IsStageTag(asScene, asStage, "RimTagged")
	ret[11] = SexLabRegistry.IsStageTag(asScene, asStage, "RimFast")
	ret[12] = SexLabRegistry.IsStageTag(asScene, asStage, "RimSlow")
	return ret
EndFunction

; -------------------------------------------------- ;
; --- Best Relation                              --- ;
; -------------------------------------------------- ;

;/mapping: Stranger=-2~2 | PersonOfInterest=3~7 | Lover=8~12 | Spouse=13~17 | LoverSpouse=18~22
w_agg=-2 | w_vic=-1 | <<stranger=0>> | w_dom=1 | w_sub=2
W_agg=3 | w_vic=4 | <<poi=5>> | w_dom=6 | w_sub=7
W_agg=8 | w_vic=9 | <<lover=10>> | w_dom=11 | w_sub=12
W_agg=13 | w_vic=14 | <<spouse=15>> | w_dom=16 | w_sub=17
W_agg=18 | w_vic=19 | <<spouse+lover=20>> | w_dom=21 | w_sub=22/;
int Function GetRelationForScene(Actor akPosition, Actor TargetRef, int ConSubStatus)
	int BaseRelation = 0
	int ContextRelation = 0
	int retRelation = 0
	bool withSpouse = False
	bool withLover = False
	
	If akPosition == PlayerRef
		If TargetRef.IsInFaction(PlayerMarriedFaction)
			withSpouse = true
			BaseRelation = 15
		EndIf
	Else
		If akPosition.HasAssociation(SpouseAssocation, TargetRef)
			withSpouse = true
			BaseRelation = 15
		EndIf
	EndIf
	If !withSpouse && akPosition.GetRelationshipRank(TargetRef) >= 4
		withLover = true
		BaseRelation = 10
	ElseIf !withLover && !withSpouse && (akPosition.GetRelationshipRank(TargetRef) >= 1) && (SexLabStatistics.GetTimesMet(akPosition, TargetRef) >= 3)
		BaseRelation = 5
	EndIf

	If ConSubStatus == CONSENT_CONSUB
		If IsVictim(akPosition)
			ContextRelation = 1
		ElseIf IsVictim(TargetRef)
			ContextRelation = 2
		EndIf
	Else
		If IsVictim(akPosition)
			ContextRelation = -2
		ElseIf IsVictim(TargetRef)
			ContextRelation = -1
		EndIf
	EndIf

	retRelation = BaseRelation + ContextRelation
	return retRelation
EndFunction

int Function GetBestRelationForScene(Actor akPosition, int ConSubStatus)
	if _Positions.Length <= 1
		return 0
	elseif _Positions.Length == 2
		if(akPosition == _Positions[0])
			return GetRelationForScene(akPosition, _Positions[1], ConSubStatus)
		else
			return GetRelationForScene(akPosition, _Positions[0], ConSubStatus)
		endif
	endIf
	int ret = -2
	int i = 0
	while i < _Positions.Length
		if _Positions[i] != akPosition
			int relation = GetRelationForScene(akPosition, _Positions[i], ConSubStatus)
			if relation > ret
				ret = relation
			endif
		endIf
		i += 1
	endWhile
	return ret
EndFunction

; -------------------------------------------------- ;
; --- Utility Functions                          --- ;
; -------------------------------------------------- ;

int Function GuessActorInterInfo(Actor akPosition, int ActorSex, bool ActorIsSub, int ConSubStatus, bool SameSexThread)
	;IMP: roles will be reversed for oral/handjob/etc (passive is giving; active is receiving)
	int ActorInterInfo = ACTORINT_NONPART
	If ConSubStatus > CONSENT_NONCONNONSUB
		bool FemDom = HasSceneTag("FemDom")
		If !SameSexThread
			If (ActorIsSub && !FemDom) || (!ActorIsSub && FemDom)
				ActorInterInfo = ACTORINT_PASSIVE
			ElseIf (!ActorIsSub && !FemDom) || (ActorIsSub && FemDom)
				ActorInterInfo = ACTORINT_ACTIVE
			EndIf
		Else
			If ActorIsSub
				ActorInterInfo = ACTORINT_PASSIVE
			ElseIf !ActorIsSub
				ActorInterInfo = ACTORINT_ACTIVE
			EndIf
		EndIf
	Else
		If !SameSexThread
			If ActorSex == 1 || ActorSex == 4
				ActorInterInfo = ACTORINT_PASSIVE
			Else ; ignoring complexities with futas
				ActorInterInfo = ACTORINT_ACTIVE
			EndIf
		Else
			If _Positions.Length > 1 && GetPositionIdx(akPosition) == 0
				ActorInterInfo = ACTORINT_PASSIVE
			Else
				ActorInterInfo = ACTORINT_ACTIVE
			EndIf
		EndIf
	EndIf
	return ActorInterInfo
EndFunction

string Function InterateForInteractionString(string[] interTypes, bool[] interActive)
	string ret = ""
	int i = 0
	int len = interActive.Length
	While (i < len)
		If interActive[i]
			If ret != ""
				ret += ","
			EndIf
			ret += interTypes[i]
		EndIf
		i += 1
	EndWhile
	return ret
EndFunction

float Function CalcInterVelocityMultiplier(Actor akActor, string actType)
	If !actType
		return 0.0
	EndIf
	;convert string to int
	string baseType = StringUtil.Substring(actType, 1)
	int CType = 0
	If baseType == "Vaginal"
		CType = CTYPE_Vaginal
	ElseIf baseType == "Anal"
		CType = CTYPE_Anal
	ElseIf baseType == "Oral"
		CType = CTYPE_Oral
	ElseIf baseType == "Grinding"
		CType = CTYPE_Grinding
	ElseIf baseType == "Deepthroat"
		CType = CTYPE_Deepthroat
	ElseIf baseType == "Skullfuck"
		CType = CTYPE_Skullfuck
	ElseIf baseType == "LickingShaft"
		CType = CTYPE_LickingShaft
	ElseIf baseType == "FootJob"
		CType = CTYPE_FootJob
	ElseIf baseType == "HandJob"
		CType = CTYPE_HandJob
	ElseIf baseType == "Kissing"
		CType = CTYPE_Kissing
	ElseIf baseType == "AnimObjFace"
		CType = CTYPE_AnimObjFace
	ElseIf baseType == "SuckingToes"
		CType = CTYPE_SuckingToes
	EndIf
	;calculate velocity multiplier
	float velocityMax = 0.03 ; have seen values upto 0.097075
	float velocityMult = 0.0
	float velocityCur = Math.Abs(GetActionVelocity(akActor, None, CType))
	If (velocityCur < 0.25)
		velocityCur = 0.25
	EndIf
	velocityMult = velocityCur/velocityMax
	return velocityMult
EndFunction

; ---------------------------------------------- ;
; --- Enjoyment Game                         --- ;
; ---------------------------------------------- ;

Function GameAdjustEnj(Actor akActor, Actor akPartner = None, int AdjustBy = 0)
    If (akPartner == None)
        akPartner = akActor
    EndIf
	If (AdjustBy != 0)
		AdjustEnjoyment(akPartner, AdjustBy)
		return
	Else
		float arousalstat = PapyrusUtil.ClampFloat(SexlabStatistics.GetStatistic(akPartner, 17), 0.0, 100.0)
		AdjustBy = PapyrusUtil.ClampInt((arousalstat as int / 50), 1, 2)
		int basesex = GetActorSex(akPartner)
		If (basesex != 0 || basesex != 3)
			AdjustBy += GetOrgasmCount(akPartner)
		EndIf
		AdjustEnjoyment(akPartner, AdjustBy)
	EndIf
EndFunction

Function GameRaiseEnjoyment(Actor akActor, Actor akPartner, float VarMod, float EnjoymentMod)
	;akActor.DamageActorValue("Stamina", akActor.GetBaseActorValue("Stamina")/(10+VarMod+EnjoymentMod))
	akActor.DamageActorValue("Stamina", Config.GameStaminaCost)
	If (Config.GameEnjReductionChance == 1) && (VarMod < 3) && (Utility.RandomInt(0,100) < ((3-VarMod)*10))
		GameAdjustEnj(akActor, akPartner, -1) ;with skills 3- upto 30 chance to decrease enjoyment
	Else
		GameAdjustEnj(akActor, akPartner)
	EndIf
EndFunction

Function GameHoldback(Actor akActor, Actor akPartner = None)
	If (akPartner == None)
		akPartner = akActor
	EndIf
	float[] OwnSkills = Stats.GetSkillLevels(akPartner)
	If (GetPositionIdx(akPartner) == 0)
		int basesex = GetActorSex(akPartner)
		If (basesex == 1 || basesex == 4)
			If (IsVaginalComplex(akPartner) || HasSceneTag("Fisting") || HasSceneTag("SixtyNine"))
				GameAdjustEnj(akActor, akPartner, (-1 - OwnSkills[Stats.kVaginal]) as int)
			ElseIf IsAnalComplex(akPartner)
				GameAdjustEnj(akActor, akPartner, (-1 - OwnSkills[Stats.kAnal]) as int)
			Else
				GameAdjustEnj(akActor, akPartner, -1)
			EndIf
		ElseIf (basesex == 0 || basesex == 2)
			If (IsAnalComplex(akPartner) || HasSceneTag("Fisting"))
				GameAdjustEnj(akActor, akPartner, (-1 - OwnSkills[Stats.kAnal]) as int)
			Else
				GameAdjustEnj(akActor, akPartner, -1)
			EndIf
		EndIf
	Else
		GameAdjustEnj(akActor, akPartner, -1)
	EndIf
EndFunction

Function GameDamageMagicka(Actor akPartner, float GameModSelfSta)
	If (akPartner == None)
		return
	EndIf
	float GameModPartMag = CalcEnjVarMod("Magicka", akPartner)
	akPartner.DamageActorValue("Magicka", \ 
		akPartner.GetBaseActorValue("Magicka")/100\
		*(10-GameModSelfSta+GameModPartMag/100)\
		*((GetEnjoyment(akPartner) as float)/100\
		*(1+GetOrgasmCount(akPartner)))*0.5)
EndFunction

float Function CalcEnjVarMod(String var = "", Actor akActor, Actor akPartner = None)
	If (akPartner == None)
		akPartner = akActor
	EndIf
	float VarMod = 0.0
	If (var == "Stamina")
		If IsVaginalComplex(akPartner)
			VarMod = Stats.GetSkillLevel(akPartner, "Vaginal")
		ElseIf IsAnalComplex(akPartner)
			VarMod = Stats.GetSkillLevel(akPartner, "Anal")
		ElseIf IsOralComplex(akPartner)
			VarMod = Stats.GetSkillLevel(akPartner, "Oral")
		Else
			VarMod = Stats.GetSkillLevel(akPartner, "Foreplay")
		EndIf
	ElseIf (var == "Magicka")
		VarMod = Stats.GetSkillLevel(akPartner, "Lewd", 0.3) - Stats.GetSkillLevel(akPartner, "Pure", 0.3)
	EndIf
	return PapyrusUtil.ClampFloat(VarMod, -6, 6)
EndFunction

Actor Function GameChangePartner(Actor akActor, int idx = -1)
	Actor akPartner = None
	Actor tempRef = None
	If (_Positions.Length > 1)
		If (idx < 0)
			int idxPartner = sslUtility.IndexTravel(GetPositionIdx(akActor), _Positions.Length)
			akPartner = ActorAlias[idxPartner].GetActorRef()
			If (akActor == PlayerRef)
				MiscUtil.PrintConsole("[EnjGame] " + akActor.GetDisplayName() + "'s current partner is " + akPartner.GetDisplayName())
			EndIf
		Else
			tempRef = ActorAlias[idx].GetActorRef()
			If (tempRef == None || tempRef == akPartner || tempRef == akActor || tempRef == PlayerRef)
				return akPartner
			EndIf
			akPartner = tempRef
			If (akActor == PlayerRef)
				MiscUtil.PrintConsole("[EnjGame] " + akActor.GetDisplayName() + " changed focus to " + akPartner.GetDisplayName())
				;Debug.Notification(akActor.GetDisplayName() + " changed focus to " + akPartner.GetDisplayName())
			EndIf
		EndIf
	EndIf
	Config.SelectedSpell.Cast(akPartner, akPartner)
	return akPartner
EndFunction

int Function GameNextPartnerIdx(Actor akActor, Actor akPartner, bool abReverse)
	int PartnerIdx = GetPositionIdx(akPartner)
    If (Positions.Length <= 2)
        return PartnerIdx
    EndIf
	int ActorIdx = GetPositionIdx(akActor)
    int step = 1
    If (abReverse)
        step = -1
    EndIf
    int NewIdx = (PartnerIdx + step)
	int PosLen = Positions.Length
    int i = 0
    While (i < PosLen)
        If (NewIdx >= PosLen)
            NewIdx = 0
        ElseIf (NewIdx < 0)
            NewIdx = PosLen - 1
        EndIf
        If (NewIdx != ActorIdx) && (NewIdx != PartnerIdx)
            return NewIdx
        EndIf
        NewIdx += step
        i += 1
    EndWhile
    return PartnerIdx
EndFunction

Function ProcessEnjGameArg(String arg = "", Actor akActor, Actor akPartner, float GameModSelfSta, float GameModSelfMag)
	Actor PartnerRef = None
	bool MentallyBroken = False
	int ActorEnjoyment = GetEnjoyment(akActor)
	float EnjoymentMod = PapyrusUtil.ClampFloat((ActorEnjoyment as float)/30, 1.0, 3.0)
	float VarMod = 0.0
	
	If (akActor == PlayerRef) && (Config.GamePlayerVictimAutoplay == 1) && IsVictim(akActor)
		MentallyBroken = True
	ElseIf (akActor.GetActorValuePercentage("Magicka") <= 0.10)
		MentallyBroken = True
	ElseIf (akActor.GetActorValuePercentage("Magicka") > 0.25 && MentallyBroken == True)
		MentallyBroken = False
	EndIf
	
	;PC only (RaiseEnjKey)
	If arg == "Stamina"
		If MentallyBroken == False
			VarMod = GameModSelfSta
			If (akActor.GetActorValuePercentage("Stamina") > 0.10)
				If (_Positions.Length == 1 || Input.IsKeyPressed(Config.GameUtilityKey))
					PartnerRef = akActor
					If (ActorEnjoyment < 85)
						GameRaiseEnjoyment(akActor, PartnerRef, VarMod, EnjoymentMod)
					ElseIf (ActorEnjoyment > 90) && (PartnerRef == PlayerRef)
						ActorAlias[GetPositionIdx(PartnerRef)].GameRegisterEdgeAttempt()
					EndIf
				ElseIf (_Positions.Length > 1)
					PartnerRef = akPartner
					GameRaiseEnjoyment(akActor, PartnerRef, VarMod, EnjoymentMod)
				EndIf
			EndIf
		EndIf
	;PC only (HoldbackKey)
	ElseIf arg == "Magicka"
		If MentallyBroken == False
			If (_Positions.Length == 1 || Input.IsKeyPressed(Config.GameUtilityKey))
				VarMod = GameModSelfMag
				If (akActor.GetActorValuePercentage("Magicka") > 0.10)
					;akActor.DamageActorValue("Magicka", akActor.GetBaseActorValue("Magicka")/(10-VarMod)*0.5)
					akActor.DamageActorValue("Magicka", Config.GameMagickaCost)
					PartnerRef = akActor
					GameHoldback(akActor, PartnerRef)
				EndIf
			ElseIf (_Positions.Length > 1)
				VarMod = GameModSelfSta
				If (akActor.GetActorValuePercentage("Stamina") > 0.10)
					;akActor.DamageActorValue("Stamina", akActor.GetBaseActorValue("Stamina")/(10+VarMod)*0.5)
					akActor.DamageActorValue("Stamina", Config.GameStaminaCost)
					PartnerRef = akPartner
					GameHoldback(akActor, PartnerRef) 
				EndIf
			EndIf
		EndIf
	;NPC and PC (auto)
	ElseIf arg == "Auto"
		If (!HasPlayer || (MentallyBroken == True) || ((akActor == PlayerRef) && Config.GamePlayerAutoplay) || ((akActor != PlayerRef) && Config.GameNPCAutoplay))
			bool withLover = (_Positions.Length == 2 && (Utility.RandomInt(0, 100) < (25+GetHighestPresentRelationshipRank(akActor)*10*2)))
			VarMod = GameModSelfSta
			If (akActor.GetActorValuePercentage("Stamina") > 0.10)
				If IsAggressor(akActor) ;aggressor
					int RelationshipRank = GetLowestPresentRelationshipRank(akActor)
					If RelationshipRank < 0 ;enemies
						VarMod = Math.Abs(RelationshipRank)
						PartnerRef = akActor
					Else ;neutrals/lovers
						If (MentallyBroken == False || _Positions.Length > 1)
							PartnerRef = akActor
						Else
							PartnerRef = akPartner
						EndIf
					EndIf
				Else ;not aggressor
					If (MentallyBroken == False || _Positions.Length > 1)
						If (Utility.RandomInt(0, 100) < (Stats.GetSkillLevel(akActor, "Lewd", 0.3)*10*1.5))
							PartnerRef = akActor ;lewdness based check --> pleasure self
						ElseIf withLover
							PartnerRef = akPartner ;relationship based check --> pleasure partner
						EndIf
					Else
						PartnerRef = akPartner ;mentally broken, pleasure partner
					EndIf
				EndIf
				GameRaiseEnjoyment(akActor, PartnerRef, VarMod, EnjoymentMod)
			EndIf
			If (withLover && Config.GameHoldbackWithPartner && (akActor.GetActorValuePercentage("Magicka") > 0.10))
				If (GetEnjoyment(akActor) > 90)
					;akActor.DamageActorValue("Magicka", akActor.GetBaseActorValue("Magicka")/(10-GameModSelfMag))
					akActor.DamageActorValue("Magicka", Config.GameMagickaCost)
					PartnerRef = akActor
					GameHoldback(akActor, PartnerRef)
				EndIf
			EndIf
		EndIf
	EndIf
	
	;GameDamageMagicka(PartnerRef, GameModSelfSta)
	;for solo/duo scenes, skip to last scene stage if any actor has ran out of stamina or male has orgasmed
	bool NoStaminaScenario = (Config.NoStaminaEndsScene == 1) && (akActor.GetActorValuePercentage("Stamina") < 0.10) && (GetSubmissives().Length == 0)
	bool MaleOrgasmEndScenario = (Config.MaleOrgasmEndsScene == 1) && (GetActorSex(akActor) == 1) && (GetOrgasmCount(akActor) > 0)
	bool NotEndStageScenario = (GetLegacyStageNum() < GetLegacyStagesCount())
	bool SoloDuoScenario = (_Positions.Length == 1 || _Positions.Length == 2) 
	If ((NoStaminaScenario || MaleOrgasmEndScenario) && NotEndStageScenario && SoloDuoScenario)
		SkipTo(SexLabRegistry.GetEndingStages(GetActiveScene())[0])
	EndIf
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