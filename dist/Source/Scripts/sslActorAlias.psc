ScriptName sslActorAlias extends ReferenceAlias
{
	Alias Script for Actors which are animated by a SexLab Thread
	See SexLabThread.psc for documentation and functions to correctly access this class
}

String Function GetActorName()
	return _ActorRef.GetLeveledActorBase().GetName()
EndFunction

bool function IsVictim()
	return _victim
endFunction

bool Function IsAggressor()
	return _Thread.IsAggressive && !IsVictim()
EndFunction

Function SetVictim(bool Victimize)
	_victim = Victimize
EndFunction

int Function GetSex()
	return _sex
EndFunction

int Function GetRaceID()
	return _raceID
EndFunction

bool Function GetIsDead()
	return _livestatus == LIVESTATUS_DEAD
EndFunction

; ------------------------------------------------------- ;
; --- Orgasms                                         --- ;
; ------------------------------------------------------- ;

function DisableOrgasm(bool bNoOrgasm)
	_CanOrgasm = !bNoOrgasm
endFunction

bool function IsOrgasmAllowed()
	return _CanOrgasm
endFunction

int function GetOrgasmCount()
	return _OrgasmCount
EndFunction

; ------------------------------------------------------- ;
; --- Enjoyment                                       --- ;
; ------------------------------------------------------- ;

int Function GetEnjoyment()
	return _FullEnjoyment
EndFunction

; Enjoyment always stays between -100 and 100. Negative enjoyment is treated as pain.
Function SetEnjoyment(int aiSet)
	_FullEnjoyment = PapyrusUtil.ClampInt(aiSet, -100, 100)
	_PainLevel = 0.0
	_PainApplied = 0.0
EndFunction

Function AdjustEnjoyment(int AdjustBy)
	_FullEnjoyment = PapyrusUtil.ClampInt(_FullEnjoyment + AdjustBy, -100, 100)
EndFunction

Function ModEnjoymentMult(float afSet, bool bAdjust)
	If bAdjust
		_ModEnjMult += afSet
	Else
		_ModEnjMult = afSet
	EndIf
EndFunction

; ------------------------------------------------------- ;
; --- Interactions Info                               --- ;
; ------------------------------------------------------- ;

bool[] Function GetCurrentInteractionFlags()
	return _CurrentInteractions
EndFunction

; ------------------------------------------------------- ;
; --- Specific Detections                             --- ;
; ------------------------------------------------------- ;

bool Function IsVaginalComplex()
	return (_CurrentInteractions[_Thread.aVaginal] || _CurrentInteractions[_Thread.pVaginal] \
	|| _CurrentInteractions[_Thread.aGrinding] || _CurrentInteractions[_Thread.pGrinding])
EndFunction

bool Function IsAnalComplex()
	return (_CurrentInteractions[_Thread.aAnal] || _CurrentInteractions[_Thread.pAnal])
EndFunction

bool Function IsOralComplex()
	return (_CurrentInteractions[_Thread.aOral] || _CurrentInteractions[_Thread.pOral] \
	|| _CurrentInteractions[_Thread.aLickingShaft] || _CurrentInteractions[_Thread.pLickingShaft] \
	|| _CurrentInteractions[_Thread.aDeepthroat] || _CurrentInteractions[_Thread.pDeepthroat])
EndFunction

bool Function ShouldMouthOpen()
	bool condNonCrotch = (_CurrentInteractions[_Thread.aSuckingToes] || _CurrentInteractions[_Thread.pAnimObjFace])
	bool condCrotch = (_CurrentInteractions[_Thread.aOral] || _CurrentInteractions[_Thread.aLickingShaft] || _CurrentInteractions[_Thread.aDeepthroat])
	If (!condNonCrotch && !condCrotch)
		return false
	ElseIf (condNonCrotch)
		return true
	EndIf
	bool FemInNonGayScene = ((_sex==1 || _sex==4) && !_HomoTypes[2])
	bool NonFemInGayScene = ((_sex==0 || _sex==2 || _sex==3) && (_HomoTypes[0] || _HomoTypes[4]))
	return (FemInNonGayScene || NonFemInGayScene)
EndFunction

bool Function IsAnalPenetrated()
	return _CurrentInteractions[_Thread.pAnal]
EndFunction

bool Function IsGenitalInteraction()
	int pSex = SexLabRegistry.GetSex(_ActorRef, false)
	bool handjob = _CurrentInteractions[_Thread.pHandJob]
	bool footjob = _CurrentInteractions[_Thread.pFootJob]
	bool oral = _CurrentInteractions[_Thread.pOral]
	If (handjob || footjob || oral)
		return true
	EndIf
	If (pSex != 0)
		bool vaginal = _CurrentInteractions[_Thread.pVaginal]
		bool grinding = _CurrentInteractions[_Thread.pGrinding]
		If (vaginal || grinding)
			return true
		EndIf
	EndIf
	If (pSex != 1)
		bool anal = _CurrentInteractions[_Thread.aAnal]
		bool vaginal = _CurrentInteractions[_Thread.aVaginal]
		bool grinding = _CurrentInteractions[_Thread.aGrinding]
		bool skull = _CurrentInteractions[_Thread.aSkullfuck]
		bool shaft = _CurrentInteractions[_Thread.pLickingShaft]
		If (anal || vaginal || grinding || skull || shaft)
			return true
		EndIf
	EndIf
	return false
EndFunction

; ------------------------------------------------------- ;
; --- Stripping                                       --- ;
; ------------------------------------------------------- ;

Function SetStripping(int aiSlots, bool abStripWeapons, bool abApplyNow)
	_stripCstm = new int[2]
	_stripCstm[0] = aiSlots
	_stripCstm[1] = abStripWeapons as int
	If (abApplyNow && GetState() == STATE_PLAYING)
		int[] set
		_equipment = StripByDataEx(0x80, set, _stripCstm, _equipment)
		_ActorRef.QueueNiNodeUpdate()
	EndIf
EndFunction

Function DeleteCustomStripping()
	_stripCstm = new int[1]
EndFunction

Function DisableStripAnimation(bool abDisable)
	_DoUndress = !abDisable
EndFunction

Function SetAllowRedress(bool abAllowRedress)
	_AllowRedress = abAllowRedress
EndFunction

; ------------------------------------------------------- ;
; --- Strapon									                        --- ;
; ------------------------------------------------------- ;

Form function GetStrapon()
	return _Strapon
endFunction

bool function IsUsingStrapon()
	return _useStrapon && _Strapon && _ActorRef.IsEquipped(_Strapon)
endFunction

Function SetStrapon(Form ToStrapon)
	Error("Called from invalid state", "SetStrapon()")
EndFunction

; ------------------------------------------------------- ;
; --- Voice                                           --- ;
; ------------------------------------------------------- ;

String Function GetActorVoice() native

Function SetActorVoiceImpl(String asNewVoice) native
Function SetActorVoice(String asNewVoice, bool abForceSilent)
	SetActorVoiceImpl(asNewVoice)
	_IsForcedSilent = abForceSilent
EndFunction

bool Function IsSilent()
	return IsSilent
EndFunction

; ------------------------------------------------------- ;
; --- Expression                                      --- ;
; ------------------------------------------------------- ;

String Function GetActorExpression() native

Function SetActorExpressionImpl(String asExpression) native
Function SetActorExpression(String asExpression)
	SetActorExpressionImpl(asExpression)
	TryRefreshExpression()
EndFunction

Function SetMouthForcedOpen(bool abForceOpen)
	OpenMouth = abForceOpen
EndFunction

; ------------------------------------------------------- ;
; --- Pathing                                         --- ;
; ------------------------------------------------------- ;

int Property PATHING_DISABLE = -1 AutoReadOnly
int Property PATHING_ENABLE = 0 AutoReadOnly
int Property PATHING_FORCE = 1 AutoReadOnly

Function SetPathing(int aiPathingFlag)
	_PathingFlag = PapyrusUtil.ClampInt(_PathingFlag, PATHING_DISABLE, PATHING_FORCE)
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

sslThreadModel _Thread
sslSystemConfig _Config

Faction _AnimatingFaction
Actor _PlayerRef

; Constants
String Property STATE_IDLE 		= "Empty" AutoReadOnly
String Property STATE_SETUP		= "Ready" AutoReadOnly
String Property STATE_PAUSED	= "Paused" AutoReadOnly
String Property STATE_PLAYING = "Animating" AutoReadOnly

String Property TRACK_ADDED 	= "Added" AutoReadOnly
String Property TRACK_START 	= "Start" AutoReadOnly
String Property TRACK_ORGASM 	= "Orgasm" AutoReadOnly
String Property TRACK_END		 	= "End" AutoReadOnly

