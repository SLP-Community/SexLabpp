scriptname sslThreadController extends sslThreadModel
{
	Controller script to recognize player actions (hotkey inputs etc) to manually interact with scene logic
}

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

Function EnableHotkeys(bool forced = false)
	If(!HasPlayer && !forced)
		return
	EndIf
	RegisterHotkeys()
	If (Config.HasVRIK)
		EnableGesturesVR()
	EndIf
EndFunction

Function DisableHotkeys()
	SexLabUtil.ToggleFreeCamera(0)
	UnregisterHotkeys()
	If (Config.HasVRIK)
		DisableGesturesVR()
	EndIf
EndFunction

; ------------------------------------------------------- ;
; --- Hotkeys Registration                            --- ;
; ------------------------------------------------------- ;

int[] Hotkeys
int Property kToggleSceneHUD    = 0  AutoReadOnly
int Property kFocusSceneHUD     = 1  AutoReadOnly
int Property kAdvanceAnimation  = 2  AutoReadOnly
int Property kEndAnimation      = 3  AutoReadOnly
int Property kGameRaiseEnj      = 4  AutoReadOnly
int Property kGameHoldback      = 5  AutoReadOnly
int Property kChangeAnimation   = 6  AutoReadOnly
int Property kMoveScene         = 7  AutoReadOnly
int Property kChangePartner     = 8  AutoReadOnly
int Property kChangePositions   = 9  AutoReadOnly

Function InitHotkeys()
	Hotkeys = new int[10]
	Hotkeys[kToggleSceneHUD]    = Config.ToggleSceneHUD
	Hotkeys[kFocusSceneHUD]     = Config.FocusSceneHUD	
	Hotkeys[kAdvanceAnimation]  = Config.AdvanceAnimation
	Hotkeys[kEndAnimation]      = Config.EndAnimation
	Hotkeys[kGameRaiseEnj]      = Config.GameRaiseEnjKey
	Hotkeys[kGameHoldback]      = Config.GameHoldbackKey
	Hotkeys[kChangeAnimation]   = Config.ChangeAnimation
	Hotkeys[kMoveScene]         = Config.MoveScene
	Hotkeys[kChangePartner]     = Config.TargetActor
	Hotkeys[kChangePositions]   = Config.ChangePositions
EndFunction

Function RegisterHotkeys()
	; required inits
	InitHotkeys()
	GetAdjustPos()
	EnjBarsChangeHighlightedPartner(_AdjustActor)
	; register for hotkeys
	RegisterForKey(Hotkeys[kToggleSceneHUD])
	RegisterForKey(Hotkeys[kFocusSceneHUD])
	RegisterForKey(Hotkeys[kAdvanceAnimation])
	RegisterForKey(Hotkeys[kEndAnimation])
	If (Config.GameEnabled && HasPlayer)
		RegisterForKey(Hotkeys[kGameRaiseEnj])
		RegisterForKey(Hotkeys[kGameHoldback])
	EndIf
	RegisterForKey(Hotkeys[kChangeAnimation])
	RegisterForKey(Hotkeys[kMoveScene])
	RegisterForKey(Hotkeys[kChangePartner])
	RegisterForKey(Hotkeys[kChangePositions])
EndFunction

Function UnregisterHotkeys()
	int i = 0
	While (i < Hotkeys.Length)
		UnregisterForKey(Hotkeys[i])
		i += 1
	EndWhile
EndFunction

; ------------------------------------------------------- ;
; --- Hotkeys Execution                               --- ;
; ------------------------------------------------------- ;

Actor _AdjustActor = None     ; The actor (!=PlayerRef) currently selected for adjustments 
bool _SkipHotkeyEvents = False

