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

Function SetEnjoyment(int aiSet)
	_FullEnjoyment = aiSet
EndFunction

Function AdjustEnjoyment(int AdjustBy)
	_FullEnjoyment += AdjustBy
EndFunction

Function ModEnjoymentMult(float afSet, bool bAdjust)
	If bAdjust
		_ModEnjMult += afSet
	Else
		_ModEnjMult = afSet
	EndIf
EndFunction

string Function GetInteractionString()
	return _TypeInterStr
EndFunction

bool Function IsAnalPenetrated()
	return _Thread.HasCollisionAction(_Thread.CTYPE_Anal, _ActorRef, none)
EndFunction

bool Function IsGenitalInteraction()
	int pSex = SexLabRegistry.GetSex(_ActorRef, false)
	bool handjob = _Thread.HasCollisionAction(_Thread.CTYPE_HandJob, none, _ActorRef)
	bool footjob = _Thread.HasCollisionAction(_Thread.CTYPE_FootJob, none, _ActorRef)
	bool oral = _Thread.HasCollisionAction(_Thread.CTYPE_Oral, none, _ActorRef)
	If (handjob || footjob || oral)
		return true
	EndIf
	If (pSex != 0)
		bool vaginal = _Thread.HasCollisionAction(_Thread.CTYPE_Vaginal, _ActorRef, none)
		bool grinding = _Thread.HasCollisionAction(_Thread.CTYPE_Grinding, _ActorRef, none)
		If (vaginal || grinding)
			return true
		EndIf
	EndIf
	If (pSex != 1)
		bool anal = _Thread.HasCollisionAction(_Thread.CTYPE_Anal, none, _ActorRef)
		bool vaginal = _Thread.HasCollisionAction(_Thread.CTYPE_Vaginal, none, _ActorRef)
		bool grinding = _Thread.HasCollisionAction(_Thread.CTYPE_Grinding, none, _ActorRef)
		bool skull = _Thread.HasCollisionAction(_Thread.CTYPE_Skullfuck, none, _ActorRef)
		bool shaft = _Thread.HasCollisionAction(_Thread.CTYPE_LickingShaft, none, _ActorRef)
		If (anal || vaginal || grinding || skull || shaft)
			return true
		EndIf
	EndIf
	return false
EndFunction

; for compatibility with SLSO-based mods
int Function GetFullEnjoyment()
	return _FullEnjoyment
EndFunction

; ------------------------------------------------------- ;
; --- Stripping									                      --- ;
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

; ------------------------------------------------------- ;
; --- AnimSpeed                                       --- ;
; ------------------------------------------------------- ;

Function UpdateBaseSpeed(float afBaseSpeed)
	_AnimationSpeedBase = afBaseSpeed
EndFunction

Function UpdateAnimationSpeed()
	If (!sslSystemConfig.HasAnimSpeedSE())
		return
	EndIf
	float animSpeed = _AnimationSpeedBase
	If (_Config.SetAnimSpeedByEnjoyment)
		animSpeed *= PapyrusUtil.ClampFloat((GetFullEnjoyment() as float) / 90, 0.8, 1.2)
	EndIf
	sslAnimSpeedHelper.SetAnimationSpeed(_ActorRef, animSpeed, UPDATE_INTERVAL / 2, 0)
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
Form _xMarker

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
bool _victim

int _livestatus
Actor _killer

int _AnimVarIsNPC
bool _AnimVarbHumanoidFootIKDisable

float _AnimationSpeedBase

; Center
ObjectReference _myMarker

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
float _BaseDelay
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
		return _Thread.HasCollisionAction(_Thread.CTYPE_Oral, _ActorRef, none) || \
			_Thread.HasCollisionAction(_Thread.CTYPE_LickingShaft, _ActorRef, none) || \
			_Thread.HasCollisionAction(_Thread.CTYPE_AnimObjFace, _ActorRef, none) || \
			_Thread.HasCollisionAction(_Thread.CTYPE_SuckingToes, _ActorRef, none)
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
		If (ProspectRef == _PlayerRef)
			Game.DisablePlayerControls(abMovement = false, abFighting = true, abCamSwitch = false, \
				abLooking = false, abSneaking = false, abMenu = false, abActivate = true, abJournalTabs = false, \
				aiDisablePOVType = 0)
		EndIf
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
		_sex = SexLabRegistry.GetSex(ProspectRef, true)

		TrackedEvent(TRACK_ADDED)
		GoToState(STATE_SETUP)
		return true
	EndFunction

	Function Clear()
		_ActorRef.StopTranslation()
		If (GetIsDead())
			If (_ActorRef.IsEssential())
				_ActorRef.GetActorBase().SetEssential(false)
			EndIf
			_ActorRef.KillSilent(_killer)
		Else
			_Thread.RequestStatisticUpdate(_ActorRef, _StartedAt)
		EndIf
		If (_ActorRef == _PlayerRef)
			Game.EnablePlayerControls()
		EndIf
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

