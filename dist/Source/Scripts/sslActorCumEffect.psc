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

; Seconds between two updates. The cum timer is counted in these steps instead of being read off
; SexLabUtil.GetCurrentGameRealTime(): that clock is game time over the current timescale, so it jumps
; whenever the timescale changes or game time is skipped
float Property UPDATE_INTERVAL = 3.0 AutoReadOnly Hidden

Event OnEffectStart(Actor TargetRef, Actor CasterRef)
	sslLog.Log("sslActorCumEffect: OnEffectStart()" + TargetRef)
	RegisterForUpdate(UPDATE_INTERVAL)
EndEvent

Event OnCellAttach()
	sslLog.Log("sslActorCumEffect: OnCellAttach()" + GetTargetActor())
	RegisterForUpdate(UPDATE_INTERVAL)
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
	int[] appliedTypes = StorageUtil.IntListToArray(targetRef, ActorLib.APPLIED_TEXTURE_LIST)
	int i = 0
	While (i < appliedTypes.Length)
		int appliedType = appliedTypes[i]
		float appliedDuration = StorageUtil.AdjustFloatValue(targetRef, ActorLib.APPLIED_SECONDS_PREFIX + appliedType, UPDATE_INTERVAL)
		If (appliedDuration > sslSystemConfig.GetSettingFlt("fCumTimer"))
			ActorLib.RemoveCumFx(targetRef, appliedType)
		EndIf
		i += 1
	EndWhile
EndEvent
