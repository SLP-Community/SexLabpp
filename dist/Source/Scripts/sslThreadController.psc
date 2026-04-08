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
	If (Config.UseSceneMenu && !Config.HasVRIK)
		EnableMenuEvents()
	EndIf
	EnableTraditionalHotkeys()
	If (Config.HasVRIK)
		EnableGesturesVR()
	EndIf
EndFunction

Function DisableHotkeys()
	SexLabUtil.ToggleFreeCamera(0) ; TFC_OFF... If free cam is active here will glitch out controls?
	If (Config.UseSceneMenu && !Config.HasVRIK)
		DisableMenuEvents()
	EndIf
	DisableTraditionalHotkeys()
	If (Config.HasVRIK)
		DisableGesturesVR()
	EndIf
EndFunction

; ------------------------------------------------------- ;
; --- Menu Events                                     --- ;
; ------------------------------------------------------- ;

String[] _MenuEvents
int _AutoAdvanceCache
bool _SkipMenuEvents = False

Function EnableMenuEvents()
	If (!TryOpenSceneMenu())
		return
	EndIf
	_AutoAdvanceCache = -1
	_MenuEvents = new String[8]
	_MenuEvents[0] = "SL_AdvanceScene"
	_MenuEvents[1] = "SL_SetSpeed"
	_MenuEvents[2] = "SL_MoveScene"
	_MenuEvents[3] = "SL_EndScene"
	_MenuEvents[4] = "SL_SetAnnotations"
	_MenuEvents[5] = "SL_SetOffset"
	_MenuEvents[6] = "SL_StartAdjustOffset"
	_MenuEvents[7] = "SL_SetActiveScene"
	int i = 0
	While (i < _MenuEvents.Length)
		RegisterForModEvent(_MenuEvents[i], "MenuEvent")
		i += 1
	EndWhile
EndFunction

Function DisableMenuEvents()
	int i = 0
	While (i < _MenuEvents.Length)
		UnregisterForModEvent(_MenuEvents[i])
		i += 1
	EndWhile
	TryCloseSceneMenu()
EndFunction

Event MenuEvent(string asEventName, string asStringArg, float afNumArg, form akSender)
	If (Utility.IsInMenuMode() || _SkipMenuEvents)
		return
	EndIf
	_SkipMenuEvents = true
	_SkipHotkeyEvents = true
	Log("MenuEvent: " + asEventName)
	If (asEventName == "SL_SetActiveScene")
		PickRandomScene(asStringArg)
	ElseIf (asEventName == "SL_AdvanceScene")
		If (afNumArg)
			GoToStage(Stage - 1)
		Else
			PlayNextImpl(asStringArg)
		EndIf
	ElseIf (asEventName == "SL_SetSpeed")
		UpdateBaseSpeed(afNumArg)
		If (afNumArg == 0.0)
			_AutoAdvanceCache = AutoAdvance as int
			AutoAdvance = false
		ElseIf (_AutoAdvanceCache != -1)
			AutoAdvance = _AutoAdvanceCache as bool
			_AutoAdvanceCache = -1
		EndIf
	ElseIf (asEventName == "SL_MoveScene")
		MoveScene()
	ElseIf (asEventName == "SL_EndScene")
		EndAnimation()
	ElseIf (asEventName == "SL_SetAnnotations")
		UpdateAnnotations(asStringArg)
	ElseIf (asEventName == "SL_SetOffset")
		If (akSender == none)
			SetSceneOffset(afNumArg, asStringArg)
		ElseIf (akSender as Actor)
			SetStageOffset(akSender as Actor, afNumArg, asStringArg)
		Else
			Log("SetOffset: Sender is not an actor")
		EndIf
	ElseIf (asEventName == "SL_StartAdjustOffset")
		; TODO: impl
	EndIf
	_SkipMenuEvents = false
	_SkipHotkeyEvents = false
EndEvent

; ------------------------------------------------------- ;
; --- Executed Functions [Menu Events]                --- ;
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

