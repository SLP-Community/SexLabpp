scriptname sslVoiceSlots extends Quest
{
	Script for accessing voice data
}

; Selects matching voice for this actor. Returns saved voice if it exists
String Function SelectVoice(Actor akActor) native global
String Function SelectVoiceByTags(Actor akActor, String asTags) native global
String Function SelectVoiceByTagsA(Actor akActor, String[] asTags) native global

String Function GetSavedVoice(Actor akActor) native global
Function StoreVoice(Actor akActor, String asVoice) native global
Function DeleteVoice(Actor akActor) native global

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

String[] Function GetAllVoices(String asRaceKey) native global
Actor[] Function GetAllCachedUniqueActorsSorted(Actor akSecondPriority) native global
String Function SelectVoiceByRace(String asRaceKey) native global

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

string[] Property Registry Hidden
	String[] Function Get()
		SyncBackend()
		Alias[] aliases = GetAliases()
		String[] ret = Utility.CreateStringArray(aliases.Length)
		int i = 0
		int ii = 0
		While (i < aliases.Length)
			sslBaseVoice it = aliases[i] as sslBaseVoice
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
sslBaseVoice[] property Voices hidden
	sslBaseVoice[] function get()
		return GetSlots(1, 128)
	endFunction
endProperty

Function SyncBackend()
	Alias[] aliases = GetAliases()
	String[] arr = GetAllVoices("")
	int i = 0
	int ii = 0
	While (i < aliases.Length && ii < arr.Length)
		sslBaseVoice v = aliases[i] as sslBaseVoice
		If (v)
			v.Registry = v.GOTTA_LOVE_PEOPLE_WHO_THINK_REGISTRATION_FUNCTIONS_ARE_JUST_DECORATION
			v.Registry = arr[ii]
			ii += 1
		EndIf
		i += 1
	EndWhile
EndFunction

; Libraries
sslSystemConfig property Config hidden
	sslSystemConfig Function Get()
		return SexLabUtil.GetConfig()
	EndFunction
EndProperty
Actor property PlayerRef hidden
	Actor Function Get()
		return Game.GetPlayer()
	EndFunction
EndProperty

; ------------------------------------------------------- ;
; --- Voice Filtering                                 --- ;
; ------------------------------------------------------- ;

sslBaseVoice[] function FilterTaggedVoices(sslBaseVoice[] VoiceList, string[] Tags, bool HasTag = true) global
	if VoiceList.Length < 1
		return VoiceList
	elseIf Tags.Length < 1
		if HasTag
			return sslUtility.VoiceArray(0)
		endIf
		return VoiceList
	endIf
	int i = VoiceList.Length
	bool[] Valid = Utility.CreateBoolArray(i)
	while i
		i -= 1
		Valid[i] = VoiceList[i].HasOneTag(Tags) == HasTag
	endWhile
	; Check results
	if Valid.Find(true) == -1
		return sslUtility.VoiceArray(0) ; No valid animations
	elseIf Valid.Find(false) == -1
		return VoiceList ; All valid animations
	endIf
	; Filter output
	i = VoiceList.Length
	int n = PapyrusUtil.CountBool(Valid, true)
	sslBaseVoice[] Output = sslUtility.VoiceArray(n)
	while i && n
		i -= 1
		if Valid[i]
			n -= 1
			Output[n] = VoiceList[i]
		endIf
	endWhile
	return Output
endFunction

sslBaseVoice[] function GetAllGender(int Gender)
	bool[] Valid = Utility.CreateBoolArray(Slotted)
	int i = Slotted
	while i
		i -= 1
		sslBaseVoice Slot = GetBySlot(i)
		Valid[i] = Slot.Registered && Slot.Enabled && !Slot.Creature && (Gender == Slot.Gender || Slot.Gender == -1)
	endwhile
	return GetList(Valid)
endFunction

