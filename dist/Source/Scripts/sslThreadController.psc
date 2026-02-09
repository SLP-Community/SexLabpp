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

Message Property RepositionInfoMsg Auto
{[Ok, Cancel, Don't show again]}


bool _SkipHotkeyEvents
int _AutoAdvanceCache

String[] _MenuEvents

Function EnableHotkeys(bool forced = false)
	If(!HasPlayer && !forced || !TryOpenSceneMenu())
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
	EnableTraditionalHotkeys()
EndFunction

Function DisableHotkeys()
	int i = 0
	While (i < _MenuEvents.Length)
		UnregisterForModEvent(_MenuEvents[i])
		i += 1
	EndWhile
	; If free cam is active here will glitch out controls?
	MiscUtil.SetFreeCameraState(false)
	TryCloseSceneMenu()
	DisableTraditionalHotkeys()
EndFunction

Event MenuEvent(string asEventName, string asStringArg, float afNumArg, form akSender)
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
		If (!sslSystemConfig.HasAnimSpeedSE())
			Log("SetSpeed: AnimSpeedSE not found")
			return
		EndIf
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
EndEvent

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
	sslActorAlias PlayerSlot = ActorAlias(PlayerRef)
	If (HasPlayer)
		PlayerSlot.TryPauseAndUnlock()
	Else
		Game.DisablePlayerControls(false, true, false, false, true)
	EndIf
	int n = 0
	While(n < Positions.Length)
		ActorAlias[n].GoToState(ActorAlias[n].STATE_PAUSED)
		n += 1
	EndWhile
	Utility.Wait(1)
	int t = 0
	While(t < 60 && !Input.IsKeyPressed(Config.MoveScene))
		Utility.Wait(0.5)
		t += 1
	EndWhile
	Game.DisablePlayerControls()	; make sure player isnt moving before resync
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
	If (!HasPlayer)
		Game.EnablePlayerControls()
	EndIf
	CenterOnObject(PlayerRef)
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

Function SetSceneOffset(float afOffsetValue, String asOffsetType)
	String activeScene = GetActiveScene()
	int idx = GetOffsetIdx(asOffsetType)
	SexLabRegistry.SetSceneOffset(activeScene, afOffsetValue, idx)
	ResetStage()
EndFunction

Function SetStageOffset(Actor akAffectedActor, float afOffsetValue, String asOffsetType)
	int idx = GetOffsetIdx(asOffsetType)
	int n = GetPositions().Find(akAffectedActor)
	String activeScene = GetActiveScene()
	String activeStage = ""
	If (sslSystemConfig.GetSettingBool("bAdjustTargetStage"))
		activeStage = GetActiveStage()
	EndIf
	SexLabRegistry.SetStageOffset(activeScene, activeStage, n, afOffsetValue, idx)
	UpdatePlacement(akAffectedActor)
EndFunction

Function EnableTraditionalHotkeys()
	RegisterForKey(Config.ChangeAnimation)
	RegisterForKey(Config.MoveScene)
EndFunction

Function DisableTraditionalHotkeys()
	UnregisterForKey(Config.ChangeAnimation)
	UnregisterForKey(Config.MoveScene)
EndFunction

Event OnKeyDown(int KeyCode)
	If(Utility.IsInMenuMode() || _SkipHotkeyEvents)
		return
	EndIf
	_SkipHotkeyEvents = true
	If(KeyCode == Config.ChangeAnimation)
		ChangeAnimation(Input.IsKeyPressed(Config.GameUtilityKey))
	ElseIf(KeyCode == Config.MoveScene)
		MoveScene()
	EndIf
	_SkipHotkeyEvents = false
EndEvent

Function ChangeAnimation(bool backwards = false)
	string[] Scenes = GetPlayingScenes()
	If(Scenes.Length < 2)
		return
	EndIf
	UnregisterForUpdate()
	int current = Scenes.Find(GetActiveScene())
	String newScene
	If (!Config.AdjustStagePressed())
		newScene = Scenes[sslUtility.IndexTravel(current, Scenes.Length, backwards)]
	Else
		int r = Utility.RandomInt(0, Scenes.Length - 1)
		While(r == current)
			r = Utility.RandomInt(0, Scenes.Length - 1)
		EndWhile
		newScene = Scenes[r]
	EndIf
	Log("Changing running scene from " + GetActiveScene() + " to " + newScene)
	SendThreadEvent("AnimationChange")
	ResetScene(newScene)
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

sslActorAlias AdjustAlias		; The actor currently selected for position adjustments

int[] Hotkeys
int Property kAdvanceAnimation = 0 AutoReadOnly
int Property kChangeAnimation  = 1 AutoReadOnly
int Property kChangePositions  = 2 AutoReadOnly
int Property kAdjustChange     = 3 AutoReadOnly
int Property kAdjustForward    = 4 AutoReadOnly
int Property kAdjustSideways   = 5 AutoReadOnly
int Property kAdjustUpward     = 6 AutoReadOnly
int Property kRealignActors    = 7 AutoReadOnly
int Property kRestoreOffsets   = 8 AutoReadOnly
int Property kMoveScene        = 9 AutoReadOnly
int Property kRotateScene      = 10 AutoReadOnly
int Property kEndAnimation     = 11 AutoReadOnly
int Property kAdjustSchlong    = 12 AutoReadOnly
;/
Event OnKeyDown(int KeyCode)
	If(Utility.IsInMenuMode() || _SkipHotkeyEvents)
		return
	EndIf
	_SkipHotkeyEvents = true
	int hotkey = Hotkeys.Find(KeyCode)
	If(hotkey == kAdvanceAnimation)
		If (Config.BackwardsPressed())
			AdvanceStage(true)
		Else
			AdvanceStage(false)
		EndIf
	ElseIf(hotkey == kChangeAnimation)
		ChangeAnimation(Config.BackwardsPressed())
	ElseIf(hotkey == kAdjustForward)
		AdjustForward(Config.BackwardsPressed(), Config.AdjustStagePressed())
	ElseIf(hotkey == kAdjustUpward)
		AdjustUpward(Config.BackwardsPressed(), Config.AdjustStagePressed())
	ElseIf(hotkey == kAdjustSideways)
		AdjustSideways(Config.BackwardsPressed(), Config.AdjustStagePressed())
	ElseIf(hotkey == kRotateScene)
		RotateScene(Config.BackwardsPressed())
	ElseIf(hotkey == kAdjustSchlong)
		; AdjustSchlongEx(Config.BackwardsPressed(), Config.AdjustStagePressed())
	ElseIf(hotkey == kAdjustChange) ; Change Adjusting Position
		AdjustChange(Config.BackwardsPressed())
	ElseIf(hotkey == kRealignActors)
		RealignActors()
	ElseIf(hotkey == kChangePositions)
		ChangePositions()
	ElseIf(hotkey == kRestoreOffsets)
		RestoreOffsets()
	ElseIf(hotkey == kMoveScene)
		MoveScene()
	ElseIf(hotkey == kEndAnimation)
		EndAnimation()
	EndIf
	_SkipHotkeyEvents = false
EndEvent
/;
int Function GetAdjustPos()
	int AdjustPos = -1
	if AdjustAlias && AdjustAlias.ActorRef
		AdjustPos = Positions.Find(AdjustAlias.ActorRef)
	endIf
	if AdjustPos == -1 && Config.TargetRef
		AdjustPos = Positions.Find(Config.TargetRef)
	endIf
	if AdjustPos == -1
		AdjustPos = (ActorCount > 1) as int
	endIf
	if Positions[AdjustPos] != PlayerRef
		Config.TargetRef = Positions[AdjustPos]
	endIf
	AdjustAlias = PositionAlias(AdjustPos)
	return AdjustPos
EndFunction

Function AdvanceStage(bool backwards = false)
	If(!backwards)
		GoToStage(Stage + 1)
	Elseif(Config.IsAdjustStagePressed())
		GoToStage(1)
	ElseIf(Stage > 1)
		GoToStage(Stage - 1)
	EndIf
EndFunction

Function AdjustCoordinate(bool abBackwards, bool abStageOnly, float afValue, int aiKeyIdx, int aiOffsetType)
	; aiOffsetType := [X, Y, Z, Rotation]
	UnregisterForUpdate()
	String scene_ = GetActiveScene()
	String stage_ = ""
	If (!abStageOnly)
		stage_ = GetActiveStage()
	EndIf
	int AdjustPos = GetAdjustPos()
	bool first_pass = true
	While(true)
		PlayHotkeyFX(0, abBackwards)
		SexLabRegistry.SetStageOffset(scene_, stage_, AdjustPos, afValue, aiOffsetType)
		; UpdatePlacement(AdjustAlias.GetActorReference())
		Utility.Wait(0.1)
		If(!Input.IsKeyPressed(Hotkeys[aiKeyIdx]))
			UpdateTimer(5)
			OnUpdate()
			return
		ElseIf (first_pass)
			first_pass = false
			Utility.Wait(0.4)
		EndIf
	EndWhile
EndFunction
Function AdjustForward(bool backwards = false, bool AdjustStage = false)
	float value = 0.5 - (backwards as float)
	AdjustCoordinate(backwards, AdjustStage, value, kAdjustForward, 0)
EndFunction
Function AdjustSideways(bool backwards = false, bool AdjustStage = false)
	float value = 0.5 - (backwards as float)
	AdjustCoordinate(backwards, AdjustStage, value, kAdjustSideways, 1)
EndFunction
Function AdjustUpward(bool backwards = false, bool AdjustStage = false)
	float value = 0.5 - (backwards as float)
	AdjustCoordinate(backwards, AdjustStage, value, kAdjustUpward, 2)
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
		If(!Input.IsKeyPressed(Hotkeys[kRotateScene]))
			RegisterForSingleUpdate(0.2)
			return
		ElseIf (first_pass)
			first_pass = false
			Utility.Wait(0.4)
		EndIf
	EndWhile
EndFunction

Function AdjustChange(bool backwards = false)
	If(Positions.Length <= 1)
		return
	EndIf
	int i = GetAdjustPos()
	i = sslUtility.IndexTravel(i, ActorCount, backwards)
	If(Positions[i] != PlayerRef)
		Config.TargetRef = Positions[i]
	EndIf
	AdjustAlias = ActorAlias[i]
	Config.SelectedSpell.Cast(Positions[i])	; SFX for visual feedback
	PlayHotkeyFX(0, !backwards)
	String msg = "Adjusting Position For: " + AdjustAlias.GetActorName()
	Debug.Notification(msg)
	SexLabUtil.PrintConsole(msg)
EndFunction

Function RestoreOffsets()
	SexLabRegistry.ResetStageOffsetA(GetActiveScene(), GetActiveStage())
	RealignActors()
EndFunction

Function ChangePositions(bool backwards = false)
	If(Positions.Length < 2)
		return
	EndIf
	String activeScene = GetActiveScene()
	Actor actor_adj = AdjustAlias.GetActorReference()
	int i_adj = GetAdjustPos()
	int i = i_adj + 1
	While(i < Positions.Length + i_adj)
		If(i >= Positions.Length)
			i -= Positions.Length
		EndIf
		If(SexLabRegistry.CanFillPosition(activeScene, i_adj, Positions[i]) && \
				SexLabRegistry.CanFillPosition(activeScene, i, actor_adj))
			Actor tmpAct = Positions[i_adj]
			Positions[i_adj] = Positions[i]
			Positions[i] = tmpAct

			sslActorAlias tmpAli = ActorAlias[i_adj]
			ActorAlias[i_adj] = ActorAlias[i]
			ActorAlias[i] = tmpAli

			SendThreadEvent("PositionChange")
			ResetStage()
			return
		EndIf
		i += 1
	EndWhile
	Debug.Notification("Selected actor cannot switch positions")
EndFunction

Function PlayHotkeyFX(int i, bool backwards)
	if backwards
		Config.HotkeyDown[i].Play(PlayerRef)
	else
		Config.HotkeyUp[i].Play(PlayerRef)
	endIf
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