; ------------------------------------------------------- ;
; --- Traditional Hotkeys                             --- ;
; ------------------------------------------------------- ;

int[] Hotkeys
int Property kChangeAnimation   = 0  AutoReadOnly
int Property kMoveScene         = 1  AutoReadOnly
int Property kChangePartner     = 2  AutoReadOnly
int Property kGameRaiseEnj      = 3  AutoReadOnly
int Property kGameHoldback      = 4  AutoReadOnly
int Property kAdvanceAnimation  = 5  AutoReadOnly
int Property kEndAnimation      = 6  AutoReadOnly
int Property kChangePositions   = 7  AutoReadOnly
int Property kOffsetAdjustMode  = 8 AutoReadOnly
int Property kToggleAdjustStage = 9 AutoReadOnly
int Property kRestoreOffsets    = 10 AutoReadOnly
int Property kDirectionUp       = 11 AutoReadOnly
int Property kDirectionDown     = 12 AutoReadOnly
int Property kDirectionLeft     = 13 AutoReadOnly
int Property kDirectionRight    = 14 AutoReadOnly
int Property kSceneSelector     = 15 AutoReadOnly

int Property AdjMode_None     = 0 AutoReadOnly
int Property AdjMode_PosXY    = 1 AutoReadOnly
int Property AdjMode_PosRZ    = 2 AutoReadOnly
int Property AdjMode_SceneXY  = 3 AutoReadOnly
int Property AdjMode_SceneRZ  = 4 AutoReadOnly

Actor _AdjustActor = None     ; The actor (!=PlayerRef) currently selected for adjustments 
int _AdjustMode = 0           ; Determines offsets adjustment mode
bool _SkipHotkeyEvents = False

Function EnableTraditionalHotkeys()
	InitLegacyHotkeys()
	GetAdjustPos()
	RegisterForKey(Hotkeys[kChangeAnimation])
	RegisterForKey(Hotkeys[kMoveScene])
	RegisterForKey(Hotkeys[kChangePartner])
	If (Config.GameEnabled && HasPlayer)
		RegisterForKey(Hotkeys[kGameRaiseEnj])
		RegisterForKey(Hotkeys[kGameHoldback])
	EndIf
	If (!Config.UseSceneMenu)
		RegisterForKey(Hotkeys[kAdvanceAnimation])
		RegisterForKey(Hotkeys[kEndAnimation])
		RegisterForKey(Hotkeys[kChangePositions])
		RegisterForKey(Hotkeys[kOffsetAdjustMode])
		RegisterForKey(Hotkeys[kToggleAdjustStage])
		RegisterForKey(Hotkeys[kRestoreOffsets])
		RegisterForKey(Hotkeys[kDirectionUp])
		RegisterForKey(Hotkeys[kDirectionDown])
		RegisterForKey(Hotkeys[kDirectionLeft])
		RegisterForKey(Hotkeys[kDirectionRight])
		RegisterForKey(Hotkeys[kSceneSelector])
	EndIf
EndFunction

Function DisableTraditionalHotkeys()
	int i = 0
	While (i < Hotkeys.Length)
		UnregisterForKey(Hotkeys[i])
		i += 1
	EndWhile
EndFunction