int Property LIVESTATUS_ALIVE 			= 0 AutoReadOnly
int Property LIVESTATUS_DEAD 				= 1 AutoReadOnly
int Property LIVESTATUS_UNCONSCIOUS = 2 AutoReadOnly

; ------------------------------------------------------- ;
; --- Alias Data                                      --- ;
; ------------------------------------------------------- ;

; Actor
Actor _ActorRef
Actor Property ActorRef
	Actor Function Get()
		return _ActorRef
	EndFunction
EndProperty

int _sex
int _raceID
bool _victim

int _livestatus
Actor _killer

int _AnimVarIsNPC
bool _AnimVarbHumanoidFootIKDisable

bool _ActorLocked

; Orgasms
int _OrgasmCount
bool _CanOrgasm
bool _hasOrgasm

; Stripping
int _stripData		; Strip data as provided by the animation
int[] _stripCstm	; Strip data as provided by the author -> [ArmorFlag, bStripWeapon]	
Form[] _equipment	; [HighHeelSpell, WeaponRight, WeaponLeft, Armor...]

bool _AllowRedress
bool property DoRedress
	bool function get()
		return _AllowRedress && (!IsVictim() || _Config.RedressVictim)
	endFunction
	function set(bool value)
		_AllowRedress = value
	endFunction
endProperty

bool _DoUndress
bool property DoUndress
	bool function get()
		return _Config.UndressAnimation && _DoUndress && GetState() != STATE_PLAYING
	endFunction
	function set(bool value)
		_DoUndress = value
	endFunction
endProperty

; Strapon
bool _useStrapon
Form _Strapon			; Strapon used by the animation
Form _HadStrapon	; Strapon worn prior to animation start

; Voice
bool _IsForcedSilent
float _VoiceDelay
float _ExpressionDelay

bool property IsSilent hidden
	bool function get()
		return _IsForcedSilent || GetActorVoice() == ""
	endFunction
endProperty

; Expressions
bool Property ForceOpenMouth Auto Hidden
bool Property OpenMouth
	bool Function Get()
		If (ForceOpenMouth)
			return true
		EndIf
		return ShouldMouthOpen()
	EndFunction
	Function Set(bool abSet)
		ForceOpenMouth = abSet
	EndFunction
EndProperty

; Pathing
int _PathingFlag
bool property DoPathToCenter
	bool function get()
		return _PathingFlag == PATHING_FORCE || (_PathingFlag == PATHING_ENABLE && _Config.DisableTeleport)
	endFunction
endProperty

; Time
float _StartedAt
float _LastOrgasm

; ------------------------------------------------------- ;
; --- Alias IDLE                                      --- ;
; ------------------------------------------------------- ;
;/
	An Idle state waiting for the owning thread to be initialized
	In this state, the reference is null and all related data is invalid

	This state will fill the alias and alias related data, then move to the "Ready" state
/;

Auto State Empty
	bool Function SetActor(Actor ProspectRef)
		ForceRefTo(ProspectRef)
		_ActorRef = ProspectRef
		If (_ActorRef.IsDead())
			_livestatus = LIVESTATUS_DEAD
			_killer = _ActorRef.GetKiller()
		ElseIf (_ActorRef.IsUnconscious())
			_livestatus = LIVESTATUS_UNCONSCIOUS
		Else
			_livestatus = LIVESTATUS_ALIVE
		EndIf
		_sex = SexLabRegistry.GetSex(_ActorRef, true)
		_raceID = SexLabRegistry.GetRaceID(_ActorRef)
		_ActorRef.SetFactionRank(_AnimatingFaction, 1)
		TrackedEvent(TRACK_ADDED)
		GoToState(STATE_SETUP)
		return true
	EndFunction

	Function Clear()
		If (GetIsDead())
			If (_ActorRef.IsEssential())
				_ActorRef.GetActorBase().SetEssential(false)
			EndIf
			_ActorRef.KillSilent(_killer)
		Else
			_Thread.RequestStatisticUpdate(_ActorRef, _StartedAt)
		EndIf
		_ActorRef.SetFactionRank(_AnimatingFaction, -1)
		_Thread.UpdateAnimatingActorMovement(_ActorRef) ;MOVEMENT_RELEASE
		Parent.Clear()
	EndFunction

	String Function GetActorName()
		return "EMPTY"
	EndFunction

	Event OnEndState()
		RegisterForModEvent("SSL_CLEAR_Thread" + _Thread.tid, "OnRequestClear")
	EndEvent
EndState

bool Function SetActor(Actor ProspectRef)
	Error("Not in idle phase", "SetActor")
	return false
EndFunction

Function NativeActorLockApplied()
	If (_ActorLocked)
		return
	EndIf
	_Thread.UpdateAnimatingActorMovement(_ActorRef) ;MOVEMENT_LOCK
	If (_ActorRef == _PlayerRef)
		_Config.ToggleVRIK(true, _Config.VRIK_FPP_HMD)
		If(_Config.AutoTFC)
			SexLabUtil.ToggleFreeCamera(1) ;TFC_ON
		EndIf
	EndIf
	Log("Locked Actor: " + GetActorName())
	_ActorLocked = True
EndFunction

; ------------------------------------------------------- ;
; --- Alias SETUP                                     --- ;
; ------------------------------------------------------- ;
;/
	Pre animation start. The alias is waiting for the underlying thread to begin the animation
/;

State Ready
	Event OnBeginState()
		RegisterForModEvent("SSL_PREPARE_Thread" + _Thread.tid, "OnDoPrepare")
	EndEvent

	Function SetStrapon(Form ToStrapon)
		_Strapon = ToStrapon
	EndFunction
	
	Function SetActorVoice(String asNewVoice, bool abForceSilent)
		_IsForcedSilent = abForceSilent
		If (asNewVoice)
			sslVoiceSlots.StoreVoice(ActorRef, asNewVoice)
		EndIf
	EndFunction

	Event OnDoPrepare(string asEventName, string asStringArg, float afNumArg, form akPathTo)
		UnregisterForModEvent("SSL_PREPARE_Thread" + _Thread.tid)
		WaitForPathToCenter(akPathTo)
		If (_sex <= 2)
			_AnimVarIsNPC = _ActorRef.GetAnimationVariableInt("IsNPC")
			_AnimVarbHumanoidFootIKDisable = _ActorRef.GetAnimationVariableBool("bHumanoidFootIKDisable")
		EndIf		
		GoToState(STATE_PAUSED)
		; Delayed Initialization
		If (_sex <= 2)
			If (_sex == 0)
				_VoiceDelay = _Config.MaleVoiceDelay
			Else
				_VoiceDelay = _Config.FemaleVoiceDelay
				If (_sex == 1)
					_HadStrapon = _Config.WornStrapon(_ActorRef)
					If (!_HadStrapon)
						_Strapon = _Config.GetStrapon()
					ElseIf (!_Strapon)
						_Strapon = _HadStrapon
					EndIf
				EndIf
			EndIf
		Else	; Creature
			_VoiceDelay = 3.0
		EndIf
		_ExpressionDelay = 1.0
		If (_Config.DebugMode)
			Log("Strapon[" + _Strapon + "] Voice[" + GetActorVoice() + "] Expression[" + GetActorExpression() + "]")
		EndIf
		If (asStringArg != "skip")
			_Thread.PrepareDone()
		EndIf
	EndEvent

	Function WaitForPathToCenter(form akPathTo)	
		If(_ActorRef == _PlayerRef)
			bool bVRTPP = _Config.HasVRIK && (_Config.POVModeVR == 2)
			Game.DisablePlayerControls(abMovement=!bVRTPP, abFighting=false, abCamSwitch=false, abLooking=false, abSneaking=false, abMenu=false, abActivate=false, abJournalTabs=false)
			return
		EndIf
		_Config.CheckBardAudience(_ActorRef, true)
		If(!akPathTo || !DoPathToCenter)
			return
		EndIf
		ObjectReference target = akPathTo as ObjectReference
		float distance = _ActorRef.GetDistance(target)		
		float target_distance = _Thread.CalcPathingTargetDistance(_raceID)
		If(distance > target_distance && distance <= 6144.0)
			_ActorRef.SetFactionRank(_AnimatingFaction, 2)
			_Thread.UpdateAnimatingActorMovement(_ActorRef) ;MOVEMENT_UNLOCK
			float fallback_timer = 15.0
			float prev_dist = distance + 1.0
			Utility.Wait(2.0)
			float interval = 0.05
			While (distance > target_distance && Math.abs(prev_dist - distance) > 0.5 && fallback_timer > 0)
				fallback_timer -= interval
				Utility.Wait(interval)
				prev_dist = distance
				distance = _ActorRef.GetDistance(target)
			EndWhile
		EndIf
	EndFunction

	Function Clear()
		GoToState(STATE_IDLE)
		Clear()
	EndFunction
	Function Initialize()
		Clear()
		Initialize()
	EndFunction

