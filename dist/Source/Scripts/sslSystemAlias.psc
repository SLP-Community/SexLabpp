scriptname sslSystemAlias extends ReferenceAlias
{
	Internal Script to manage script re/initialization
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

; Framework
SexLabFramework property SexLab auto
sslSystemConfig property Config auto
sslThreadLibrary property ThreadLib auto
sslThreadSlots property ThreadSlots auto

bool property IsInstalled hidden
	bool function get()
		return SexLab.GetState() == "Enabled"
	endFunction
endProperty

; ------------------------------------------------------- ;
; --- System Startup                                  --- ;
; ------------------------------------------------------- ;

Event OnInit()
	If (!GetReference())
		return
	ElseIf (!Config.CheckSystem())
		return
	EndIf
	Quest UnboundQ = Quest.GetQuest("MQ101")
	While (UnboundQ.GetStage() < 1000 && !UnboundQ.GetStageDone(250) && UnboundQ.GetStage() > 0)
		Utility.Wait(30.0)
	EndWhile
	InstallSystem()
EndEvent

Event OnPlayerLoadGame()
	If (!Config.CheckSystem())
		return
	ElseIf (IsInstalled)
		Config.Reload()
		ThreadSlots.StopAll()
		ModEvent.Send(ModEvent.Create("SexLabGameLoaded"))
	Else
		InstallSystem()
	EndIf
EndEvent

; ------------------------------------------------------- ;
; --- System Install/Update                           --- ;
; ------------------------------------------------------- ;

bool Function SetupSystem()
	SexLab.GoToState("Disabled")
	LoadLibs()
	SexLab.Setup()
	Config.Setup()
	ThreadLib.Setup()
	ThreadSlots.Setup()
	SexLab.GoToState("Enabled")
	sslLog.Log("SexLab v" + SexLabUtil.GetStringVer() + " - Ready!", true)
	return true
EndFunction

Event InstallSystem()
	LogAll("SexLab v" + SexLabUtil.GetStringVer() + " - Installing...")
	SetupSystem()
	int eid = ModEvent.Create("SexLabInstalled")
	ModEvent.PushInt(eid, SexLabUtil.GetVersion())
	ModEvent.Send(eid)
EndEvent

function LoadLibs(bool Forced = false)
	Form SexLabQuestFramework = Game.GetFormFromFile(0xD62, "SexLab.esm")
	SexLab      = SexLabQuestFramework as SexLabFramework
	Config      = SexLabQuestFramework as sslSystemConfig
	ThreadLib   = SexLabQuestFramework as sslThreadLibrary
	ThreadSlots = SexLabQuestFramework as sslThreadSlots
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

bool property PreloadDone = true auto hidden

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
sslObjectFactory property Factory Hidden
	sslObjectFactory Function Get()
		return Game.GetFormFromFile(0x78818, "SexLab.esm") as sslObjectFactory
	EndFunction
EndProperty
sslActorStats property Stats Hidden
	sslActorStats Function Get()
		return Game.GetFormFromFile(0xD62, "SexLab.esm") as sslActorStats
	EndFunction
EndProperty
sslExpressionSlots property ExpressionSlots hidden
	sslExpressionSlots Function Get()
		return Game.GetFormFromFile(0x664FB, "SexLab.esm") as sslExpressionSlots
	EndFunction
EndProperty
sslVoiceSlots property VoiceSlots Hidden
  sslVoiceSlots Function Get()
	  return Game.GetFormFromFile(0x664FB, "SexLab.esm") as sslVoiceSlots
  EndFunction
EndProperty
sslActorLibrary property ActorLib Hidden
	sslActorLibrary Function Get()
		return Game.GetFormFromFile(0xD62, "SexLab.esm") as sslActorLibrary
	EndFunction
EndProperty

function Log(string Log, string Type = "NOTICE")
	sslLog.Log(Log)
endFunction
function LogAll(string Log)
	sslLog.Log(Log, true)
endFunction

bool property UpdatePending hidden
	bool function get()
		return false
	endFunction
endProperty

int property CurrentVersion hidden
	int function get()
		return SexLabUtil.GetVersion()
	endFunction
endProperty

function CleanTrackedActors()
endFunction

function CleanTrackedFactions()
endFunction

event UpdateSystem(int OldVersion, int NewVersion)
endEvent

function SendVersionEvent(string VersionEvent)
	int eid = ModEvent.Create(VersionEvent)
	ModEvent.PushInt(eid, SexLabUtil.GetVersion())
	ModEvent.Send(eid)
endFunction

bool function IsActor(Form FormRef) global
	if FormRef
		int Type = FormRef.GetType()
		return Type == 43 || Type == 44 || Type == 62 ; kNPC = 43 kLeveledCharacter = 44 kCharacter = 62
	endIf
	return false
endFunction

function MenuWait()
	Utility.Wait(0.1)
endFunction

; ------------------------------------------------------- ;
; --- System Cleanup                                  --- ;
; ------------------------------------------------------- ;

function CleanActorStorage()
endFunction

function ClearFromActorStorage(Form FormRef)
endFunction

bool function IsImportant(Actor ActorRef, bool Strict = false) global
	if ActorRef == Game.GetPlayer()
		return true
	elseIf !ActorRef || ActorRef.IsDead() || ActorRef.IsDeleted() || ActorRef.IsChild()
		return false
	elseIf !Strict
		return true
	endIf
	ActorBase BaseRef = ActorRef.GetLeveledActorBase()
	return BaseRef.IsUnique() || BaseRef.IsEssential() || BaseRef.IsInvulnerable() || BaseRef.IsProtected() || ActorRef.IsPlayerTeammate() || ActorRef.Is3DLoaded()
endFunction
