ScriptName SexLabFramework extends Quest

; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;
; ----------------------------------------------------------------------------------------------------------------------------------------- ;
;                                                                                                                                           ;
;     ███████╗███████╗██╗  ██╗██╗      █████╗ ██████╗     ███████╗██████╗  █████╗ ███╗   ███╗███████╗██╗    ██╗ ██████╗ ██████╗ ██╗  ██╗    ;
;     ██╔════╝██╔════╝╚██╗██╔╝██║     ██╔══██╗██╔══██╗    ██╔════╝██╔══██╗██╔══██╗████╗ ████║██╔════╝██║    ██║██╔═══██╗██╔══██╗██║ ██╔╝    ;
;     ███████╗█████╗   ╚███╔╝ ██║     ███████║██████╔╝    █████╗  ██████╔╝███████║██╔████╔██║█████╗  ██║ █╗ ██║██║   ██║██████╔╝█████╔╝     ;
;     ╚════██║██╔══╝   ██╔██╗ ██║     ██╔══██║██╔══██╗    ██╔══╝  ██╔══██╗██╔══██║██║╚██╔╝██║██╔══╝  ██║███╗██║██║   ██║██╔══██╗██╔═██╗     ;
;     ███████║███████╗██╔╝ ██╗███████╗██║  ██║██████╔╝    ██║     ██║  ██║██║  ██║██║ ╚═╝ ██║███████╗╚███╔███╔╝╚██████╔╝██║  ██║██║  ██╗    ;
;     ╚══════╝╚══════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚═════╝     ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝ ╚══╝╚══╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝    ;
;                                                                                                                                           ;
; ----------------------------------------------------------------------------------------------------------------------------------------- ;
; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;
; ----------------------------------------------------------------------------------------------------------------------------------------- ;
;                                  Created by Ashal@LoversLab.com [http://www.loverslab.com/user/1-ashal/]                                  ;
; ----------------------------------------------------------------------------------------------------------------------------------------- ;
;                                   SexLab P+ maintained by Scrab [https://www.patreon.com/ScrabJoseline]                                   ;
; ----------------------------------------------------------------------------------------------------------------------------------------- ;
; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;

; Integer ID of the current SexLab version
int function GetVersion()
  return SexLabUtil.GetVersion()
endFunction

; A user friendly string representing the current SexLab version
string function GetStringVer()
  return SexLabUtil.GetStringVer()
endFunction

; Is SexLab is currently enabled and able to start a new scene?
bool property Enabled hidden
  bool function get()
    return GetState() != "Disabled"
  endFunction
endProperty

; Is there any SexLab thread currently active and animating?
bool property IsRunning hidden
  bool function get()
    return ThreadSlots.IsRunning()
  endFunction
endProperty

; The number of active/running scenes
int property ActiveAnimations hidden
  int function get()
    return ThreadSlots.ActiveThreads()
  endFunction
endProperty

; If creatures are currently enabled
bool property AllowCreatures hidden
  bool function get()
    return Config.AllowCreatures
  endFunction
endProperty

; If creatures genders are currently enabled
bool property CreatureGenders hidden
  bool function get()
    return Config.UseCreatureGender
  endFunction
endProperty

;#------------------------------------------------------------------------------------------------------------------------------------------#;
;#                                                                                                                                          #;
;#                                                            MAIN API FUNCTIONS                                                            #;
;#                                                                                                                                          #;
;#------------------------------------------------------------------------------------------------------------------------------------------#;

; The preferred way to create a SexLab Scene
; --- Params:
; akPositions:    The actors to animate
; asTags:         Requested animation tags (may be empty). Supported prefixes: '-' to disable a tag, '~' for OR-conjunctions
;                 Example: "~A, B, ~C, -D" <=> Animation has tag B, does NOT have tag D and has EITHER tag A or C 
; akSubmissive:   Must be one of the participating actors. If specified, the given actor is considered submissive for the context of the animation
; akCenter:       If specified, SexLab will try to place all actors near or on the given reference
; aiFurniture:    Furniture preference. Must be one of the following: 0 - Disable; 1 - Allow; 2 - Prefer
; asHook:         A callback string to receive callback events. See 'Hooks' section below for details
; --- Return:
; SexLabThread:   An API instance to interact with the started scene. See SexLabThread.psc for more info
; None:           If an error occured
SexLabThread Function StartScene(Actor[] akPositions, String asTags, Actor akSubmissive = none, ObjectReference akCenter = none, int aiFurniture = 1, String asHook = "")
  return StartSceneA(akPositions, asTags, MakeActorArray(akSubmissive), akCenter, aiFurniture, asHook)
EndFunction
SexLabThread Function StartSceneA(Actor[] akPositions, String asTags, Actor[] akSubmissives, ObjectReference akCenter = none, int aiFurniture = 1, String asHook = "")
  String[] scenes = SexLabRegistry.LookupScenesA(akPositions, asTags, akSubmissives, aiFurniture, akCenter)
  If (!scenes.Length)
    Log("StartScene() - Failed to find valid animations")
    return none
  EndIf
  return StartSceneImpl(akPositions, scenes, asTags, akSubmissives, akCenter, aiFurniture, asHook)
EndFunction

; Start a scene with pre-defined animations
SexLabThread Function StartSceneEx(Actor[] akPositions, String[] asScenes, Actor akSubmissive = none, String asContext = "", \
    ObjectReference akCenter = none, int aiFurniture = 1, String asHook = "")
  return StartSceneExA(akPositions, asScenes, MakeActorArray(akSubmissive), asContext, akCenter, aiFurniture, asHook)
EndFunction
SexLabThread Function StartSceneExA(Actor[] akPositions, String[] asScenes, Actor[] akSubmissives, String asContext = "", \
    ObjectReference akCenter = none, int aiFurniture = 1, String asHook = "")
  return StartSceneImpl(akPositions, asScenes, asContext, akSubmissives, akCenter, aiFurniture, asHook)
EndFunction

; Wrapper function for StartScene which takes Actors one-by-one instead of an array
SexLabThread Function StartSceneQuick(Actor akActor1, Actor akActor2 = none, Actor akActor3 = none, Actor akActor4 = none, Actor akActor5 = none, \
                                        Actor akSubmissive = none, String asTags = "", String asHook = "")
  Actor[] Positions = SexLabUtil.MakeActorArray(akActor1, akActor2, akActor3, akActor4, akActor5)
  return StartScene(Positions, asTags, akSubmissive, asHook = asHook)
EndFunction

;#-----------------------------------------------------------------------------------------------------------------------------------------#
;#                                                                                                                                         #
;#                                                            THREAD FUNCTIONS                                                             #
;#                                                                                                                                         #
;#-----------------------------------------------------------------------------------------------------------------------------------------#

; Get the thread API associated with the given thread id
SexLabThread Function GetThread(int aiThreadID)
  return ThreadSlots.GetThread(aiThreadID)
EndFunction

; Get the thread API representing the thread that most recently animated this actor
SexLabThread Function GetThreadByActor(Actor akActor)
  return ThreadSlots.GetThreadByActor(akActor)
EndFunction

;#------------------------------------------------------------------------------------------------------------------------------------------#;
;#                                                                                                                                          #;
;#                                                              HOOK FUNCTIONS                                                              #;
;#                                                                                                                                          #;
;#------------------------------------------------------------------------------------------------------------------------------------------#;
;#------------------------------------------------------------------------------------------------------------------------------------------#;
;#                                                                                                                                          #;
;#  ABOUT HOOKS IN SEXLAB                                                                                                                   #;
;# Hooks are used to react and interact with running threads in SexLab utilizing events invoked while the thread executes                   #;
;#                                                                                                                                          #;
;# SexLab differentiates two types of Hooks: Blocking and non-blocking                                                                      #;
;# Non-Blocking Hooks are the more common implementation, these are to asynchronously react to the flow of an animation, for example        #;
;#   to advance your story once the animation is over, or to react to other peoples animations starting                                     #;
;# Blocking hooks on the other hand are synchronized and will *halt* a threads execution until the Hook returns. As such, they are          #;
;#   more invasive to the players gameplay experience and should thus be used sparringly                                                    #;
;#                                                                                                                                          #;
;#                                                                                                                                          #;   
;#  HOW TO USE HOOKS                                                                                                                        #;
;# 1. Non-Blocking Hooks                                                                                                                    #;
;# Non-Blocking hooks use mod events to do their work. This is how they are async and why they are very easy to maintain                    #;
;#   There are a variety of different hooks you can use here, and all follow the same schema:                                               #;
;# - First, you want to register for a mod event, like so:                                                                                  #;
;#        RegisterForModEvent("Hook<ModEventType>", "<EventName>")                                                                          #;
;#   Change "ModEventType" to one of the types listed below (e.g. AnimationStart) and the event name to anything you want (e.g. MyEvent)    #;
;# - Next, elsewhere in your script you want to add a new event function, using the following signature:                                    #;
;#        Event <EventName>(int aiThreadID, bool abHasPlayer)                                                                               #;
;#        EndEvent                                                                                                                          #;
;#   Change <EventName> to the same name that you used above, in our example it would be "Event MyEvent(int aiThreadID, bool abHasPlayer)"  #;
;#   And thats all! Now every time an animation starts you will receive the event you have registered for                                   #;
;#                                                                                                                                          #;
;# 1.1 Local Hooks                                                                                                                          #;
;# Sometimes you do not want to react to *every* event that is being send, but only to some events for scenes you started yourself          #;
;#   This is where the "asHook" parameter comes into play that you can set when requested a scene through this API. This parameter is used  #;
;#   to create specialized thread-local hooks that are only send from thread which know about this hook                                     #;
;# These Local Hooks function basically the same as the global ones, the ID of Event that is being send undergoes a slight change however:  #;
;#   We already discussed how the ID for a global hook is `Hook<ModEventType>`, for our local hook we use a similar approach but append     #;
;#   a special suffix to the ID to signify that we only care about a specific hook: `_<MyLocalHookID>`. For example, if we want to hook     #;
;#   "AnimationStart" with a Local Hook named "MyLocalHook", our signature will be `HookAnimationStart_MyLocalHook` and to let the started  #;
;#   thread know about the hook id, we pass "MyLocalHook" into the "asHook" parameter!                                                      #;
;#                                                                                                                                          #;
;# 1.2 Types of Events                                                                                                                      #;
;#  AnimationStart    - Send when the animation starts                                                                                      #;
;#  AnimationEnd      - Send when the animation is fully terminated                                                                         #;
;#  LeadInStart       - Send when the animation starts and has a LeadIn                                                                     #;
;#  LeadInEnd         - Send when a LeadIn animation ends                                                                                   #;
;#  StageStart        - Send for every Animation Stage that starts                                                                          #;
;#  StageEnd          - Send for every Animation Stage that is completed                                                                    #;
;#  OrgasmStart       - Send when an actor reaches the final stage                                                                          #;
;#  OrgasmEnd         - Send when the final stage is completed                                                                              #;
;#  AnimationChange   - Send if the Animation that was playing is changed by the HotKey                                                     #;
;#  PositionChange    - Send if the Positions of the animation (the involved actors) are changed                                            #;
;#  ActorsRelocated   - Send if the actors gets a new alignment                                                                             #;
;#  ActorChangeStart  - Send when the function ChangeActors is called                                                                       #;
;#  ActorChangeEnd    - Send when the replacement of actors, by the function ChangeActors is completed                                      #;
;#                                                                                                                                          #;
;#                                                                                                                                          #;
;# 2. Blocking Hooks                                                                                                                        #;
;# Another, more complex type of Hook are Blocking Hooks. These should generally be avoided as they halt the threads execution but there    #;
;#   are situations were time is of essence and asynchronous hooks simply don't cut it.                                                     #;
;# To implement a blocking hook, you first want to create a new reference alias and attach a script to it. The script can have any name     #;
;#   you want, important is that it extends "SexLabThreadHook" instead of "ReferenceAlias" and fill in the properties. This is all you      #;
;#   need to do to implement a blocking hook. To read on how to actually use these Hooks, see "SexLabThreadHook.psc"                        #;
;#                                                                                                                                          #;
;#------------------------------------------------------------------------------------------------------------------------------------------#;

; Register a new blocking hook to receive events from running threads
; Return if the hook has been successfully installed
bool Function RegisterHook(SexLabThreadHook akHook)
  return Config.AddHook(akHook)
EndFunction

; Unregister a hook to no longer receive events
; Return if the hook was unregistered successfully
bool Function UnregisterHook(SexLabThreadHook akHook)
  return Config.RemoveHook(akHook)
EndFunction

; Check if the given hook is currently registered
bool Function IsHooked(SexLabThreadHook akHook)
  return Config.IsHooked(akHook)
EndFunction

;#-----------------------------------------------------------------------------------------------------------------------------------------#
;#                                                                                                                                         #
;#                                                             ACTOR FUNCTIONS                                                             #
;#                  These functions are used to handle and get info on the actors that will participate in the animations.                 #
;#                                                                                                                                         #
;#-----------------------------------------------------------------------------------------------------------------------------------------#

; If this actor is currently being animated by a SexLab Thread
bool function IsActorActive(Actor ActorRef)
  return ActorRef.IsInFaction(AnimatingFaction)
endFunction

; Return this actors sex
; Mapping: Male = 0 | Female = 1 | Futa = 2 | CrtMale = 3 | CrtFemale = 4
int Function GetSex(Actor akActor)
  return SexlabRegistry.GetSex(akActor, false)
EndFunction
int[] Function GetSexAll(Actor[] akPositions)
  return sslActorLibrary.GetSexAll(akPositions)
EndFunction

; Force an actor to be considered male, female or futa by SexLab
; --- Param:
; akActor:    The actor which's sex to overwrite/force
; aiSexTag:   The actors new sex; 0 - Male, 1 - Female, 2 - Futa
Function TreatAsSex(Actor akActor, int aiSexTag)
  ActorLib.TreatAsSex(akActor, aiSexTag)
EndFunction
Function TreatAsMale(Actor ActorRef)
  TreatAsSex(ActorRef, 0)
EndFunction
Function TreatAsFemale(Actor ActorRef)
  TreatAsSex(ActorRef, 1)
EndFunction
Function TreatAsFuta(Actor ActorRef)
  TreatAsSex(ActorRef, 2)
EndFunction

; Clear a forced sex assignment previously established with "TreatAsSex"
Function ClearForcedSex(Actor akActor)
  ActorLib.ClearForcedSex(akActor)
EndFunction

; Given an array of actors, create an array of length 5 representing the number of individual sexes contained in that array,
; The returned array lists the number of males/females/... inside the array at their respective index; s.t.
;   [0] represents the number of human males
;   [1] represents the number of human females
;   [2] represents the number of human futas
;   [3] represents the number of creature males
;   [4] represents the number of creature females
int[] Function CountSexAll(Actor[] akPositions)
  return sslActorLibrary.CountSexAll(akPositions)
EndFunction
int Function CountMale(Actor[] akPositions)
	return sslActorLibrary.CountMale(akPositions)
EndFunction
int Function CountFemale(Actor[] akPositions)
	return sslActorLibrary.CountFemale(akPositions)
EndFunction
int Function CountFuta(Actor[] akPositions)
	return sslActorLibrary.CountFuta(akPositions)
EndFunction
int Function CountCreatures(Actor[] akPositions)
	return sslActorLibrary.CountCreatures(akPositions)
EndFunction
int Function CountCrtMale(Actor[] akPositions)
	return sslActorLibrary.CountCrtMale(akPositions)
EndFunction
int Function CountCrtFemale(Actor[] akPositions)
	return sslActorLibrary.CountCrtFemale(akPositions)
EndFunction

; Checks if the given actor is a valid target for SexLab animations.
; --- Return: 
;     1 if the actor is valid, otherwise...
;    -1 = The Actor does not exists (it is None)
;    -2 = The Actor is from a disabled race
;   -10 = The Actor is already part of a SexLab animation
;   -11 = The Actor is forbidden form SexLab animations
;   -12 = The Actor does not have the 3D loaded
;   -13 = The Actor is dead (He's dead Jim.)
;   -14 = The Actor is disabled (Model or AI)
;   -15 = The Actor is flying (so it cannot be SexLab animated)
;   -16 = The Actor is on mount (so it cannot be SexLab animated)
;   -17 = The Actor is a creature but creature animations are disabled
;   -18 = The Actor is a creature that is not supported by SexLab
int function ValidateActor(Actor ActorRef)
  return ActorLib.ValidateActor(ActorRef)
endFunction
bool function IsValidActor(Actor ActorRef)
  return ActorLib.ValidateActor(ActorRef) == 1
endFunction

;/* ForbidActor
* * Makes an actor to be never allowed to engage in SexLab Animations.
* * @param: ActorRef, the actor to forbid from SexLab use.
*/;
function ForbidActor(Actor ActorRef)
  ActorLib.ForbidActor(ActorRef)
endFunction

;/* AllowActor
* * Removes an actor from the forbidden list, undoing the effects of ForbidActor()
* * 
* * @param: ActorRef, the actor to remove from the forbid list.
*/;
function AllowActor(Actor ActorRef)
  ActorLib.AllowActor(ActorRef)
endFunction

;/* IsForbidden
* * Checks if an actor is currently forbidden from use in SexLab scenes.
* * 
* * @param: ActorRef, the actor to check.
* * @return: True if the actor is forbidden from use.
*/;
bool function IsForbidden(Actor ActorRef)
  return ActorLib.IsForbidden(ActorRef)
endFunction

; ---------------------------------------------------
; * CUM FX FUNCTIONS
; ---------------------------------------------------
; NOTE: Various functions here include an `int aiType` argument, valid types can be viewed here:
; https://github.com/Scrabx3/SexLabpp/blob/master/src/Registry/CumFx.h (FxType enum) or sslActorLibrary.psc (in case of mismatch, CumFx.h takes precedence)
; Additionally, setting aiType to -1 will apply to all types.

; Apply cum effects to the given actor for the specified interaction type
Function AddCumFx(Actor akActor, int aiType)
  ActorLib.AddCumFx(akActor, aiType)
EndFunction
Function AddCumFxLayers(Actor akActor, int aiType, int aiLayers)
  int i = 0
  While (i < aiLayers)
    AddCumFx(akActor, aiType)
    i += 1
  EndWhile
EndFunction

; Remove cum effects from the given actor for the specified interaction type
Function RemoveCumFx(Actor akActor, int aiType = -1)
  ActorLib.RemoveCumFx(akActor, aiType)
EndFunction

; Count the layers of applied cum fx on the actor
int Function CountCumFx(Actor ActorRef, int aiType = -1)
  return ActorLib.CountCumFx(ActorRef, aiType)
EndFunction
int Function CountCumVaginal(Actor ActorRef)
  return ActorLib.CountCumFx(ActorRef, ActorLib.FX_Vaginal)
EndFunction
int Function CountCumOral(Actor ActorRef)
  return ActorLib.CountCumFx(ActorRef, ActorLib.FX_ORAL)
EndFunction
int Function CountCumAnal(Actor ActorRef)
  return ActorLib.CountCumFx(ActorRef, ActorLib.FX_ANAL)
EndFunction

;/* StripActor
* * Strips an actor using SexLab's strip settings as chosen by the user from the SexLab MCM
* * 
* * @param: Actor ActorRef - The actor whose equipment shall be unequipped.
* * @param: Actor VictimRef [OPTIONAL] - If ActorRef matches VictimRef victim strip settings are used. If VictimRef is set but doesn't match, aggressor settings are used.
* * @param: bool DoAnimate [OPTIONAL true by default] - Whether or not to play the actor stripping animations during the strip
* * @param: bool LeadIn [OPTIONAL false by default] - If TRUE and VictimRef == none, Foreplay strip settings will be used.
* * @return: Form[] - An array of all equipment stripped from ActorRef
*/;
Form[] function StripActor(Actor ActorRef, Actor VictimRef = none, bool DoAnimate = true, bool LeadIn = false)
  return ActorLib.StripActor(ActorRef, VictimRef, DoAnimate, LeadIn)
endFunction

;/* StripSlots
* * Strips an actor of equipment using a custom selection of biped objects / slot masks.
* * See for the slot values: http://www.creationkit.com/Biped_Object
* * 
* * @param: Actor ActorRef - The actor whose equipment shall be unequipped.
* * @param: bool[] Strip - MUST be a bool array with a length of exactly 33 items. Any index set to TRUE will be stripped using nth + 30 = biped object / slot mask. The extra index Strip[32] is used to strip weapons
* * @param: bool DoAnimate - Whether or not to play the actor stripping animation during
* * @param: bool AllowNudesuit - Whether to allow the use of nudesuits, if the user has that option enabled in the MCM (the poor fool)
* * @return: Form[] - An array of all equipment stripped from ActorRef
*/;
Form[] function StripSlots(Actor ActorRef, bool[] Strip, bool DoAnimate = false, bool AllowNudesuit = true)
  return ActorLib.StripSlots(ActorRef, Strip, DoAnimate, AllowNudesuit)
endFunction

;/* UnstripActor
* * Equips an actor with the given equipment. Intended for reversing the results of the Strip functions using their return results.
* * 
* * @param: Actor ActorRef - The actor whose equipment shall be re-equipped.
* * @param: Form[] Stripped - A form array of all the equipment to be equipped on ActorRef. Typically the saved result of StripActor() or StripSlots()
* * @param: bool IsVictim - If TRUE and the user has the SexLab MCM option for Victims Redress disabled, the actor will not actually re-equip their gear.
*/;
function UnstripActor(Actor ActorRef, Form[] Stripped, bool IsVictim = false)
  ActorLib.UnstripActor(ActorRef, Stripped, IsVictim)
endFunction

;/* IsStrippable
* * Checks if a given item can be unequipped from actors by the SexLab strip functions.
* * 
* * @param: Form ItemRef - The item you want to check.
* * @return: bool - TRUE if the item does not have the keyword with the word "NoStrip" in it, or is flagged as "Always Strip" in the SexLab MCM Strip Editor.
*/;
bool function IsStrippable(Form ItemRef)
  return ActorLib.IsStrippable(ItemRef)
endFunction

;/* StripSlot
* * Removes and unequip an item from an actor that is in the position defined by the given slot mask.
* * The item is unequipped only if it is considered strippable by SexLab.
* * 
* * @param: Actor ActorRef - The actor to unequip the slot from
* * @param: int SlotMask - The slot mask id for your chosen biped object. See more: http://www.creationkit.com/Slot_Masks_-_Armor
* * @return: Form - The item equipped on the SlotMask if removed. None if it was not removed or nothing was there.
*/;
Form function StripSlot(Actor ActorRef, int SlotMask)
  return ActorLib.StripSlot(ActorRef, SlotMask)
endFunction

;/* WornStrapon
* * Checks and returns for any strapon that equipped by the actor and is considered as a registered strapon by SexLab. (Check LoadStrapon() to find how to add new strapons to SexLab)
* * 
* * @param: Actor ActorRef - The actor to look for a strapon on.
* * @return: Form - The SexLab registered strapon actor is currently wearing, if any.
*/;
Form function WornStrapon(Actor ActorRef)
  return Config.WornStrapon(ActorRef)
endFunction

;/* HasStrapon
* * Checks if the actor is wearing, or has in its inventory, any of the registered SexLab strapons.
* * 
* * @param: Actor ActorRef - The actor to look for a strapon on.
* * @return: bool - TRUE if the actor has a SexLab registered strapon equipped or in their inventory.
*/;
bool function HasStrapon(Actor ActorRef)
  return Config.HasStrapon(ActorRef)
endFunction

;/* PickStrapon
* * Picks a strapon from the SexLab registered strapons for the actor to use.
* * 
* * @param: Actor ActorRef - The actor to look for a strapon to use.
* * @return: Form - A randomly selected strapon or the strapon the actor already has in inventory, if any.
*/;
Form function PickStrapon(Actor ActorRef)
  return Config.PickStrapon(ActorRef)
endFunction

; Add an armor object to the list of available strapons
; --- Parameters:
; esp:    The .esp/.esm file containing the object to search for
; id:     The objects form id
; --- Return:
; Armor:  The object that has been added to the list
; None:   If there is no form with the given esp under the given form id
Armor function LoadStrapon(string esp, int id)
  return Config.LoadStrapon(esp, id)
EndFunction
Function LoadStraponEx(Armor akStrapon)
  Config.LoadStraponEx(akStrapon)
EndFunction

;#-----------------------------------------------------------------------------------------------------------------------------------------#
;#                                                                                                                                         #
;#                                                       BEGIN BED FUNCTIONS                                                            #
;#                                                                                                                                         #
;#-----------------------------------------------------------------------------------------------------------------------------------------#

;/* FindBed
* * Searches for a bed within a given radius from a provided center, and returns its ObjectReference.
* * 
* * @param: ObjectReference CenterRef - An object/actor/marker to use as the center point of your search.
* * @param: float Radius - The radius distance to search within the given CenterRef for a bed. 
* * @param: bool IgnoreUsed - When searching for beds, attempt to check if any actor is currently using the bed, in this case the bed will be ignored. 
* * @param: ObjectReference IgnoreRef1/IgnoreRef2 - A bed object that might be within the search radius, but you know you don't want.
* * @return: ObjectReference - The found valid bed within the radius. NONE if no bed found. 
*/;
ObjectReference function FindBed(ObjectReference CenterRef, float Radius = 1000.0, bool IgnoreUsed = true, ObjectReference IgnoreRef1 = none, ObjectReference IgnoreRef2 = none)
  return ThreadLib.FindBed(CenterRef, Radius, IgnoreUsed, IgnoreRef1, IgnoreRef2)
endFunction

;/* IsBedRoll
* * Checks if a given bed is considered a bed roll.
* * 
* * @param: ObjectReference BedRef - The bed object you want to check.
* * @return: bool - TRUE if BedRef is considered a bed roll.
*/;
bool function IsBedRoll(ObjectReference BedRef)
  return ThreadLib.IsBedRoll(BedRef)
endFunction

;/* IsDoubleBed
* * Checks if a given bed is considered a 2 person bed.
* * 
* * @param: ObjectReference BedRef - The bed object you want to check.
* * @return: bool - TRUE if BedRef is considered a 2 person bed.
*/;
bool function IsDoubleBed(ObjectReference BedRef)
  return ThreadLib.IsDoubleBed(BedRef)
endFunction

;/* IsSingleBed
* * Checks if a given bed is considered a single bed.
* * 
* * @param: ObjectReference BedRef - The bed object you want to check.
* * @return: bool - TRUE if BedRef is considered a single bed.
*/;
bool function IsSingleBed(ObjectReference BedRef)
  return ThreadLib.IsSingleBed(BedRef)
endFunction

;/* IsBedAvailable
* * Checks if a given bed is appears to be in use by another actor.
* * 
* * @param: ObjectReference BedRef - The bed object you want to check.
* * @return: bool - TRUE if BedRef is not being used, FALSE if a NPC is sleeping on it or is used by another SexLab thread.
*/;
bool function IsBedAvailable(ObjectReference BedRef)
  return ThreadLib.IsBedAvailable(BedRef)
endFunction

;/* AddCustomBed
* * Adds a new bed to the list of beds SexLab will search for when starting an animation.
* * 
* * @param: Form BaseBed - The base object of the bed you wish to add.
* * @param: int BedType - Defines what kind of bed it is. 0 = normal bed, 1 = bedroll, 2 = double bed.
* * @return: bool - TRUE if bed was successfully added to the bed list. 
*/;
bool function AddCustomBed(Form BaseBed, int BedType = 0)
  return Config.AddCustomBed(BaseBed, BedType)
endFunction

;/* SetCustomBedOffset
* * Override the default bed offsets used by SexLab [30, 0, 37, 0] for a given base bed object during animation.
* * 
* * @param: Form BaseBed - The base object of the bed you wish to add custom offsets.
* * @param: float Forward - The amount the actor(s) should be pushed forward on the bed when playing an animation.
* * @param: float Sideward - The amount the actor(s) should be pushed sideward on the bed when playing an animation.
* * @param: float Upward - The amount the actor(s) should be pushed upward on the bed when playing an animation. (NOTE: Ignored for bedrolls)
* * @param: float Rotation - The amount the actor(s) should be rotated on the bed when playing an animation.
* * @return: bool - TRUE if BedRef if the bed succesfully had it's default offsets overriden.
*/;
bool function SetCustomBedOffset(Form BaseBed, float Forward = 30.0, float Sideward = 0.0, float Upward = 37.0, float Rotation = 0.0)
  return Config.SetCustomBedOffset(BaseBed, Forward, Sideward, Upward, Rotation)
endFunction

;/* ClearCustomBedOffset
* * Removes any bed offset overrides set by the SetCustomBedOffset() function. Reverting it's offsets to the SexLab default.
* * 
* * @param: Form BaseBed - The base object of the bed you wish to remove custom offsets from.
* * @return: bool - TRUE if BedRef if the bed succesfully had it's default offsets restored. FALSE if it didn't have any to begin with.
*/;
bool function ClearCustomBedOffset(Form BaseBed)
  return Config.ClearCustomBedOffset(BaseBed)
endFunction

;/* GetBedOffsets
* * Get an array of offsets that would be used by the given bed. 
 the 
* * @param: ObjectReference BedRef - The bed object you want to get offsets for.
* * @return: float[] - The array of offsets organized as [Forward, Sideward, Upward, Rotation]. If no customs defined, the default is returned.
*/;
float[] function GetBedOffsets(Form BaseBed)
  return Config.GetBedOffsets(BaseBed)
endFunction

;#-----------------------------------------------------------------------------------------------------------------------------------------#
;#                                                                                                                                         #
;#                                                        BEGIN TRACKING FUNCTIONS                                                         #
;#                                                                                                                                         #
;#-----------------------------------------------------------------------------------------------------------------------------------------#


;#-----------------------------------------------------------------------------------------------------------------------------------------#
;#                                                                                                                                         #
;#  TRACKING USAGE INSTRUCTIONS                                                                                                            #
;#                                                                                                                                         #
;# An actor is tracked either by specifically it being marked for tracking, or because it belongs to a faction that is tracked.            #
;# Tracked actors will receive special mod events.                                                                                         #
;# NOTE: The player has a default tracked event associated with them using the callback "PlayerTrack"                                      #
;#                                                                                                                                         #
;# The default tracked event types are: Added, Start, End, Orgasm.                                                                         #
;# Which correspond with an actor being added to a thread, starting an animation, ending an animation, and having an orgasm.               #
;#                                                                                                                                         #
;# Once you register a callback for an actor or faction, the mod event that is sent will be "<custom callback>_<event type>".              #
;#                                                                                                                                         #
;# Example:                                                                                                                                #
;# If you want to run some code, whenever a specific Actor finishes a SexLab animation, then you can do something like this:               #
;#                                                                                                                                         #
;#  Actor myActor = ...                              <-- you get your actor in any way you want                                            #
;#  SexLab.TrackActor(ActorRef, "MyHook")            <-- here you start to track the actor, and the hook that will be used is MyHook       #
;#  RegisterForModEvent("MyHook_End", "DoSomething")                                                                                        #
;#                                                                                                                                         #
;#  Event DoSomething(Form FormRef, int tid)                                                                                               #
;#    Debug.MessageBox("The Actor " + myActor.getDisplayname() just ended an animation.")                                                  #
;#  EndEvent                                                                                                                               #
;#                                                                                                                                         #
;# In the received event, the first parameter FormRef will be the Actor (you may want to cast it),                                         #
;# and the second parameter tid is the ID of the Tread Controller                                                                          #
;#                                                                                                                                         #
;# For an advanced description of the events management look into the HOOKS section.                                                       #
;#                                                                                                                                         #
;#                                                                                                                                         #
;# NOTE: In the following functions the parameter Callback is NOT a function, is a part of the name of the event that is generated.        #
;#                                                                                                                                         #
;#                                                                                                                                         #
;#-----------------------------------------------------------------------------------------------------------------------------------------#

;/* TrackActor
* * Associates a specific actor with a unique callback mod event that is sent whenever the actor performs certain actions within SexLab animations.
* * You need to RegisterForModEvents for an event with name <Callback>_<Event>, where events can be:
* * "Added" - The actor is added to a SexLab animation
* * "Start" - The SexLab animations where the actor was added is starting
* * "Orgasm" - The actor is having an orgasm
* * "End" - The SexLab animations where the actor was added is ended
* * 
* * @param: Actor ActorRef - The actor you want to receive tracked events for.
* * @param: string Callback - The unique callback name you want to associate with this actor.
*/;
function TrackActor(Actor ActorRef, string Callback)
  ThreadLib.TrackActor(ActorRef, Callback)
endFunction

;/* UntrackActor
* * Removes an associated callback name from an actor.
* * Mod Events of type <Callback>_Start, <Callback>_End, <Callback>_Orgasm, and <Callback>_Added, are no more sent for this actor.
* * Warning, do not remove the player, or some old mods may fail to work.
* * 
* * @param: Actor ActorRef - The actor you want to remove the tracked events for.
* * @param: string Callback - The unique callback event you want to disable.
*/;
function UntrackActor(Actor ActorRef, string Callback)
  ThreadLib.UntrackActor(ActorRef, Callback)
endFunction

;/* 
* * Associates a specific Faction with a unique callback mod event that is sent whenever an actor that is in this faction, performs certain actions within SexLab Animations.
* * You need to RegisterForModEvents for an event with name <Callback>_<Event>, where events can be:
* * "Added" - The actor is added to a SexLab animation
* * "Start" - The SexLab animations where the actor was added is starting
* * "Orgasm" - The actor is having an orgasm
* * "End" - The SexLab animations where the actor was added is ended
* * 
* * @param: Faction FactionRef - The faction whose members you want to receive tracked events for.
* * @param: string Callback - The unique callback name you want to associate with this faction's actors.
*/;
function TrackFaction(Faction FactionRef, string Callback)
  ThreadLib.TrackFaction(FactionRef, Callback)
endFunction

;/* UntrackFaction
* * Removes an associated callback from a faction.
* * 
* * @param: Faction FactionRef - The faction you want to remove the tracked events for.
* * @param: string Callback - The unique callback event you want to disable.
*/;
function UntrackFaction(Faction FactionRef, string Callback)
  ThreadLib.UntrackFaction(FactionRef, Callback)
endFunction

;/* IsActorTracked
* * Checks if a given actor will receive any tracked events. Will always return TRUE if used on the player, due to the built in "PlayerTrack" callback.
* * 
* * @param: Actor ActorRef - The actor to check.
* * @return: bool - TRUE if the actor has any associated callbacks, or belongs to any tracked factions.
*/;
bool function IsActorTracked(Actor ActorRef)
  return ThreadLib.IsActorTracked(ActorRef)
endFunction

;#-----------------------------------------------------------------------------------------------------------------------------------------#
;#                                                                                                                                         #
;#                                                          BEGIN VOICE FUNCTIONS                                                          #
;#                                                                                                                                         #
;#-----------------------------------------------------------------------------------------------------------------------------------------#

; Select a Voice matching this actor
; --- Params
; akActor:  An actor to match a voice on. May be none for Tag versions, in which case any voice with given tags will be selected
; asTags:   A string/list of tags. Supports prefixing (-/~)
; --- Return:
; A String ID representing a unique voice object
String Function SelectVoice(Actor akActor)
  sslVoiceSlots.SelectVoice(akActor)
EndFunction
String Function SelectVoiceByTags(Actor akActor, String asTags)
  sslVoiceSlots.SelectVoiceByTags(akActor, asTags)
EndFunction
String Function SelectVoiceByTagsA(Actor akActor, String[] asTags)
  sslVoiceSlots.SelectVoiceByTagsA(akActor, asTags)
EndFunction

; Reserve a voice that this actor will prefer over a randomly selected one
Function StoreVoice(Actor akActor, String asVoice)
  sslVoiceSlots.StoreVoice(akActor, asVoice)
EndFunction
bool Function HasStoredVoice(Actor akActor)
  return GetStoredVoice(akActor) != ""
EndFunction
String Function GetStoredVoice(Actor akActor)
  return sslVoiceSlots.GetSavedVoice(akActor)
EndFunction
Function ClearVoice(Actor akActor)
  sslVoiceSlots.DeleteVoice(akActor)
EndFunction

;#-----------------------------------------------------------------------------------------------------------------------------------------#
;#                                                                                                                                         #
;#  ^^^                                                      END VOICE FUNCTIONS                                                      ^^^  #
;#                                                                                                                                         #
;#-----------------------------------------------------------------------------------------------------------------------------------------#

;#-----------------------------------------------------------------------------------------------------------------------------------------#
;#                                                                                                                                         #
;#                                                        BEGIN EXPRESSION FUNCTION                                                        #
;#                                                                                                                                         #
;#-----------------------------------------------------------------------------------------------------------------------------------------#

;/* GetCurrentMFG
* * Get an array with the mood, phonemes, and modifiers currently applied to the actor.
* * Mirrored function of a global in sslBaseExpressions. Is advised to use, in your scripts, the global one instead of this one.
* *
* * @param: Actor ActorRef - The actors whose expression values should be returned.
* * @return: float[] - An float array of Length 32 that match the format and structure of the Preset parameter in the ApplyPresetFloats function.
*/;
float[] function GetCurrentMFG(Actor ActorRef)
  return sslBaseExpression.GetCurrentMFG(ActorRef)
endFunction

;/* ClearMFG
* * Resets an actors mood, phonemes, and modifiers.
* * Mirrored function of a global in sslBaseExpressions. Is advised to use, in your scripts, the global one instead of this one.
* *
* * @param: Actor ActorRef - The actors whose expression should return to normal.
*/;
function ClearMFG(Actor ActorRef)
  sslBaseExpression.ClearMFG(ActorRef)
endFunction

;/* ClearPhoneme
* * Resets all of an actors phonemes to 0.
* * Mirrored function of a global in sslBaseExpressions. Is advised to use, in your scripts, the global one instead of this one.
* *
* * @param: Actor ActorRef - The actor to clear phonemes on.
*/;
function ClearPhoneme(Actor ActorRef)
  sslBaseExpression.ClearPhoneme(ActorRef)
endFunction

;/* ClearModifier
* * Resets all of an actors modifiers to 0.
* * Mirrored function of a global in sslBaseExpressions. Is advised to use, in your scripts, the global one instead of this one.
* *
* * @param: Actor ActorRef - The actor to clear modifiers on.
*/;
function ClearModifier(Actor ActorRef)
  sslBaseExpression.ClearModifier(ActorRef)
endFunction

;/* ApplyPresetFloats
* * Applies an array of values to an actor, automatically setting their phonemes, modifiers, and mood.
* * Mirrored function of a global in sslBaseExpressions. Is advised to use, in your scripts, the global one instead of this one.
* *
* * @param: Actor ActorRef - The actors to apply the preset to.
* * @param: float[] Preset - Must be a 32 length array. Each index corresponds to an MFG id. Values range from 0.0 to 1.0, with the exception of mood type.
* *                          Phonemes   0-15 = Preset[0]  to Preset[15]
* *                          Modifiers  0-13 = Preset[16] to Preset[29]
* *                          Mood Type       = Preset[30]
* *                          Mood Value      = Preset[31]
*/;
function ApplyPresetFloats(Actor ActorRef, float[] Preset)
  sslBaseExpression.ApplyPresetFloats(ActorRef, Preset)
endfunction

;#-----------------------------------------------------------------------------------------------------------------------------------------#
;#                                                                                                                                         #
;#                                                         START UTILITY FUNCTIONS                                                         #
;#                                                                                                                                         #
;#-----------------------------------------------------------------------------------------------------------------------------------------#

; Create an actor array only containing the argument actors that are *not* none
Actor[] Function MakeActorArray(Actor Actor1 = none, Actor Actor2 = none, Actor Actor3 = none, Actor Actor4 = none, Actor Actor5 = none)
  return SexLabUtil.MakeActorArray(Actor1, Actor2, Actor3, Actor4, Actor5)
EndFunction

; Format a given amount of seconds into HH:MM:SS format
String Function ParseTime(int time)
  return sslActorStats.ParseTime(time)
EndFunction

; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;
; ----------------------------------------------------------------------------------------------------------------------------------------- ;
;                                                                                                                                           ;
;                                      ██╗███╗   ██╗████████╗███████╗██████╗ ███╗   ██╗ █████╗ ██╗                                          ;
;                                      ██║████╗  ██║╚══██╔══╝██╔════╝██╔══██╗████╗  ██║██╔══██╗██║                                          ;
;                                      ██║██╔██╗ ██║   ██║   █████╗  ██████╔╝██╔██╗ ██║███████║██║                                          ;
;                                      ██║██║╚██╗██║   ██║   ██╔══╝  ██╔══██╗██║╚██╗██║██╔══██║██║                                          ;
;                                      ██║██║ ╚████║   ██║   ███████╗██║  ██║██║ ╚████║██║  ██║███████╗                                     ;
;                                      ╚═╝╚═╝  ╚═══╝   ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚══════╝                                     ;
;                                                                                                                                           ;
; ----------------------------------------------------------------------------------------------------------------------------------------- ;
; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;
; ----------------------------------------------------------------------------------------------------------------------------------------- ;
;                                                     This is the end of the public API                                                     ;
;                                    Do not use or access any of the below listed functions or properties                                   ;  
; ----------------------------------------------------------------------------------------------------------------------------------------- ;
; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;

;#-----------------------------------------------------------------------------------------------------------------------------------------#;
;#                                                                                                                                         #;
;#                                                 DEPRECATED FUNCTIONS - DO NOT USE THEM                                                  #;
;#         Replace these functions, if used in your mod, with the applicable new versions for easier usage and better performance.         #;
;#                                                                                                                                         #;
;#-----------------------------------------------------------------------------------------------------------------------------------------#;

;/ DEPRECATED /;
sslThreadController function HookController(string argString)
  return ThreadSlots.GetController(argString as int)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation function HookAnimation(string argString)
  return ThreadSlots.GetController(argString as int).Animation
endFunction

;/* DEPRECATED! */;
int function HookStage(string argString)
  return ThreadSlots.GetController(argString as int).Stage
endFunction

;/* DEPRECATED! */;
Actor function HookVictim(string argString)
  return ThreadSlots.GetController(argString as int).VictimRef
endFunction

;/* DEPRECATED! */;
Actor[] function HookActors(string argString)
  return ThreadSlots.GetController(argString as int).Positions
endFunction

;/* DEPRECATED! */;
float function HookTime(string argString)
  return ThreadSlots.GetController(argString as int).TotalTime
endFunction

;/* DEPRECATED! */;
bool function HasCreatureAnimation(Race CreatureRace, int Gender = -1)
  return CreatureSlots.RaceHasAnimation(CreatureRace, -1, Gender)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] function GetAnimationsByTag(int ActorCount, string Tag1, string Tag2 = "", string Tag3 = "", string TagSuppress = "", bool RequireAll = true)
  return AnimSlots.GetByTags(ActorCount, sslUtility.MakeArgs(",", Tag1, Tag2, Tag3), TagSuppress, RequireAll)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] function GetCreatureAnimationsByTags(int ActorCount, string Tags, string TagSuppress = "", bool RequireAll = true)
  return CreatureSlots.GetByTags(ActorCount, Tags, TagSuppress, RequireAll)
endFunction

;/* DEPRECATED! */;
sslBaseVoice function GetVoiceByTag(string Tag1, string Tag2 = "", string TagSuppress = "", bool RequireAll = true)
  return VoiceSlots.GetByTags(sslUtility.MakeArgs(",", Tag1, Tag2), TagSuppress, RequireAll)
endFunction

;/* DEPRECATED! */;
function ApplyCum(Actor ActorRef, int CumID)
  ActorLib.ApplyCum(ActorRef, CumID)
endFunction

;/* DEPRECATED! */;
form function StripWeapon(Actor ActorRef, bool RightHand = true)
  return none ; ActorLib.StripWeapon(ActorRef, RightHand)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] property Animations hidden
  sslBaseAnimation[] function get()
    return AnimSlots.GetSlots(0, 128)
  endFunction