Event OnKeyDown(int aiKey)
	If (Utility.IsInMenuMode() || _SkipHotkeyEvents)
		return
	EndIf
	_SkipHotkeyEvents = true
	_SkipMenuEvents = true
	bool abModifier = Config.ModifierPressed()
	bool abAdjustTarget = abModifier
	If (aiKey == Hotkeys[kChangeAnimation])
		PickRandomScene("")
	ElseIf (aiKey == Hotkeys[kMoveScene])
		MoveScene()
	ElseIf (aiKey == Hotkeys[kChangePartner])
		ChangeTargetPartner(abModifier)
	EndIf
	If (Config.GameEnabled && HasPlayer)
		If (aiKey == Hotkeys[kGameRaiseEnj])
			ProcessEnjGameArg("Stamina", _AdjustActor, abAdjustTarget)
		ElseIf (aiKey == Hotkeys[kGameHoldback])
			ProcessEnjGameArg("Magicka", _AdjustActor, abAdjustTarget)
		EndIf
	EndIf
	; Legacy
	If (Config.UseSceneMenu)
		_SkipHotkeyEvents = false
		_SkipMenuEvents = false
		return
	EndIf
	If (aiKey == Hotkeys[kAdvanceAnimation])
		AdvanceStage(abModifier)
	ElseIf (aiKey == Hotkeys[kEndAnimation])
		EndAnimation()
	ElseIf (aiKey == Hotkeys[kChangePositions])
		ChangePositions(abAdjustTarget)
	ElseIf (aiKey == Hotkeys[kOffsetAdjustMode])
		CycleOffsetAdjustModes(abModifier)
	ElseIf (aiKey == Hotkeys[kToggleAdjustStage])
		Config.AdjustStage = !Config.AdjustStage
		Debug.Notification("SexLab: AdjustStage: " + Config.AdjustStage)
	ElseIf (aiKey == Hotkeys[kRestoreOffsets])
		RestoreOffsets()
	ElseIf (aiKey == Hotkeys[kSceneSelector])
		SceneSelectorMenu()
	EndIf
	If (_AdjustMode > AdjMode_None)
		string[] asOffsetType = DetermineOffsetAdjustInputType(aiKey)
		HandleOffsetAdjustment(asOffsetType, aiKey, abAdjustTarget)
	EndIf
	_SkipHotkeyEvents = false
	_SkipMenuEvents = false
EndEvent

Function InitLegacyHotkeys()
	Hotkeys = new int[16]
	Hotkeys[kChangeAnimation]   = Config.ChangeAnimation
	Hotkeys[kMoveScene]         = Config.MoveScene
	Hotkeys[kChangePartner]     = Config.TargetActor
	Hotkeys[kGameRaiseEnj]      = Config.GameRaiseEnjKey
	Hotkeys[kGameHoldback]      = Config.GameHoldbackKey
	;Legacy
	Hotkeys[kAdvanceAnimation]  = Config.AdvanceAnimation
	Hotkeys[kEndAnimation]      = Config.EndAnimation
	Hotkeys[kChangePositions]   = Config.ChangePositions
	Hotkeys[kOffsetAdjustMode]  = Config.OffsetAdjustMode
	Hotkeys[kToggleAdjustStage] = Config.ToggleAdjustStage
	Hotkeys[kRestoreOffsets]    = Config.RestoreOffsets
	Hotkeys[kDirectionUp]       = Config.DirectionUp
	Hotkeys[kDirectionDown]     = Config.DirectionDown
	Hotkeys[kDirectionLeft]     = Config.DirectionLeft
	Hotkeys[kDirectionRight]    = Config.DirectionRight
	Hotkeys[kSceneSelector]     = Config.SceneSelectorMenu
EndFunction

; ------------------------------------------------------- ;
; --- Executed Functions [Legacy Hotkeys]             --- ;
; ------------------------------------------------------- ;

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

int Function GetOffsetAdjustMode()
	return _AdjustMode
EndFunction

Function SetOffsetAdjustMode(int aiSet)
	If (aiSet < AdjMode_None || aiSet > AdjMode_SceneRZ)
		return
	EndIf
	_AdjustMode = aiSet
	If (_AdjustMode == AdjMode_None)
		Debug.Notification("SexLab: Disabled offset adjustments")
		return
	EndIf
	If (_AdjustMode == AdjMode_PosXY)
		Debug.Notification("SexLab: Adjusting Position X-Y")
	ElseIf (_AdjustMode == AdjMode_PosRZ)
		Debug.Notification("SexLab: Adjusting Position R-Z")
	ElseIf (_AdjustMode == AdjMode_SceneXY)
		Debug.Notification("SexLab: Adjusting Scene X-Y")
	ElseIf (_AdjustMode == AdjMode_SceneRZ)
		Debug.Notification("SexLab: Adjusting Scene R-Z")
	EndIf