sslBaseVoice function PickGender(int Gender = 1)
	sslBaseVoice[] ret = GetAllGender(Gender)
	If (!ret.Length)
		return none
	EndIf
	return ret[Utility.RandomInt(0, ret.Length - 1)]
endFunction

sslBaseVoice function PickVoice(Actor ActorRef)
	String v = SelectVoice(ActorRef)
	If (!v)
		return none
	EndIf
	return GetbyRegistrar(v)

	; COMEBACK: Check what this all does. Might be interesting for native implementation
	; ; Pick a taged voice based on gender and scale
	; ActorBase BaseRef = ActorRef.GetLeveledActorBase()
	; float ActorScale = ActorRef.GetScale()
	; string Tags = "Male"
	; string SuppressTags = ""
	; string[] Filters
	; VoiceType ActorVoice = BaseRef.GetVoiceType()
	; string ActorVoiceString = ""
	; if ActorVoice
	; 	ActorVoiceString = ActorVoice as String
	; 	Log(ActorVoiceString)
	; 	if StringUtil.Find(ActorVoiceString, "Orc") >= 0 || StringUtil.Find(ActorVoiceString, "Brute") >= 0
	; 		Filters = PapyrusUtil.PushString(Filters, "Rough")
	; 	endIf
	; 	if StringUtil.Find(ActorVoiceString, "Toned") >= 0 || StringUtil.Find(ActorVoiceString, "Shrill") >= 0
	; 		Filters = PapyrusUtil.PushString(Filters, "Loud")
	; 	endIf
	; 	if StringUtil.Find(ActorVoiceString, "Sultry") >= 0
	; 		Filters = PapyrusUtil.PushString(Filters, "Excited")
	; 	endIf
	; 	if StringUtil.Find(ActorVoiceString, "Coward") >= 0
	; 		Filters = PapyrusUtil.PushString(Filters, "Quiet")
	; 	endIf
	; endIf
	; if BaseRef.GetSex() == 1
	; 	Tags = "Female"
	; endIf
	; if StringUtil.Find(ActorVoiceString, "Old") >= 0 || StringUtil.Find(ActorVoiceString, "Druk") >= 0 || StringUtil.Find(ActorVoiceString, "Khajiit") >= 0 || StringUtil.Find(ActorVoiceString, "Argonian") >= 0
	; 	SuppressTags = "Young"
	; 	Filters = PapyrusUtil.PushString(Filters, "Old")
	; elseIf StringUtil.Find(ActorVoiceString, "Young") >= 0 || ActorScale < 0.95
	; 	SuppressTags = "Old"
	; 	Filters = PapyrusUtil.PushString(Filters, "Young")
	; else
	; 	SuppressTags += ",Young,Old"
	; endif
	; sslBaseVoice[] VoiceList = GetAllByTags(Tags,SuppressTags)
	
	; sslBaseVoice[] Filtered = FilterTaggedVoices(VoiceList, Filters, true)
	; if Filtered.Length > 0 && VoiceList.Length > Filtered.Length
	; 	Log("Filtered out '"+(VoiceList.Length - Filtered.Length)+"' voices without the tags: "+Filters)
	; 	VoiceList = Filtered
	; endIf
	; if VoiceList && VoiceList.Length > 0
	; 	int i = (Utility.RandomInt(0, (VoiceList.Length - 1)))
	; 	if !IsPlayer && Config.NPCSaveVoice
	; 		SaveVoice(ActorRef, VoiceList[i])
	; 	endIf
	; 	return VoiceList[i]
	; endIf
	; ; Pick a random voice based on gender
	; sslBaseVoice Picked = PickGender(BaseRef.GetSex())
	; ; Save the voice to NPC for reuse, if enabled
	; if Picked && !IsPlayer && Config.NPCSaveVoice
	; 	SaveVoice(ActorRef, Picked)
	; endIf
	; return Picked
endFunction