; ------------------------------------------------------- ;
; --- Alias SETUP                                     --- ;
; ------------------------------------------------------- ;
;/
	Pre animation start. The alias is waiting for the underlying thread to begin the animation
/;

bool __SETUP_DONE

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
		float min_delay = sslSystemConfig.GetMinSetupTime()
		_ActorRef.SetActorValue("Paralysis", 0.0)
		float interval = 0.05
		If(_ActorRef == _PlayerRef)
			Game.SetPlayerAIDriven()
			If (UI.IsMenuOpen("Dialogue Menu"))
				UI.InvokeString("Dialogue Menu", "_global.skse.CloseMenu", "Dialogue Menu")
				While (UI.IsMenuOpen("Dialogue Menu"))
					min_delay -= interval
					Utility.Wait(interval)
				EndWhile
			EndIf
		Else
			_Config.CheckBardAudience(_ActorRef, true)
			If(akPathTo && DoPathToCenter)
				ObjectReference target = akPathTo as ObjectReference
				float target_distance = 128.0
				float distance = _ActorRef.GetDistance(target)
				If(distance > target_distance && distance <= 6144.0)
					float fallback_timer = 15.0
					float prev_dist = distance + 1.0
					_ActorRef.SetFactionRank(_AnimatingFaction, 2)
					_ActorRef.EvaluatePackage()
					Utility.Wait(2.0)
					While (distance > target_distance && Math.abs(prev_dist - distance) > 0.5 && fallback_timer > 0)
						fallback_timer -= interval
						min_delay -= interval
						Utility.Wait(interval)
						prev_dist = distance
						distance = _ActorRef.GetDistance(target)
					EndWhile
				EndIf
			EndIf
		EndIf
		_ActorRef.SetFactionRank(_AnimatingFaction, 1)
		_ActorRef.EvaluatePackage()
		__SETUP_DONE = false
		GoToState(STATE_PAUSED)
		If (asStringArg != "skip")
			_Thread.PrepareDone()
		EndIf
		; Delayed Initialization
		_AnimVarIsNPC = _ActorRef.GetAnimationVariableInt("IsNPC")
		_AnimVarbHumanoidFootIKDisable = _ActorRef.GetAnimationVariableBool("bHumanoidFootIKDisable")
		If (_sex <= 2)	; NPC: Strapon, Expression
			If (_sex == 0)
				_BaseDelay = _Config.MaleVoiceDelay
			Else
				_BaseDelay = _Config.FemaleVoiceDelay
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
			_BaseDelay = 3.0
		EndIf
		_VoiceDelay = _BaseDelay
		_ExpressionDelay = _BaseDelay * 2
		If (min_delay > 0.0)
			Utility.Wait(min_delay)
		EndIf
		__SETUP_DONE = true
		; Post Delayed Initialization
		UpdateBaseEnjoymentCalculations()
		If (!_Config.DebugMode)
			return
		EndIf
		String LogInfo = ""
		LogInfo += "Strapon[" + _Strapon + "] "
		LogInfo += "Voice[" + GetActorVoice() + "] "
		LogInfo += "Expression[" + GetActorExpression() + "]"
		Log(LogInfo)
	EndEvent

	Function Clear()
		GoToState(STATE_IDLE)
		Clear()
	EndFunction
	Function Initialize()
		Clear()
		Initialize()
	EndFunction

	Event OnEndState()
		UnregisterForModEvent("SSL_PREPARE_Thread" + _Thread.tid)
	EndEvent
EndState

Event OnDoPrepare(string asEventName, string asStringArg, float afNumArg, form akPathTo)
	Error("Preparation request outside a valid state", "OnDoPrepare()")