EndFunction

Function CycleOffsetAdjustModes(bool abBackwards = false)
	int modesCount = 5
	int step = 1
	If (abBackwards)
		step = -1
	EndIf
	int aiMode = _AdjustMode + step
	If (aiMode >= modesCount)
		aiMode = 0
	ElseIf (aiMode < 0)
		aiMode = (modesCount - 1)
	EndIf
	SetOffsetAdjustMode(aiMode)
EndFunction

string[] Function DetermineOffsetAdjustInputType(int aiKey)
	string[] ret = Utility.CreateStringArray(2, "")
	If ((aiKey != Config.DirectionUp) && (aiKey != Config.DirectionDown) && \
		(aiKey != Config.DirectionLeft) && (aiKey != Config.DirectionRight))
		return ret
	EndIf
	bool abAdjustingRZ = (_AdjustMode == AdjMode_PosRZ) || (_AdjustMode == AdjMode_SceneRZ)
	If (aiKey == Config.DirectionLeft)
		ret[0] = "-"
		ret[1] = "R"
		If (!abAdjustingRZ)
			ret[1] = "X"
		EndIf
	ElseIf (aiKey == Config.DirectionRight)
		ret[1] = "R"
		If (!abAdjustingRZ)
			ret[1] = "X"
		EndIf
	ElseIf (aiKey == Config.DirectionUp)
		ret[1] = "Z"
		If (!abAdjustingRZ)
			ret[1] = "Y"
		EndIf
	ElseIf (aiKey == Config.DirectionDown)
		ret[0] = "-"
		ret[1] = "Z"
		If (!abAdjustingRZ)
			ret[1] = "Y"
		EndIf
	EndIf
	return ret
EndFunction

Function HandleOffsetAdjustment(String[] asOffsetType, int aiKey, bool abAdjustTarget)
	If (asOffsetType[1] == "")
		return
	EndIf
	PauseTimer(true)
	Actor akAffectedActor = GetTargetPartner()
	If (HasPlayer && !abAdjustTarget)
		akAffectedActor = PlayerRef
	EndIf
	float afValue = Config.AdjustStepSize
	bool abAdjustingPos = (_AdjustMode == AdjMode_PosXY)  || (_AdjustMode == AdjMode_PosRZ)
	ApplyOffsetAdjustment(akAffectedActor, afValue, asOffsetType, abAdjustingPos)
	While (Input.IsKeyPressed(aiKey))
		ApplyOffsetAdjustment(akAffectedActor, afValue, asOffsetType, abAdjustingPos)
		afValue += Config.AdjustStepSize * 0.1
		If (afValue > Config.AdjustStepSize * 5.0)
			afValue = Config.AdjustStepSize * 5.0
		EndIf
		Utility.Wait(0.02)
	EndWhile
	PauseTimer(false)
EndFunction

Function ApplyOffsetAdjustment(Actor akAffectedActor, float afValue, String[] asOffsetType, bool abAdjustingPos)
	If ((asOffsetType[1] == "") || (afValue == 0))
		return
	EndIf
	If (asOffsetType[0] == "-")
		afValue = -afValue
	EndIf
	If (abAdjustingPos)
		SetStageOffset(akAffectedActor, afValue, asOffsetType[1], true)
	Else
		SetSceneOffset(afValue, asOffsetType[1], true)
	EndIf
EndFunction

Function RestoreOffsets()
	SexLabRegistry.ResetSceneOffset(GetActiveScene())
	SexLabRegistry.ResetStageOffsetA(GetActiveScene(), GetActiveStage())
	RealignActors()
EndFunction