EndState

Event OnDoPrepare(string asEventName, string asStringArg, float afNumArg, form akPathTo)
	Error("Preparation request outside a valid state", "OnDoPrepare()")
EndEvent
Function WaitForPathToCenter(form akPathTo)
	Error("Pathing request outside a valid state", "WaitForPathToCenter()")
EndFunction

; --- Legacy

Event PrepareActor()
EndEvent
Function PathToCenter()
EndFunction

; ------------------------------------------------------- ;
; --- Alias PAUSED                                    --- ;
; ------------------------------------------------------- ;
;/
	Second idle state for a filled alias during or immediately before the actual animation start
	An actor in this state may walk around freely

	When this state is called initially, it is assumed that all relevant data has been set and the actor is waiting
	for strip information and the actual animation call
/;

State Paused
	Function LockActor()
		If (_ActorLocked)
			return
		EndIf
		_ActorRef.SetActorValue("Paralysis", 0.0)
		_ActorRef.SetFactionRank(_AnimatingFaction, 1)
		_ActorRef.SetAnimationVariableInt("IsNPC", 0)
		_ActorRef.SetAnimationVariableBool("bHumanoidFootIKDisable", 1)
		_Thread.UpdateAnimatingActorMovement(_ActorRef) ;MOVEMENT_LOCK
		If (_ActorRef == _PlayerRef)
			_Config.ToggleVRIK(true, _Config.VRIK_FPP_HMD)
			If(_Config.AutoTFC)
				SexLabUtil.ToggleFreeCamera(1) ;TFC_ON
			EndIf
		EndIf
		Log("Locked Actor: " + GetActorName())
		_ActorLocked = True
	EndFunction
	Function TryLockAndUnpause()
		LockActor()
		GoToState(STATE_PLAYING)
	EndFunction

	bool Function InitiateUndressing()
		If (_sex <= 2)
			If (DoUndress)
				If (_sex == 0)
					Debug.SendAnimationEvent(_ActorRef, "SexLab_MaleUndress")
				Else
					Debug.SendAnimationEvent(_ActorRef, "SexLab_FemaleUndress")
				EndIf
				return true
			EndIf
		EndIf
		return false
	EndFunction
	Function ReadyActor(int aiStripData, int aiPositionGenders)
		_stripData = aiStripData
		_useStrapon = _sex == 1 && Math.LogicalAnd(aiPositionGenders, 0x2) == 0
		If (_sex <= 2)
			_equipment = StripByData(_stripData, GetStripSettings(), _stripCstm)
			ResolveStrapon()
			_ActorRef.QueueNiNodeUpdate()
		EndIf
		Debug.SendAnimationEvent(_ActorRef, "SOSBend0")
		RegisterForModEvent("SSL_READY_Thread" + _Thread.tid, "OnStartPlaying")
	EndFunction

	Event OnStartPlaying(string asEventName, string asStringArg, float afNumArg, form akSender)
		UnregisterForModEvent("SSL_READY_Thread" + _Thread.tid)
		GoToState(STATE_PLAYING)
		TrackedEvent(TRACK_START)
		_StartedAt = SexLabUtil.GetCurrentGameRealTime()
		_LastOrgasm = _StartedAt
		_Thread.AnimationStart()
		If (_sex != 1 && _sex != 4)
			Utility.Wait(0.5)	; extra async call to ensure erection
			Debug.SendAnimationEvent(_ActorRef, "SOSBend0")
		EndIf
		_HomoTypes = _Thread.CheckActiveHomoTypes()
		UpdateBaseEnjoymentCalculations()
	EndEvent

	Function SetStrapon(Form ToStrapon)
		SetStraponAnimationImpl(ToStrapon)
	EndFunction
	Function ResolveStrapon(bool force = false)
		ResolveStraponImpl()
	EndFunction
	Function RemoveStrapon()
		If(_Strapon && !_HadStrapon)
			_ActorRef.RemoveItem(_Strapon, 1, true)
		EndIf
	EndFunction

	Function UnlockActor()
		_ActorRef.SetAnimationVariableInt("IsNPC", _AnimVarIsNPC)
		_ActorRef.SetAnimationVariableBool("bHumanoidFootIKDisable", _AnimVarbHumanoidFootIKDisable)
		If (_ActorRef == _PlayerRef)
			SexLabUtil.ToggleFreeCamera(0) ;TFC_OFF
			_Config.ToggleVRIK(false)
		EndIf
		SetActorCollisions(true)
		_ActorRef.SetFactionRank(_AnimatingFaction, 0)
		_Thread.UpdateAnimatingActorMovement(_ActorRef) ;MOVEMENT_UNLOCK
		Log("Unlocked Actor: " + GetActorName())
		_ActorLocked = False
	EndFunction
	Function TryPauseAndUnlock()
		UnlockActor()
	EndFunction

	Function Clear()
		If (_sex <= 2)
			Redress()
			RemoveStrapon()
			_ActorRef.ClearExpressionOverride() ;error with _sex>2
		EndIf
		If (_ActorLocked)
			UnlockActor()
		EndIf
		If (sslLovense.IsLovenseInstalled())
			sslLovense.StopAllActions()
		EndIf
		_Thread.SetAnimationPlaybackSpeed(1.0)
		UnregisterForModEvent("SSL_ORGASM_Thread" + _Thread.tid)
		StoreExcitementState("Backup")
		sslBaseExpression.CloseMouth(_ActorRef)
		_ActorRef.ResetExpressionOverrides()
		sslBaseExpression.ClearMFG(_ActorRef)
		SendDefaultAnimEvent()
		TrackedEvent(TRACK_END)
		GoToState(STATE_IDLE)
		Clear()
	EndFunction
	Function Initialize()
		Clear()
		Initialize()
	EndFunction
EndState

bool Function InitiateUndressing()
	Error("Cannot undress actors outside of idle state", "InitiateUndressing()")
	return false
EndFunction
Function ReadyActor(int aiStripData, int aiPositionGenders)
	Error("Cannot ready outside of idle state", "ReadyActor()")
EndFunction
Event OnStartPlaying(string asEventName, string asStringArg, float afNumArg, form akSender)
	Error("Playing request outside of idle state", "OnStartPlaying()")
EndEvent
Function RemoveStrapon()
	Error("Removing strapon from invalid state", "RemoveStrapon()")
EndFunction

;	Lock/Unlock actor if in idling state, otherwise do nothing
Function LockActor()
	Error("Cannot lock actor outside of paused state", "LockActor()")
EndFunction
Function UnlockActor()
	Error("Cannot unlock actor outside of paused state", "UnlockActor()")
EndFunction
Function TryLockAndUnpause()
EndFunction
; This stays functional in STATE_PLAYING as well
Function TryPauseAndUnlock()
EndFunction

Function SetActorCollisions(bool abEnable) native

Form[] Function StripByData(int aiStripData, int[] aiDefaults, int[] aiOverwrites) native

; ------------------------------------------------------- ;
; --- Alias PLAYING                                   --- ;
; ------------------------------------------------------- ;
;/
	Main logic loop for in-animation actors
	This state will handle requipment status, orgasms, sounds, etc

	This section is divided into 2 parts:
	First the "paused" part which stores actors as they can move around freely and have no further actions applied to them,
	this is the state they originaly go into from the Setup state and will wait for further intrusctions (stripping, lock position, ...)
	And the "Playing" state, in this state actors are assumed in the animation and have voice logic and all applied to them
/;

float Property UPDATE_INTERVAL = 0.250 AutoReadOnly Hidden

float _LoopVoiceDelay
float _LoopExpressionDelay
float _LoopLovenseDelay
bool _LovenseGenital
bool _LovenseAnal

bool[] _CurrentInteractions
bool[] _HomoTypes

int _VRIKRestoreInTicks