EndEvent

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
	; Only called once the first time the main thread enters animating state
	Function ReadyActor(int aiStripData, int aiPositionGenders)
		_stripData = aiStripData
		_useStrapon = _sex == 1 && Math.LogicalAnd(aiPositionGenders, 0x2) == 0
		RegisterForModEvent("SSL_READY_Thread" + _Thread.tid, "OnStartPlaying")
	EndFunction
	Event OnStartPlaying(string asEventName, string asStringArg, float afNumArg, form akSender)
		UnregisterForModEvent("SSL_READY_Thread" + _Thread.tid)
		While (!__SETUP_DONE)
			Utility.Wait(0.05)
		EndWhile
		LockActor()
		If (_sex <= 2)
			If (DoUndress)
				DoUndress = false
				If (_sex == 0)
					Debug.SendAnimationEvent(_ActorRef, "Arrok_Undress_G1")
				Else
					Debug.SendAnimationEvent(_ActorRef, "Arrok_Undress_G1")
				EndIf
				Utility.Wait(0.6)
			EndIf
			_equipment = StripByData(_stripData, GetStripSettings(), _stripCstm)
			ResolveStrapon()
			_ActorRef.QueueNiNodeUpdate()
		EndIf
		_StartedAt = SexLabUtil.GetCurrentGameRealTime()
		_LastOrgasm = _StartedAt
		_Thread.AnimationStart()
		TrackedEvent(TRACK_START)
		Utility.Wait(1)	; Wait for schlong to update
		Debug.SendAnimationEvent(_ActorRef, "SOSBend0")
	EndEvent

	Function SetStrapon(Form ToStrapon)
		SetStraponAnimationImpl(ToStrapon)
	EndFunction
	Function ResolveStrapon(bool force = false)
		ResolveStraponImpl()
	EndFunction

	Function TryLock()
		LockActor()
	EndFunction
	Function LockActor()
		If (_ActorRef == _PlayerRef)
			If (Game.GetCameraState() == 0)
				Game.ForceThirdPerson()
			EndIf
			Game.DisablePlayerControls(abMovement = false, abFighting = true, abCamSwitch = true, \
				abLooking = false, abSneaking = false, abMenu = false, abActivate = true, abJournalTabs = false, \
				aiDisablePOVType = 0)
			If(_Config.AutoTFC)
				MiscUtil.SetFreeCameraState(true)
				MiscUtil.SetFreeCameraSpeed(_Config.AutoSUCSM)
			EndIf
		Else
			ActorUtil.AddPackageOverride(_ActorRef, _Thread.DoNothingPackage, 100, 1)
			_ActorRef.EvaluatePackage()
		EndIf
		Debug.SendAnimationEvent(_ActorRef, "IdleFurnitureExit")
		Debug.SendAnimationEvent(_ActorRef, "AnimObjectUnequip")
		Debug.SendAnimationEvent(_ActorRef, "IdleStop")
		LockActorImpl()
		_ActorRef.SetAnimationVariableInt("IsNPC", 0)
		_ActorRef.SetAnimationVariableBool("bHumanoidFootIKDisable", 1)
		If (!sslActorLibrary.HasVehicle(_ActorRef))
			If (!_myMarker)
				_myMarker = _ActorRef.PlaceAtMe(_xMarker)
			EndIf
			_ActorRef.SetVehicle(_myMarker)
		EndIf
		SendDefaultAnimEvent()
		GoToState(STATE_PLAYING)
	EndFunction
	
	Function RemoveStrapon()
		If(_Strapon && !_HadStrapon)
			_ActorRef.RemoveItem(_Strapon, 1, true)
		EndIf
	EndFunction

	Function Clear()
		If (_sex <= 2)
			Redress()
			RemoveStrapon()
		EndIf
		TrackedEvent(TRACK_END)
		GoToState(STATE_IDLE)
		Clear()
	EndFunction
	Function Initialize()
		Clear()
		Initialize()
	EndFunction
EndState

Function ReadyActor(int aiStripData, int aiPositionGenders)
	Error("Cannot ready outside of idle state", "ReadyActor()")
EndFunction
Function LockActor()
	Error("Cannot lock actor outside of idle state", "LockActor()")
EndFunction
Event OnStartPlaying(string asEventName, string asStringArg, float afNumArg, form akSender)
	Error("Playing request outside of idle state", "OnStartPlaying()")
EndEvent
Function RemoveStrapon()
	Error("Removing strapon from invalid state", "RemoveStrapon()")
EndFunction

;	Lock actor iff in idling state, otherwise do nothing
Function TryLock()
EndFunction

; Take this actor out of combat and clear all actor states, return true if the actor was the player
Function LockActorImpl() native
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

float _LoopDelay
float _LoopLovenseDelay
float _LoopEnjoymentDelay
float _LoopContextCheckDelay