Function SceneSelectorMenu()
	If (Game.GetModByName("UIExtensions.esp") == 255)
		return
	EndIf
	string asActiveScene = GetActiveScene()
	string asActSceneName = SexlabRegistry.GetSceneName(asActiveScene)
	string[] asPlayingScenes = GetPlayingScenes()
	int aiPlayingLen = asPlayingScenes.Length
	; Init Menus
	UIListMenu ListMenu = UIExtensions.GetMenu("UIListMenu") as UIListMenu
	int alOffsetAdjMode = ListMenu.AddEntryItem("$SSL_SS_OffsetAdjustMode", entryHasChildren=True)
	int alPlayingScenes = ListMenu.AddEntryItem("$SSL_SS_ChangeCurrentScene", entryHasChildren=True)
	int alCustomInput   = ListMenu.AddEntryItem("$SSL_SS_ResetScenesByTagName")
	ListMenu.AddEntryItem("$SSL_SS_AdjNone", entryParent=alOffsetAdjMode)
	ListMenu.AddEntryItem("$SSL_SS_AdjPosXY", entryParent=alOffsetAdjMode)
	ListMenu.AddEntryItem("$SSL_SS_AdjPosRZ", entryParent=alOffsetAdjMode)
	ListMenu.AddEntryItem("$SSL_SS_AdjSceneXY", entryParent=alOffsetAdjMode)
	ListMenu.AddEntryItem("$SSL_SS_AdjSceneRZ", entryParent=alOffsetAdjMode)
	If (aiPlayingLen > 120) ; Range for AddEntryItem 0-127 (hardcoded: 8 above)
		aiPlayingLen = 120
		asPlayingScenes = SexLabUtil.ShuffleStringArray(asPlayingScenes, asActiveScene, 120)
	EndIf
	int i = 0
	While (i < aiPlayingLen)
		If (asPlayingScenes[i] == asActiveScene)
			ListMenu.AddEntryItem(">>> " + asActSceneName, entryParent=alPlayingScenes)
		Else
			string asSceneName = SexlabRegistry.GetSceneName(asPlayingScenes[i])
			ListMenu.AddEntryItem(asSceneName, entryParent=alPlayingScenes)
		EndIf
		i += 1
	EndWhile
	; Exec Menus
	ListMenu.OpenMenu()
	string asOptSelected = ListMenu.GetResultString()
	If (asOptSelected == "")
		return
	EndIf
	If (asOptSelected == "$SSL_SS_ResetScenesByTagName")
		UITextEntryMenu TextMenu = UIExtensions.GetMenu("UITextEntryMenu") as UITextEntryMenu
		TextMenu.OpenMenu()
		string asTypedText = TextMenu.GetResultString()
		If (asTypedText == "")
			return
		EndIf
		bool aiNewScenes = ResetAnimationQuick(GetPositions(), asTypedText, false)
		If (!aiNewScenes)
			string asNewScene = SexLabRegistry.GetSceneByName(asTypedText)
			If (asNewScene)
				ResetScene(asNewScene)
			EndIf
		EndIf
	ElseIf (asOptSelected == "$SSL_SS_AdjNone")
		SetOffsetAdjustMode(AdjMode_None)
	ElseIf (asOptSelected == "$SSL_SS_AdjPosXY")
		SetOffsetAdjustMode(AdjMode_PosXY)
	ElseIf (asOptSelected == "$SSL_SS_AdjPosRZ")
		SetOffsetAdjustMode(AdjMode_PosRZ)
	ElseIf (asOptSelected == "$SSL_SS_AdjSceneXY")
		SetOffsetAdjustMode(AdjMode_SceneXY)
	ElseIf (asOptSelected == "$SSL_SS_AdjSceneRZ")
		SetOffsetAdjustMode(AdjMode_SceneRZ)		
	Else
		If (asOptSelected != ">>> " + asActSceneName)
			string asSelectedScene = SexLabRegistry.GetSceneByName(asOptSelected)
			ResetScene(asSelectedScene)
		EndIf
	EndIf
EndFunction