endProperty

;/* DEPRECATED! */;
sslBaseAnimation[] property CreatureAnimations hidden
  sslBaseAnimation[] function get()
    return CreatureSlots.GetSlots(0, 128)
  endFunction
endProperty

;/* DEPRECATED! */;
sslBaseVoice[] property Voices hidden
  sslBaseVoice[] function get()
    return VoiceSlots.Voices
  endFunction
endProperty

;/* DEPRECATED! */;
sslBaseExpression[] property Expressions hidden
  sslBaseExpression[] function get()
    return ExpressionSlots.Expressions
  endFunction
endProperty

;/* DEPRECATED! */;
sslBaseExpression function RandomExpressionByTag(string Tag)
  return ExpressionSlots.RandomByTag(Tag)
endFunction

;/* DEPRECATED! */;
function ApplyPreset(Actor ActorRef, int[] Preset)
  sslBaseExpression.ApplyPreset(ActorRef, Preset)
endFunction

;/* DEPRECATED! */;
sslThreadController[] property Threads hidden
  sslThreadController[] function get()
    return ThreadSlots.Threads
  endFunction
endProperty

;/* DEPRECATED! */;
int function StartSex(Actor[] Positions, sslBaseAnimation[] Anims, Actor Victim = none, ObjectReference CenterOn = none, bool AllowBed = true, string Hook = "")
  sslThreadModel thread = NewThread()
  If (!thread)
    Log("StartSex() - Failed to claim an available thread")
    return -1
  ElseIf (!thread.AddActors(Positions, Victim))
    Log("StartSex() - Failed to add some actors to thread")
    return -1
  EndIf
  thread.SetAnimations(Anims)
  thread.CenterOnObject(CenterOn)
  thread.DisableBedUse(!AllowBed)
  thread.SetHook(Hook)
  thread.SetConsent(Victim == none)
  If (thread.StartThread())
    return thread.GetThreadID()
  EndIf
  return -1