Event OnKeyDown(int aiKey)
	If (Utility.IsInMenuMode() || _SkipHotkeyEvents)
		return
	EndIf
	; SceneHUD
	If (aiKey == Hotkeys[kToggleSceneHUD])
		ToggleVisibilitySceneHUD()
		return
	EndIf
	If (aiKey == Hotkeys[kFocusSceneHUD])
		ToggleFocusSceneHUD()
		return
	EndIf
	If (_bFocusedSceneHUD)
		return
	EndIf
	; Generic
	_SkipHotkeyEvents = true
	bool abModifier = Config.ModifierPressed()
	bool abAdjustTarget = abModifier
	If (Config.GameEnabled && HasPlayer)
		If (aiKey == Hotkeys[kGameRaiseEnj])
			ProcessEnjGameArg("Stamina", _AdjustActor, abAdjustTarget)
		ElseIf (aiKey == Hotkeys[kGameHoldback])
			ProcessEnjGameArg("Magicka", _AdjustActor, abAdjustTarget)
		EndIf
	EndIf
	If (aiKey == Hotkeys[kAdvanceAnimation])
		AdvanceStage(abModifier)
	ElseIf (aiKey == Hotkeys[kEndAnimation])
		EndAnimation()
	ElseIf (aiKey == Hotkeys[kChangeAnimation])
		PickRandomScene("")
	ElseIf (aiKey == Hotkeys[kMoveScene])
		MoveScene()
	ElseIf (aiKey == Hotkeys[kChangePartner])
		CycleTargetPartner(abModifier)
	ElseIf (aiKey == Hotkeys[kChangePositions])
		ChangePositions(abAdjustTarget)
	EndIf
	_SkipHotkeyEvents = false
EndEvent

; ------------------------------------------------------- ;
; --- SCENE HUD                                       --- ;
; ------------------------------------------------------- ;
bool _bOpenedSceneHUD = false
bool _bFocusedSceneHUD = false

Function ToggleVisibilitySceneHUD(int aiForceState = 0)
	;[-1:ForceClose, 0:Toggle, 1:ForceOpen]
	If (aiForceState == -1 || (aiForceState == 0 && _bOpenedSceneHUD))
		If (_bFocusedSceneHUD)
			ToggleFocusSceneHUD()
		EndIf
		TryCloseSceneHUD()
		_bOpenedSceneHUD = false
	ElseIf (aiForceState == 1 || (aiForceState == 0 && !_bOpenedSceneHUD))
		TryInitSceneHUD()
		_bOpenedSceneHUD = true
	EndIf
EndFunction

Function ToggleFocusSceneHUD()
	If (!_bOpenedSceneHUD)
		return
	EndIf
	ToggleFocusSceneHUDImpl()
	If (_bFocusedSceneHUD)
		_bFocusedSceneHUD = false
		PauseTimer(false)
	Else
		_bFocusedSceneHUD = true
		PauseTimer(true)
	EndIf
EndFunction

; ------------------------------------------------------- ;
; --- Functions [ IN USE ]                            --- ;
; ------------------------------------------------------- ;

