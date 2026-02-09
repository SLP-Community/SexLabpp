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

Event OnEffectStart(Actor TargetRef, Actor CasterRef)
	sslLog.Log("sslActorCumEffect: OnEffectStart()" + TargetRef)
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
	float currentTime = SexLabUtil.GetCurrentGameRealTime()
	int[] appliedTypes = StorageUtil.IntListToArray(targetRef, ActorLib.APPLIED_TEXTURE_LIST)
	int i = 0
	While (i < appliedTypes.Length)
		int appliedType = appliedTypes[i]
		float appliedAt = StorageUtil.GetFloatValue(targetRef, ActorLib.LAST_APPLIED_TIME_PREFIX + appliedType)
		float appliedDuration = currentTime - appliedAt
		If (appliedDuration > sslSystemConfig.GetSettingFlt("fCumTimer"))
			ActorLib.RemoveCumFx(targetRef, appliedType)
		EndIf
		i += 1
	EndWhile
EndEvent