EndFunction

;/* DEPRECATED! */;
sslThreadController Function QuickStart(Actor Actor1, Actor Actor2 = none, Actor Actor3 = none, Actor Actor4 = none, Actor Actor5 = none, Actor Victim = none, string Hook = "", string AnimationTags = "")
  Actor[] Positions = SexLabUtil.MakeActorArray(Actor1, Actor2, Actor3, Actor4, Actor5)
  return StartScene(Positions, AnimationTags, Victim, asHook = Hook) as sslThreadController
EndFunction

;/* DEPRECATED! */;
string function MakeAnimationGenderTag(Actor[] Positions)
  return ActorLib.MakeGenderTag(Positions)
endFunction

;/* DEPRECATED! */;
string function GetGenderTag(int Females = 0, int Males = 0, int Creatures = 0)
  return ActorLib.GetGenderTag(Females, Males, Creatures)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] function GetAnimationsByTags(int ActorCount, string Tags, string TagSuppress = "", bool RequireAll = true)
  return AnimSlots.GetByTags(ActorCount, Tags, TagSuppress, RequireAll)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] function GetAnimationsByType(int ActorCount, int Males = -1, int Females = -1, int StageCount = -1, bool Aggressive = false, bool Sexual = true)
  return AnimSlots.GetByType(ActorCount, Males, Females, StageCount, Aggressive, Sexual)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] function PickAnimationsByActors(Actor[] Positions, int Limit = 64, bool Aggressive = false)
  return AnimSlots.PickByActors(Positions, limit, aggressive)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] function GetAnimationsByDefault(int Males, int Females, bool IsAggressive = false, bool UsingBed = false, bool RestrictAggressive = true)
  return AnimSlots.GetByDefault(Males, Females, IsAggressive, UsingBed, RestrictAggressive)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] function GetAnimationsByDefaultTags(int Males, int Females, bool IsAggressive = false, bool UsingBed = false, bool RestrictAggressive = true, string Tags, string TagsSuppressed = "", bool RequireAll = true)
  return AnimSlots.GetByDefaultTags(Males, Females, IsAggressive, UsingBed, RestrictAggressive, Tags, TagsSuppressed, RequireAll)