bool _LovenseGenital
bool _LovenseAnal

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
		If(_Thread.GetStatus() != _Thread.STATUS_INSCENE)
			return
		EndIf
		If (_LoopContextCheckDelay >= _ContextCheckDelay)
			_LoopContextCheckDelay = 0
			If (_Thread.IdentifyConsentSubStatus()) != _ConSubStatus
				UpdateBaseEnjoymentCalculations()
			EndIf
		EndIf
		If (_LoopEnjoymentDelay >= _EnjoymentDelay)
			_LoopEnjoymentDelay = 0
			UpdateEffectiveEnjoymentCalculations()
		EndIf
		UpdateAnimationSpeed()
		int strength = CalcReaction()
		If (_LoopDelay >= _VoiceDelay && !IsSilent)
			_LoopDelay = 0.0
			bool lipsync = !OpenMouth && _Config.UseLipSync && _sex <= 2
			Sound snd = _Thread.GetAliasSound(Self, GetActorVoice(), strength)
			sslBaseVoice.PlaySound(_ActorRef, snd, strength, lipsync)
		EndIf
		If ((_FullEnjoyment > 90) && (_Config.SeparateOrgasms || _Config.InternalEnjoymentEnabled))
			DoOrgasm()
		EndIf
		If (_LoopLovenseDelay <= 0)
			If (_ActorRef == _PlayerRef && sslLovense.IsLovenseInstalled())
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
			EndIf
		Else
			_LoopLovenseDelay -= UPDATE_INTERVAL
		EndIf
		RefreshExpressionEx(strength)
		; Loop
		_LoopDelay += UPDATE_INTERVAL
		_LoopEnjoymentDelay += UPDATE_INTERVAL
		_LoopContextCheckDelay += UPDATE_INTERVAL
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
			If (_Config.DebugMode)
				Log("Expression? " + expression + "; Strength? " + afStrength + "; OpenMouth? " + OpenMouth, "sslBaseExpression.ApplyExpression()")
			EndIf
		EndIf
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
			If (_Config.SeparateOrgasms || _Config.InternalEnjoymentEnabled)
				If ((_FullEnjoyment > 90 && _FullEnjoyment < 180) && ((time - _lastHoldBack) < FindEdgingTimeWindow()))
					GameRewardTimedEdging()
					_hasOrgasm = false
					return
				EndIf
				If (_FullEnjoyment < 100)
					_hasOrgasm = false
					return
				EndIf
			EndIf
		EndIf
		UnregisterForUpdate()
		; SFX
		If(_Config.OrgasmEffects)
			If (_ActorRef == _PlayerRef && _Config.ShakeStrength > 0 && Game.GetCameraState() >= 8)
				Game.ShakeCamera(none, _Config.ShakeStrength, _Config.ShakeStrength + 1.0)
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
		_OrgasmCount += 1
		; Enjoyment
		_FullEnjoyment = 0
		_EdgeSpamPenalty = 0
		_arousalBase = 0
		SexlabStatistics.SetStatistic(_ActorRef, 17, _arousalBase)
		_EnjFactor = _BaseFactor
		If (_sex == 0 || _sex == 3)
			If (_OrgasmCount > _Config.MaxNoPainOrgasmMale)
				_FullEnjoyment -= (_OrgasmCount - _Config.MaxNoPainOrgasmMale) * 20
			EndIf
		Else
			If (_OrgasmCount > _Config.MaxNoPainOrgasmFemale)
				_FullEnjoyment -= (_OrgasmCount - _Config.MaxNoPainOrgasmFemale) * 20
			EndIf
		EndIf
		UpdateEffectiveEnjoymentCalculations()
		RegisterForSingleUpdate(UPDATE_INTERVAL)
		_hasOrgasm = false
		Log(GetActorName() + ": Orgasms[" + _OrgasmCount + "] FullEnjoyment [" + _FullEnjoyment + "]")
	EndFunction

	Function TryUnlock()
		UnlockActor()
	EndFunction
	Function UnlockActor()
		_ActorRef.SetVehicle(none)
		_ActorRef.SetAnimationVariableInt("IsNPC", _AnimVarIsNPC)
		_ActorRef.SetAnimationVariableBool("bHumanoidFootIKDisable", _AnimVarbHumanoidFootIKDisable)
		If (_ActorRef == _PlayerRef)
			MiscUtil.SetFreeCameraState(false)
			Game.EnablePlayerControls(abFighting = false, abActivate = false)
			If (sslLovense.IsLovenseInstalled())
				sslLovense.StopAllActions()
			EndIf
		Else
			ActorUtil.RemovePackageOverride(_ActorRef, _Thread.DoNothingPackage)
			_ActorRef.EvaluatePackage()
		EndIf
		UnlockActorImpl()
		GoToState(STATE_PAUSED)
	EndFunction
	
	Function ResetPosition(int aiStripData, int aiPositionGenders)
		_stripData = aiStripData
		_equipment = StripByDataEx(_stripData, GetStripSettings(), _stripCstm, _equipment)
		_useStrapon = _sex == 1 && Math.LogicalAnd(aiPositionGenders, 0x2) == 0
		ResolveStrapon()
		_ActorRef.QueueNiNodeUpdate()
	EndFunction

	Function Clear()
		If (sslSystemConfig.HasAnimSpeedSE())
			sslAnimSpeedHelper.ResetAnimationSpeed(_ActorRef)
		EndIf
		UnlockActor() ; will go to idle state
		Clear()
	EndFunction
	Function Initialize()
		Clear()
		Initialize()
	EndFunction

	Event OnKeyDown(int KeyCode)
		If (Utility.IsInMenuMode() || !_Config.GameEnabled)
			return
		EndIf
		If (KeyCode == _Config.GamePauseKey) && Input.IsKeyPressed(_Config.GameUtilityKey)
			_bGamePaused = !_bGamePaused
			MiscUtil.PrintConsole("[EnjGame] Game paused: " + _bGamePaused)
		EndIf
		If !_bGamePaused
			If (KeyCode == _Config.GameRaiseEnjKey)
				RunEnjoymentGame("Stamina")
			ElseIf (KeyCode == _Config.GameHoldbackKey)
				RunEnjoymentGame("Magicka")
			ElseIf (KeyCode == _Config.GameSelectNextPos)
				If _EnjGamePartner
					int newIdx = _Thread.GameNextPartnerIdx(_ActorRef, _EnjGamePartner, Input.IsKeyPressed(_Config.GameUtilityKey))
					_EnjGamePartner = _Thread.GameChangePartner(_ActorRef, newIdx)
				EndIf
			EndIf
		EndIf
	EndEvent
	
	Event OnEndState()
		UnregisterForModEvent("SSL_ORGASM_Thread" + _Thread.tid)
		UnregisterEnjGameKeys()
		StoreExcitementState("Backup")
		sslBaseExpression.CloseMouth(_ActorRef)
		_ActorRef.ClearExpressionOverride()
		_ActorRef.ResetExpressionOverrides()
		sslBaseExpression.ClearMFG(_ActorRef)
		SendDefaultAnimEvent()
	EndEvent