State Animating
	Event OnBeginState()
		RegisterForModEvent("SSL_ORGASM_Thread" + _Thread.tid, "OnOrgasm")
		_LoopLovenseDelay = 0
		_LovenseGenital = false
		_LovenseAnal = false
		RegisterForSingleUpdate(UPDATE_INTERVAL)
	EndEvent

	Function UpdateNext(int aiStripData)
		If (_stripData != aiStripData)
			_stripData = aiStripData
			_equipment = StripByDataEx(_stripData, GetStripSettings(), _stripCstm, _equipment)
			_ActorRef.QueueNiNodeUpdate()
		EndIf
		_VoiceDelay -= Utility.RandomFloat(0.1, 0.3)
		if _VoiceDelay < 0.8
			_VoiceDelay = 0.8
		endIf
	EndFunction

	Function SetStrapon(Form ToStrapon)
		SetStraponAnimationImpl(ToStrapon)
	EndFunction
	Function ResolveStrapon(bool force = false)
		ResolveStraponImpl()
	EndFunction

	Event OnUpdate()
		If ((_Thread.GetStatus() != _Thread.STATUS_INSCENE) || (GetState() != STATE_PLAYING))
			return
		EndIf
		If (SexLabUtil.IsGamePausedOrFrozen())
			RegisterForSingleUpdate(UPDATE_INTERVAL)
			return
		EndIf
		_CurrentInteractions = _Thread.ListDetectedInteractionsInternal(_ActorRef)
		; Keep the remainder so the 0.8s enjoyment loop stays accurate on 0.25s updates.
		If (_bEnjEnabled && !_Thread.EnjoymentPaused)
			_LoopEnjoymentDelay += UPDATE_INTERVAL
		EndIf
		UpdateEffectiveEnjoymentCalculations()
		If (_bEnjEnabled && !_Thread.EnjoymentPaused && _Thread.ElementUI_EnjBars)
			EnjBarsUpdateSlider(_FullEnjoyment as float, _Thread.GetCurrentInteractionString(_ActorRef))
		EndIf
		int strength = CalcReaction()
		; Reaction uses absolute pain for voices/expressions, orgasms only use positive enjoyment.
		If (_FullEnjoyment >= 100)
			DoOrgasm()
		EndIf
		If (_LoopVoiceDelay >= _VoiceDelay && !IsSilent)
			_LoopVoiceDelay = 0.0
			bool lipsync = !OpenMouth && _Config.UseLipSync && _sex <= 2
			Sound snd = _Thread.GetAliasSound(Self, GetActorVoice(), strength)
			sslBaseVoice.PlaySound(_ActorRef, snd, strength, lipsync)
		EndIf
		If (_LoopExpressionDelay >= _ExpressionDelay)
			_LoopExpressionDelay = 0.0
			RefreshExpressionEx(strength)
		EndIf
		If (_LoopLovenseDelay <= 0)
			RefreshLovenseActions()
		Else
			_LoopLovenseDelay -= UPDATE_INTERVAL
		EndIf
		; VRIK (Comeback: Is this always needed or upon offset changes only?) (How does this interfere with POV switching?)
		If ((_ActorRef == _PlayerRef) && (_Config.POVModeVR == _Config.VRIK_FPP_FREE))
			_VRIKRestoreInTicks = _Config.UpdatePositioningVRIK(_VRIKRestoreInTicks)
			If (_VRIKRestoreInTicks < 0)
				_VRIKRestoreInTicks = 0
				_Config.RestoreHmdVRIK()
			EndIf
			If (_VRIKRestoreInTicks > 0)
				_VRIKRestoreInTicks -= 1
				If _VRIKRestoreInTicks <= 1
					_VRIKRestoreInTicks = -1
				EndIf
			EndIf
		EndIf
		; Loop
		_LoopVoiceDelay += UPDATE_INTERVAL
		_LoopExpressionDelay += UPDATE_INTERVAL
		RegisterForSingleUpdate(UPDATE_INTERVAL)
	EndEvent

	Function TryRefreshExpression()
		RefreshExpression()
	EndFunction
	Function RefreshExpressionEx(float afStrength)
		If (_sex > 2)
			return
		ElseIf (sslBaseExpression.IsMouthOpen(_ActorRef))
			If (!OpenMouth)
				sslBaseExpression.CloseMouth(_ActorRef)
			EndIf
		ElseIf (OpenMouth)
			sslBaseExpression.OpenMouth(_ActorRef)
			Utility.Wait(0.7)
		EndIf
		String expression = GetActorExpression()
		If (expression && _Config.UseExpressions && _livestatus == LIVESTATUS_ALIVE)
			sslBaseExpression.ApplyExpression(expression, _ActorRef, afStrength)
		EndIf
		If (_Config.DebugMode)
			Log("Expression? " + expression + "; Strength? " + afStrength + "; OpenMouth? " + OpenMouth, "sslBaseExpression.ApplyExpression()")
		EndIf
	EndFunction

	Function RefreshLovenseActions()
		If ((_ActorRef != _PlayerRef) || (!sslLovense.IsLovenseInstalled()))
			return
		EndIf
		int lovenseStrength = sslSystemConfig.GetSettingInt("iLovenseStrength")
		bool LovenseGenital = IsGenitalInteraction()
		bool LovenseAnal = IsAnalPenetrated()
		If (!LovenseGenital && !LovenseAnal && (_LovenseGenital || _LovenseAnal))
			sslLovense.StopAllActions()
		Else
			If (LovenseGenital)
				If (!_LovenseGenital)
					sslLovense.StartGenitalAction(lovenseStrength)
				EndIf
			ElseIf (_LovenseGenital)
				sslLovense.StopGenitalAction(!LovenseAnal)
			EndIf
			If (LovenseAnal)
				If (!_LovenseAnal)
					sslLovense.StartAnalAction(lovenseStrength)
				EndIf
			ElseIf (_LovenseAnal)
				sslLovense.StopAnalAction(!LovenseGenital)
			EndIf
		EndIf
		_LovenseGenital = LovenseGenital
		_LovenseAnal = LovenseAnal
	EndFunction

	Function PlayLouder(Sound SFX, ObjectReference FromRef, float Volume)
		Sound.SetInstanceVolume(SFX.Play(FromRef), Volume)
	EndFunction

	Event OnOrgasm(string eventName, string strArg, float numArg, Form sender)
		DoOrgasm()
	EndEvent
	Function DoOrgasm(bool Forced = false)
		If (_hasOrgasm)
			return
		EndIf
		_hasOrgasm = true
		If (!Forced)
			If (!_CanOrgasm)
				_hasOrgasm = false
				return
			EndIf
			float time = SexLabUtil.GetCurrentGameRealTime()
			int cmp = 10
			If (_sex == 0 || _sex == 3)
				cmp = 20
			EndIf
			If (time - _LastOrgasm < cmp)
				_hasOrgasm = false
				return
			EndIf
		EndIf
		UnregisterForUpdate()
		_OrgasmCount += 1
		; SFX
		If(_Config.OrgasmEffects)
			If (_ActorRef == _PlayerRef)
				If (_Config.HasVRIK)
					_Config.DoWhiteOutEfffect(_OrgasmCount)
				ElseIf (_Config.ShakeStrength > 0 && Game.GetCameraState() >= 8)
					Game.ShakeCamera(none, _Config.ShakeStrength, _Config.ShakeStrength + 1.0)
				EndIf
			EndIf
			If (!IsSilent)
				Sound snd = _Thread.GetAliasOrgasmSound(Self, GetActorVoice())
				PlayLouder(snd, _ActorRef, _Config.VoiceVolume)
			EndIf
			PlayLouder(_Config.OrgasmFX, _ActorRef, _Config.SFXVolume)
			If (sslLovense.IsLovenseInstalled())
				_LoopLovenseDelay = sslSystemConfig.GetSettingFlt("fLovenseDurationOrgasm")
				int strength = sslSystemConfig.GetSettingInt("iLovenseStrengthOrgasm")
				float duration = sslSystemConfig.GetSettingInt("fLovenseDurationOrgasm")
				sslLovense.StartOrgasmAction(strength, duration)
			EndIf
		EndIf
		If (_sex != 1 && _sex != 4)
			_Thread.ApplyCumFX(_ActorRef)
		EndIf
		; Events
		int eid = ModEvent.Create("SexLabOrgasm")
		ModEvent.PushForm(eid, _ActorRef)
		ModEvent.PushInt(eid, _FullEnjoyment)
		ModEvent.PushInt(eid, _OrgasmCount)
		ModEvent.Send(eid)
		Int handle = ModEvent.Create("SexlabOrgasmSeparate")
		ModEvent.PushForm(handle, _ActorRef)
		ModEvent.PushInt(handle, _Thread.tid)
		ModEvent.Send(handle)
		TrackedEvent(TRACK_ORGASM)
		_LastOrgasm = SexLabUtil.GetCurrentGameRealTime()
		; Enjoyment
		If (_bEnjEnabled)
			_FullEnjoyment = 0
			_PainLevel = 0.0
			_PainApplied = 0.0
			_arousalBase = 0
			SexlabStatistics.SetStatistic(_ActorRef, 17, _arousalBase)
			_EnjFactor = _BaseFactor
			If (_sex == 0 || _sex == 3)
				If (_OrgasmCount > _Config.MaxNoPainOrgasmMale)
					_FullEnjoyment -= (_OrgasmCount - _Config.MaxNoPainOrgasmMale) * 20
				EndIf
				_Thread.EnjBasedSkipToLastStage(_Config.MaleOrgasmEndsScene)
			Else
				If (_OrgasmCount > _Config.MaxNoPainOrgasmFemale)
					_FullEnjoyment -= (_OrgasmCount - _Config.MaxNoPainOrgasmFemale) * 20
				EndIf
			EndIf
			; The extra orgasm penalty can stack, but it shouldn't go below -100.
			_FullEnjoyment = PapyrusUtil.ClampInt(_FullEnjoyment, -100, 100)
			UpdateEffectiveEnjoymentCalculations()
		EndIf
		RegisterForSingleUpdate(UPDATE_INTERVAL)
		_hasOrgasm = false
		Log(GetActorName() + ": Orgasms[" + _OrgasmCount + "] FullEnjoyment [" + _FullEnjoyment + "]")
	EndFunction
	
	Function ResetPosition(int aiStripData, int aiPositionGenders)
		_stripData = aiStripData
		_equipment = StripByDataEx(_stripData, GetStripSettings(), _stripCstm, _equipment)
		_useStrapon = _sex == 1 && Math.LogicalAnd(aiPositionGenders, 0x2) == 0
		ResolveStrapon()
		_ActorRef.QueueNiNodeUpdate()
	EndFunction

	Function TryPauseAndUnlock()
		GoToState(STATE_PAUSED)
		UnlockActor()
	EndFunction

	Function Clear()
		GoToState(STATE_PAUSED)
		Clear()
	EndFunction
	Function Initialize()
		Clear()
		Initialize()
	EndFunction

	Event OnEndState()
		UnregisterForUpdate()
		SendDefaultAnimEvent()
	EndEvent