endFunction

;/* DEPRECATED! */;
Actor[] function SortCreatures(Actor[] Positions, sslBaseAnimation Animation = none)
  return ThreadLib.SortCreatures(Positions, Animation)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] function GetCreatureAnimationsByRace(int ActorCount, Race RaceRef)
  return CreatureSlots.GetByRace(ActorCount, RaceRef)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] function GetCreatureAnimationsByRaceTags(int ActorCount, Race RaceRef, string Tags, string TagSuppress = "", bool RequireAll = true)
  return CreatureSlots.GetByRaceTags(ActorCount, RaceRef, Tags, TagSuppress, RequireAll)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] function GetCreatureAnimationsByRaceGenders(int ActorCount, Race RaceRef, int MaleCreatures = 0, int FemaleCreatures = 0, bool ForceUse = false)
  return CreatureSlots.GetByRaceGenders(ActorCount, RaceRef, MaleCreatures, FemaleCreatures, ForceUse)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] function GetCreatureAnimationsByRaceGendersTags(int ActorCount, Race RaceRef, int MaleCreatures = 0, int FemaleCreatures = 0, string Tags, string TagSuppress = "", bool RequireAll = true)
  return CreatureSlots.GetByRaceGendersTags(ActorCount, RaceRef, MaleCreatures, FemaleCreatures, Tags, TagSuppress, RequireAll)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] function GetCreatureAnimationsByRaceKey(int ActorCount, string RaceKey)
  return CreatureSlots.GetByRaceKey(ActorCount, RaceKey)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] function GetCreatureAnimationsByRaceKeyTags(int ActorCount, string RaceKey, string Tags, string TagSuppress = "", bool RequireAll = true)
  return CreatureSlots.GetByRaceKeyTags(ActorCount, RaceKey, Tags, TagSuppress, RequireAll)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] function GetCreatureAnimationsByActors(int ActorCount, Actor[] Positions)
  return CreatureSlots.GetByCreatureActors(ActorCount, Positions)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] function GetCreatureAnimationsByActorsTags(int ActorCount, Actor[] Positions, string Tags, string TagSuppress = "", bool RequireAll = true)
  return CreatureSlots.GetByCreatureActorsTags(ActorCount, Positions, Tags, TagSuppress, RequireAll)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation function RegisterAnimation(string Registrar, Form CallbackForm = none, ReferenceAlias CallbackAlias = none)
  return none
