Scriptname sslMatchMakerMain extends Quest
{SexLab MatchMaker Main Script.}

SexLabFramework property SexLab auto
sslSystemConfig property Config auto
Actor property PlayerRef auto
float _StartCall

String Function GetSexString(int sexIndex)
  If sexIndex == 0
    return "Male"
  ElseIf sexIndex == 1
    return "Female"
  ElseIf sexIndex == 2
    return "Futa"
  ElseIf sexIndex == 3
    return "Male Creature"
  ElseIf sexIndex == 4
    return "Female Creature"
  EndIf
  return ""
EndFunction
String Function Parse_Sex(Actor akTarget)
  int sex = SexLabRegistry.GetSex(akTarget, false)
  return GetSexString(sex)
EndFunction

String Function Parse_Sexes_And_Races(int[] aiSexes, Actor[] akActors)
  String[] sSexesA = PapyrusUtil.StringArray(aiSexes.Length)
  int index = 0
  While index < aiSexes.Length
    int sexIndex = aiSexes[index]
    Actor currentActor = akActors[index]
    If currentActor
      sSexesA[index] = "[" + GetSexString(sexIndex) + ": " + SexLabRegistry.GetRaceKey(currentActor) + "]"
    EndIf
    index += 1
  EndWhile
  return PapyrusUtil.StringJoin(sSexesA, "; ")
EndFunction

; --- Adding Actors
; Function here is only used by 3p+ Scenes

Actor[] sceneActors

bool Function AddActors(Actor akTarget)
  If (sceneActors.Find(akTarget) > -1)
    return true
  ElseIf (SexLab.ValidateActor(akTarget) < 0)
    Config.Log("[SexLab MatchMaker] - Actor " + SexLabUtil.ActorName(akTarget) + " was invalid")
    return false
  EndIf
  If (sceneActors.Length < 5)
    sceneActors = new Actor[5]
  EndIf

  int where = sceneActors.Find(none)
  If (where > -1)
    sceneActors[where] = akTarget
    Debug.Notification("Added Actor: " + SexLabUtil.ActorName(akTarget))
    Config.Log("[SexLab MatchMaker] - Actor " + SexLabUtil.ActorName(akTarget) + " was added to the array.")
    Config.Log("[SexLab MatchMaker] - Actor " + SexLabUtil.ActorName(akTarget) + " is considered as: " + Parse_Sex(akTarget))
    RegisterForSingleUpdate(10.0)
    return true
  EndIf
  TriggerSex(sceneActors)
  sceneActors = new Actor[5]
  return false
EndFunction
Event OnUpdate()
  If (sceneActors.Find(none) == 0)
    return
  EndIf
  TriggerSex(sceneActors)
  sceneActors = new Actor[5]
EndEvent

; --- Start Scene

Function TriggerSex(Actor[] akPassed)
  RegisterForModEvent("HookAnimationStart_SSLMatchMaker", "AnimationStarted")
  RegisterForModEvent("HookAnimationEnd_SSLMatchMaker", "AnimationEnded")

  akPassed = PapyrusUtil.RemoveActor(akPassed, none)
  If (akPassed.Length < 1)
    Config.Log("[SexLab Matchmaker] Cannot start animation; invalid actor count")
    return
  Else
    Config.Log("[SexLab MatchMaker] Starting Scene with Actors: " + SexLabUtil.ActorNames(akPassed))
  EndIf

  Actor[] sub = new Actor[2]
  int plp = akPassed.Find(PlayerRef)
  If (Config.SubmissivePlayer && plp > -1)
    sub[0] = PlayerRef
  EndIf
  If (Config.SubmissiveTarget)
    If (plp != 0)
      sub[1] = akPassed[0]
    ElseIf (akPassed.Length > 1)
      sub[1] = akPassed[1]
    EndIf
  EndIf
  sub = PapyrusUtil.RemoveActor(sub, none)

  String tags = sslSystemConfig.ParseMMTagString()
  _StartCall = Utility.GetCurrentRealTime()
  While (!SexLab.StartSceneA(akPassed, tags, sub, asHook = "SSLMatchMaker"))
    If (!sub.Length || Config.SubmissivePlayer && plp > -1 && sub.Length == 1)
      Debug.Notification("Failed to start scene")
      float retTimeA = Utility.GetCurrentRealTime()
      String actorinfo = Parse_Sexes_And_Races(SexLab.GetSexAll(akPassed), akPassed)
      Config.Log("[SexLab MatchMaker] Unable to start Scene for Actors [" + actorinfo + "] / Tags " + tags + "; Aborting after " + (retTimeA - _StartCall))
      return
    EndIf
    sub = PapyrusUtil.RemoveActor(sub, sub[sub.Length - 1])
  EndWhile
EndFunction

Event AnimationStarted(int aiThread, bool abHasPlayer)
  float retTime = Utility.GetCurrentRealTime()
  UnregisterForUpdate()
  SexLabThread thread = SexLab.GetThread(aiThread)
  Debug.Notification("Scene started: " + SexLabRegistry.GetSceneName(thread.GetActiveScene()))
  Config.Log("[SexLab MatchMaker] - ###### START LOGGING SCENE DATA #####")
  Config.Log("[SexLab MatchMaker] - Startup time: " + (retTime - _StartCall))
  Config.Log("[SexLab MatchMaker] - Current thread id: " + thread.GetThreadID())
  Config.Log("[SexLab MatchMaker] - Current playing scene: " + thread.GetPlayingScenes())
  Config.Log("[SexLab MatchMaker] - Current active scene: " + thread.GetActiveScene())
  Config.Log("[SexLab MatchMaker] - Current scene name: " + SexLabRegistry.GetSceneName(thread.GetActiveScene()))
  Config.Log("[SexLab MatchMaker] - Current active stage: " + thread.GetActiveStage())
  Config.Log("[SexLab MatchMaker] - Current submissive actor(s): " + thread.GetSubmissives())
  Config.Log("[SexLab MatchMaker] - ###### END LOGGING SCENE DATA #####")
EndEvent

Event AnimationEnded(int aiThread, bool abHasPlayer)
  SexLabThread thread = SexLab.GetThread(aiThread)
  Config.Log("[SexLab MatchMaker] - Scene " + SexLabRegistry.GetSceneName(thread.GetActiveScene()) + " ended successfully")
EndEvent