EndState

Function UnlockActor()
	Error("Cannot unlock actor outside of playing state", "UnlockActor()")
EndFunction
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
function DoOrgasm(bool Forced = false)
	Error("Cannot create an orgasm outside of playing state", "DoOrgasm()")
endFunction
Function PlayLouder(Sound SFX, ObjectReference FromRef, float Volume)
	Error("Cannot play sound outside of playing state", "PlayLouder()")
EndFunction
Event OnOrgasm(string eventName, string strArg, float numArg, Form sender)
	Error("Cannot create orgasm effects outside of playing state", "OnOrgasm()")
EndEvent

Function TryUnlock()
EndFunction
Function TryRefreshExpression()
EndFunction

; Undo "LockActor()" persistent changes
Function UnlockActorImpl() native
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
	Debug.SendAnimationEvent(_ActorRef, "ReturnDefaultState") 	; chicken, hare and slaughterfish before the "ReturnToDefault"
	Debug.SendAnimationEvent(_ActorRef, "ReturnToDefault") 			; rest creature-animal
	Debug.SendAnimationEvent(_ActorRef, "FNISDefault") 					; dwarvenspider and chaurus
	Debug.SendAnimationEvent(_ActorRef, "IdleReturnToDefault") 	; Werewolves and VampirwLords
	Debug.SendAnimationEvent(_ActorRef, "ForceFurnExit") 				; Trolls afther the "ReturnToDefault" and draugr, daedras and all dwarven exept spiders
	Debug.SendAnimationEvent(_ActorRef, "Reset") 								; Hagravens afther the "ReturnToDefault" and Dragons
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
; --- Orgasm FX                                  --- ;
; ------------------------------------------------------- ;

function ApplyCum()	; NOTE: Temporary?
	; TODO: _Tread.ApplyCumFX(Source = _ActorRef)

	; Log("START", "ApplyCum")
	if _ActorRef && _ActorRef.Is3DLoaded()
		Cell ParentCell = _ActorRef.GetParentCell()

		bool vaginalPen = _Thread.IsVaginalComplex(_ActorRef)
		bool oralPen = _Thread.IsOralComplex(_ActorRef)
		bool analPen = _Thread.IsAnalComplex(_ActorRef)

		If _Config.DebugMode
			Log("ApplyCum(): Adding v = " + vaginalPen + " o = " + oralPen + " a = " + analPen)
		EndIf

		if (vaginalPen || oralPen || analPen) && ParentCell && ParentCell.IsAttached() 
			; thanks a lot for removing ActorLib scrab
			(Game.GetFormFromFile(0xD62, "SexLab.esm") as sslActorLibrary).AddCum(_ActorRef, vaginalPen, oralPen, analPen)
		endIf
	endIf
endFunction

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
	; _xMarker = Game.GetFormFromFile(0x045A93, "SexLab.esm") ; 0x3B)
	_xMarker = Game.GetForm(0x3B)

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
	_AllowRedress = true
	ForceOpenMouth = false
	; Integers
	_sex = -1
	_livestatus = 0
	_PathingFlag = 0
	_OrgasmCount = 0
	_stripCstm = new int[1]
	; Floats
	_LastOrgasm = 0.0
	_StartedAt = 0.0
	_AnimationSpeedBase = 1.0
	ResetEnjoymentVariables()
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

Function UpdateEnjoyment(float afEnjoyment) native

; Defaults
float _EnjoymentDelay
float _ContextCheckDelay
; Base
bool _CrtMaleHugePP
int _ConSubStatus
int _ActorInterInfo
float _PainContext
float _EnjFactor
float _BaseFactor
float _arousalBase
; Interaction
string _TypeInterStr
float _InterFactor
; Effective
float _ModEnjMult
float _arousalStat
float _PainInterTimer
float _PainInterBackup
float _PainInterDecayBackup
int _FullEnjoyment
; Game
bool _bGamePaused
Actor _EnjGamePartner
float _GameModSelfSta
float _GameModSelfMag
float _lastHoldBack
float _EdgeSpamPenalty