endFunction

;/* DEPRECATED! */;
sslBaseAnimation function RegisterCreatureAnimation(string Registrar, Form CallbackForm = none, ReferenceAlias CallbackAlias = none)
  return none
endFunction

;/* DEPRECATED! */;
sslBaseAnimation function NewAnimationObject(string Token, Form Owner)
  return none
endFunction

;/* DEPRECATED! */;
sslBaseAnimation function GetSetAnimationObject(string Token, string Callback, Form Owner)
  return none
endFunction

;/* DEPRECATED! */;
sslBaseAnimation function NewAnimationObjectCopy(string Token, sslBaseAnimation CopyFrom, Form Owner)
  return none
endFunction

;/* DEPRECATED! */;
sslBaseAnimation function GetAnimationObject(string Token)
  String ret = SexLabRegistry.GetSceneByName(Token)
  return AnimSlots.GetSetAnimation(ret)
endFunction

;/* DEPRECATED! */;
sslBaseAnimation[] function GetOwnerAnimations(Form Owner)
  return none
endFunction

;/* DEPRECATED! */;
sslBaseAnimation function MakeAnimationRegistered(string Token)
  return none
endFunction

;/* DEPRECATED! */;
bool function RemoveRegisteredAnimation(string Registrar)
  return AnimSlots.UnregisterAnimation(Registrar)
endFunction

;/* DEPRECATED! */;
bool function RemoveRegisteredCreatureAnimation(string Registrar)
  return CreatureSlots.UnregisterAnimation(Registrar)
endFunction

; --- Removed since P+ Phase 2

int function GetGender(Actor ActorRef)
  return ActorLib.GetGender(ActorRef)
endFunction
Function TreatAsGender(Actor ActorRef, bool AsFemale) ;  See TreatAsSex()
  ActorLib.TreatAsGender(ActorRef, AsFemale)
EndFunction
function ClearForcedGender(Actor ActorRef)  ; See ClearForcedSex()
  ActorLib.ClearForcedGender(ActorRef)
endFunction
int[] function GenderCount(Actor[] Positions)
  return ActorLib.GenderCount(Positions)
endFunction
int[] function TransGenderCount(Actor[] Positions)
  return ActorLib.TransCount(Positions)
endFunction
int function MaleCount(Actor[] Positions)
  return ActorLib.MaleCount(Positions)
endFunction
int function FemaleCount(Actor[] Positions)
  return ActorLib.FemaleCount(Positions)
endFunction
int function CreatureCount(Actor[] Positions)
  return ActorLib.CreatureCount(Positions)
endFunction
int function TransMaleCount(Actor[] Positions)
  return ActorLib.TransCount(Positions)[0]
endFunction
int function TransFemaleCount(Actor[] Positions)
  return ActorLib.TransCount(Positions)[1]
endFunction
int function TransCreatureCount(Actor[] Positions)
  int[] TransCount = ActorLib.TransCount(Positions)
  return TransCount[2] + TransCount[3]
endFunction
Form function EquipStrapon(Actor ActorRef)
  return Config.EquipStrapon(ActorRef)
endFunction
function UnequipStrapon(Actor ActorRef)
  Config.UnequipStrapon(ActorRef)
endFunction
int function CountCum(Actor ActorRef, bool Vaginal = true, bool Oral = true, bool Anal = true)
  return ActorLib.CountCum(ActorRef, Vaginal, Oral, Anal)
endFunction
function AddCum(Actor ActorRef, bool Vaginal = true, bool Oral = true, bool Anal = true)
  ActorLib.AddCum(ActorRef, Vaginal, Oral, Anal)
endFunction
function ClearCum(Actor ActorRef)
  ActorLib.ClearCum(ActorRef)
endFunction
bool function CheckBardAudience(Actor ActorRef, bool RemoveFromAudience = true)
  return Config.CheckBardAudience(ActorRef, RemoveFromAudience)
endFunction
Actor[] function SortActors(Actor[] Positions, bool FemaleFirst = true)
  return ThreadLib.SortActors(Positions, FemaleFirst)
endFunction
Actor[] function SortActorsByScene(String asSceneID, Actor[] akPositions, Actor[] akSubmissives)
  return ThreadLib.SortActorsByAnimationImpl(asSceneID, akPositions, akSubmissives)
endFunction
Actor function FindAvailableActor(ObjectReference CenterRef, float Radius = 5000.0, int FindGender = -1, Actor IgnoreRef1 = none, Actor IgnoreRef2 = none, Actor IgnoreRef3 = none, Actor IgnoreRef4 = none)
  return ThreadLib.FindAvailableActor(CenterRef, Radius, FindGender, IgnoreRef1, IgnoreRef2, IgnoreRef3, IgnoreRef4)
endFunction
Actor function FindAvailableActorByFaction(Faction FactionRef, ObjectReference CenterRef, float Radius = 5000.0, int FindGender = -1, Actor IgnoreRef1 = none, Actor IgnoreRef2 = none, Actor IgnoreRef3 = none, Actor IgnoreRef4 = none, bool HasFaction = True)
  return ThreadLib.FindAvailableActorInFaction(FactionRef, CenterRef, Radius, FindGender, IgnoreRef1, IgnoreRef2, IgnoreRef3, IgnoreRef4, HasFaction)
