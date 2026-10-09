scriptname sslActorCumEffect extends ActiveMagicEffect
{
	Script to control duration of the default cum effect spell
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

sslActorLibrary Property ActorLib Auto

; Game time (in days) at the last update. The timer adds up the game time that passed between two updates,
; each piece turned into seconds at the timescale it passed at. Waiting, sleeping and time spent away all
; count. Comparing two readings of SexLabUtil.GetCurrentGameRealTime() instead divides the whole game time
; by the current timescale, so every change of timescale moved the timer by hours
float _lastGameTime

Event OnEffectStart(Actor TargetRef, Actor CasterRef)
	sslLog.Log("sslActorCumEffect: OnEffectStart()" + TargetRef)
	_lastGameTime = Utility.GetCurrentGameTime()
	RegisterForUpdate(3.0)
EndEvent

Event OnCellAttach()
	sslLog.Log("sslActorCumEffect: OnCellAttach()" + GetTargetActor())
	RegisterForUpdate(3.0)
EndEvent

Event OnCellDetach()
	sslLog.Log("sslActorCumEffect: OnCellDetach()" + GetTargetActor())
	UnregisterForUpdate()
EndEvent

Event OnUpdate()
	Actor targetRef = GetTargetActor()
	If (targetRef.IsSwimming() && sslSystemConfig.GetSettingBool("bSwimmingCleans"))
		ActorLib.RemoveCumFx(targetRef, ActorLib.FX_ALL)
		return
	EndIf
	float secondsPassed = SecondsSinceLastUpdate()
	int[] appliedTypes = StorageUtil.IntListToArray(targetRef, ActorLib.APPLIED_TEXTURE_LIST)
	int i = 0
	While (i < appliedTypes.Length)
		int appliedType = appliedTypes[i]
		float appliedDuration = StorageUtil.AdjustFloatValue(targetRef, ActorLib.APPLIED_SECONDS_PREFIX + appliedType, secondsPassed)
		If (appliedDuration > sslSystemConfig.GetSettingFlt("fCumTimer"))
			ActorLib.RemoveCumFx(targetRef, appliedType)
		EndIf
		i += 1
	EndWhile
EndEvent

float Function SecondsSinceLastUpdate()
	float now = Utility.GetCurrentGameTime()
	float last = _lastGameTime
	_lastGameTime = now
	If (last <= 0.0 || now <= last)
		; an effect that was already running when this counter was added, or no game time passed
		return 0.0
	EndIf
	float timescale = (Game.GetFormFromFile(0x3A, "Skyrim.esm") as GlobalVariable).GetValue()
	If (timescale < 1.0)
		timescale = 1.0
	EndIf
	return (now - last) * 86400.0 / timescale
EndFunction
