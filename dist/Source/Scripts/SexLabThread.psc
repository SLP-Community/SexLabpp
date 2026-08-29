ScriptName SexLabThread extends Quest
{
  API Script to directly interact with individual SexLab Threads
}

; The thread ID of the current thread
; These are unique and can be used to reference this specific thread throughout other parts of the framework
int Function GetThreadID()
EndFunction

; ------------------------------------------------------- ;
; --- Thread Status                                   --- ;
; ------------------------------------------------------- ;
;/
	View and manipulate runtime data
/;

int Property STATUS_UNDEF	= 0 AutoReadOnly  ; Undefined
int Property STATUS_IDLE	= 1 AutoReadOnly  ; Idling (Inactive)
int Property STATUS_SETUP	= 2 AutoReadOnly  ; Preparing an animation. Available data may be incomplete
int Property STATUS_INSCENE	= 3 AutoReadOnly  ; Playing an animation
int Property STATUS_ENDING	= 4 AutoReadOnly  ; Ending. Data is still available but most functionality is disabled

; Return the current status of the thread. This status divides the threads functionality in sub sections
; Some functionality may depend on current thread status
int Function GetStatus()
EndFunction

; Get the currently running scene
String Function GetActiveScene()
EndFunction
; Get the currently running stage
String Function GetActiveStage()
EndFunction

; Get all scenes available to the current animation
String[] Function GetPlayingScenes()
EndFunction

; Force the argument scene to be played instead of the currently active one
; On success, will delete stage history and sort actors to the new scene
bool Function ResetScene(String asScene)
EndFunction

; Branch or skip from the currently playing stage. Will fail if called outside of playing state
; If the given branch/stage does not exist will end the scene
Function BranchTo(int aiNextBranch)
EndFunction
Function SkipTo(String asNextStage)
EndFunction

; Return a list of all played stages (including the currently playing one)
; This list may include duplicates if the scene looped (e.g. A -> B -> C -> A) and resets when the scene changes
; This creates a copy of the internal history, dont call this repeatedly when you can cache the result
String[] Function GetStageHistory()
EndFunction
; Same as above, but only returns the length of the history
int Function GetStageHistoryLength()
EndFunction

; Stop this threads animation. Will fail if the thread is idling/ending
Function StopAnimation()
EndFunction

; ------------------------------------------------------- ;
; --- Tags		                                        --- ;
; ------------------------------------------------------- ;
;/
	Tags are used to further describe a scene, they have different scopes:
		- ThreadTags combine tags shared by every scene the thread has been initiaited with, Example: if we have 2 scenes: 
				["doggy", "loving", "behind"] and ["doggy", "loving", "hugging", "kissing"], then the thread tags will be ["doggy", "loving"]
		- SceneTags describe the playing scene loosely, for each tag there is at least one stage that uses it
		- StageTags only describe the currently playing stage
/;

; If this thread is tagged with the given argument
bool Function HasTag(String asTag)
EndFunction
; If the active scene is tagged with the given argument
bool Function HasSceneTag(String asTag)
EndFunction
; If the active stage is tagged with the given argument
bool Function HasStageTag(String asTag)
EndFunction

bool Function IsSceneVaginal()
	return HasSceneTag("Vaginal")
EndFunction
bool Function IsSceneAnal()
	return HasSceneTag("Anal")
EndFunction
bool Function IsSceneOral()
	return HasSceneTag("Oral")
EndFunction

; ------------------------------------------------------- ;
; --- Context                                         --- ;
; ------------------------------------------------------- ;
;/
	Context data are thread owned, custom tags used to specify the scenes context
    Custom contexts can be used to indirectly communicate with other mods
/;

; If the thread owns some context
bool Function HasContext(String asTag)
EndFunction

; Add or remove some context to/from the scene
Function AddContext(String asContext)
EndFunction
Function RemoveContext(String asContext)
EndFunction

; If the current animation is assumed to be consent
bool Function IsConsent()
EndFunction
Function SetConsent(bool abIsConsent)
EndFunction

; ------------------------------------------------------- ;
; --- Time Data                                       --- ;
; ------------------------------------------------------- ;
;/
	Time related data
/;

; The timestamp at which the thread has started
; Time is returned as real time seconds since the save has been created
float Function GetTime()
EndFunction
; Returns the threads current total runtime
float Function GetTimeTotal()
EndFunction

; ------------------------------------------------------- ;
; --- Position Info                                   --- ;
; ------------------------------------------------------- ;
;/
	Functions to view and manipulate position related data
/;

; If this actor is pariticpating in the scene
bool Function HasActor(Actor akActor)
EndFunction
bool Function HasPlayer()
EndFunction

; Retrieve all positions in the current scene. Order of actors is unspecified
Actor[] Function GetPositions()
EndFunction

; Retrieve the index of this actors position within the thread
int Function GetPositionIdx(Actor akActor)
EndFunction

; Retrive the sex of this position as used by the thread
int Function GetActorSex(Actor akActor)
EndFunction
int Function GetNthPositionSex(int n)
EndFunction
int[] Function GetPositionSexes()
EndFunction