EndState

Function UpdateNext(int aiStripData)
	Error("Cannot update to next stage outside of playing state", "UpdateNext()")
EndFunction
Function ResetPosition(int aiStripData, int aiPositionGenders)
	Error("Cannot reset position outside of playing state", "ResetPosition()")
EndFunction
function RefreshExpression()
	int strength = CalcReaction()
	RefreshExpressionEx(strength)
endFunction
Function RefreshExpressionEx(float afStrength)
	Error("Cannot refresh expression outside of playing state", "RefreshExpressionEx()")
EndFunction
Function RefreshLovenseActions()
	Error("Cannot process lovense actions outside of playing state", "RefreshLovenseActions()")
EndFunction

function DoOrgasm(bool Forced = false)
	Error("Cannot create an orgasm outside of playing state", "DoOrgasm()")
endFunction
Function PlayLouder(Sound SFX, ObjectReference FromRef, float Volume)
	Error("Cannot play sound outside of playing state", "PlayLouder()")
EndFunction
Event OnOrgasm(string eventName, string strArg, float numArg, Form sender)
	Error("Cannot create orgasm effects outside of playing state", "OnOrgasm()")
EndEvent

Function TryRefreshExpression()
EndFunction

Form[] Function StripByDataEx(int aiStripData, int[] aiDefaults, int[] aiOverwrites, Form[] akMergeWith) native

; ------------------------------------------------------- ;
; --- State Independent                               --- ;
; ------------------------------------------------------- ;
;/
	Main logic loop for in-animation actors
	This state will handle requipment status, orgasms, sounds, etc
/;

Function SendDefaultAnimEvent(bool Exit = False)
	Debug.SendAnimationEvent(_ActorRef, "AnimObjectUnequip")
	Debug.SendAnimationEvent(_ActorRef, "IdleForceDefaultState")
	If(_sex <= 2)
		return
	EndIf
	Debug.SendAnimationEvent(_ActorRef, "ReturnDefaultState")		; chicken, hare and slaughterfish before the "ReturnToDefault"
	Debug.SendAnimationEvent(_ActorRef, "ReturnToDefault")			; rest creature-animal
	Debug.SendAnimationEvent(_ActorRef, "FNISDefault")				; dwarvenspider and chaurus
	Debug.SendAnimationEvent(_ActorRef, "IdleReturnToDefault")		; Werewolves and VampirwLords
	Debug.SendAnimationEvent(_ActorRef, "ForceFurnExit")			; Trolls afther the "ReturnToDefault" and draugr, daedras and all dwarven exept spiders
	Debug.SendAnimationEvent(_ActorRef, "Reset")					; Hagravens afther the "ReturnToDefault" and Dragons
EndFunction

function TrackedEvent(string EventName)
	sslThreadLibrary.SendTrackingEvents(_ActorRef, EventName, _Thread.tid)
endFunction

Function ResolveStrapon(bool force = false)
	Error("Called from invalid state", "ResolveStrapon()")
EndFunction
Function SetStraponAnimationImpl(Form akNewStrapon)
	If (_Strapon == akNewStrapon)
		return
	ElseIf (_Strapon && !_HadStrapon)
		_ActorRef.RemoveItem(_Strapon, 1, true)
	EndIf
	_Strapon = akNewStrapon
	ResolveStrapon()
EndFunction
Function ResolveStraponImpl()
	If (!_Strapon)
		return
	EndIf
	bool equipped = _ActorRef.IsEquipped(_Strapon)
	If(!equipped && _useStrapon)
		_ActorRef.EquipItem(_Strapon, true, true)
	ElseIf(equipped && !_useStrapon)
		_ActorRef.UnequipItem(_Strapon, true, true)
	EndIf
EndFunction

int[] Function GetStripSettings()
	If (_Thread.IsConsent())
		return sslSystemConfig.GetStripForms(_sex == 1 || _sex == 2, false)
	Else
		return sslSystemConfig.GetStripForms(IsVictim(), true)
	EndIf
EndFunction

Function Redress()
	If (!DoRedress)
		return
	EndIf
	; _equipment := [HighHeelSpell, WeaponRight, WeaponLeft, Armor...]
	If(_equipment[1])
		_ActorRef.EquipItemEx(_equipment[1], _ActorRef.EquipSlot_RightHand, equipSound = false)
	EndIf
	If(_equipment[2])
		_ActorRef.EquipItemEx(_equipment[2], _ActorRef.EquipSlot_LeftHand, equipSound = false)
	EndIf
	int i = 3
	While (i < _equipment.Length)
		_ActorRef.EquipItemEx(_equipment[i], _ActorRef.EquipSlot_Default, equipSound = false)
		i += 1
	EndWhile
	Spell HDTHeelSpell = _equipment[0] as Spell
	If(HDTHeelSpell && _ActorRef.GetWornForm(0x00000080) && !_ActorRef.HasSpell(HDTHeelSpell))
		_ActorRef.AddSpell(HDTHeelSpell, false)
	EndIf
EndFunction

; ------------------------------------------------------- ;
; --- Initialization                                  --- ;
; ------------------------------------------------------- ;
;/
	Functions for re/initialization
/;

; Only called on re/initialization of the owning Thread
Function Setup()
	Form SexLabQuestFramework = Game.GetFormFromFile(0xD62, "SexLab.esm")
	_Config = SexLabQuestFramework as sslSystemConfig

	_Thread = GetOwningQuest() as sslThreadModel
	_AnimatingFaction = _Config.AnimatingFaction
	_PlayerRef = Game.GetPlayer()

	Initialize()
EndFunction

; Initialize will clear the alias and reset all of the data accordingly
Function Initialize()
	UnregisterForAllModEvents()
	TryToClear()
	; Forms
	_ActorRef = none
	_HadStrapon = none
	_Strapon = none
	; Voice
	_IsForcedSilent = false
	; Flags
	_victim = false
	_CanOrgasm = true
	_hasOrgasm = false
	_DoUndress = true
	_AllowRedress = true
	ForceOpenMouth = false
	_ActorLocked = false
	; Integers
	_sex = -1
	_raceID = -1
	_livestatus = 0
	_PathingFlag = 0
	_OrgasmCount = 0
	_stripCstm = new int[1]
	; Floats
	_LastOrgasm = 0.0
	_StartedAt = 0.0
	; Array
	_CurrentInteractions = Utility.CreateBoolArray(_Thread.SUPPORTED_INTER_COUNT, false)
	_HomoTypes = Utility.CreateBoolArray(5, false)
	ResetEnjoymentVariables()
	_VRIKRestoreInTicks = 0