int Function GetAdjustPos()
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

Function ChangeTargetPartner(bool abBackwards = false)
	If (Positions.Length < 3)
		return
	EndIf
	int curIdx = GetAdjustPos()
	int newIdx = IndexTravelComplex(curIdx, abBackwards, PlayerRef)
	_AdjustActor = GetIdxPosition(newIdx)
	Config.SetTargetActor(_AdjustActor)
	Config.SelectedSpell.Cast(_AdjustActor)	; SFX for visual feedback
	PlayHotkeyFX(0, !abBackwards)
	Debug.Notification("SexLab partner selected: " + SexLabUtil.ActorName(_AdjustActor))
	Log("ChangeTargetPartner(), currently focused partner: " + SexLabUtil.ActorName(_AdjustActor))
EndFunction

Function PlayHotkeyFX(int i, bool abBackwards)
	If (abBackwards)
		Config.HotkeyDown[i].Play(PlayerRef)
	Else
		Config.HotkeyUp[i].Play(PlayerRef)
	EndIf
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
	; L1 -> EnjGame + TargetActor
	RegisterGesture(1, "ToggleAdjustSelf")          ; L (tap) = Left Thumbstick Press or Trackpad Tap
	If (Config.GameEnabled && HasPlayer)
		RegisterGesture(2, "GameRaiseEnj")          ; L + up
		RegisterGesture(3, "GameHoldback")          ; L + down
	EndIf
	RegisterGesture(4, "TargetPartnerPrev")         ; L + left
	RegisterGesture(5, "TargetPartnerNext")         ; L + right
	RegisterGesture(6, "SwitchPOVPrev")             ; L + back
	RegisterGesture(7, "SwitchPOVNext")             ; L + forward
	; R1 -> SceneControl Main
	RegisterGesture(14, "ToggleFreeCam")            ; R (tap) = Right Thumbstick Press or Trackpad Tap
	RegisterGesture(15, "SceneChange")              ; R + up
	RegisterGesture(16, "SceneEnd")                 ; R + down
	RegisterGesture(17, "StagePrev")                ; R + left
	RegisterGesture(18, "StageNext")                ; R + right
	; L2 -> OffsetAdjust Inputs
	RegisterGesture(27, "AdjustStageToggle")        ; L2 (tap) = Left Index Touchpad Press
	RegisterGesture(28, "OffsetUp")                 ; L2 + up
	RegisterGesture(29, "OffsetDown")               ; L2 + down
	RegisterGesture(30, "OffsetLeft")               ; L2 + left
	RegisterGesture(31, "OffsetRight")              ; L2 + right
	; R2 -> SceneControl Complex
	RegisterGesture(40, "SceneSelectorMenu")        ; R2 (tap) = Right Index Touchpad Press
	RegisterGesture(41, "AdjOffsetModeNext")        ; R2 + up
	RegisterGesture(42, "AdjOffsetModePrev")        ; R2 + down
	RegisterGesture(43, "ChangePositions")          ; R2 + left
	;RegisterGesture(44, "ChangePosBackward")        ; R2 + right
	RegisterGesture(45, "RestoreOffsets")           ; R2 + back
	RegisterGesture(46, "MoveScene")                ; R2 + forward
EndFunction

Function DisableGesturesVR()
	If (!Config.UseGestures)
		return
	EndIf
	VRIK.VrikEndGestureProfile()
	UnregisterGesture("ToggleAdjustSelf")
	UnregisterGesture("GameRaiseEnj")
	UnregisterGesture("GameHoldback")
	UnregisterGesture("TargetPartnerPrev")
	UnregisterGesture("TargetPartnerNext")
	UnregisterGesture("SwitchPOVPrev")
	UnregisterGesture("SwitchPOVNext")
	UnregisterGesture("ToggleFreeCam")
	UnregisterGesture("SceneChange")
	UnregisterGesture("SceneEnd")
	UnregisterGesture("StagePrev")
	UnregisterGesture("StageNext")
	UnregisterGesture("AdjustStageToggle")
	UnregisterGesture("OffsetUp")
	UnregisterGesture("OffsetDown")
	UnregisterGesture("OffsetLeft")
	UnregisterGesture("OffsetRight")
	UnregisterGesture("SceneSelectorMenu")
	UnregisterGesture("AdjOffsetModeNext")
	UnregisterGesture("AdjOffsetModePrev")
	UnregisterGesture("ChangePositions")
	;UnregisterGesture("ChangePosBackward")
	UnregisterGesture("RestoreOffsets")
	UnregisterGesture("MoveScene")