; --- Submission

; Return if the given actor is a submissive or not
bool Function GetSubmissive(Actor akActor)
EndFunction
Function SetIsSubmissive(Actor akActor, bool abIsSubmissive)
EndFunction
; Get all submissives for the current animation
Actor[] Function GetSubmissives()
EndFunction

; --- Stripping

; Set custom strip settings for this actor
; aiSlots represents a slot mask of all slots that should be unequipped (if possible)
Function SetCustomStrip(Actor akActor, int aiSlots, bool abWeapon, bool abApplyNow)
EndFunction
Function ResetCustomStrip(Actor akActor)
EndFunction
; If the actor will play a short animation on scene start when undressing. Only used before entering playing state
bool Function IsUndressAnimationAllowed(Actor akActor)
EndFunction
Function SetIsUndressAnimationAllowed(Actor akActor, bool abAllowed)
EndFunction
; if the actor will re-equip their gear after the animation (and they are not a submissive)
bool Function IsRedressAllowed(Actor akActor)
EndFunction
Function SetIsRedressAllowed(Actor akActor, bool abAllowed)
EndFunction

; --- Voice

; Update the given actors voice
Function SetActorVoice(Actor akActor, String asVoice, bool abForceSilent)
EndFunction
String Function GetActorVoice(Actor akActor)
EndFunction

; --- Expressions

; Update the given actors expression
Function SetActorExpression(Actor akActor, String asExpression)
EndFunction
String Function GetActorExpression(Actor akActor)
EndFunction

; --- Enjoyment

; Return the current enjoyment/arousal level for this actor
int Function GetEnjoyment(Actor ActorRef)
EndFunction
; Set/Adjust the current enjoyment for this actor to/by a specified value
Function SetEnjoyment(Actor ActorRef, int aiSet)
EndFunction
Function AdjustEnjoyment(Actor ActorRef, int AdjustBy)
EndFunction
; Modify the rate at which enjoyment raises for an actor
; afSet == 2 will double the EnjRaise, while afSet == 0 will stop EnjRaise
Function ModEnjoymentMult(Actor ActorRef, float afSet, bool bAdjust = False)
EndFunction

; --- Orgasms

; Disable or enable orgasm events for the stated actor
Function DisableOrgasm(Actor ActorRef, bool OrgasmDisabled = true)
EndFunction
bool Function IsOrgasmAllowed(Actor ActorRef)
EndFunction
; Create an orgasm event for the given actor
Function ForceOrgasm(Actor ActorRef)
EndFunction

; If the given actor has a chance of impregnation at some point during this scene. That is, the function will check
; if at any point during this scene this actor had vaginal contact with an orgasming male actor, either direct or indirect
; This function only considers stages that have already been played
; --- Arguments
; abAllowFutaImpregnation	- if akActor is a futa, can they still be impregnated?
; abFutaCanPregnate				- if the orgasming actor is a futa, can they impregnate?
; abCreatureCanPregnate		- if the orgasming actor is a creature, can they impregnate?
; --- Return
; All actors that had vaginal intercourse with the given actor
Actor[] Function CanBeImpregnated(Actor akActor, bool abAllowFutaImpregnation, bool abFutaCanPregnate, bool abCreatureCanPregnate)
EndFunction

; --- Strapons

; Set the strapon this actor should use. Will fail if the actor isnt a valid target for strapon usage
Function SetStrapon(Actor ActorRef, Form ToStrapon)
endfunction
Form Function GetStrapon(Actor ActorRef)
endfunction
; if the given actor is currently using a strapon
bool Function IsUsingStrapon(Actor ActorRef)
EndFunction

; --- Pathing

int Property PATHING_DISABLE = -1 AutoReadOnly	; Always be teleported
int Property PATHING_ENABLE = 0 AutoReadOnly		; Let the user config decide (default)
int Property PATHING_FORCE = 1 AutoReadOnly			; Always try to walk unless the distance is too great

; Set the pathing flag of the position, determing if this actor can walk to the center or should be teleported to it
; This can only be set before playing state
Function SetPathingFlag(Actor akActor, int aiPathingFlag)
EndFunction

; ------------------------------------------------------- ;
; --- Interactions Info                               --- ;
; ------------------------------------------------------- ;
;/
	Detailed interpretations of an actor's interactions during scene at any given moment.
	These detections rely upon collision type-guessing and optionally upon position tags as fallback.
/;

