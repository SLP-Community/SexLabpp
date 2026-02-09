scriptname sslBaseObject extends ReferenceAlias hidden

String _name
string Property Name
	String Function Get()
		return _GetName()
	EndFunction
	Function Set(String aSet)
		_SetName(aSet)
	EndFunction
EndProperty
String Function _GetName()
	return _name
EndFunction
Function _SetName(String aSet)
	_name = aSet
EndFunction

bool _enabled
bool Property Enabled
	bool Function Get()
		return _GetEnabled()
	EndFunction
	Function Set(bool aSet)
		_SetEnabled(aSet)
	EndFunction
EndProperty
bool Function _GetEnabled()
	return _enabled
EndFunction
Function _SetEnabled(bool aSet)
	_enabled
EndFunction

String _registryID
string Property Registry
	String Function Get()
		return _GetRegistryID()
	EndFunction
	Function Set(String asSet)
		_SetRegistryID(asSet)
	EndFunction
EndProperty
bool Property Registered hidden
	bool Function get()
		return Registry != ""
	EndFunction
EndProperty
String Function _GetRegistryID()
	return _registryID
EndFunction
Function _SetRegistryID(String asSet)
	_registryID = asSet
EndFunction

String Property GOTTA_LOVE_PEOPLE_WHO_THINK_REGISTRATION_FUNCTIONS_ARE_JUST_DECORATION = "This variable exists because certain mods (Im aware only of one though, which is unfortunately a little too popular to just ignore it) registers SSL objects manually, that is, without using the pre-established registration functions. This creates a little bit of an issue with the backwards compatibility function P+ has implemented, since there is no clear entry point where registration of legacy objects happens (and if Im just stupid and it does exist (which it probably does), it is too complicated and time intensive to properly isolate without risking issues with mods that do follow the intended registration process and thus delaying work on more important aspects of the framework). Thus, this variable serves the sole purpose of acting as a marker, to let the framework now that the setting of the registry in this call, and the next following one, should be ignored by the system, that is, it will not attempt to create a new legacy object in the main system, which may have happened otherwise, creating an error in either the .dll log or in the Papyrus log due to the uniqueness property of the object then being harmed (as the frameworks 'official' way of integrating would have already created the object explicitely beforehand)" AutoReadOnly Hidden

; ------------------------------------------------------- ;
; --- Tagging System                                  --- ;
; ------------------------------------------------------- ;

String[] _Tags
string[] Property Tags Hidden
	String[] Function Get()
		return _GetTags()
	EndFunction
	Function Set(String[] asSet)
		_SetTags(asSet)
	EndFunction
EndProperty
String[] Function _GetTags()
	return _Tags
EndFunction
Function _SetTags(String[] asSet)
	_Tags = asSet
EndFunction

string[] function GetTags()
	return PapyrusUtil.ClearEmpty(Tags)
endFunction

bool Function HasTag(string Tag)
	return Tag && !Tags.Length || Tags.Find(Tag) != -1
EndFunction

bool function AddTag(string Tag)
	if Tag != "" && !Tags.Length || Tags.Find(Tag) == -1
		Tags = PapyrusUtil.PushString(Tags, Tag)
		return true
	endIf
	return false
endFunction

bool function RemoveTag(string Tag)
	if Tag != "" && !Tags.Length || Tags.Find(Tag) != -1
		Tags = PapyrusUtil.RemoveString(Tags, Tag)
		return true
	endIf
	return false
endFunction

function AddTags(string[] TagList)
	int i = TagList.Length
	while i
		i -= 1
		AddTag(TagList[i])
	endWhile
endFunction

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

sslSystemConfig Property Config Hidden
	sslSystemConfig Function Get()
		return SexLabUtil.GetConfig()
	EndFunction
EndProperty

function Log(string Log, string Type = "NOTICE")
	sslLog.Log(Log)
endFunction

function Initialize()
	Registry = ""
endFunction

; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*	;
;																																											;
;									██╗     ███████╗ ██████╗  █████╗  ██████╗██╗   ██╗									;
;									██║     ██╔════╝██╔════╝ ██╔══██╗██╔════╝╚██╗ ██╔╝									;
;									██║     █████╗  ██║  ███╗███████║██║      ╚████╔╝ 									;
;									██║     ██╔══╝  ██║   ██║██╔══██║██║       ╚██╔╝  									;
;									███████╗███████╗╚██████╔╝██║  ██║╚██████╗   ██║   									;
;									╚══════╝╚══════╝ ╚═════╝ ╚═╝  ╚═╝ ╚═════╝   ╚═╝   									;
;																																											;
; *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*--*-*-*-*-*-*-*-*-*-*-*-*-*-**-*-*-*-*-*-*	;

int Property SlotID Hidden
	int Function Get()
		return -1
	EndFunction
EndProperty

bool Property Saved hidden
	bool function get()
		return true
	endFunction
endProperty
function Save(int id = -1)
endFunction

string function Key(string type = "")
	return Registry+"."+type
endFunction

string[] function GetRawTags()
	return GetTags()
endFunction

bool function CheckTags(string[] CheckTags, bool RequireAll = true, bool Suppress = false)
	bool Valid = ParseTags(CheckTags, RequireAll)
	return (Valid && !Suppress) || (!Valid && Suppress)
endFunction

bool function ParseTags(string[] TagList, bool RequireAll = true)
	return (RequireAll && HasAllTag(TagList)) || (!RequireAll && HasOneTag(TagList))
endFunction

bool function TagSearch(string[] TagList, string[] Suppress, bool RequireAll)
	return ((RequireAll && HasAllTag(TagList)) || (!RequireAll && HasOneTag(TagList))) \ 
		&& (!Suppress || !HasOneTag(Suppress))
endFunction

bool function HasOneTag(string[] TagList)
	int i = TagList.Length
	while i
		i -= 1
		if HasTag(TagList[i])
			return true
		endIf
	endWhile
	return false
endFunction

bool function HasAllTag(string[] TagList)
	int i = TagList.Length
	while i
		i -= 1
		if (!HasTag(TagList[i]))
			return false
		endIf
	endWhile
	return true
endFunction

bool function AddTagConditional(string Tag, bool AddTag)
	If(AddTag)
		return AddTag(Tag)
	Else
		return RemoveTag(Tag)
	EndIf
endFunction

function SetTags(string TagList)
	AddTags(PapyrusUtil.StringSplit(TagList))
endFunction

bool function ToggleTag(string Tag)
	return (RemoveTag(Tag) || AddTag(Tag)) && HasTag(Tag)
endFunction

Form Property Storage = none Auto Hidden
bool Property Ephemeral hidden
	bool function get()
		return Storage != none
	endFunction
endProperty

function MakeEphemeral(string Token, Form OwnerForm)
	Log("MakeEphemeral() is no longer supported; '" + Token + "'", Storage)
endFunction