EndFunction

Function VRHandleGesture(String asEventName, String Foobar, float Presses, Form Sender)
	If (Utility.IsInMenuMode() || _SkipGestureEvents)
		return
	EndIf
	_SkipGestureEvents = true
	bool abAdjustTarget = !_AdjustSelfVR
	If (HasPlayer && Config.GameEnabled)
		If (asEventName == "SLVR_GameRaiseEnj")
			ProcessEnjGameArg("Stamina", GetTargetPartner(), abAdjustTarget)
		ElseIf (asEventName == "SLVR_GameHoldback")
			ProcessEnjGameArg("Magicka", GetTargetPartner(), abAdjustTarget)
		EndIf
	EndIf
	If (asEventName == "SLVR_ToggleAdjustSelf")
		_AdjustSelfVR = !_AdjustSelfVR
		Debug.Notification("SexLab: AdjustSelf: " + _AdjustSelfVR)
	ElseIf (asEventName == "SLVR_TargetPartnerPrev")
		ChangeTargetPartner(true)
	ElseIf (asEventName == "SLVR_TargetPartnerNext")
		ChangeTargetPartner()
	ElseIf (asEventName == "SLVR_SwitchPOVPrev")
		CyclePOVModesVR(true)
	ElseIf (asEventName == "SLVR_SwitchPOVNext")
		CyclePOVModesVR()
	ElseIf (asEventName == "SLVR_ToggleFreeCam")
		SexLabUtil.ToggleFreeCamera()
	ElseIf (asEventName == "SLVR_SceneChange")
		PickRandomScene("")
	ElseIf (asEventName == "SLVR_SceneEnd")
		EndAnimation()
	ElseIf (asEventName == "SLVR_StagePrev")
		AdvanceStage(true)
	ElseIf (asEventName == "SLVR_StageNext")
		AdvanceStage()
	ElseIf (asEventName == "SLVR_AdjustStageToggle")
		Config.AdjustStage = !Config.AdjustStage
		Debug.Notification("SexLab: AdjustStage: " + Config.AdjustStage)
	ElseIf (asEventName == "SLVR_AdjOffsetModeNext")
		If (GetOffsetAdjustMode() == AdjMode_None)
			SexLabUtil.ForceThirdPerson()
		EndIf
		CycleOffsetAdjustModes()
	ElseIf (asEventName == "SLVR_AdjOffsetModePrev")
		If (GetOffsetAdjustMode() == AdjMode_None)
			SexLabUtil.ForceThirdPerson()
		EndIf
		CycleOffsetAdjustModes(true)
	ElseIf (asEventName == "SLVR_ChangePositions")
		ChangePositions(abAdjustTarget)
	ElseIf (asEventName == "SLVR_MoveScene")
		MoveScene()
	ElseIf (asEventName == "SLVR_RestoreOffsets")
		RestoreOffsets()
	ElseIf (asEventName == "SLVR_SceneSelectorMenu")
		SceneSelectorMenu()
	EndIf
	If (GetOffsetAdjustMode() > AdjMode_None)
		string[] asOffsetType = Utility.CreateStringArray(2, "")
		If (asEventName == "SLVR_OffsetUp")
			asOffsetType = DetermineOffsetAdjustInputType(Config.DirectionUp)
		ElseIf (asEventName == "SLVR_OffsetDown")
			asOffsetType = DetermineOffsetAdjustInputType(Config.DirectionDown)
		ElseIf (asEventName == "SLVR_OffsetLeft")
			asOffsetType = DetermineOffsetAdjustInputType(Config.DirectionLeft)
		ElseIf (asEventName == "SLVR_OffsetRight")
			asOffsetType = DetermineOffsetAdjustInputType(Config.DirectionRight)
		EndIf
		HandleOffsetAdjustmentVR(asOffsetType, abAdjustTarget)
	EndIf
	_SkipGestureEvents = false