Function ResetEnjoymentVariables()
	; Defaults
	_EnjoymentDelay = 1.5
	_ContextCheckDelay = 8.0
	; Base
	_CrtMaleHugePP = False
	_ConSubStatus = _Thread.CONSENT_CONNONSUB
	_ActorInterInfo = _Thread.ACTORINT_NONPART
	_PainContext = 0.0
	_EnjFactor = 0.0
	_BaseFactor = 0.0
	_arousalBase = 0.0
	; Interaction
	_TypeInterStr = ""
	_InterFactor = 0.0
	; Effective
	_ModEnjMult = 1.0
	_arousalStat = 0.0
	_PainInterTimer = 0.0
	_PainInterBackup = 0.0
	_PainInterDecayBackup = 0.0
	_FullEnjoyment = 0
	; Game
	_bGamePaused = False
	_EnjGamePartner = None
	_GameModSelfSta = 0.0
	_GameModSelfMag = 0.0
	_lastHoldBack = 0.0
	_EdgeSpamPenalty = 0.0
EndFunction

Function UpdateBaseEnjoymentCalculations()
	If (!_Config.InternalEnjoymentEnabled || !_Config.SeparateOrgasms || GetIsDead())
		return
	EndIf
	ResetEnjoymentVariables()
	RegisterEnjGameKeys()
	StoreExcitementState("Restore")
	RunEnjoymentGame("Initiate")
	_CrtMaleHugePP = _Thread.CrtMaleHugePP()
	_ConSubStatus = _Thread.IdentifyConsentSubStatus()
	bool SameSexThread = _Thread.SameSexThread()
	float BestRelation  = _Thread.GetBestRelationForScene(_ActorRef, _ConSubStatus) as float
	_ActorInterInfo = _Thread.GuessActorInterInfo(_ActorRef, _sex, _victim, _ConSubStatus, SameSexThread)
	_arousalBase = PapyrusUtil.ClampFloat(SexlabStatistics.GetStatistic(_ActorRef, 17), 0.0, 100.0)
	_PainContext = CalcContextPain(BestRelation)
	_EnjFactor = CalcContextEnjFactor(SameSexThread, BestRelation)
	_BaseFactor = _EnjFactor
	If _Config.DebugMode
		DebugBaseCalcVariables()
	EndIf
EndFunction

Function UpdateEffectiveEnjoymentCalculations()
	If (!_Config.InternalEnjoymentEnabled || !_Config.SeparateOrgasms || GetIsDead())
		return
	EndIf
	; Interactions
	_TypeInterStr = _Thread.CreateInteractionString(_ActorRef, _ActorInterInfo)
	_InterFactor = _Thread.CalculateInteractionFactor(_ActorRef, _TypeInterStr)
	; Enjoyment
	float EnjEffective = CalcEffectiveEnjoyment()
	_FullEnjoyment = (EnjEffective - _EdgeSpamPenalty) as int
	UpdateEnjoyment(_FullEnjoyment as float)
	UpdateArousalStat()
	; Debug
	If _Config.DebugMode
		DebugEffectiveCalcVariables()
	EndIf
EndFunction

float Function CalcContextPain(float BestRelation)
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
	If _ConSubStatus == _Thread.CONSENT_CONSUB
		PainContext -= (BestRelation * PainContext * 0.03)
	EndIf
	return PainContext
EndFunction

float Function CalcInteractionPain()
	float vaginalXP = SexlabStatistics.GetStatistic(_ActorRef, 2)
	float analXP = SexlabStatistics.GetStatistic(_ActorRef, 3)
	bool PainCondVaginal = (StringUtil.Find(_TypeInterStr, "pVaginal") != -1) && (vaginalXP < _Config.NoPainRequiredXP as float)
	bool PainCondAnal = (StringUtil.Find(_TypeInterStr, "pAnal") != -1) && (analXP < _Config.NoPainRequiredXP as float)
	float PainInter = 0.0
	If (PainCondVaginal || PainCondAnal)
		_PainInterTimer += _EnjoymentDelay
		float PainFactor = (2 - ((1 / (_Config.NoPainRequiredXP as float * 2)) * (1 + vaginalXP + analXP)))
		If _CrtMaleHugePP
			PainFactor += _Config.PainHugePPMult
		EndIf
		PainInter = (PainFactor * _InterFactor * _Config.EnjRaiseMultInter * 5)
	EndIf
	return PainInter
EndFunction

float Function CalcEffectivePain()
	float PainInter = 0
	float SceneDuration = _Thread.GetTimeTotal()
	If (_PainContext && (SceneDuration < _Config.NoPainRequiredTime as float))
		float PainContextDecay = _PainContext * ((1 / _Config.NoPainRequiredTime as float) * SceneDuration)
		_PainContext -= PainContextDecay
	EndIf
	If (_PainInterTimer < _Config.NoPainRequiredTime as float)
		float PainInterTemp = CalcInteractionPain()
		If (PainInterTemp || _PainInterBackup)
			float PainInterDecayCur = PainInterTemp * (_PainInterTimer / _Config.NoPainRequiredTime as float)
			float PainInterDecayIncr = PainInterDecayCur - _PainInterDecayBackup
			float PainInterCur = PainInterTemp - PainInterDecayIncr
			PainInter = PainInterCur - _PainInterBackup 
			_PainInterBackup = PainInterCur
			_PainInterDecayBackup = PainInterDecayCur
		EndIf
	EndIf
	float PainEffective = _PainContext + PainInter
	return PainEffective