sslBaseVoice function GetByTags(string Tags, string TagsSuppressed = "", bool RequireAll = true)
	sslBaseVoice[] Found = GetAllByTags(Tags, TagsSuppressed, RequireAll)
	if Found.Length
		return Found[(Utility.RandomInt(0, (Found.Length - 1)))]
	endIf
	return none
endFunction

sslBaseVoice[] function GetAllByTags(string Tags, string TagsSuppressed = "", bool RequireAll = true)
	String[] arg = SexLabUtil.MergeSplitTags(Tags, TagsSuppressed, RequireAll)
	String v = SelectVoiceByTagsA(none, arg)
	If (!v)
		return sslUtility.VoiceArray(0)
	EndIf
	sslBaseVoice[] ret = new sslBaseVoice[1]
	ret[0] = GetbyRegistrar(v)
	return ret
endFunction

sslBaseVoice function PickByRaceKey(string RaceKey)
	String v = SelectVoiceByRace(RaceKey)
	If (!v)
		return none
	EndIf
	return GetbyRegistrar(v)
endFunction

int function FindSaved(Actor ActorRef)
	return FindByRegistrar(GetSaved(ActorRef))
endFunction

sslBaseVoice function GetSaved(Actor ActorRef)
	String v = GetSavedVoice(ActorRef)
	If (!v)
		return none
	EndIf
	return GetbyRegistrar(v)
endFunction

string function GetSavedName(Actor ActorRef)
	If (!ActorRef)
		return "$SSL_Random"
	EndIf
	String v = GetSavedVoice(ActorRef)
	If (!v)
		return "$SSL_Random"
	EndIf
	return v
endFunction

function SaveVoice(Actor ActorRef, sslBaseVoice Saving)
	StoreVoice(ActorRef, Saving.Registry)
endFunction

function ForgetVoice(Actor ActorRef)
	DeleteVoice(ActorRef)
endFunction

bool function HasCustomVoice(Actor ActorRef)
	return GetSavedVoice(ActorRef)
endFunction

; ------------------------------------------------------- ;
; --- Slotting Common                                 --- ;
; ------------------------------------------------------- ;

sslBaseVoice[] function GetList(bool[] Valid)
	sslBaseVoice[] Output
	if Valid.Length > 0 && Valid.Find(true) != -1
		int n = Valid.Find(true)
		int i = PapyrusUtil.CountBool(Valid, true)
		; Trim over 100 to random selection
		if i > 100
			int end = Valid.RFind(true) - 1
			while i > 100
				int rand = Valid.Find(true, Utility.RandomInt(n, end))
				if rand != -1 && Valid[rand]
					Valid[rand] = false
					i -= 1
				endIf
				if i == 101 ; To be sure only 100 stay
					i = PapyrusUtil.CountBool(Valid, true)
					n = Valid.Find(true)
					end = Valid.RFind(true) - 1
				endIf
			endWhile
		endIf
		; Get list
		Output = sslUtility.VoiceArray(i)
		while n != -1 && i > 0
			i -= 1
			Output[i] = GetNthAlias(n) as sslBaseVoice
			n += 1
			if n < Slotted
				n = Valid.Find(true, n)
			else
				n = -1
			endIf
		endWhile
	endIf
	return Output
endFunction

string[] function GetNames(sslBaseVoice[] SlotList)
	int i = SlotList.Length
	string[] Names = Utility.CreateStringArray(i)
	while i
		i -= 1
		if SlotList[i]
			Names[i] = SlotList[i].Name
		endIf
	endWhile
	if Names.Find("") != -1
		Names = PapyrusUtil.RemoveString(Names, "")
	endIf
	return Names
endFunction

; ------------------------------------------------------- ;
; --- Registry Access                                     ;
; ------------------------------------------------------- ;

sslBaseVoice function GetBySlot(int index)
	if index < 0 || index >= GetNumAliases()
		return none
	endIf
	return GetNthAlias(index) as sslBaseVoice