EndFunction

Event OnRequestClear(string asEventName, string asStringArg, float afDoStatistics, form akSender)
	Clear()
EndEvent

; ------------------------------------------------------- ;
; --- Escape Events                                   --- ;
; ------------------------------------------------------- ;
;/
	Events which if triggered should stop the underlying animation
/;

Event OnCellDetach()
	Log("An Alias is out of range and cannot be animated anymore. Stopping Thread...")
	_Thread.EndAnimation()
EndEvent
Event OnUnload()
	Log("An Alias is out of range and cannot be animated anymore. Stopping Thread...")
	_Thread.EndAnimation()
EndEvent
Event OnDying(Actor akKiller)
	Log("An Alias is dying and cannot be animated anymore. Stopping Thread...")
	_Thread.EndAnimation()
EndEvent

; ------------------------------------------------------- ;
; --- Logging                                         --- ;
; ------------------------------------------------------- ;
;/
	Generic logging utility
/;

function Log(string msg, string src = "")
	msg = "Thread[" + _Thread.tid + "] ActorAlias[" + GetActorName() + "] State [" + GetState() + "] " + msg
	sslLog.Log(msg)
endFunction

Function Error(String msg, string src = "")
	msg = "Thread[" + _Thread.tid + "] ActorAlias[" + GetActorName() + "] State [" + GetState() + "] " + msg
	sslLog.Error(msg)
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
; COMEBACK: This is probably better off moved into the C++ instance. Prbly wanna do this when the enjoyment is considered complete.

Function EnjBarsUpdateSlider(float afEnjoyment, string asInteractions) native
Function RegisterRaiseEnjAttempt(float afNextTimeCycle) native

; Defaults
float _EnjoymentDelay
float _LoopEnjoymentDelay
; Base
bool _bEnjEnabled
bool _CrtMaleHugePP
int _ConSubStatus
float _PainContext
float _EnjFactor
float _BaseFactor
float _arousalBase
; Interaction
float _InterFactor
; Effective
float _ModEnjMult
float _arousalStat
float _PainInterTimer
float _PainInterCur
float _PainLevel
float _PainApplied
int _FullEnjoyment

Function ResetEnjoymentVariables()
	; Defaults
	_EnjoymentDelay = 0.8
	_LoopEnjoymentDelay = 0.0
	; Base
	_bEnjEnabled = False
	_CrtMaleHugePP = False
	_ConSubStatus = _Thread.CONSENT_CONNONSUB
	_PainContext = 0.0
	_EnjFactor = 0.0
	_BaseFactor = 0.0
	_arousalBase = 0.0
	; Interaction
	_InterFactor = 0.0
	; Effective
	_ModEnjMult = 1.0
	_arousalStat = 0.0
	_PainInterTimer = 0.0
	_PainInterCur = 0.0
	_PainLevel = 0.0
	_PainApplied = 0.0
	_FullEnjoyment = 0
EndFunction

Function UpdateBaseEnjoymentCalculations()
	If (!_Config.InternalEnjoymentEnabled || !_Config.SeparateOrgasms || GetIsDead() || !_Thread.HasPlayer)
		return
	EndIf
	ResetEnjoymentVariables()
	_bEnjEnabled = True
	StoreExcitementState("Restore")
	_CrtMaleHugePP = _Thread.CrtMaleHugePP()
	_ConSubStatus = _Thread.IdentifyConsentSubStatus()
	bool SameSexThread = (_HomoTypes[1] || _HomoTypes[2] || _HomoTypes[3])
	bool WithLover  = _Thread.ActorIsWithLover(_ActorRef)
	_arousalBase = PapyrusUtil.ClampFloat(SexlabStatistics.GetStatistic(_ActorRef, 17), 0.0, 100.0)
	_PainContext = CalcContextPain()
	_EnjFactor = CalcContextEnjFactor(SameSexThread, WithLover)
	_BaseFactor = _EnjFactor
	If (_Config.DebugMode)
		DebugBaseCalcVariables()
	EndIf
EndFunction

Function UpdateEffectiveEnjoymentCalculations()
	If (!_bEnjEnabled)
		return
	EndIf
	If (_FullEnjoyment >= 100)
		_FullEnjoyment = 100
		DoOrgasm()
		return
	EndIf
	If ((_Thread.EnjoymentPaused) || (_LoopEnjoymentDelay < _EnjoymentDelay))
		return
	EndIf
	bool NoStaminaEndScenario = (_Config.NoStaminaEndsScene && !_victim && _ActorRef.GetActorValuePercentage("Stamina") < 0.10)
	If (NoStaminaEndScenario)
		_Thread.EnjBasedSkipToLastStage(true)
		return
	EndIf
	_LoopEnjoymentDelay -= _EnjoymentDelay
	If (_LoopEnjoymentDelay < 0.0)
		_LoopEnjoymentDelay = 0.0
	EndIf
	_InterFactor = _Thread.CalculateInteractionFactor(_ActorRef, _CurrentInteractions)
	; A single update can jump past either end, so clamp it before anything else uses it.
	_FullEnjoyment = PapyrusUtil.ClampInt(CalcEffectiveEnjoyment() as int, -100, 100)
	UpdateArousalStat()
	If (_Config.DebugMode)
		DebugEffectiveCalcVariables()
	EndIf
EndFunction

float Function CalcContextPain()
	float PainContext = 0.0
	bool SubsPresent = _Thread.GetSubmissives().Length
	If (!SubsPresent || (SubsPresent && !_victim))
		return 0
	ElseIf (_Thread.HasSceneTag("Humiliation"))
		return 25
	ElseIf (_Thread.HasSceneTag("Forced"))
		return 35
	ElseIf (_Thread.HasSceneTag("Ryona"))
		return 45
	EndIf
	If (_Thread.HasSceneTag("Spanking"))
		PainContext += 5
	EndIf
	If (_Thread.HasSceneTag("Dominant"))
		PainContext += 10
	EndIf
	If (_Thread.HasSceneTag("Asphyxiation"))
		PainContext += 15
	EndIf
	return PainContext
EndFunction

float Function CalcContextEnjFactor(bool SameSexThread, bool WithLover)
	float EnjMult = 1.0
	;arousal
	EnjMult += (_arousalBase / 50)
	;creature
	if (_raceID > 0)
		return (EnjMult + 0.5)
	EndIf
	;relation
	If WithLover
		EnjMult += 1.0
	EndIf
	;sexuality
	int actorSexuality = SexlabStatistics.GetSexuality(_ActorRef)
	If (actorSexuality == 0 && SameSexThread) || (actorSexuality == 1 && !SameSexThread)
		EnjMult -= 0.5
	EndIf
	;context
	If _ConSubStatus > _Thread.CONSENT_NONCONNONSUB
		If _victim
			EnjMult -= 0.35
		ElseIf !_victim
			EnjMult += 0.30
		EndIf
	EndIf
	return EnjMult
EndFunction

float Function CalcInteractionPain()
	float vaginalXP = SexlabStatistics.GetStatistic(_ActorRef, 2)
	float analXP = SexlabStatistics.GetStatistic(_ActorRef, 3)
	bool PainCondVaginal = _CurrentInteractions[_Thread.pVaginal] && (vaginalXP < _Config.NoPainRequiredXP as float)
	bool PainCondAnal = _CurrentInteractions[_Thread.pAnal] && (analXP < _Config.NoPainRequiredXP as float)
	float PainInter = 0.0
	If (PainCondVaginal || PainCondAnal)
		_PainInterTimer += _EnjoymentDelay
		; Only use XP for the penetration that's actually causing pain.
		float relevantXP
		If (PainCondVaginal && PainCondAnal)
			relevantXP = (vaginalXP + analXP) / 2
		ElseIf (PainCondVaginal)
			relevantXP = vaginalXP
		Else
			relevantXP = analXP
		EndIf
		float PainFactor = (2 - (0.01 * relevantXP))
		PainFactor = PapyrusUtil.ClampFloat(PainFactor, 0, 2)
		If (_CrtMaleHugePP)
			PainFactor += _Config.PainHugePPMult
		EndIf
		PainInter = (PainFactor * _InterFactor * _Config.EnjRaiseMultInter)
	EndIf
	return PainInter
EndFunction