EndFunction
		
float Function CalcContextEnjFactor(bool SameSexThread, float BestRelation)
	float EnjMult = 1.0
	;arousal
	EnjMult += (_arousalBase / 50) 
	;relation
	EnjMult += (BestRelation / 11) 
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

float Function CalcEffectiveEnjoyment()
	float ConSubMult = EnjFindConSubStatusMult()
	float EffectivePain = CalcEffectivePain()
	float EnjInter = 0
	If !(EffectivePain - _PainContext > 0)
		EnjInter = (_EnjFactor * _InterFactor * _Config.EnjRaiseMultInter * ConSubMult * _ModEnjMult)
		RunEnjoymentGame("Auto")
	EndIf
	float EnjEffective = _FullEnjoyment + EnjInter - EffectivePain
	return EnjEffective
EndFunction

Function UpdateArousalStat()
	_arousalStat = SexlabStatistics.GetStatistic(_ActorRef, 17)
	If (_arousalStat < 100)
		_arousalStat = _arousalBase + (_FullEnjoyment as float / 2)
		SexlabStatistics.SetStatistic(_ActorRef, 17, _arousalStat)
	Else
		float dif = (_arousalStat - _arousalBase)
		_EnjFactor = _EnjFactor + (dif / 50)
	EndIf
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
	
Function InternalCompensateStageSkip()
	_FullEnjoyment += _Config.EnjGainOnStageSkip
EndFunction

int function CalcReaction()
	int ret = Math.Abs(_FullEnjoyment) as int
	return PapyrusUtil.ClampInt(ret, 0, 100)
EndFunction

bool Function WaitForOrgasm()
	If _Config.InternalEnjoymentEnabled
		bool EnjScenario = (_Config.HighEnjOrgasmWait && ((_FullEnjoyment > 80 && _OrgasmCount == 0) || (_FullEnjoyment > 100)))
		bool PlayerSceanrio = (_Config.PlayerMustOrgasm && (_ActorRef == _PlayerRef) && _OrgasmCount == 0)
		bool DomScenario = (_Config.DomMustOrgasm && !_victim && _OrgasmCount == 0 && \
		(_ConSubStatus == _Thread.CONSENT_CONSUB || _ConSubStatus == _Thread.CONSENT_NONCONSUB))
		If (EnjScenario || DomScenario || PlayerSceanrio)
			return true
		EndIf
	EndIf
	return false
EndFunction

Function StoreExcitementState(String arg = "")
	string ActorName = GetActorName()
	If (arg == "Backup")
		StorageUtil.SetFloatValue(None, ("EnjBackupTime_" + ActorName),  SexLabUtil.GetCurrentGameRealTime())
		StorageUtil.SetIntValue(None, ("LastOrgasmCount_" + ActorName), _OrgasmCount)
		If _FullEnjoyment > 10
			StorageUtil.SetIntValue(None, ("LastEnjoyment_" + ActorName), _FullEnjoyment)
		EndIf
	ElseIf (arg == "Restore")
		float TimeSinceEnjBackup = (SexLabUtil.GetCurrentGameRealTime() - StorageUtil.GetFloatValue(None, ("EnjBackupTime_" + ActorName)))
		If (TimeSinceEnjBackup < 60)
			_OrgasmCount = StorageUtil.GetIntValue(None, ("LastOrgasmCount_" + ActorName))
			int LastEnjoyment = StorageUtil.GetIntValue(None, ("LastEnjoyment_" + ActorName))
			_FullEnjoyment = (LastEnjoyment as float * (1 - (TimeSinceEnjBackup/60))) as int
		EndIf
	EndIf
EndFunction

Function RegisterEnjGameKeys()
	bool condition = (_Config.GameEnabled && (_Config.GamePlayerAutoplay != 1) && \
	(_ActorRef == _PlayerRef) && !(_victim && _Config.GamePlayerVictimAutoplay == 1))
	If !condition
		return
	EndIf
	RegisterForKey(_Config.GameUtilityKey)
	RegisterForKey(_Config.GamePauseKey)
	RegisterForKey(_Config.GameRaiseEnjKey)
	RegisterForKey(_Config.GameHoldbackKey)
	RegisterForKey(_Config.GameSelectNextPos)
EndFunction

Function UnregisterEnjGameKeys()
	UnregisterForKey(_Config.GameUtilityKey)
	UnregisterForKey(_Config.GamePauseKey)
	UnregisterForKey(_Config.GameRaiseEnjKey)
	UnregisterForKey(_Config.GameHoldbackKey)
	UnregisterForKey(_Config.GameSelectNextPos)
