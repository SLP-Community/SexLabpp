scriptname sslExpressionSlots extends Quest
{
	Script for handling Expression logic
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

int Property STATUS_NONE 				= 0 AutoReadOnly Hidden
int Property STATUS_SUBMISSIVE 	= 1 AutoReadOnly Hidden
int Property STATUS_DOMINANT		= 2 AutoReadOnly Hidden

String[] Function GetAllProfileIDs() native global
String[] Function GetExpressionsByStatus(Actor akActor, int aiVictimStatus) native global
String[] Function GetExpressionsByTags(Actor akActor, String asTags) native global

; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;
; ----------------------------------------------------------------------------- ;
;								██╗     ███████╗ ██████╗  █████╗  ██████╗██╗   ██╗							;
;								██║     ██╔════╝██╔════╝ ██╔══██╗██╔════╝╚██╗ ██╔╝							;
;								██║     █████╗  ██║  ███╗███████║██║      ╚████╔╝ 							;
;								██║     ██╔══╝  ██║   ██║██╔══██║██║       ╚██╔╝  							;
;								███████╗███████╗╚██████╔╝██║  ██║╚██████╗   ██║   							;
;								╚══════╝╚══════╝ ╚═════╝ ╚═╝  ╚═╝ ╚═════╝   ╚═╝   							;
; ----------------------------------------------------------------------------- ;
; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-* ;

String[] Property Registry
	String[] Function Get()
		SyncBackend()
		Alias[] aliases = GetAliases()
		String[] ret = Utility.CreateStringArray(aliases.Length)
		int i = 0
		int ii = 0
		While (i < aliases.Length)
			sslBaseExpression it = aliases[i] as sslBaseExpression
			If (!it)
				i = aliases.Length
			ElseIf (it.Registered)
				ret[ii] = it.Name
				ii += 1
			EndIf
			i += 1
		EndWhile
		return PapyrusUtil.ClearEmpty(ret)
	EndFunction
EndProperty
int property Slotted hidden
	int Function Get()
		return Registry.Length
	EndFunction
EndProperty
sslBaseExpression[] property Expressions hidden
	sslBaseExpression[] function get()
		return GetSlots(1)
	endFunction
endProperty

Function SyncBackend()
	Alias[] aliases = GetAliases()
	String[] profiles = GetAllProfileIDs()
	int i = 0
	int ii = 0
	While (i < aliases.Length && ii < profiles.Length)
		sslBaseExpression expr = aliases[i] as sslBaseExpression
		If (expr)
			expr.Registry = expr.GOTTA_LOVE_PEOPLE_WHO_THINK_REGISTRATION_FUNCTIONS_ARE_JUST_DECORATION
			expr.Registry = profiles[ii]
			ii += 1
		EndIf
		i += 1
	EndWhile
EndFunction

Actor property PlayerRef
	Actor Function Get()
		return Game.GetPlayer()
	EndFunction
EndProperty

sslSystemConfig property Config
	sslSystemConfig Function Get()
		return SexLabutil.GetConfig()
	EndFunction
EndProperty

; ------------------------------------------------------- ;
; --- Expression Filtering                            --- ;
; ------------------------------------------------------- ;

sslBaseExpression function PickExpression(Actor ActorRef, Actor VictimRef = none)
	return PickByStatus(ActorRef, (VictimRef && VictimRef == ActorRef), (VictimRef && VictimRef != ActorRef))
endFunction

sslBaseExpression function PickByStatus(Actor ActorRef, bool IsVictim = false, bool IsAggressor = false)
	sslBaseExpression[] ret = GetByStatus(ActorRef, IsVictim, IsAggressor)
	If (!ret.Length)
		return none
	EndIf
	return ret[Utility.RandomInt(0, ret.length)]
endFunction

sslBaseExpression[] function GetByStatus(Actor ActorRef, bool IsVictim = false, bool IsAggressor = false)
	string Tag
	if IsVictim
		Tag = "Victim"
	elseIf IsAggressor
		Tag = "Aggressor"
	else
		Tag = "Normal"
	endIf
	return GetByTag(Tag, ActorRef.GetLeveledActorBase().GetSex() == 1)
endFunction

sslBaseExpression function RandomByTag(string Tag, bool ForFemale = true)
	sslBaseExpression[] ret = GetByTag(Tag, ForFemale)
	If (!ret.Length)
		return none
	EndIf
	return ret[Utility.RandomInt(0, ret.length)]
endFunction

sslBaseExpression[] function GetByTag(string Tag, bool ForFemale = true)
	bool[] Valid = Utility.CreateBoolArray(Slotted)
	Alias[] aliases = GetAliases()
	int i = 0
	int ii = 0
	While (i < aliases.Length)
		sslBaseExpression it = aliases[i] as sslBaseExpression
		If (it && it.Registered)
			Valid[ii] = it.Enabled && it.HasTag(Tag) && ((ForFemale && it.PhasesFemale > 0) || (!ForFemale && it.PhasesMale > 0))
			ii += 1
		EndIf
		i += 1
	EndWhile
	return GetList(Valid)
endFunction

sslBaseExpression function SelectRandom(bool[] Valid)
	int n = Utility.RandomInt(0, (Slotted - 1))
	int Slot = Valid.Find(true, n)
	if Slot == -1
		Slot = Valid.RFind(true, n)
	endIf
	Alias[] aliases = GetAliases()
	int i = 0
	While (i < aliases.Length)
		sslBaseExpression it = aliases[i] as sslBaseExpression
		If (slot == 0)
			return it
		Else
			Slot -= 1
		EndIf
		i += 1
	EndWhile
	return none
endFunction

; ------------------------------------------------------- ;
; --- Slotting Common                                 --- ;
; ------------------------------------------------------- ;

sslBaseExpression[] function GetList(bool[] Valid)
	sslBaseExpression[] Output
	If (Valid.Length <= 0 || Valid.Find(true) == -1)
		return Output
	EndIf
	Output = sslUtility.ExpressionArray(PapyrusUtil.CountBool(Valid, true))
	Alias[] aliases = GetAliases()
	int i = 0
	int ii = 0
	While (i < aliases.Length && ii < Output.Length)
		sslBaseExpression it = aliases[i] as sslBaseExpression
		If (it && it.Registered && Valid[ii])
			Output[ii] = it
			ii += 1
		EndIf
		i += 1
	EndWhile
	return Output
endFunction

string[] function GetNames(sslBaseExpression[] SlotList)
	int i = SlotList.Length
	string[] Names = Utility.CreateStringArray(i)
	while i
		i -= 1
		if SlotList[i]
			Names[i] = SlotList[i].Name
		endIf
	endWhile
	return PapyrusUtil.ClearEmpty(Names)
endFunction

; ------------------------------------------------------- ;
; --- Registry Access                                     ;
; ------------------------------------------------------- ;

sslBaseExpression function GetBySlot(int index)
	if index < 0 || index >= GetNumAliases()
		return none
	endIf
	return GetNthAlias(index) as sslBaseExpression
endFunction

bool function IsRegistered(string Registrar)
	return FindByRegistrar(Registrar) != -1
endFunction

int function FindByRegistrar(string Registrar)
	If (Registrar == "")
		return -1
	EndIf
	Alias[] aliases = GetAliases()
	int i = 0
	While (i < aliases.Length)
		sslBaseExpression it = aliases[i] as sslBaseExpression
		If (it && it.Registry == Registrar)
			return i
		EndIf
		i += 1
	EndWhile
	return -1
endFunction

int function FindByName(string FindName)
	return FindByRegistrar(FindName)
endFunction

sslBaseExpression function GetByName(string FindName)
	return GetBySlot(FindByName(FindName))
endFunction

sslBaseExpression function GetbyRegistrar(string Registrar)
	return GetBySlot(FindByRegistrar(Registrar))
endFunction

; ------------------------------------------------------- ;
; --- Object MCM Pagination                               ;
; ------------------------------------------------------- ;

int function PageCount(int perpage = 125)
	return Math.Ceiling(Slotted as float / perpage as float)
endFunction

int function FindPage(string Registrar, int perpage = 125)
	int i = Registry.Find(Registrar)
	if i != -1
		return (i / perpage) + 1
	endIf
	return -1
endFunction

string[] function GetSlotNames(int page = 1, int perpage = 125)
	return GetNames(GetSlots(page, perpage))
endfunction

sslBaseExpression[] function GetSlots(int page = 1, int perpage = 125)
	SyncBackend()
	perpage = PapyrusUtil.ClampInt(perpage, 1, 128)
	if page > PageCount(perpage) || page < 1
		return sslUtility.ExpressionArray(0)
	endIf
	sslBaseExpression[] PageSlots
	int skippages = (page - 1) * perpage
	if page == PageCount(perpage)
		PageSlots = sslUtility.ExpressionArray(Slotted - skippages)
	else
		PageSlots = sslUtility.ExpressionArray(perpage)
	endIf
	Alias[] aliases = GetAliases()
	int i = 0
	int ii = 0
	While (i < aliases.Length && ii < PageSlots.Length)
		sslBaseExpression it = aliases[i] as sslBaseExpression
		If (it && it.Registered)
			If (skippages == 0)
				PageSlots[ii] = it
				ii += 1
			Else
				skippages -= 1
			EndIf
		EndIf
		i += 1
	EndWhile
	return PageSlots
endFunction

; ------------------------------------------------------- ;
; --- Object Registration                                 ;
; ------------------------------------------------------- ;

int Function FindEmpty()
	int n = Slotted
	If (GetNthAlias(n + 1))
		return n + 1
	EndIf
	return -1
EndFunction

bool RegisterLock = false
int function Register(string Registrar)
	if Registrar == "" || Registry.Find(Registrar) != -1
		return -1
	endIf
	while RegisterLock
		Utility.WaitMenuMode(0.5)
	endWhile
	RegisterLock = true
	int ret = FindEmpty()
	If (ret == -1 || !sslBaseExpression.CreateEmptyProfile(Registrar))
		sslBaseExpression it = GetBySlot(ret) as sslBaseExpression
		it.Registry = it.GOTTA_LOVE_PEOPLE_WHO_THINK_REGISTRATION_FUNCTIONS_ARE_JUST_DECORATION
		it.Registry = Registrar
		RegisterLock = false
		return -1
	EndIf
	RegisterLock = false
	return ret
endFunction

sslBaseExpression function RegisterExpression(string Registrar, Form CallbackForm = none, ReferenceAlias CallbackAlias = none)
	SyncBackend()
	Alias[] aliases = GetAliases()
	int i = 0
	While (i < aliases.Length)
		sslBaseExpression it = aliases[i] as sslBaseExpression
		If (it && it.Registry == Registrar)
			sslObjectFactory.SendCallback(Registrar, i, CallbackForm, CallbackAlias)
			return it
		EndIf
		i += 1
	EndWhile
	int where = Register(Registrar)
	If (where == -1)
		return none
	EndIf
	sslObjectFactory.SendCallback(Registrar, where, CallbackForm, CallbackAlias)
	return GetNthAlias(where) as sslBaseExpression
endFunction

function RegisterSlots()
endFunction

bool function UnregisterExpression(string Registrar)
	return false
endFunction

; ------------------------------------------------------- ;
; --- System Use Only                                 --- ;
; ------------------------------------------------------- ;

function Setup()
endFunction

bool function TestSlots()
	return true
endFunction

function Log(string msg)
	sslLog.Log(msg)
endFunction