endFunction
Actor function FindAvailableActorWornForm(int slotMask, ObjectReference CenterRef, float Radius = 5000.0, int FindGender = -1, Actor IgnoreRef1 = none, Actor IgnoreRef2 = none, Actor IgnoreRef3 = none, Actor IgnoreRef4 = none, bool AvoidNoStripKeyword = True, bool HasWornForm = True)
  return ThreadLib.FindAvailableActorWornForm(slotMask, CenterRef, Radius, FindGender, IgnoreRef1, IgnoreRef2, IgnoreRef3, IgnoreRef4, AvoidNoStripKeyword, HasWornForm)
endFunction
Actor function FindAvailableCreature(string RaceKey, ObjectReference CenterRef, float Radius = 5000.0, int FindGender = 2, Actor IgnoreRef1 = none, Actor IgnoreRef2 = none, Actor IgnoreRef3 = none, Actor IgnoreRef4 = none)
  return ThreadLib.FindAvailableActor(CenterRef, Radius, FindGender, IgnoreRef1, IgnoreRef2, IgnoreRef3, IgnoreRef4, RaceKey)
endFunction
Actor function FindAvailableCreatureByFaction(string RaceKey, Faction FactionRef, ObjectReference CenterRef, float Radius = 5000.0, int FindGender = -1, Actor IgnoreRef1 = none, Actor IgnoreRef2 = none, Actor IgnoreRef3 = none, Actor IgnoreRef4 = none, bool HasFaction = True)
  return ThreadLib.FindAvailableActorInFaction(FactionRef, CenterRef, Radius, FindGender, IgnoreRef1, IgnoreRef2, IgnoreRef3, IgnoreRef4, HasFaction, RaceKey)
endFunction
Actor function FindAvailableCreatureWornForm(string RaceKey, int slotMask, ObjectReference CenterRef, float Radius = 5000.0, int FindGender = -1, Actor IgnoreRef1 = none, Actor IgnoreRef2 = none, Actor IgnoreRef3 = none, Actor IgnoreRef4 = none, bool AvoidNoStripKeyword = True, bool HasWornForm = True)
  return ThreadLib.FindAvailableActorWornForm(slotMask, CenterRef, Radius, FindGender, IgnoreRef1, IgnoreRef2, IgnoreRef3, IgnoreRef4, AvoidNoStripKeyword, HasWornForm, RaceKey)
endFunction
Actor[] function FindAvailablePartners(Actor[] Positions, int TotalActors, int Males = -1, int Females = -1, float Radius = 10000.0)
  return ThreadLib.FindAvailablePartners(Positions, TotalActors, Males, Females, Radius)
endFunction

function SendTrackedEvent(Actor ActorRef, string Hook, int id = -1)
  ThreadLib.SendTrackedEvent(ActorRef, Hook, id)
endFunction

; --- Old Threading API

sslThreadController function GetController(int tid)
  return ThreadSlots.GetController(tid)
endFunction
int function FindActorController(Actor ActorRef)
  return ThreadSlots.FindActorController(ActorRef)
endFunction
int function FindPlayerController()
  return ThreadSlots.FindActorController(PlayerRef)
endFunction
sslThreadController function GetActorController(Actor ActorRef)
  return ThreadSlots.GetActorController(ActorRef)
endFunction
sslThreadController function GetPlayerController()
  return ThreadSlots.GetActorController(PlayerRef)
endFunction
int function GetEnjoyment(int tid, Actor ActorRef)
  return ThreadSlots.GetController(tid).GetEnjoyment(ActorRef)
endfunction
bool function IsVictim(int tid, Actor ActorRef)
  return ThreadSlots.GetController(tid).IsVictim(ActorRef)
endFunction
bool function IsAggressor(int tid, Actor ActorRef)
  return ThreadSlots.GetController(tid).IsAggressor(ActorRef)
endFunction
bool function IsUsingStrapon(int tid, Actor ActorRef)
  return ThreadSlots.GetController(tid).ActorAlias(ActorRef).IsUsingStrapon()
endFunction
bool function PregnancyRisk(int tid, Actor ActorRef, bool AllowFemaleCum = false, bool AllowCreatureCum = false)
  return ThreadSlots.GetController(tid).PregnancyRisk(ActorRef, AllowFemaleCum, AllowCreatureCum)
endfunction

; --- Legacy Animation Functions

Actor[] function SortActorsByAnimation(Actor[] Positions, sslBaseAnimation Animation = none)
  return ThreadLib.SortActorsByAnimation(Positions, Animation)
endFunction
sslBaseAnimation function GetAnimationByName(string FindName)
  return AnimSlots.GetByName(FindName)
endFunction
sslBaseAnimation function GetAnimationByRegistry(string Registry)
  return AnimSlots.GetByRegistrar(Registry)
endFunction
int function FindAnimationByName(string FindName)
  return AnimSlots.FindByName(FindName)
endFunction
int function GetAnimationCount(bool IgnoreDisabled = true)
  return AnimSlots.GetCount(IgnoreDisabled)
endFunction
sslBaseAnimation[] function MergeAnimationLists(sslBaseAnimation[] List1, sslBaseAnimation[] List2)
  return sslUtility.MergeAnimationLists(List1, List2)
endFunction
sslBaseAnimation[] function RemoveTagged(sslBaseAnimation[] Anims, string Tags)
  return sslUtility.RemoveTaggedAnimations(Anims, PapyrusUtil.StringSplit(Tags))
endFunction
int function CountTag(sslBaseAnimation[] Anims, string Tags)
  return AnimSlots.CountTag(Anims, Tags)
endFunction
int function CountTagUsage(string Tags, bool IgnoreDisabled = true)
  return AnimSlots.CountTagUsage(Tags, IgnoreDisabled)
endFunction
int function CountCreatureTagUsage(string Tags, bool IgnoreDisabled = true)
  return CreatureSlots.CountTagUsage(Tags, IgnoreDisabled)
endFunction
string[] function GetAllAnimationTags(int ActorCount = -1, bool IgnoreDisabled = true)
  return AnimSlots.GetAllTags(ActorCount, IgnoreDisabled)
endFunction
string[] function GetAllAnimationTagsInArray(sslBaseAnimation[] List)
  return sslUtility.GetAllAnimationTagsInArray(List)
endFunction

; --- Legacy Creature Functions

sslBaseAnimation function GetCreatureAnimationByName(string FindName)
  return CreatureSlots.GetByName(FindName)
endFunction
sslBaseAnimation function GetCreatureAnimationByRegistry(string Registry)
  return CreatureSlots.GetByRegistrar(Registry)
endFunction
bool function HasCreatureRaceAnimation(Race CreatureRace, int ActorCount = -1, int Gender = -1)
  return CreatureSlots.RaceHasAnimation(CreatureRace, ActorCount, Gender)
endFunction
bool function HasCreatureRaceKeyAnimation(string RaceKey, int ActorCount = -1, int Gender = -1)
  return CreatureSlots.RaceKeyHasAnimation(RaceKey, ActorCount, Gender)
endFunction
bool function AllowedCreature(Race CreatureRace)
  return CreatureSlots.AllowedCreature(CreatureRace)
endFunction
bool function AllowedCreatureCombination(Race CreatureRace, Race CreatureRace2)
  return CreatureSlots.AllowedCreatureCombination(CreatureRace, CreatureRace2)
endFunction
string[] function GetAllCreatureAnimationTags(int ActorCount = -1, bool IgnoreDisabled = true)
  return CreatureSlots.GetAllTags(ActorCount, IgnoreDisabled)
endFunction
string[] function GetAllBothAnimationTags(int ActorCount = -1, bool IgnoreDisabled = true)
  string[] Output = PapyrusUtil.MergeStringArray(AnimSlots.GetAllTags(ActorCount, IgnoreDisabled), CreatureSlots.GetAllTags(ActorCount, IgnoreDisabled))
  PapyrusUtil.SortStringArray(Output)
  return Output
endFunction

; --- Legacy Statistics Functions

int function RegisterStat(string Name, string Value, string Prepend = "", string Append = "")
  return Stats.RegisterStat(Name, Value, Prepend, Append)
endFunction
function Alter(string Name, string NewName = "", string Value = "", string Prepend = "", string Append = "")
  Stats.Alter(Name, NewName, Value, Prepend, Append)
endFunction
int function FindStat(string Name)
  return Stats.FindStat(Name)
endFunction
string function GetActorStat(Actor ActorRef, string Name)
  return Stats.GetStat(ActorRef, Name)
endFunction
int function GetActorStatInt(Actor ActorRef, string Name)
  return Stats.GetStatInt(ActorRef, Name)
endFunction
float function GetActorStatFloat(Actor ActorRef, string Name)
  return Stats.GetStatFloat(ActorRef, Name)
endFunction
string function SetActorStat(Actor ActorRef, string Name, string Value)
  return Stats.SetStat(ActorRef, Name, Value)
endFunction
int function ActorAdjustBy(Actor ActorRef, string Name, int AdjustBy)
  return Stats.AdjustBy(ActorRef, Name, AdjustBy)
endFunction
string function GetActorStatFull(Actor ActorRef, string Name)
  return Stats.GetStatFull(ActorRef, Name)
endFunction
string function GetStatFull(string Name)
  return Stats.GetStatFull(PlayerRef, Name)
endFunction
string function GetStat(string Name)
  return Stats.GetStat(PlayerRef, Name)
endFunction
int function GetStatInt(string Name)
  return Stats.GetStatInt(PlayerRef, Name)
endFunction
float function GetStatFloat(string Name)
  return Stats.GetStatFloat(PlayerRef, Name)
endFunction
string function SetStat(string Name, string Value)
  return Stats.SetStat(PlayerRef, Name, Value)
endFunction
int function AdjustBy(string Name, int AdjustBy)
  return Stats.AdjustBy(PlayerRef, Name, AdjustBy)
endFunction
bool function IsImpure(Actor ActorRef)
  return Stats.IsLewd(ActorRef)
endFunction
float function AdjustPurity(Actor ActorRef, float amount)
  return Stats.AdjustPurity(ActorRef, amount)
endFunction
float function GetPurity(Actor ActorRef)
  return Stats.GetPurity(ActorRef)
endFunction
int function GetPurityLevel(Actor ActorRef)
  return Stats.GetPurityLevel(ActorRef)
endFunction
string function GetPurityTitle(Actor ActorRef)
  return Stats.GetPurityTitle(ActorRef)
endFunction
bool function IsPure(Actor ActorRef)
  return Stats.IsPure(ActorRef)
endFunction
bool function IsLewd(Actor ActorRef)
  return Stats.IsLewd(ActorRef)
endFunction
float function AdjustPlayerPurity(float amount)
  return Stats.AdjustPurity(PlayerRef, amount)
endFunction
int function GetPlayerPurityLevel()
  return Stats.GetPurityLevel(PlayerRef)
endFunction
string function GetPlayerPurityTitle()
  return Stats.GetPurityTitle(PlayerRef)
endFunction
float function DaysSinceLastSex(Actor ActorRef)
  return Stats.DaysSinceLastSex(ActorRef)
endFunction
float function HoursSinceLastSex(Actor ActorRef)
  return Stats.HoursSinceLastSex(ActorRef)
