scriptname sslThreadLibrary extends sslSystemLibrary
{
  Generic Utility to simplify thread building
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

; ------------------------------------------------------- ;
; --- Bed Utility                                     --- ;
; ------------------------------------------------------- ;

int Property BedType_None = 0 AutoReadOnly
int Property BedType_BedRoll = 1 AutoReadOnly
int Property BedType_Single = 2 AutoReadOnly
int Property BedType_Double = 3 AutoReadOnly

ObjectReference[] Function FindBeds(ObjectReference akCenterRef, float afRadius = 4096.0, float afRadiusZ = 512.0) native global
int Function GetBedTypeImpl(ObjectReference akReference) native global
bool Function IsBed(ObjectReference akReference) native global

int Function GetBedType(ObjectReference BedRef)
  return GetBedTypeImpl(BedRef)
EndFunction
bool Function IsBedRoll(ObjectReference BedRef)
  return GetBedType(BedRef) == BedType_BedRoll
EndFunction
bool function IsDoubleBed(ObjectReference BedRef)
  return GetBedType(BedRef) == BedType_Single
endFunction
bool function IsSingleBed(ObjectReference BedRef)
  return GetBedType(BedRef) == BedType_Double
endFunction

bool Function IsBedAvailable(ObjectReference BedRef)
  If(BedRef.IsFurnitureInUse(true))
    return false
  EndIf
  int i = 0
  While(i < ThreadSlots.Threads.Length)
    If(ThreadSlots.Threads[i].IsLocked && ThreadSlots.Threads[i].BedRef == BedRef)
      return false
    EndIf
    i += 1
  Endwhile
  return true
EndFunction

ObjectReference Function FindBed(ObjectReference CenterRef, float Radius = 1000.0, bool IgnoreUsed = true, ObjectReference IgnoreRef1 = none, ObjectReference IgnoreRef2 = none)
  ObjectReference[] beds = FindBeds(CenterRef, Radius)
  int i = 0
  While(i < beds.Length)
    If(beds[i] != IgnoreRef1 && beds[i] != IgnoreRef2 && (IgnoreUsed || IsBedAvailable(beds[i])))
      return beds[i]
    EndIf
    i += 1
  EndWhile
  return none
endFunction

ObjectReference Function GetNearestUnusedBed(ObjectReference akCenterRef, float afRadius)
  ObjectReference[] beds = FindBeds(akCenterRef, afRadius)
  int i = 0
  While(i < beds.Length)
    If(IsBedAvailable(beds[i]))
      return beds[i]
    EndIf
    i += 1
  EndWhile
  return none
EndFunction

; ------------------------------------------------------- ;
; --- Position Sorting                                --- ;
; ------------------------------------------------------- ;
;/
  Note that ordering in SL Scenes is unspecified
  Further, SL will sort all actors before starting its animation, so calling these functions will not benefit performance in any way

  SortActors() will strictly sort your array by gender, with females/males (dep. on parameter) before creatures
  however this sorting is highly unlikely to be the same order that the animation expects. If you wish to sort
  actors based on an animation, use "SortActorsByAnimation/Impl" instead. The returned arrays ordering may feel random
  but will be compatible with the given animation. There is no guarantee that some other animation follows the same ordering however
/;

Actor[] Function SortActors(Actor[] Positions, bool FemaleFirst = true)
  Actor[] retVal = PapyrusUtil.RemoveActor(Positions, none)
  int[] sexes = sslActorLibrary.GetSexAll(retVal)
  int i = 1
  While(i < retVal.Length)
    Actor it = retVal[i]
    int _it = sexes[i]
    int n = i - 1
    While(n >= 0 && !IsLesserGender(sexes[n], _it, FemaleFirst))
      retVal[n + 1] = retVal[n]
      sexes[n + 1] = sexes[n]
      n -= 1
    EndWhile
    retVal[n + 1] = it
    sexes[n + 1] = _it
    i += 1
  EndWhile
  return retVal
EndFunction
bool Function IsLesserGender(int i, int n, bool abFemaleFirst)
  If (n == i)
    return false
  ElseIf (n == 0 && !abFemaleFirst)
    return true
  EndIf
  return i < n
EndFunction

Actor[] Function SortActorsByAnimationImpl(String asSceneID, Actor[] akPositions, Actor[] akVictims) native
Actor[] function SortActorsByAnimation(actor[] Positions, sslBaseAnimation Animation = none)
  If (!Animation || !Animation.Registry)
    return SortActors(Positions)
  EndIf
  return SortActorsByAnimationImpl(Animation.Registry, Positions, PapyrusUtil.ActorArray(0))
endFunction

; ------------------------------------------------------- ;
; --- Cell Searching                                  --- ;
; ------------------------------------------------------- ;

Actor[] function FindAvailableActors(ObjectReference CenterRef, float Radius = 5000.0, int FindGender = -1, Actor IgnoreRef1 = none, Actor IgnoreRef2 = none, Actor IgnoreRef3 = none, Actor IgnoreRef4 = none, string RaceKey = "") native
Actor function FindAvailableActor(ObjectReference CenterRef, float Radius = 5000.0, int FindGender = -1, Actor IgnoreRef1 = none, Actor IgnoreRef2 = none, Actor IgnoreRef3 = none, Actor IgnoreRef4 = none, string RaceKey = "") native
Actor function FindAvailableActorInFaction(Faction FactionRef, ObjectReference CenterRef, float Radius = 5000.0, int FindGender = -1, Actor IgnoreRef1 = none, Actor IgnoreRef2 = none, Actor IgnoreRef3 = none, Actor IgnoreRef4 = none, bool HasFaction = True, string RaceKey = "", bool JustSameFloor = False) native
Actor function FindAvailableActorWornForm(int slotMask, ObjectReference CenterRef, float Radius = 5000.0, int FindGender = -1, Actor IgnoreRef1 = none, Actor IgnoreRef2 = none, Actor IgnoreRef3 = none, Actor IgnoreRef4 = none, bool AvoidNoStripKeyword = True, bool HasWornForm = True, string RaceKey = "", bool JustSameFloor = False) native

Actor[] function FindAvailablePartners(actor[] Positions, int total, int males = -1, int females = -1, float radius = 10000.0) native
Actor[] function FindAnimationPartnersImpl(String asAnimID, ObjectReference akCenterRef, float afRadius, Actor[] akIncludes) native
Actor[] function FindAnimationPartners(sslBaseAnimation Animation, ObjectReference CenterRef, float Radius = 5000.0, Actor IncludedRef1 = none, Actor IncludedRef2 = none, Actor IncludedRef3 = none, Actor IncludedRef4 = none)
  Actor[] includes = new Actor[4]
  includes[0] = IncludedRef1
  includes[1] = IncludedRef2
  includes[2] = IncludedRef3
  includes[3] = IncludedRef4
  return FindAnimationPartnersImpl(Animation.Registry, CenterRef, Radius, PapyrusUtil.RemoveActor(includes, none))
endFunction

; ------------------------------------------------------- ;
; --- Actor Tracking                                  --- ;
; ------------------------------------------------------- ;

Function TrackActorImpl(Actor akActor, String asCallback, bool abTrack) native global
Function TrackFactionImpl(Faction akFaction, String asCallback, bool abTrack) native global

bool Function IsActorTrackedImpl(Actor akActor) native global
String[] Function GetAllTrackingEvents(Actor akTrackedActor, String asHook) native global

function SendTrackingEvents(Actor akActor, string asHook, int aiID) global
  String[] events = GetAllTrackingEvents(akActor, asHook)
  int i = 0
  While (i < events.Length)
    MakeTrackingEvent(akActor, events[i], aiID)
    i += 1
  EndWhile
EndFunction
function MakeTrackingEvent(Actor akActor, string asCallback, int aiID) global
  int eid = ModEvent.Create(asCallback)
  ModEvent.PushForm(eid, akActor)
  ModEvent.PushInt(eid, aiID)
  ModEvent.Send(eid)
endFunction

; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;
; ----------------------------------------------------------------------------- ;
;               ██╗     ███████╗ ██████╗  █████╗  ██████╗██╗   ██╗              ;
;               ██║     ██╔════╝██╔════╝ ██╔══██╗██╔════╝╚██╗ ██╔╝              ;
;               ██║     █████╗  ██║  ███╗███████║██║      ╚████╔╝               ;
;               ██║     ██╔══╝  ██║   ██║██╔══██║██║       ╚██╔╝                ;
;               ███████╗███████╗╚██████╔╝██║  ██║╚██████╗   ██║                 ;
;               ╚══════╝╚══════╝ ╚═════╝ ╚═╝  ╚═╝ ╚═════╝   ╚═╝                 ;
; ----------------------------------------------------------------------------- ;
; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;

Keyword property FurnitureBedRoll Auto
FormList property BedsList Auto
FormList property DoubleBedsList Auto
FormList property BedRollsList Auto

bool function SameFloor(ObjectReference BedRef, float Z, float Tolerance = 15.0)
  return BedRef && Math.Abs(Z - BedRef.GetPositionZ()) <= Tolerance
endFunction

bool function CheckActor(Actor CheckRef, int CheckGender = -1)
  if !CheckRef
    return false
  endIf
  int IsGender = ActorLib.GetGender(CheckRef)
  return ((CheckGender < 2 && IsGender < 2) || (CheckGender >= 2 && IsGender >= 2)) && (CheckGender == -1 || IsGender == CheckGender) && ActorLib.IsValidActor(CheckRef)
endFunction

int function FindNext(Actor[] Positions, sslBaseAnimation Animation, int offset, bool FindCreature)
  while offset
    offset -= 1
    if Animation.HasRace(Positions[offset].GetLeveledActorBase().GetRace()) == FindCreature
      return offset
    endIf
  endwhile
  return -1
endFunction
bool function CheckBed(ObjectReference BedRef, bool IgnoreUsed = true)
  return BedRef && BedRef.IsEnabled() && BedRef.Is3DLoaded() && (!IgnoreUsed || (IgnoreUsed && IsBedAvailable(BedRef)))
endFunction
bool function LeveledAngle(ObjectReference ObjectRef, float Tolerance = 5.0)
  return ObjectRef && Math.Abs(ObjectRef.GetAngleX()) <= Tolerance && Math.Abs(ObjectRef.GetAngleY()) <= Tolerance
endFunction

Actor[] function SortCreatures(actor[] Positions, sslBaseAnimation Animation = none)
  return SortActorsByAnimation(Positions, Animation)
endFunction

function TrackActor(Actor ActorRef, string Callback)
  TrackActorImpl(ActorRef, Callback, true)
endFunction
function TrackFaction(Faction FactionRef, string Callback)
  TrackFactionImpl(FactionRef, Callback, true)
endFunction
function UntrackActor(Actor ActorRef, string Callback)
  TrackActorImpl(ActorRef, Callback, false)
endFunction
function UntrackFaction(Faction FactionRef, string Callback)
  TrackFactionImpl(FactionRef, Callback, false)
endFunction
bool function IsActorTracked(Actor ActorRef)
  return IsActorTrackedImpl(ActorRef)
endFunction
function SendTrackedEvent(Actor ActorRef, string Hook = "", int id = -1)
  SendTrackingEvents(ActorRef, Hook, id)
EndFunction
function SetupActorEvent(Actor ActorRef, string Callback, int id = -1)
  return MakeTrackingEvent(ActorRef, Callback, id)
endFunction
