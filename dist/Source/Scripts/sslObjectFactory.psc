scriptname sslObjectFactory extends sslSystemLibrary
{
  LEGACY SCRIPT, DO NOT USE
  THIS SCRIPT IS STRICTLY REDUNDANT AND NON FUNCTIONAL
	ACCESSING IT IS **NOT** SUPPORTED AND **WILL** CREATE ISSUES
}

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

; ------------------------------------------------------- ;
; --- Readonly Flags                                  --- ;
; ------------------------------------------------------- ;

; Gender Types
int function Male() global
	return 0
endFunction
int function Female() global
	return 1
endFunction
int function MaleFemale() global
	return -1
endFunction
int function Creature() global
	return 2
endFunction
int function CreatureMale() global
	return 2
endFunction
int function CreatureFemale() global
	return 3
endFunction
; CumID Types
int function Vaginal() global
	return 1
endFunction
int function Oral() global
	return 2
endFunction
int function Anal() global
	return 3
endFunction
int function VaginalOral() global
	return 4
endFunction
int function VaginalAnal() global
	return 5
endFunction
int function OralAnal() global
	return 6
endFunction
int function VaginalOralAnal() global
	return 7
endFunction
; SFX Types
Sound function Squishing() global
	return Game.GetFormFromFile(0x65A31, "SexLab.esm") as Sound
endFunction
Sound function Sucking() global
	return Game.GetFormFromFile(0x65A32, "SexLab.esm") as Sound
endFunction
Sound function SexMix() global
	return Game.GetFormFromFile(0x65A33, "SexLab.esm") as Sound
endFunction
Sound function Squirting() global
	return Game.GetFormFromFile(0x65A34, "SexLab.esm") as Sound
endFunction
; MFG Types
int function Phoneme() global
	return 0
endFunction
int function Modifier() global
	return 16
endFunction
int function Expression() global
	return 30
endFunction

; ------------------------------------------------------- ;
; --- Ephemeral Animations                            --- ;
; ------------------------------------------------------- ;

sslBaseAnimation[] function GetOwnerAnimations(Form Owner)
	sslBaseAnimation[] ret
	return ret
endFunction
sslBaseAnimation function NewAnimation(string Token, Form Owner)
	return none
endFunction
sslBaseAnimation function GetSetAnimation(string Token, string Callback, Form Owner)
	return none
endFunction
sslBaseAnimation function NewAnimationCopy(string Token, sslBaseAnimation CopyFrom, Form Owner)
	return none
endFunction
sslBaseAnimation function GetAnimation(string Token)
	return none
endFunction
int function FindAnimation(string Token)
	return -1
endFunction
bool function HasAnimation(string Token)
	return false
endFunction
bool function ReleaseAnimation(string Token)
	return false
endFunction
int function ReleaseOwnerAnimations(Form Owner)
	return 0
endFunction
sslBaseAnimation function MakeAnimationRegistered(string Token)
	return none
endFunction

; ------------------------------------------------------- ;
; --- Ephemeral Voices                                --- ;
; ------------------------------------------------------- ;

sslBaseVoice[] function GetOwnerVoices(Form Owner)
	sslBaseVoice[] Output
	return Output
endFunction
sslBaseVoice function NewVoice(string Token, Form Owner)
	return none
endFunction
sslBaseVoice function GetSetVoice(string Token, string Callback, Form Owner)
	return none
endFunction
sslBaseVoice function NewVoiceCopy(string Token, sslBaseVoice CopyFrom, Form Owner)
	return none
endFunction
sslBaseVoice function GetVoice(string Token)
	return none
endFunction
int function FindVoice(string Token)
	return -1
endFunction
bool function HasVoice(string Token)
	return false
endFunction
bool function ReleaseVoice(string Token)
	return false
endFunction
int function ReleaseOwnerVoices(Form Owner)
	return 0
endFunction
sslBaseVoice function MakeVoiceRegistered(string Token)
	return none
endFunction

; ------------------------------------------------------- ;
; --- Ephemeral Expressions                           --- ;
; ------------------------------------------------------- ;

sslBaseExpression[] function GetOwnerExpressions(Form Owner)
	sslBaseExpression[] Output
	return Output
endFunction
sslBaseExpression function NewExpression(string Token, Form Owner)
	return none
endFunction
sslBaseExpression function GetSetExpression(string Token, string Callback, Form Owner)
	return none
endFunction
sslBaseExpression function NewExpressionCopy(string Token, sslBaseExpression CopyFrom, Form Owner)
	return none
endFunction
sslBaseExpression function GetExpression(string Token)
	return none
endFunction
int function FindExpression(string Token)
	return -1
endFunction
bool function HasExpression(string Token)
	return false
endFunction
bool function ReleaseExpression(string Token)
	return false
endFunction
int function ReleaseOwnerExpressions(Form Owner)
	return 0
endFunction
sslBaseExpression function MakeExpressionRegistered(string Token)
	return none
endFunction

; ------------------------------------------------------- ;
; --- System Use Only                                 --- ;
; ------------------------------------------------------- ;

function SendCallback(string Token, int Slot, Form CallbackForm = none, ReferenceAlias CallbackAlias = none) global
	if CallbackForm
		CallbackForm.RegisterForModEvent(Token, Token)
	endIf
	if CallbackAlias
		CallbackAlias.RegisterForModEvent(Token, Token)
	endIf
	int e = ModEvent.Create(Token)
	ModEvent.PushInt(e, Slot)
	ModEvent.Send(e)
	Utility.WaitMenuMode(0.5)
	if CallbackForm
		CallbackForm.UnregisterForModEvent(Token)
	endIf
	if CallbackAlias
		CallbackAlias.UnregisterForModEvent(Token)
	endIf
endFunction

function Cleanup()
endFunction

sslBaseAnimation function CopyAnimation(sslBaseAnimation Copy, sslBaseAnimation Orig)
	Copy.Registry = Copy.GOTTA_LOVE_PEOPLE_WHO_THINK_REGISTRATION_FUNCTIONS_ARE_JUST_DECORATION
	Copy.Registry = Orig.Registry
	return Copy
endFunction

sslBaseVoice function CopyVoice(sslBaseVoice Copy, sslBaseVoice Orig)
	Copy.Registry = Copy.GOTTA_LOVE_PEOPLE_WHO_THINK_REGISTRATION_FUNCTIONS_ARE_JUST_DECORATION
	Copy.Registry = Orig.Registry
	return Copy
endFunction

sslBaseExpression function CopyExpression(sslBaseExpression Copy, sslBaseExpression Orig)
	Copy.Registry = Copy.GOTTA_LOVE_PEOPLE_WHO_THINK_REGISTRATION_FUNCTIONS_ARE_JUST_DECORATION
	Copy.Registry = Orig.Registry
	return Copy
endFunction

; ------------------------------------------------------- ;
; --- DEPRECATED - DO NOT USE                         --- ;
; ------------------------------------------------------- ;

int function Misc() global
	return 0
endFunction
int function Sexual() global
	return 1
endFunction
int function Foreplay() global
	return 2
endFunction