EndFunction

Function CyclePOVModesVR(bool abBackwards = false)
	int aiMode = Config.VRIK_TPP_FREE
	If (GetOffsetAdjustMode() == AdjMode_None)
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
	EndIf
	Config.SetPOVModeVRIK(aiMode)
EndFunction

Function HandleOffsetAdjustmentVR(String[] asOffsetType, bool abAdjustTarget)
	If (asOffsetType[1] == "")
		return
	EndIf
	PauseTimer(true)
	Actor akAffectedActor = GetTargetPartner()
	If (HasPlayer && !abAdjustTarget)
		akAffectedActor = PlayerRef
	EndIf
	float afValue = Config.AdjustStepSize
	int aiAdjMode = GetOffsetAdjustMode()
	bool abAdjustingPos = (aiAdjMode == AdjMode_PosXY)  || (aiAdjMode == AdjMode_PosRZ)
	ApplyOffsetAdjustment(akAffectedActor, afValue, asOffsetType, abAdjustingPos)
	float refX = VRIK.VrikGetHandX(true)
	float refY = VRIK.VrikGetHandY(true)
	float refZ = VRIK.VrikGetHandZ(true)
	float newX = 0.0
	float newY = 0.0
	float newZ = 0.0
	float curDrift = 0.0
	While (VRIK.VrikIsTriggerPressed(true))
		newX = VRIK.VrikGetHandX(true)
		newY = VRIK.VrikGetHandY(true)
		newZ = VRIK.VrikGetHandZ(true)
		curDrift = Math.Abs(newX - refX) + Math.Abs(newY - refY) + Math.Abs(newZ - refZ)
		If (curDrift > 30.0) ;hand moved too much, not roughly in place
			PauseTimer(false)
			return
		EndIf
		ApplyOffsetAdjustment(akAffectedActor, afValue, asOffsetType, abAdjustingPos)
		Utility.Wait(0.02)
		afValue += Config.AdjustStepSize * 0.1
		If (afValue > Config.AdjustStepSize * 5.0)
			afValue = Config.AdjustStepSize * 5.0
		EndIf
	EndWhile
	If (Config.GestureHaptics)
		VRIK.VrikHapticPulse(true, 2, 800)
	EndIf
	PauseTimer(false)
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
	float Amount = 15.0
	If(Config.IsAdjustStagePressed())
		Amount = 180.0
	ElseIf(backwards)
		Amount = -15.0
	EndIf
	
	bool first_pass = true
	While(true)
		PlayHotkeyFX(1, !backwards)
		float[] coords
		coords[5] = coords[5] + Amount
		If(coords[5] >= 360.0)
			coords[5] = coords[5] - 360.0
		ElseIf(coords[5] < 0.0)
			coords[5] = coords[5] + 360.0
		EndIf
		CenterOnCoords(coords[0], coords[1], coords[2], 0, 0, coords[5], true)
		Utility.Wait(0.03)
		If(!Input.IsKeyPressed(Hotkeys[kDirectionLeft])) ;kRotateScene
			RegisterForSingleUpdate(0.2)
			return
		ElseIf (first_pass)
			first_pass = false
			Utility.Wait(0.4)
		EndIf
	EndWhile
EndFunction

Function AdjustChange(bool backwards = false)
	ChangeTargetPartner(backwards)
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
	; AdjustSchlongEx(backwards, true)
EndFunction