endFunction
float function MinutesSinceLastSex(Actor ActorRef)
  return Stats.MinutesSinceLastSex(ActorRef)
endFunction
float function SecondsSinceLastSex(Actor ActorRef)
  return Stats.SecondsSinceLastSex(ActorRef)
endFunction
string function LastSexTimerString(Actor ActorRef)
  return Stats.LastSexTimerString(ActorRef)
endFunction
float function LastSexRealTime(Actor ActorRef)
  return Stats.LastSexRealTime(ActorRef)
endFunction
float function DaysSinceLastSexRealTime(Actor ActorRef)
  return Stats.DaysSinceLastSexRealTime(ActorRef)
endFunction
float function HoursSinceLastSexRealTime(Actor ActorRef)
  return Stats.HoursSinceLastSexRealTime(ActorRef)
endFunction
float function MinutesSinceLastSexRealTime(Actor ActorRef)
  return Stats.MinutesSinceLastSexRealTime(ActorRef)
endFunction
float function SecondsSinceLastSexRealTime(Actor ActorRef)
  return Stats.SecondsSinceLastSexRealTime(ActorRef)
endFunction
string function LastSexTimerStringRealTime(Actor ActorRef)
  return Stats.LastSexTimerStringRealTime(ActorRef)
endFunction
int function CalcSexuality(bool IsFemale, int males, int females)
  return Stats.CalcSexuality(IsFemale, males, females)
endFunction
int function CalcLevel(float total, float curve = 0.65)
  return sslActorStats.CalcLevel(total, curve)
endFunction
int function GetPlayerStatLevel(string Skill)
  return Stats.GetSkillLevel(PlayerRef, Skill)
endFunction
string function GetPlayerSexualityTitle()
  return sslActorStats.GetSexualityTitle(PlayerRef)
endFunction
string function GetPlayerSkillTitle(string Skill)
  return Stats.GetSkillTitle(PlayerRef, Skill)
endFunction
int function GetPlayerSkillLevel(string Skill)
  return Stats.GetSkillLevel(PlayerRef, Skill)
endFunction
string function GetSexualityTitle(Actor ActorRef)
  return sslActorStats.GetSexualityTitle(ActorRef)
endFunction
string function GetSkillTitle(Actor ActorRef, string Skill)
  return Stats.GetSkillTitle(ActorRef, Skill)
endFunction
int function PlayerSexCount(Actor ActorRef)
  return Stats.PlayerSexCount(ActorRef)
endFunction
bool function HadPlayerSex(Actor ActorRef)
  return Stats.HadPlayerSex(ActorRef)
endFunction
Actor function MostUsedPlayerSexPartner()
  return Stats.MostUsedPlayerSexPartner()
endFunction
Actor[] function MostUsedPlayerSexPartners(int MaxActors = 5)
  return Stats.MostUsedPlayerSexPartners(MaxActors)
endFunction
float Function LastSexGameTime(Actor ActorRef)
  return Stats.LastSexGameTime(ActorRef)
EndFunction
Actor function LastSexPartner(Actor ActorRef)
  return Stats.LastSexPartner(ActorRef)
endFunction
bool function HasHadSexTogether(Actor ActorRef1, Actor ActorRef2)
  return Stats.HasHadSexTogether(ActorRef1, ActorRef2)
endfunction
Actor function LastAggressor(Actor ActorRef)
  return Stats.LastAggressor(ActorRef)
endFunction
bool function WasVictimOf(Actor VictimRef, Actor AggressorRef)
  return Stats.WasVictimOf(VictimRef, AggressorRef)
endFunction
Actor function LastVictim(Actor ActorRef)
  return Stats.LastVictim(ActorRef)
endFunction
bool function WasAggressorTo(Actor AggressorRef, Actor VictimRef)
  return Stats.WasAggressorTo(AggressorRef, VictimRef)
endFunction
function SetSexuality(Actor ActorRef, int amount)
  Stats.SetSkill(ActorRef, "Sexuality", PapyrusUtil.ClampInt(amount, 1, 100))
endFunction
function SetSexualityStraight(Actor ActorRef)
  Stats.SetSkill(ActorRef, "Sexuality", 100)
endFunction
function SetSexualityBisexual(Actor ActorRef)
  Stats.SetSkill(ActorRef, "Sexuality", 50)
endFunction
function SetSexualityGay(Actor ActorRef)
  Stats.SetSkill(ActorRef, "Sexuality", 1)
endFunction
int function GetSexuality(Actor ActorRef)
  return Stats.GetSexuality(ActorRef)
endFunction
int function GetSkill(Actor ActorRef, string Skill)
  return Stats.GetSkill(ActorRef, Skill)
endFunction
int function GetSkillLevel(Actor ActorRef, string Skill)
  return Stats.GetSkillLevel(ActorRef, Skill)
endFunction
bool function IsStraight(Actor ActorRef)
  return Stats.IsStraight(ActorRef)
endFunction
bool function IsBisexual(Actor ActorRef)
  return Stats.IsBisexual(ActorRef)
endFunction
bool function IsGay(Actor ActorRef)
  return Stats.IsGay(ActorRef)
endFunction
int function SexCount(Actor ActorRef)
  return Stats.SexCount(ActorRef)
endFunction
bool function HadSex(Actor ActorRef)
  return Stats.HadSex(ActorRef)
endFunction

; --- Legacy Factory Functions
; ALL OF THESE FUNCTIONS RETURN DEFAULT VALUES, THEIR IMPLEMENTATION HAS BEEN REMOVED

sslBaseVoice function NewVoiceObject(string Token, Form Owner)
  return Factory.NewVoice(Token, Owner)
endFunction
sslBaseExpression function NewExpressionObject(string Token, Form Owner)
  return Factory.NewExpression(Token, Owner)
endFunction
sslBaseVoice function GetSetVoiceObject(string Token, string Callback, Form Owner)
  return Factory.GetSetVoice(Token, Callback, Owner)
endFunction
sslBaseExpression function GetSetExpressionObject(string Token, string Callback, Form Owner)
  return Factory.GetSetExpression(Token, Callback, Owner)
endFunction
sslBaseVoice function NewVoiceObjectCopy(string Token, sslBaseVoice CopyFrom, Form Owner)
  return Factory.NewVoiceCopy(Token, CopyFrom, Owner)
endFunction
sslBaseExpression function NewExpressionObjectCopy(string Token, sslBaseExpression CopyFrom, Form Owner)
  return Factory.NewExpressionCopy(Token, CopyFrom, Owner)
endFunction
sslBaseVoice function GetVoiceObject(string Token)
  return Factory.GetVoice(Token)
endFunction
sslBaseExpression function GetExpressionObject(string Token)
  return Factory.GetExpression(Token)
endFunction
sslBaseVoice[] function GetOwnerVoices(Form Owner)
  return Factory.GetOwnerVoices(Owner)
endFunction
sslBaseExpression[] function GetOwnerExpressions(Form Owner)
  return Factory.GetOwnerExpressions(Owner)
endFunction
bool function HasVoiceObject(string Token)
  return Factory.HasVoice(Token)
endFunction
bool function HasExpressionObject(string Token)
  return Factory.HasExpression(Token)
endFunction
bool function ReleaseVoiceObject(string Token)
  return Factory.ReleaseVoice(Token)
endFunction
bool function ReleaseExpressionObject(string Token)
  return Factory.ReleaseExpression(Token)
endFunction
int function ReleaseOwnerVoices(Form Owner)
  return Factory.ReleaseOwnerVoices(Owner)
endFunction
int function ReleaseOwnerExpressions(Form Owner)
  return Factory.ReleaseOwnerExpressions(Owner)
endFunction
sslBaseVoice function MakeVoiceRegistered(string Token)
  return Factory.MakeVoiceRegistered(Token)
endFunction
sslBaseExpression function MakeExpressionRegistered(string Token)
  return Factory.MakeExpressionRegistered(Token)
endFunction
bool function HasAnimationObject(string Token)
  return Factory.HasAnimation(Token)
endFunction
bool function ReleaseAnimationObject(string Token)
  return Factory.ReleaseAnimation(Token)
endFunction
int function ReleaseOwnerAnimations(Form Owner)
  return Factory.ReleaseOwnerAnimations(Owner)
endFunction

; --- Legacy Expression Functions

sslBaseExpression function PickExpression(Actor ActorRef, Actor VictimRef = none)
  return ExpressionSlots.PickByStatus(ActorRef, (VictimRef && VictimRef == ActorRef), (VictimRef && VictimRef != ActorRef))
endFunction
sslBaseExpression function PickExpressionByStatus(Actor ActorRef, bool IsVictim = false, bool IsAggressor = false)
  return ExpressionSlots.PickByStatus(ActorRef, IsVictim, IsAggressor)
endFunction
sslBaseExpression[] function GetExpressionsByStatus(Actor ActorRef, bool IsVictim = false, bool IsAggressor = false)
  return ExpressionSlots.GetByStatus(ActorRef, IsVictim, IsAggressor)
endFunction
sslBaseExpression function PickExpressionsByTag(Actor ActorRef, string Tag)
  sslBaseExpression[] Found =  ExpressionSlots.GetByTag(Tag, ActorRef.GetLeveledActorBase().GetSex() == 1)
  if Found && Found.Length > 0
    return Found[(Utility.RandomInt(0, (Found.Length - 1)))]
  endIf
  return none
endFunction
sslBaseExpression[] function GetExpressionsByTag(Actor ActorRef, string Tag)
  return ExpressionSlots.GetByTag(Tag, ActorRef.GetLeveledActorBase().GetSex() == 1)
endFunction
sslBaseExpression function GetExpressionByName(string findName)
  return ExpressionSlots.GetByName(findName)
endFunction
int function FindExpressionByName(string findName)
  return ExpressionSlots.FindByName(findName)
endFunction
sslBaseExpression function GetExpressionBySlot(int slot)
  return ExpressionSlots.GetBySlot(slot)
endFunction
function OpenMouth(Actor ActorRef)
  if ActorRef
    int i
    while i < ThreadSlots.Threads.Length
      int ActorSlot = Threads[i].FindSlot(ActorRef)
      if ActorSlot != -1
        Threads[i].ActorAlias[ActorSlot].ForceOpenMouth = True
      endIf
      i += 1
    endwhile
    sslBaseExpression.OpenMouth(ActorRef)
  endIf
endFunction
function CloseMouth(Actor ActorRef)
  if ActorRef
    int i
    while i < ThreadSlots.Threads.Length
      int ActorSlot = Threads[i].FindSlot(ActorRef)
      if ActorSlot != -1
        Threads[i].ActorAlias[ActorSlot].ForceOpenMouth = False
      endIf
      i += 1
    endwhile
    sslBaseExpression.CloseMouth(ActorRef)
  endIf
endFunction
bool function IsMouthOpen(Actor ActorRef)
  return sslBaseExpression.IsMouthOpen(ActorRef)
endFunction

; --- Legacy Voice Functions

sslBaseVoice function PickVoice(Actor ActorRef)
  return VoiceSlots.PickVoice(ActorRef)
endFunction
sslBaseVoice function GetVoice(Actor ActorRef)
  return VoiceSlots.PickVoice(ActorRef)
