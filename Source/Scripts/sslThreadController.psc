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

sslActorAlias AdjustAlias		; The actor currently selected for position adjustments
bool _SkipHotkeyEvents

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

String[] _MenuEvents

Function EnableHotkeys(bool forced = false)
	If(!HasPlayer && !forced || !TryOpenSceneMenu())
		return
	EndIf
	int i = 0
	While (i < _MenuEvents.Length)
		RegisterForModEvent(_MenuEvents[i], "MenuEvent")
		i += 1
	EndWhile
	; RegisterForModEvent("SL_StageAdvance", "MenuEvent")
	; RegisterForModEvent("SL_SetSpeed", "MenuEvent")
	; RegisterForModEvent("SL_EndScene", "MenuEvent")

	; Hotkeys = new int[13]
	; Hotkeys[kAdvanceAnimation] = Config.AdvanceAnimation
	; Hotkeys[kChangeAnimation] = Config.ChangeAnimation
	; Hotkeys[kChangePositions] = Config.ChangePositions
	; Hotkeys[kAdjustSideways] = Config.AdjustSideways
	; Hotkeys[kRestoreOffsets] = Config.RestoreOffsets
	; Hotkeys[kAdjustForward] = Config.AdjustForward
	; Hotkeys[kRealignActors] = Config.RealignActors
	; Hotkeys[kAdjustSchlong] = Config.AdjustSchlong
	; Hotkeys[kAdjustUpward] = Config.AdjustUpward
	; Hotkeys[kAdjustChange] = Config.AdjustChange
	; Hotkeys[kEndAnimation] = Config.EndAnimation
	; Hotkeys[kRotateScene] = Config.RotateScene
	; Hotkeys[kMoveScene] = Config.MoveScene
	; int i = 0
	; While(i < Hotkeys.Length)
	; 	RegisterForKey(Hotkeys[i])
	; 	i += 1
	; Endwhile
EndFunction

Function DisableHotkeys()
	int i = 0
	While (i < _MenuEvents.Length)
		UnregisterForModEvent(_MenuEvents[i])
		i += 1
	EndWhile
	TryCloseSceneMenu()
EndFunction

Event MenuEvent(string asEventName, string asStringArg, float afNumArg, form akSender)
	sslLog.Log("MenuEvent: " + asEventName)
	If(asEventName == "SL_StageAdvance")
		PlayNextImpl(asStringArg)
	ElseIf(asEventName == "SL_SetSpeed")
		If (!sslSystemConfig.HasAnimSpeedSE())
			sslLog.Log("SetSpeed: AnimSpeedSE not found")
			return
		EndIf
		int i = 0
		While (i < ActorAlias.Length)
			ActorAlias[i].UpdateBaseSpeed(afNumArg)
			i += 1
		EndWhile
	ElseIf(asEventName == "SL_EndScene")
		EndAnimation()
	EndIf
EndEvent

Event OnKeyDown(int KeyCode)
	If(Utility.IsInMenuMode() || _SkipHotkeyEvents)
		return
	EndIf
	_SkipHotkeyEvents = true
	int hotkey = Hotkeys.Find(KeyCode)
	If(hotkey == kAdvanceAnimation)
		If (Config.BackwardsPressed())
			AdvanceStage(true)
			int i = 0
			While (i < Positions.Length)
				ActorAlias[i].InternalCompensateStageSkip()
				i += 1
			EndWhile
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
		SexLabRegistry.UpdateOffset(scene_, stage_, AdjustPos, afValue, aiOffsetType)
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
	SexLabRegistry.ResetOffsetA(GetActiveScene(), GetActiveStage())
	RealignActors()
EndFunction

Function MoveScene()
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
	; Make sure the player cannot activate anything, change worldspaces or start combat on their own
	Game.DisablePlayerControls(false, true, false, false, true)
	sslActorAlias PlayerSlot = ActorAlias(PlayerRef)
	int n = 0
	While(n < Positions.Length)
		If(ActorAlias[n] == PlayerSlot)
			ActorAlias[n].TryUnlock()
		Else
			ActorAlias[n].SendDefaultAnimEvent(true)
		EndIf
		n += 1
	EndWhile
	Utility.Wait(1)
	int i = 0
	While(i < 60 && !Input.IsKeyPressed(Hotkeys[kMoveScene]))
		Utility.Wait(0.5)
		i += 1
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
	If(PlayerSlot)
		PlayerSlot.LockActor()
	EndIf
	Game.EnablePlayerControls()		; placing doesnt interact with player controls
	CenterOnObject(PlayerRef)			; Will re-register the update loop
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

; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;
; ----------------------------------------------------------------------------- ;
;								██╗     ███████╗ ██████╗  █████╗  ██████╗██╗   ██╗							;
;								██║     ██╔════╝██╔════╝ ██╔══██╗██╔════╝╚██╗ ██╔╝							;
;								██║     █████╗  ██║  ███╗███████║██║      ╚████╔╝ 							;
;								██║     ██╔══╝  ██║   ██║██╔══██║██║       ╚██╔╝  							;
;								███████╗███████╗╚██████╔╝██║  ██║╚██████╗   ██║   							;
;								╚══════╝╚══════╝ ╚═════╝ ╚═╝  ╚═╝ ╚═════╝   ╚═╝   							;
; ----------------------------------------------------------------------------- ;
; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;

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