Message Property RepositionInfoMsg Auto
{[Ok, Cancel, Don't show again]}

Function PickRandomScene(String asNewScene)
	String[] sceneSet = GetPlayingScenes()
	If(sceneSet.Length < 2)
		Log("PickRandomScene: No other scenes to pick from")
		return
	EndIf
	UnregisterForUpdate()
	If (asNewScene == "")
		int i = sceneSet.Find(GetActiveScene())
		int r = Utility.RandomInt(0, sceneSet.Length - 1)
		While(r == i)
			r = Utility.RandomInt(0, sceneSet.Length - 1)
		EndWhile
		asNewScene = sceneSet[r]
	EndIf
	Log("Changing running scene from " + GetActiveScene() + " to " + asNewScene)
	SendThreadEvent("AnimationChange")
	ResetScene(asNewScene)
EndFunction

Function MoveScene()
	If (!SexLabRegistry.IsCompatibleCenter(GetActiveScene(), Game.GetPlayer()))
		Debug.Notification("This scene does not support repositioning")
		return
	EndIf
	UnregisterForUpdate()
	If (StorageUtil.GetIntValue(none, "SEXLAB_REPOSITIONMSG_INFO", 0) == 0)
		; "You have 30 secs to position yourself to a new center location.\nHold down the 'Move Scene' hotkey to relocate the center instantly to your current position"
		int choice = RepositionInfoMsg.Show()
		If (choice == 1)
			return
		ElseIf (choice == 2)
			StorageUtil.SetIntValue(none, "SEXLAB_REPOSITIONMSG_INFO", 1)
		EndIf
	EndIf
	int n = 0
	While(n < Positions.Length)
		ActorAlias[n].GoToState(ActorAlias[n].STATE_PAUSED)
		If (ActorAlias[n] == ActorAlias(PlayerRef))
			ActorAlias[n].TryPauseAndUnlock()
		EndIf
		n += 1
	EndWhile
	SexLabUtil.SetActorMovement(PlayerRef, 1) ;MOVEMENT_UNLOCK
	Utility.Wait(1)
	int t = 0
	While(t < 60 && !Input.IsKeyPressed(Config.MoveScene))
		Utility.Wait(0.5)
		t += 1
	EndWhile
	SexLabUtil.SetActorMovement(PlayerRef, 2)	; MOVEMENT_LOCK... make sure player isnt moving before resync
	float x = PlayerRef.X
	float y = PlayerRef.Y
	float z = PlayerRef.Z
	Utility.Wait(0.5)							; wait for momentum to stop
	While(x != PlayerRef.X || y != PlayerRef.Y || z != PlayerRef.Z)
		x = PlayerRef.X
		y = PlayerRef.Y
		z = PlayerRef.Z
		Utility.Wait(0.5)
	EndWhile
	int j = 0
	While(j < Positions.Length)
		ActorAlias[j].TryLockAndUnpause()
		j += 1
	EndWhile
	CenterOnObject(PlayerRef)
	If (!HasPlayer)
		MoveActorsAwayFromPlayer(true)
		Config.DisableThreadControl(self)
	EndIf
EndFunction

Function AdvanceStage(bool abBackwards = false)
	If (!abBackwards)
		GoToStage(Stage + 1)
	ElseIf (Stage > 1)
		GoToStage(Stage - 1)
	EndIf
EndFunction

Function ChangePositions(bool abAdjustTarget = false)
	If (GetPositions().Length < 2)
		return
	EndIf
	Actor akAffectedActor = PlayerRef
	If (abAdjustTarget)
		akAffectedActor = GetTargetPartner()
	EndIf
	If (SetNextPermutation(akAffectedActor))
		SendThreadEvent("PositionChange")
		return
	EndIf
	Debug.Notification("Selected actor cannot switch positions")
EndFunction

int Function GetAdjustPos()
	If (_AdjustActor)
		return GetPositionIdx(_AdjustActor)
	EndIf
	int AdjustIdx = -1
	If (HasPlayer)
		AdjustIdx = IndexTravelComplex(GetPositionIdx(PlayerRef))
	Else
		AdjustIdx = (GetPositions().Length > 1) as int
	EndIf
	_AdjustActor = GetIdxPosition(AdjustIdx)
	Config.SetTargetActor(_AdjustActor)
	return AdjustIdx
EndFunction

Actor Function GetTargetPartner()
	If (!_AdjustActor)
		GetAdjustPos()
	EndIf
	return _AdjustActor
EndFunction

Function CycleTargetPartner(bool abBackwards = false)
	int len = GetPositions().Length
	If ((HasPlayer && len < 3) || (!HasPlayer && len < 2))
		return
	EndIf
	int curIdx = GetAdjustPos()
	int newIdx = IndexTravelComplex(curIdx, abBackwards, PlayerRef)
	UpdateTargetPartner(newIdx, abBackwards)
EndFunction

Function SelectTargetPartner(Actor akSelected)
	int len = GetPositions().Length
	If ((!akSelected) || (HasPlayer && len < 3) || (!HasPlayer && len < 2))
		return
	EndIf
	int curIdx = GetAdjustPos()
	int selectedIdx = GetPositionIdx(akSelected)
	If (selectedIdx < 0) || (selectedIdx == curIdx)
		return
	EndIf
	UpdateTargetPartner(selectedIdx)
EndFunction

Function UpdateTargetPartner(int targetIdx, bool abBackwards = false)
	_AdjustActor = GetIdxPosition(targetIdx)
	Config.SetTargetActor(_AdjustActor)
	Config.SelectedSpell.Cast(_AdjustActor)	; SFX for visual feedback
	EnjBarsChangeHighlightedPartner(_AdjustActor)
	PlayHotkeyFX(0, !abBackwards)
	Debug.Notification("SexLab partner selected: " + SexLabUtil.ActorName(_AdjustActor))
	Log("UpdateTargetPartner(), currently focused partner: " + SexLabUtil.ActorName(_AdjustActor))
EndFunction

Function PlayHotkeyFX(int i, bool abBackwards)
	If (abBackwards)
		Config.HotkeyDown[i].Play(PlayerRef)
	Else
		Config.HotkeyUp[i].Play(PlayerRef)
	EndIf
EndFunction

; ------------------------------------------------------- ;
; --- Functions [ NOT IN USE ]                        --- ;
; ------------------------------------------------------- ;

Function UpdateAnnotations(string asString)
	String activeScene = GetActiveScene()
	String[] annotations = PapyrusUtil.StringSplit(asString, ",")
	int i = 0
	While(i < annotations.Length)
		SexLabRegistry.AddSceneAnnotation(activeScene, annotations[i])
		i += 1
	EndWhile
EndFunction

int Function GetOffsetIdx(String asOffsetType)
	String[] types = new String[4]
	types[0] = "X"
	types[1] = "Y"
	types[2] = "Z"
	types[3] = "R"
	return types.Find(asOffsetType)
EndFunction

Function SetSceneOffset(float afOffsetValue, String asOffsetType, bool abIncrement = false)
	String activeScene = GetActiveScene()
	int idx = GetOffsetIdx(asOffsetType)
	If (abIncrement)
		afOffsetValue += SexLabRegistry.GetSceneOffset(activeScene)[idx]
	EndIf
	SexLabRegistry.SetSceneOffset(activeScene, afOffsetValue, idx)
	ResetStage()
EndFunction

Function SetStageOffset(Actor akAffectedActor, float afOffsetValue, String asOffsetType, bool abIncrement = false)
	int idx = GetOffsetIdx(asOffsetType)
	int n = GetPositions().Find(akAffectedActor)
	String activeScene = GetActiveScene()
	String activeStage = ""
	If (Config.AdjustStage)
		activeStage = GetActiveStage()
	EndIf
	If (abIncrement)
		float afEditedValue = SexLabRegistry.GetStageOffset(activeScene, activeStage, n)[idx]
		If (idx != 3)
			afOffsetValue += afEditedValue
		Else
			afOffsetValue += Math.RadiansToDegrees(afEditedValue)
		EndIf
	EndIf
	SexLabRegistry.SetStageOffset(activeScene, activeStage, n, afOffsetValue, idx)
	UpdatePlacement(akAffectedActor)
EndFunction

Function RestoreOffsets()
	SexLabRegistry.ResetSceneOffset(GetActiveScene())
	SexLabRegistry.ResetStageOffsetA(GetActiveScene(), GetActiveStage())
	RealignActors()
EndFunction

; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;
; ----------------------------------------------------------------------------- ;
;                          ██╗   ██╗██████╗ ██╗██╗  ██╗                         ;
;                          ██║   ██║██╔══██╗██║██║ ██╔╝                         ;
;                          ██║   ██║██████╔╝██║█████╔╝                          ;
;                          ╚██╗ ██╔╝██╔══██╗██║██╔═██╗                          ;
;                           ╚████╔╝ ██║  ██║██║██║  ██╗                         ;
;                            ╚═══╝  ╚═╝  ╚═╝╚═╝╚═╝  ╚═╝                         ;
; ----------------------------------------------------------------------------- ;
; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;

;# Requires: Thumbstick OR Trackpad (L+R)
;#--------------------------------------------------#;
;#   L_Tap     : ToggleAdjustSelf
;#   L_Up      : GameRaiseEnj
;#   L_Down    : GameHoldback
;#   L_Left    : TargetPartnerPrev
;#   L_Right   : TargetPartnerNext
;#   L_Back    : ToggleFreeCam
;#   L_Forward : CyclePOVModes
;#   R_Tap     : ToggleFocusSceneHUD
;#   R_Up      : SceneChange
;#   R_Down    : SceneEnd
;#   R_Left    : StagePrev
;#   R_Right   : StageNext
;#   R_Back    : MoveScene
;#   R_Forward : ChangePositions
;#--------------------------------------------------#;

bool _SkipGestureEvents = False
bool _AdjustSelfVR = True

Function RegisterGesture(int aiGesture, String asName)
	VRIK.VrikSetProfileAction(aiGesture, "SLVR_"+asName)
	RegisterForModEvent("SLVR_"+asName, "VRHandleGesture")
EndFunction

Function UnregisterGesture(String asName)
	UnregisterForModEvent("SLVR_" + asName)
EndFunction

Function EnableGesturesVR()
	If (!Config.UseGestures)
		return
	EndIf
	VRIK.VrikBeginGestureProfile()
	RegisterGesture(1, "L_Tap")
	RegisterGesture(2, "L_Up")
	RegisterGesture(3, "L_Down")
	RegisterGesture(4, "L_Left")
	RegisterGesture(5, "L_Right")
	RegisterGesture(6, "L_Back")
	RegisterGesture(7, "L_Forward")
	RegisterGesture(14, "R_Tap")
	RegisterGesture(15, "R_Up")
	RegisterGesture(16, "R_Down")
	RegisterGesture(17, "R_Left")
	RegisterGesture(18, "R_Right")
	RegisterGesture(19, "R_Back")
	RegisterGesture(20, "R_Forward")
EndFunction

Function DisableGesturesVR()
	If (!Config.UseGestures)
		return
	EndIf
	VRIK.VrikEndGestureProfile()
	UnregisterGesture("L_Tap")
	UnregisterGesture("L_Up")
	UnregisterGesture("L_Down")
	UnregisterGesture("L_Left")
	UnregisterGesture("L_Right")
	UnregisterGesture("L_Back")
	UnregisterGesture("L_Forward")
	UnregisterGesture("R_Tap")
	UnregisterGesture("R_Up")
	UnregisterGesture("R_Down")
	UnregisterGesture("R_Left")
	UnregisterGesture("R_Right")
	UnregisterGesture("R_Back")
	UnregisterGesture("R_Forward")
EndFunction

Function VRHandleGesture(String asEventName, String Foobar, float Presses, Form Sender)
	If (Utility.IsInMenuMode() || _SkipGestureEvents)
		return
	EndIf
	string asEvent = StringUtil.Substring(asEventName, 5)
	; SceneHUD
	If (asEvent == "R_Tap")
		ToggleFocusSceneHUD()
		return
	EndIf
	If (_bFocusedSceneHUD)
		return
	EndIf
	; General
	If (asEvent == "L_Tap")
		_AdjustSelfVR = !_AdjustSelfVR
		Debug.Notification("SexLab: AdjustSelf: " + _AdjustSelfVR)
		return
	EndIf
	_SkipGestureEvents = true
	bool abAdjustTarget = !_AdjustSelfVR
	If (HasPlayer && Config.GameEnabled)
		If (asEvent == "L_Up")
			ProcessEnjGameArg("Stamina", GetTargetPartner(), abAdjustTarget)
		ElseIf (asEvent == "L_Down")
			ProcessEnjGameArg("Magicka", GetTargetPartner(), abAdjustTarget)
		EndIf
	EndIf
	If (asEvent == "L_Left")
		CycleTargetPartner(true)
	ElseIf (asEvent == "L_Right")
		CycleTargetPartner()
	ElseIf (asEvent == "L_Back")
		SexLabUtil.ToggleFreeCamera()
	ElseIf (asEvent == "L_Forward")
		CyclePOVModesVR()
	ElseIf (asEvent == "R_Up")
		PickRandomScene("")
	ElseIf (asEvent == "R_Down")
		EndAnimation()
	ElseIf (asEvent == "R_Left")
		AdvanceStage(true)
	ElseIf (asEvent == "R_Right")
		AdvanceStage()
	ElseIf (asEvent == "R_Back")
		MoveScene()
	ElseIf (asEvent == "R_Forward")
		ChangePositions(abAdjustTarget)
	EndIf
	_SkipGestureEvents = false
EndFunction

Function CyclePOVModesVR(bool abBackwards = false)
	int aiMode = Config.VRIK_TPP_FREE
	int modesCount = 3
	int step = 1
	If (abBackwards)
		step = -1
	EndIf
	aiMode = Config.POVModeVR + step
	If (aiMode >= modesCount)
		aiMode = 0
	ElseIf (aiMode < 0)
		aiMode = (modesCount - 1)
	EndIf
	Config.SetPOVModeVRIK(aiMode)
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

Function ChangeAnimation(bool backwards = false)
	return PickRandomScene("")
EndFunction

Function AdjustCoordinate(bool abBackwards, bool abStageOnly, float afValue, int aiKeyIdx, int aiOffsetType)
	LogRedundant("AdjustCoordinate")
EndFunction
Function AdjustForward(bool backwards = false, bool AdjustStage = false)
	LogRedundant("AdjustForward")
EndFunction
Function AdjustSideways(bool backwards = false, bool AdjustStage = false)
	LogRedundant("AdjustSideways")
EndFunction
Function AdjustUpward(bool backwards = false, bool AdjustStage = false)
	LogRedundant("AdjustUpward")
EndFunction

Function RotateScene(bool backwards = false)
	LogRedundant("RotateScene")
EndFunction

Function AdjustChange(bool backwards = false)
	CycleTargetPartner(backwards)
EndFunction

float Function GetAnimationRunTime()
	return Animation.GetTimersRunTime(Timers)
EndFunction

Function ResetPositions()
	RealignActors()
EndFunction

ObjectReference Function GetCenterFX()
	if CenterRef != none && CenterRef.Is3DLoaded()
		return CenterRef
	else
		int i = 0
		while i < ActorCount
			if Positions[i] != none && Positions[i].Is3DLoaded()
				return Positions[i]
			endIf
			i += 1
		endWhile
	endIf
EndFunction

Function AdjustSchlong(bool backwards = false)
	LogRedundant("AdjustSchlong")
EndFunction