float Function CalcEffectivePain()
	float NoPainTime = _Config.NoPainRequiredTime as float
	float SceneDuration = _Thread.GetTimeTotal()
	float DecayMult = 0.0
	float _PainContextCur = 0.0
	_PainInterCur = 0
	If (NoPainTime < 1)
		NoPainTime = 1
	EndIF
	; Context Pain
	If (SceneDuration < NoPainTime)
		DecayMult = 1.0 - (SceneDuration / NoPainTime)
		DecayMult = PapyrusUtil.ClampFloat(DecayMult, 0, 1)
		_PainContextCur = _PainContext * DecayMult
	EndIf
	; Interaction Pain
	If (_PainInterTimer < NoPainTime)
		; This is the current pain level, not an amount to subtract every update.
		float PainInter = CalcInteractionPain() * 5
		float PainInterDecay = 1.0 - (_PainInterTimer / NoPainTime)
		_PainInterCur = PainInter * PapyrusUtil.ClampFloat(PainInterDecay, 0, 1)
	EndIf
	return (_PainContextCur + _PainInterCur)
EndFunction

float Function CalcEffectiveEnjoyment()
	; ConSubMult [Default: 0.8 to 1.2], EnjRaiseMultInter [Default: 0.8], EnjFactor [Range: 0.17 to 4.3]
	float ConSubMult = EnjFindConSubStatusMult()
	float EffectivePain = CalcEffectivePain()
	float EnjInter = 0.0
	If !((_PainInterCur > 10) || (_Config.GameRequiredOnHighEnj && (_FullEnjoyment > 80) && (_ActorRef == _PlayerRef)))
		EnjInter = (_EnjFactor * _InterFactor * _Config.EnjRaiseMultInter * ConSubMult * _ModEnjMult)
	EndIf
	float EnjoymentBeforePain = _FullEnjoyment + EnjInter
	float PainChange = EffectivePain - _PainLevel
	_PainLevel = EffectivePain
	; Only apply the change in pain level. Decay releases the pain that was actually applied.
	If (PainChange > 0)
		float PainRoom = EnjoymentBeforePain + 100
		If (PainRoom > 0)
			float PainAdded = PapyrusUtil.ClampFloat(PainChange, 0, PainRoom)
			_PainApplied += PainAdded
			return (EnjoymentBeforePain - PainAdded)
		EndIf
	ElseIf (PainChange < 0 && _PainApplied > 0)
		float PainReleased = PapyrusUtil.ClampFloat(-PainChange, 0, _PainApplied)
		_PainApplied -= PainReleased
		float EnjoymentRoom = 100 - EnjoymentBeforePain
		If (EnjoymentRoom > 0)
			PainReleased = PapyrusUtil.ClampFloat(PainReleased, 0, EnjoymentRoom)
			return (EnjoymentBeforePain + PainReleased)
		EndIf
	EndIf
	return EnjoymentBeforePain
EndFunction

Function UpdateArousalStat()
	_arousalStat = _arousalBase + (_FullEnjoyment as float / 2)
	SexlabStatistics.SetStatistic(_ActorRef, 17, _arousalStat)
EndFunction

float Function EnjFindConSubStatusMult()
	float ret = 1.0
	If _ConSubStatus == _Thread.CONSENT_NONCONSUB
		If _victim
			ret = _Config.EnjMultVictim
		Else
			ret = _Config.EnjMultAggressor
		EndIf
	ElseIf _ConSubStatus == _Thread.CONSENT_CONSUB
		If _victim
			ret = _Config.EnjMultSub
		Else
			ret = _Config.EnjMultDom
		EndIf
	EndIf
	return ret
EndFunction

int function CalcReaction()
	If (_bEnjEnabled)
		int ret = Math.Abs(_FullEnjoyment) as int
		return PapyrusUtil.ClampInt(ret, 0, 100)
	EndIf
	return 50
EndFunction

bool Function WaitForOrgasm()
	If (!_bEnjEnabled || !_CanOrgasm)
		return False
	EndIf
	bool EnjScenario = (_Config.HighEnjOrgasmWait && (_FullEnjoyment > 80))
	bool PlayerSceanrio = (_Config.PlayerMustOrgasm && (_ActorRef == _PlayerRef) && _OrgasmCount == 0)
	bool DomScenario = (_Config.DomMustOrgasm && !_victim && _OrgasmCount == 0 && \
	(_ConSubStatus == _Thread.CONSENT_CONSUB || _ConSubStatus == _Thread.CONSENT_NONCONSUB))
	If (EnjScenario || DomScenario || PlayerSceanrio)
		return True
	EndIf
	return False
EndFunction

Function StoreExcitementState(String arg = "")
	; Form IDs keep same-name actors from sharing enjoyment backups.
	string ActorKey = _ActorRef.GetFormID() as string
	If (arg == "Backup")
		StorageUtil.SetFloatValue(None, ("EnjBackupTime_" + ActorKey),  SexLabUtil.GetCurrentGameRealTime())
		StorageUtil.SetIntValue(None, ("LastOrgasmCount_" + ActorKey), _OrgasmCount)
		If _FullEnjoyment > 10
			StorageUtil.SetIntValue(None, ("LastEnjoyment_" + ActorKey), _FullEnjoyment)
		Else
			StorageUtil.SetIntValue(None, ("LastEnjoyment_" + ActorKey), 0)
		EndIf
	ElseIf (arg == "Restore")
		float TimeSinceEnjBackup = (SexLabUtil.GetCurrentGameRealTime() - StorageUtil.GetFloatValue(None, ("EnjBackupTime_" + ActorKey)))
		If (TimeSinceEnjBackup >= 0 && TimeSinceEnjBackup < 60)
			_OrgasmCount = StorageUtil.GetIntValue(None, ("LastOrgasmCount_" + ActorKey))
			int LastEnjoyment = StorageUtil.GetIntValue(None, ("LastEnjoyment_" + ActorKey))
			; Decay the saved enjoyment, then keep it in the normal range.
			_FullEnjoyment = PapyrusUtil.ClampInt((LastEnjoyment as float * (1 - (TimeSinceEnjBackup/60))) as int, -100, 100)
		EndIf
	EndIf
EndFunction

Function InitRaiseEnjAttempt()
	If (_ActorRef != _PlayerRef)
		return
	EndIf
	If (_FullEnjoyment >= 100)
		_FullEnjoyment = 100
		DoOrgasm()
		return
	EndIf
	; As enjoyment gets higher, the "green zone" gets narrower
	; At 80 Enj (2.0s nextTimeCycle): Window will be 25% of the bar (0.375 to 0.625)
	; At 100 Enj (0.8s nextTimeCycle): Window will be 15% of the bar (0.425 to 0.575)
	float nextTimeCycle = 6.8 - (_FullEnjoyment * 0.06)
	RegisterRaiseEnjAttempt(nextTimeCycle)
EndFunction

Function OnRaiseEnjAttemptResult(bool abSuccess)
	; Hits and spam penalties move several points at once, so don't let them skip past either end.
	If (abSuccess)
		_FullEnjoyment = PapyrusUtil.ClampInt(_FullEnjoyment + _Config.GameEnjAdjAmount * 2, -100, 100)
		; COMEBACK: High-enj attempts don't charge upfront, so these restores currently grant resources.
		_PlayerRef.RestoreActorValue("Stamina", _Config.GameStaminaCost)
		_PlayerRef.RestoreActorValue("Magicka", _Config.GameMagickaCost)
		If (_FullEnjoyment == 100)
			DoOrgasm()
		EndIf
	ElseIf (_Config.GameSpamDelayPenalty)
		If (_EnjFactor > 0)
			_FullEnjoyment = PapyrusUtil.ClampInt(_FullEnjoyment - _Config.GameEnjAdjAmount * 2, -100, 100)
			_EnjFactor -= 0.04
		Else ; penalty for too many badly timed atttempts
			_FullEnjoyment = PapyrusUtil.ClampInt(_FullEnjoyment - 50, -100, 100)
			_EnjFactor = (_BaseFactor / 2)
		EndIf
		If (!SexLabUtil.IsGodModeEnabled())
			_PlayerRef.DamageActorValue("Stamina", 2 * _Config.GameStaminaCost)
			_PlayerRef.DamageActorValue("Magicka", 2 * _Config.GameMagickaCost)
		EndIf
	EndIf
EndFunction

Function DebugBaseCalcVariables()
	string BaseCalcLog = "[ENJ] EnjFactor: " + _EnjFactor + ", BaseArousal: " + _arousalBase + ", SameSexThread: " \
	+ (_HomoTypes[2]||_HomoTypes[3]||_HomoTypes[4]) + ", Sexuality: " + SexlabStatistics.GetSexuality(_ActorRef) + ", ConSubStatus: " \
	+ _ConSubStatus + ", IsVictim: " + _victim + ", HugePP: " + _CrtMaleHugePP + ", ContextPain: " + _PainContext as int
	Log(BaseCalcLog)
