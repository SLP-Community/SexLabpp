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
; --- Interaction		                                  --- ;
; ------------------------------------------------------- ;
;/
	Lookup interaction data
	This data is based on the actors 3D, its reliably thus heavily depends on 
	how well aligned the animation is
/;

int Property CTYPE_ANY					= -1 	AutoReadOnly
int Property CTYPE_Vaginal 			= 1 	AutoReadOnly	; Position is being penetrated by partner
int Property CTYPE_Anal 				= 2 	AutoReadOnly	; Position is being penetrated by partner
int Property CTYPE_Oral 				= 3 	AutoReadOnly	; Position is licking/sucking partner
int Property CTYPE_Grinding 		= 4 	AutoReadOnly	; Position is being grinded against by partner (crotch area)
int Property CTYPE_Deepthroat 	= 5 	AutoReadOnly	; Implies Oral, partner's penis close to/at maximum depth
int Property CTYPE_Skullfuck 		= 6 	AutoReadOnly	; Positions head penetrated in an unexpected way by partner (Usually gore)
int Property CTYPE_LickingShaft = 7 	AutoReadOnly	; Position licking partners shaft
int Property CTYPE_FootJob 			= 8 	AutoReadOnly	; Position pleasuring partner using at least one foot
int Property CTYPE_HandJob 			= 9 	AutoReadOnly	; Position pleasuring partner using at least one hand
int Property CTYPE_Kissing 			= 10 	AutoReadOnly	; Position kissing partner
int Property CTYPE_Facial 			= 11 	AutoReadOnly	; Positions face in front of partner penis
int Property CTYPE_AnimObjFace 	= 12 	AutoReadOnly	; Position mouth close to partner anim object node
int Property CTYPE_SuckingToes	= 13	AutoReadOnly	; Position mouth close to partner toes

; If collision related data is currently available or not
bool Function IsInteractionRegistered()
EndFunction

; Get a list of all types the two actors interact with another
; If akPartner is none, returns all interactions with any partner
; This function is NOT commutative, see type description for interaction direction
int[] Function GetInteractionTypes(Actor akPosition, Actor akPartner)
EndFunction

; If akPosition interacts with akPartner under a given type
; If akPartner is none, checks against any available partner
; If akPosition is none, iterates over all possible positions
; If both are none, returns if the given type is present among any positions
bool Function HasInteractionType(int aiType, Actor akPosition, Actor akPartner)
EndFunction

; Return the first actor that interacts with akPosition by the given type
; The array versions may be NONE, in which case all actors involved in the given type are returned
; in this case it holds that GetPartnersByType() == GetPartnersByTypeRev()
; (respecting interaction direction as stated by type)
; (Returned value will be a subset of all positions in the scene)
Actor Function GetPartnerByType(Actor akPosition, int aiType)
EndFunction
Actor[] Function GetPartnersByType(Actor akPosition, int aiType)
EndFunction
; Same as above but gathers the data in reverse, e.g.
; GetPartnersByType(Act, ORAL) returns all actors Act is receiving Oral from
; GetPartnersByTypeRev(Act, ORAL) returns all actors Act is giving Oral to
Actor Function GetPartnerByTypeRev(Actor akPartner, int aiType)
EndFunction
Actor[] Function GetPartnersByTypeRev(Actor akPartner, int aiType)
EndFunction

; Returns a string containing detected CTYPEs, separated by ","
; String CTYPEs can start with various prefixes, such as
; a: active (akPosition is giving/doing CTYPE), e.g. aAnimObjFace
; p: passive (akPosition is receiving/taking CTYPE) e.g. pHandJob
string Function GetInteractionString(Actor akPosition)
EndFunction

; Return the velocity of the specified interaction type
; Velocity may be positive or negative, depending on the direction of movement
float Function GetVelocity(Actor akPosition, Actor akPartner, int aiType)
EndFunction

; ------------------------------------------------------- ;
; --- Time Data			                                  --- ;
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