EndFunction

Function RunEnjoymentGame(String arg = "")
	If (_bGamePaused || !_Config.GameEnabled)
		return
	EndIf
	If (arg == "Initiate")
		_EnjGamePartner = _Thread.GameChangePartner(_ActorRef)
		_GameModSelfSta = _Thread.CalcEnjVarMod("Stamina", _ActorRef)
		_GameModSelfMag = _Thread.CalcEnjVarMod("Magicka", _ActorRef) 
		_Thread.ProcessEnjGameArg("", _ActorRef, _EnjGamePartner, _GameModSelfSta, _GameModSelfMag)
	ElseIf (arg == "Auto")
		_Thread.ProcessEnjGameArg("Auto", _ActorRef, _EnjGamePartner, _GameModSelfSta, _GameModSelfMag)
	ElseIf (arg == "Stamina")
		_Thread.ProcessEnjGameArg("Stamina", _ActorRef, _EnjGamePartner, _GameModSelfSta, _GameModSelfMag)
	ElseIf (arg == "Magicka")
		_Thread.ProcessEnjGameArg("Magicka", _ActorRef, _EnjGamePartner, _GameModSelfSta, _GameModSelfMag)
	EndIf
EndFunction

Function GameRegisterEdgeAttempt()
	; IDEA: expose fWindow as some UI bar when making custom widget; can be a cool minigame feature
	If (_ActorRef != _PlayerRef)
		return
	EndIf
	If (_lastHoldBack > 0.0)
		float window = FindEdgingTimeWindow()
		If (SexLabUtil.GetCurrentGameRealTime() - _lastHoldBack) < (window / 2)
			_EdgeSpamPenalty = 2 * (window * 2)
			_EnjFactor -= 0.03
			If (_EnjFactor < 0.0)
				_FullEnjoyment -= 50 ; penalty for excessive edging spam
			EndIf
			int PenaltyType = _Config.EdgeSpamPunishType
			If PenaltyType == 1
				_ActorRef.DamageActorValue("Stamina", _Config.GameStaminaCost)
			ElseIf PenaltyType == 2
				_ActorRef.DamageActorValue("Magicka", _Config.GameMagickaCost)
			ElseIf PenaltyType == 3	
				_ActorRef.DamageActorValue("Health", _Config.GameStaminaCost)
			EndIf	
		EndIf
	EndIf
	_lastHoldBack = SexLabUtil.GetCurrentGameRealTime()
EndFunction

Function GameRewardTimedEdging()
	If (_ActorRef != _PlayerRef)
		return
	EndIf
	;_EnjoymentDelay/UPDATE_INTERVAL==6); boost in 3s == 0.24
	_EnjFactor += 0.02
	int RewardType = _Config.EdgingRewardType
	If RewardType == 1
		_ActorRef.RestoreActorValue("Stamina", _Config.GameStaminaCost)
	ElseIf RewardType == 2
		_ActorRef.RestoreActorValue("Magicka", _Config.GameMagickaCost)
	ElseIf RewardType == 3
		_ActorRef.RestoreActorValue("Health", _Config.GameStaminaCost)
	EndIf
	If (_FullEnjoyment > 175)
		If RewardType == 1
			_ActorRef.ModActorValue("Stamina", 5)
		ElseIf RewardType == 2
			_ActorRef.ModActorValue("Magicka", 5)
		ElseIf RewardType == 3
			_ActorRef.ModActorValue("Health", 5)
		EndIf
	EndIf
EndFunction

float Function FindEdgingTimeWindow()
	return 3.7 - _FullEnjoyment * 0.0185
EndFunction

Function DebugBaseCalcVariables()
	string BaseCalcLog = "[ENJ] EnjFactor: " + _EnjFactor + ", BaseArousal: " + _arousalBase + ", SameSexThread: " + _Thread.SameSexThread()\
	+ ", Sexuality: " + SexlabStatistics.GetSexuality(_ActorRef) + ", BestRelation: " + _Thread.GetBestRelationForScene(_ActorRef, _ConSubStatus)\
	+ ", ConSubStatus: " + _ConSubStatus + ", IsVictim: " + _victim + ", CrtMaleHugePP: " + _CrtMaleHugePP + ", ActorInterInfo: " + _ActorInterInfo\
	+ ", VaginalXP: " + SexlabStatistics.GetStatistic(_ActorRef, 2) as int + ", AnalXP: " + SexlabStatistics.GetStatistic(_ActorRef, 3) as int\
	+ ", ContextPain: " + _PainContext as int
	Log(BaseCalcLog)
EndFunction

Function DebugEffectiveCalcVariables()
	string EffectiveCalcLog = "[ENJ] Enjoyment: " + _FullEnjoyment + ", TypeInterStr: "\
	+ _TypeInterStr + ", IntFactor: " + _InterFactor + ", EnjFactor: " + _EnjFactor
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