int Property bKissing       = 0  AutoReadOnly Hidden	; Position involved in kissing interaction.
int Property aAnimObjFace   = 1  AutoReadOnly Hidden	; Position licks/sucks the sex-toy on partner's crotch/hand.
int Property pAnimObjFace   = 2  AutoReadOnly Hidden	; Position has sex-toy on crotch/hand that partner licks/sucks.
int Property aGrinding      = 3  AutoReadOnly Hidden	; Position's buttocks grinds against partner's crotch.
int Property pGrinding      = 4  AutoReadOnly Hidden	; Position's crotch is being grinded against by partner's buttocks.
int Property aSuckingToes   = 5  AutoReadOnly Hidden	; Position licks/sucks partner's toe/foot.
int Property pSuckingToes   = 6  AutoReadOnly Hidden	; Position's toes/foot is licked/sucked by partner.
int Property aHandJob       = 7  AutoReadOnly Hidden	; Position's hands move around partner's genital.
int Property pHandJob       = 8  AutoReadOnly Hidden	; Position's genital is stimulated by partner's hands.
int Property aFootJob       = 9  AutoReadOnly Hidden	; Position's feet move around partner's genital.
int Property pFootJob       = 10 AutoReadOnly Hidden	; Position's genital is stimulated by partner's feet.
int Property aBoobJob       = 11 AutoReadOnly Hidden	; Position's breasts move around partner's penis.
int Property pBoobJob       = 12 AutoReadOnly Hidden	; Position's penis is stimulated by partner's breasts.
int Property aFacial        = 13 AutoReadOnly Hidden	; Position's climaxing genital is in front of partner's face.
int Property pFacial        = 14 AutoReadOnly Hidden	; Position's face is in front of partner's climaxing genital.
int Property aLickingShaft  = 15 AutoReadOnly Hidden	; Position licks partner's penis/dildo.
int Property pLickingShaft  = 16 AutoReadOnly Hidden	; Position's penis/dildo is licked by partner.
int Property aOral          = 17 AutoReadOnly Hidden	; Position licks/sucks partner's genital.
int Property pOral          = 18 AutoReadOnly Hidden	; Position's genital is licked/sucked by partner.
int Property aDeepthroat    = 19 AutoReadOnly Hidden	; Position deep-throats partner's penis.
int Property pDeepthroat    = 20 AutoReadOnly Hidden	; Position's penis is deep-throated by partner.
int Property aSkullfuck     = 21 AutoReadOnly Hidden	; Position's penis/dildo is penetrating partner's skull (Gore).
int Property pSkullfuck     = 22 AutoReadOnly Hidden	; Position's skull is penetrated by partner's penis/dildo (Gore).
int Property aVaginal       = 23 AutoReadOnly Hidden	; Position's penis/dildo is penetrating partner's vagina.
int Property pVaginal       = 24 AutoReadOnly Hidden	; Position's vagina is penetrated by partner's penis/dildo.
int Property aAnal          = 25 AutoReadOnly Hidden	; Position's penis/dildo is penetrating partner's anus.
int Property pAnal          = 26 AutoReadOnly Hidden	; Position's anus is penetrated by partner's penis/dildo.

int Property SUPPORTED_INTER_COUNT = 27 AutoReadOnly Hidden

; If physics-based collision related data is currently available or not
bool Function IsInteractionRegistered()
EndFunction

; Returns an array of 28 bools listed in order mentioned above for specified actor
; Only those idx-es in the array will be TRUE in which the actor is engaged
; Example Use: You need to detect if a female is engaged in double penetration
;	SexLabThread _Thread = SexLab.GetThreadByActor(akPosition)
;	bool[] interFlags = _Thread.GetCurrentInteractionFlags(akPosition)
;	If (interFlags[_Thread.pVaginal] && interFlags[_Thread.pAnal])
;		// DO_SOMETHING
bool[] Function GetInteractionFlags(Actor akPosition)
EndFunction

; Checks if the given InterType is detected for the specified actor.
bool Function HasActiveInteraction(Actor akPosition, int InterType)
EndFunction
bool Function HasActiveInteractionAll(Actor akPosition, int[] InterTypes)
EndFunction
bool Function HasActiveInteractionAny(Actor akPosition, int[] InterTypes)
EndFunction

; Return the first or all partners for whom akPosition has the given InterType
Actor Function GetPartnerByInteractionType(Actor akPosition, int InterTypes)
EndFunction
Actor[] Function GetPartnersByInteractionType(Actor akPosition, int InterTypes)
EndFunction

; Velocity may be positive or negative, depending on the direction of movement
float Function GetInteractionVelocity(Actor akPosition, Actor akPartner, int InterTypes)
EndFunction

; ------------------------------------------------------- ;
; --- VRIK Configs + Init                             --- ;
; ------------------------------------------------------- ;

; Function to temporarily dictate SL's VRIK configs for the duration of the active scene.
; Example Use: restrain hands to anim: SexLabUtil.SetConfigsVRIK(true, true, aiTrackHands=0)
Function SetConfigsVRIK(bool abEnabled=true, bool abOverrideConfig=false, int aiPOVMode=-1, \
	int abLockHeight=-1, float afHeightAdjSpeed=-1.0, int abTrackHead=-1, int aiTrackHands=-1, \
	float afDistHideHead=-1.0, float afDistNearClip=-1.0, int aiLockHmdToBody=-1, \
	float afLockHmdDistance=-1.0, float afLockHmdTolerance=-1.0, float afLockHmdSpeed=-1.0)
EndFunction