endFunction

bool function IsRegistered(string Registrar)
	return FindByRegistrar(Registrar) != -1
endFunction

int function FindByRegistrar(string Registrar)
	if Registrar != ""
		return Registry.Find(Registrar)
	endIf
	return -1
endFunction

int function FindByName(string FindName)
	return FindByRegistrar(FindName)
endFunction

sslBaseVoice function GetByName(string FindName)
	return GetBySlot(FindByName(FindName))
endFunction

sslBaseVoice function GetbyRegistrar(string Registrar)
	return GetBySlot(FindByRegistrar(Registrar))
endFunction

; ------------------------------------------------------- ;
; --- Object MCM Pagination                               ;
; ------------------------------------------------------- ;

int function PageCount(int perpage = 125)
	return ((Slotted as float / perpage as float) as int) + 1
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

sslBaseVoice[] function GetSlots(int page = 1, int perpage = 125)
	perpage = PapyrusUtil.ClampInt(perpage, 1, 128)
	if page > PageCount(perpage) || page < 1
		return sslUtility.VoiceArray(0)
	endIf
	int n
	sslBaseVoice[] PageSlots
	if page == PageCount(perpage)
		n = Slotted
		PageSlots = sslUtility.VoiceArray((Slotted - ((page - 1) * perpage)))
	else
		n = page * perpage
		PageSlots = sslUtility.VoiceArray(perpage)
	endIf
	int i = PageSlots.Length
	while i
		i -= 1
		n -= 1
		PageSlots[i] = GetNthAlias(n) as sslBaseVoice
	endWhile
	return PageSlots
endFunction

string[] function GetNormalSlotNames(bool WithRandom = false)
	string[] Output = Utility.CreateStringArray(GetCount(1) + (WithRandom as int))
	int n = Output.Length
	int i = Slotted
	while i
		i -= 1
		sslBaseVoice Voice = GetBySlot(i)
		if Voice && !Voice.Creature
			n -= 1
			Output[n] = Voice.Name
		endIf
	endWhile
	if WithRandom
		Output[0] = "$SSL_Random"
	endIf
	return Output
endFunction

int function GetCount(int flag = 0) ; 0 = all, 1 = normal, -1 = creatures
	if flag == 0
		return Slotted
	endIf
	int count
	int i = Slotted
	while i
		i -= 1
		count += (GetBySlot(i).Creature == (flag == -1)) as int
	endWhile
	return count
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
	If (ret > -1 && !sslBaseVoice.InitializeVoiceObject(Registrar))
		RegisterLock = false
		return -1
	EndIf
	RegisterLock = false
	return ret
endFunction

sslBaseVoice function RegisterVoice(string Registrar, Form CallbackForm = none, ReferenceAlias CallbackAlias = none)
	; Return existing Voice
	if Registrar == "" || FindByRegistrar(Registrar) != -1
		return GetbyRegistrar(Registrar)
	endIf
	; Get free Voice slot
	int id = Register(Registrar)
	sslBaseVoice Slot = GetBySlot(id)
	if id != -1 && Slot != none
		Slot.Initialize()
		Slot.Registry = Slot.GOTTA_LOVE_PEOPLE_WHO_THINK_REGISTRATION_FUNCTIONS_ARE_JUST_DECORATION
		Slot.Registry = Registrar
		Slot.Enabled  = true
		sslObjectFactory.SendCallback(Registrar, id, CallbackForm, CallbackAlias)
	endIf
	return Slot
endFunction

function RegisterSlots()
endFunction

bool function UnregisterVoice(string Registrar)
	return false
endFunction

; ------------------------------------------------------- ;
; --- System Use Only                                 --- ;
; ------------------------------------------------------- ;

function Setup()
endFunction

function Log(string msg)
	sslLog.Log(msg)
endFunction

bool function TestSlots()
	return true
endFunction