endFunction
function SaveVoice(Actor ActorRef, sslBaseVoice Saving)
  VoiceSlots.SaveVoice(ActorRef, Saving)
endFunction
function ForgetVoice(Actor ActorRef)
  VoiceSlots.ForgetVoice(ActorRef)
endFunction
sslBaseVoice function GetSavedVoice(Actor ActorRef)
  return VoiceSlots.GetSaved(ActorRef)
endFunction
bool function HasCustomVoice(Actor ActorRef)
  return VoiceSlots.HasCustomVoice(ActorRef)
endFunction
sslBaseVoice function GetVoiceByGender(int Gender)
  return VoiceSlots.PickGender(Gender)
endFunction
sslBaseVoice[] function GetVoicesByGender(int Gender)
  return VoiceSlots.GetAllGender(Gender)
endFunction
sslBaseVoice function GetVoiceByName(string FindName)
  return VoiceSlots.GetByName(FindName)
endFunction
int function FindVoiceByName(string FindName)
  return VoiceSlots.FindByName(FindName)
endFunction
sslBaseVoice function GetVoiceBySlot(int slot)
  return VoiceSlots.GetBySlot(slot)
endFunction
sslBaseVoice function GetVoiceByTags(string Tags, string TagSuppress = "", bool RequireAll = true)
  return VoiceSlots.GetByTags(Tags, TagSuppress, RequireAll)
endFunction
sslBaseVoice[] function GetVoicesByTags(string Tags, string TagSuppress = "", bool RequireAll = true)
  return VoiceSlots.GetAllByTags(Tags, TagSuppress, RequireAll)
endFunction

;#-----------------------------------------------------------------------------------------------------------------------------------------#
;#                                                                                                                                         #
;# ^^^                                            END DEPRECATED FUNCTIONS - DO NOT USE THEM                                           ^^^ #
;#                                                                                                                                         #
;#-----------------------------------------------------------------------------------------------------------------------------------------#
;/
  NOTE: Following functions are not yet legacy but will likely be in some future update. Do not use them to stay compatible with future versions!
/;

;/* RegisterVoice
* * Find an available SexLabVoice slot and starts the callback to register it.
* * In case the SexLabVoice was already registered you get the already registered SexLabVoice without any update
* *
* * @param: string Registrar, the ID of the SexLabVoice, no spaces allowed.
* * @param: Form CallbackForm, the script (as object) that has the code to register the SexLabVoice, the script has to have an Event with the same name of the registrar
* * @param: ReferenceAlias CallbackAlias, can be used alternatively to CallbackForm, in case the script is inside a ReferenceAlias
* * @return: sslBaseVoice, the actual SexLabVoice registered
*/;
sslBaseVoice function RegisterVoice(string Registrar, Form CallbackForm = none, ReferenceAlias CallbackAlias = none)
  return VoiceSlots.RegisterVoice(Registrar, CallbackForm, CallbackAlias)
endFunction

;/* RegisterExpression
* * Find an available SexLabExpression slot and starts the callback to register it.
* * In case the SexLabExpression was already registered you get the already registered SexLabExpression without any update
* *
* * @param: string Registrar, the ID of the SexLabExpression, no spaces allowed.
* * @param: Form CallbackForm, the script (as object) that has the code to register the SexLabExpression, the script has to have an Event with the same name of the registrar
* * @param: ReferenceAlias CallbackAlias, can be used alternatively to CallbackForm, in case the script is inside a ReferenceAlias
* * @return: sslBaseVoice, the actual SexLabExpression registered
*/;
sslBaseExpression function RegisterExpression(string Registrar, Form CallbackForm = none, ReferenceAlias CallbackAlias = none)
  return ExpressionSlots.RegisterExpression(Registrar, CallbackForm, CallbackAlias)
endFunction

;/* RemoveRegisteredVoice
* * TODO
* * 
* * @param: 
* * @return: 
*/;
bool function RemoveRegisteredVoice(string Registrar)
  return VoiceSlots.UnregisterVoice(Registrar)
endFunction

;/* RemoveRegisteredExpression
* * TODO
* * 
* * @param: 
* * @return: 
*/;
bool function RemoveRegisteredExpression(string Registrar)
  return ExpressionSlots.UnregisterExpression(Registrar)
endFunction

;#-----------------------------------------------------------------------------------------------------------------------------------------#
;#                                                                                                                                         #
;#                                    THE FOLLOWING PROPERTIES AND FUNCTION ARE FOR INTERNAL USE ONLY                                      #
;#                                                                                                                                         #
;#                                                                                                                                         #
;#                             ****       ***         *     *   ***   *******     *     *   ******  *******                                #
;#                             *   **    *   *        **    *  *   *     *        *     *  *      * *                                      #
;#                             *     *  *     *       * *   * *     *    *        *     *  *        *                                      #
;#                             *      * *     *       *  *  * *     *    *        *     *   ******  *****                                  #
;#                             *     *  *     *       *   * * *     *    *        *     *         * *                                      #
;#                             *   **    *   *        *    **  *   *     *         *   *   *      * *                                      #
;#                             ****       ***         *     *   ***      *          ***     ******  *******                                #
;#                                                                                                                                         #
;#                                                                                                                                         #
;#-----------------------------------------------------------------------------------------------------------------------------------------#

sslSystemConfig property Config Auto
sslActorLibrary property ActorLib Auto
sslThreadLibrary property ThreadLib Auto
sslThreadSlots property ThreadSlots Auto

Function Setup()
	Form SexLabQuestFramework = Game.GetFormFromFile(0xD62, "SexLab.esm")
	Config = SexLabQuestFramework as sslSystemConfig
	ThreadLib = SexLabQuestFramework as sslThreadLibrary
	ThreadSlots = SexLabQuestFramework as sslThreadSlots
	ActorLib = SexLabQuestFramework as sslActorLibrary

  Log(self + " - Loaded SexLabFramework")
EndFunction

sslThreadModel Function NewThread(float TimeOut = 5.0)
  return ThreadSlots.PickModel(TimeOut)
EndFunction

SexLabThread Function StartSceneImpl(Actor[] akPositions, String[] asScenes, String asContext, Actor[] akSubmissive, ObjectReference akCenter, int aiFurniture, String asHook)
  sslThreadModel thread = NewThread()
  If (!thread)
    Log("StartSceneImpl() - Failed to claim an available thread")
    return none
  ElseIf (!thread.AddActorsA(akPositions, akSubmissive))
    Log("StartSceneImpl() - Failed to add some actors to thread")
    return none
  EndIf
  thread.SetScenes(asScenes)
  thread.CenterOnObject(akCenter)
  thread.SetFurnitureStatus(aiFurniture)
  thread.AddContextEx(asContext)
  thread.SetHook(asHook)
  return thread.StartThread()
EndFunction

Function Log(string Log, string Type = "NOTICE")
  If(Type == "FATAL")
    sslLog.Error(Log)
  Else
    sslLog.Log(Log)
  EndIf
EndFunction

auto state Disabled
  sslThreadModel function NewThread(float TimeOut = 5.0)
    LogDisabled("NewThread")
    return none
  endFunction
  SexLabThread Function StartScene(Actor[] akPositions, String asTags, Actor akSubmissive = none, ObjectReference akCenter = none, int aiFurniture = 1, String asHook = "")
    LogDisabled("StartScene")
    return none
  EndFunction
  SexLabThread Function StartSceneA(Actor[] akPositions, String asTags, Actor[] akSubmissives, ObjectReference akCenter = none, int aiFurniture = 1, String asHook = "")
    LogDisabled("StartSceneA")
  EndFunction
  SexLabThread Function StartSceneEx(Actor[] akPositions, String[] asAnims, Actor akSubmissive = none, String asContext = "", \
      ObjectReference akCenter = none, int aiFurniture = 1, String asHook = "")
    LogDisabled("StartSceneEx")
  EndFunction
  SexLabThread Function StartSceneExA(Actor[] akPositions, String[] asScenes, Actor[] akSubmissives, String asContext = "", \
      ObjectReference akCenter = none, int aiFurniture = 1, String asHook = "")
    LogDisabled("StartSceneExA")
  EndFunction
  SexLabThread Function StartSceneQuick(Actor akActor1, Actor akActor2 = none, Actor akActor3 = none, Actor akActor4 = none, Actor akActor5 = none, \
                                          Actor akSubmissive = none, String asTags = "", String asHook = "")
    LogDisabled("StartSceneQuick")
  EndFunction
  SexLabThread Function StartSceneImpl(Actor[] akPositions, String[] asScenes, String asContext, Actor[] akSubmissive, ObjectReference akCenter, int aiFurniture, String asHook)
    LogDisabled("StartSceneImpl")
    return none
  EndFunction
  sslThreadController function QuickStart(Actor Actor1, Actor Actor2 = none, Actor Actor3 = none, Actor Actor4 = none, Actor Actor5 = none, Actor Victim = none, string Hook = "", string AnimationTags = "")
    LogDisabled("QuickStart")
    return none
  endFunction
  int function StartSex(Actor[] Positions, sslBaseAnimation[] Anims, Actor Victim = none, ObjectReference CenterOn = none, bool AllowBed = true, string Hook = "")
    LogDisabled("StartSex")
    return -1
  endFunction
  event OnBeginState()
    Log("SexLabFramework - Disabled")
    ModEvent.Send(ModEvent.Create("SexLabDisabled"))
  endEvent
endState

Function LogDisabled(String asFunc)
  Log(asFunc + "() - Failed to make new thread model; system is currently disabled or not installed", "FATAL")
EndFunction

state Enabled
  event OnBeginState()
    Log("SexLabFramework - Enabled")
    ModEvent.Send(ModEvent.Create("SexLabEnabled"))
  endEvent
endState

Actor Property PlayerRef
  Actor Function Get()
    return Game.GetPLayer()
  EndFunction
EndProperty

Faction Property AnimatingFaction
  Faction Function Get()
    return Config.AnimatingFaction
  EndFunction
EndProperty
sslAnimationSlots property AnimSlots Hidden
  sslAnimationSlots Function Get()
    return Game.GetFormFromFile(0x639DF, "SexLab.esm") as sslAnimationSlots
  EndFunction
EndProperty
sslCreatureAnimationSlots property CreatureSlots Hidden
  sslCreatureAnimationSlots Function Get()
    return Game.GetFormFromFile(0x664FB, "SexLab.esm") as sslCreatureAnimationSlots
  EndFunction
EndProperty
sslVoiceSlots property VoiceSlots Hidden
  sslVoiceSlots Function Get()
	  return Game.GetFormFromFile(0x664FB, "SexLab.esm") as sslVoiceSlots
  EndFunction
EndProperty
sslObjectFactory property Factory Hidden
  sslObjectFactory Function Get()
    return Game.GetFormFromFile(0x78818, "SexLab.esm") as sslObjectFactory
  EndFunction
EndProperty
sslActorStats Property Stats Hidden
  sslActorStats Function Get()
	  return Game.GetFormFromFile(0xD62, "SexLab.esm") as sslActorStats
  EndFunction
EndProperty
sslExpressionSlots property ExpressionSlots Hidden
  sslExpressionSlots Function Get()
    return Game.GetFormFromFile(0x664FB, "SexLab.esm") as sslExpressionSlots
  EndFunction
EndProperty

event OnInit()
endEvent