EndFunction

Function DebugEffectiveCalcVariables()
	string EffectiveCalcLog = "[ENJ] Enjoyment: " + _FullEnjoyment + ", IntFactor: " \
	+ _InterFactor + ", EnjFactor: " + _EnjFactor + ", PainInterCur: " + _PainInterCur \
	+ ", PainApplied: " + _PainApplied + ", Interactions: " + _Thread.GetCurrentInteractionString(_ActorRef)
	Log(EffectiveCalcLog)
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

function OffsetCoords(float[] Output, float[] CenterCoords, float[] OffsetBy) global
	If (OffsetBy.Length != 4 && CenterCoords.Length != 6 && Output.Length != 6)
		return
	EndIf
	float pi = 2.0 * Math.asin(1.0)
	float x2 = CenterCoords[0]
	float y2 = CenterCoords[1]

	float argX = CenterCoords[5] * pi / 180
	x2 += Math.sin(argX) * OffsetBy[0]
	y2 += Math.cos(argX) * OffsetBy[0]
	If (OffsetBy[1] != 0) 
		float argY = CenterCoords[5] + 90
		argY = argY * pi / 180
		x2 += Math.sin(argY) * OffsetBy[1]
		y2 += Math.cos(argY) * OffsetBy[1]
	EndIf
	
	Output[0] = x2
	Output[1] = y2
	Output[2] = CenterCoords[2] + OffsetBy[2]
	Output[3] = CenterCoords[3]
	Output[4] = CenterCoords[4]
	Output[5] = CenterCoords[5] + OffsetBy[3]
EndFunction
bool function IsInPosition(Actor CheckActor, ObjectReference CheckMarker, float maxdistance = 30.0) global
	return CheckActor.GetDistance(CheckMarker) < maxdistance
EndFunction
int function CalcEnjoyment(float[] XP, float[] SkillsAmounts, bool IsLeadin, bool IsFemaleActor, float Timer, int OnStage, int MaxStage) global
	return 0
EndFunction

int Property Position
	int Function Get()
		return _Thread.Positions.Find(_ActorRef)
	EndFunction
EndProperty

bool property UseStrapon hidden
	bool function get()
		return _useStrapon
	endFunction
endProperty

bool _DoRagdoll
bool property DoRagdoll hidden
	bool function get()
		return !_DoRagdoll ; && _Config.RagdollEnd
	endFunction
	function set(bool value)
		_DoRagdoll = !value
	endFunction
endProperty

int property Schlong hidden
	int function get()
		return 0
	endFunction
endProperty

bool property MalePosition hidden
	bool function get()
		return _Thread.Animation.GetGender(Position) == 0
	endFunction
endProperty

sslBaseExpression function GetExpression()
	return _Config.ExpressionSlots.GetByRegistrar(GetActorExpression())
endFunction
Function SetExpression(sslBaseExpression ToExpression)
	SetActorExpression(ToExpression.Registry)
EndFunction

sslBaseVoice function GetVoice()
	return _Config.VoiceSlots.GetByRegistrar(GetActorVoice())
endFunction
Function SetVoice(sslBaseVoice ToVoice = none, bool ForceSilence = false)
	If (ToVoice)
		SetActorVoice(ToVoice.Registry, ForceSilence)
	Else
		SetActorVoice("", ForceSilence)
	EndIf
EndFunction

int function GetGender()
	int ret = SexLabRegistry.GetSex(_ActorRef, false)
	If (ret >= 2)
		ret -= 1
	EndIf
	return ret
endFunction

function DisablePathToCenter(bool disabling)
	If (disabling)
		_PathingFlag = PATHING_DISABLE
	ElseIf (_PathingFlag == PATHING_DISABLE)
		_PathingFlag = PATHING_ENABLE
	EndIf
endFunction

function ForcePathToCenter(bool forced)
	If (forced)
		_PathingFlag = PATHING_FORCE
	Else
		_PathingFlag = PATHING_ENABLE
	EndIf
endFunction

function AttachMarker()
endFunction
function SyncThread()
endFunction

function OverrideStrip(bool[] SetStrip)
	if SetStrip.Length != 33
		_Thread.Log("Invalid strip override bool[] - Must be length 33 - was "+SetStrip.Length, "OverrideStrip()")
		return
	endif
	_stripCstm = new int[2]
	int i = 0
	int ii = 0
	While(i < 32)
		If(SetStrip[i])
			ii += Math.LeftShift(1, i)
		EndIF
		i += 1
	EndWhile
	_stripCstm[0] = ii
	_stripCstm[1] = SetStrip[32] as int
endFunction

Function Strip()
	_equipment = StripByDataEx(0x80, GetStripSettings(), _stripCstm, _equipment)
	_ActorRef.QueueNiNodeUpdate()
EndFunction
Function UnStrip()
	Redress()
EndFunction

function SetEndAnimationEvent(string EventName)
endFunction
function SetStartAnimationEvent(string EventName, float PlayTime)
endFunction

int function GetPain()
	if (_FullEnjoyment < 0)
		return Math.Abs(_FullEnjoyment) as int
	endIf
	return 0	
endFunction

; for compatibility with SLSO-based mods
int function GetFullEnjoyment()
	return _FullEnjoyment
endFunction
function BonusEnjoyment(Actor akActor = none, int AdjustBy = 0)
	return _Thread.AdjustEnjoyment(akActor, AdjustBy)
endFunction

function OrgasmEffect()
	DoOrgasm()
endFunction
event OrgasmStage()
	DoOrgasm()
endEvent
bool function NeedsOrgasm()
	return _FullEnjoyment >= 100
endFunction
function SetOrgasmCount(int value)
	; Will mess with internal enjoyment, deemed redundant!
EndFunction

function RegisterEvents()
endFunction
function ClearEvents()
endFunction

function EquipStrapon()
	; if _Strapon && !_ActorRef.IsEquipped(_Strapon)
	; 	_ActorRef.EquipItem(_Strapon, true, true)
	; endIf
endFunction
function UnequipStrapon()
	; if _Strapon && _ActorRef.IsEquipped(_Strapon)
	; 	_ActorRef.UnequipItem(_Strapon, true, true)
	; endIf
endFunction

function RefreshLoc()
	_Thread.RealignActors()
endFunction
function SyncLocation(bool Force = false)
	_Thread.RealignActors()
endFunction
function Snap()
	_Thread.RealignActors()
endFunction

function SetAdjustKey(string KeyVar)
endfunction
function LoadShares()
endFunction

bool function ContinueStrip(Form ItemRef, bool DoStrip = true)
	return sslActorLibrary.ContinueStrip(ItemRef, DoStrip)
endFunction

int function IntIfElse(bool check, int isTrue, int isFalse)
	if check
		return isTrue
	endIf
	return isFalse
endfunction

function ClearAlias()
	Clear()
endFunction

bool function PregnancyRisk()
	return _Thread.PregnancyRisk(_ActorRef)
endFunction

Function DoStatistics()
	; Thread handles Position statistics based on History and Participants
EndFunction

String function GetActorKey()
	return ""
endFunction

; Below functions are all strictly redundant
; Their functionality is either unnecessary or has absorbed into some other function directly
; Most of these functions had a specific functionality to operate on the underlying actor, allowing them to be invoked illegally
; would create issues in the framework itself while having them fail silently would potentially introduce issues on 
; the code illegally calling these functions, hence they all fail with an error message
Function LogRedundant(String asFunction)
	Debug.MessageBox("[SexLab]\nState '" + GetState() + "'; Function '" + asFunction + "' is a strictiyl redundant function that should not be called under any circumstance. See Papyrus Logs for more information.")
	Debug.TraceStack("[SexLab] Invoking Legacy Function " + asFunction)
EndFunction

function GetPositionInfo()
	LogRedundant("GetPositionInfo")
endFunction
function SyncActor()
	LogRedundant("SyncActor")
endFunction
function SyncAll(bool Force = false)
	LogRedundant("SyncAll")
endFunction
function RefreshActor()
	LogRedundant("RefreshActor")
endFunction
function RestoreActorDefaults()
	LogRedundant("RestoreActorDefaults")
endFunction
function SendAnimation()
	LogRedundant("SendAnimation")
endFunction
function StopAnimating(bool Quick = false, string ResetAnim = "IdleForceDefaultState")
	LogRedundant("StopAnimating")
endFunction
function StartAnimating()
	LogRedundant("OnBeginState")
endFunction
event ResetActor()
	LogRedundant("ResetActor")
endEvent
function ClearEffects()
	LogRedundant("ClearEffects")
endFunction
