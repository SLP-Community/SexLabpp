ScriptName sslActorLibrary extends sslSystemLibrary
{
	Threading/Animation related Actor specific utility
}

; ------------------------------------------------------- ;
; --- Actor Effects Functions                         --- ;
; ------------------------------------------------------- ;

String Property ACTIVE_SET_PREFIX = "SexLabActiveSet" AutoReadOnly Hidden
String Property ACTIVE_LAYER_PREFIX = "SexLabActiveLayer" AutoReadOnly Hidden
String Property LAST_APPLIED_TIME_PREFIX = "SexLabLastAppliedTime" AutoReadOnly Hidden
String Property LAST_APPLIED_TEXTURE_PREFIX = "SexLabLastAppliedTexture" AutoReadOnly Hidden
String Property APPLIED_TEXTURE_LIST = "SexLabAppliedTextureList" AutoReadOnly Hidden

int Property FX_ALL = -1 AutoReadOnly Hidden
int Property FX_VAGINAL = 0 AutoReadOnly Hidden
int Property FX_ANAL = 1 AutoReadOnly Hidden
int Property FX_ORAL = 2 AutoReadOnly Hidden

Spell Property abCumFX Auto
Spell property CumVaginalSpell Auto
Spell property CumOralSpell Auto
Spell property CumAnalSpell Auto

Function AddCumFx(Actor akActor, int aiType)
	If (!akActor.HasSpell(abCumFX))
		akActor.AddSpell(abCumFX)
	EndIf
	If (aiType == FX_ALL)
		BeginOverlay(akActor, FX_VAGINAL)
		BeginOverlay(akActor, FX_ANAL)
		BeginOverlay(akActor, FX_ORAL)
		akActor.AddSpell(CumVaginalSpell)
		akActor.AddSpell(CumOralSpell)
		akActor.AddSpell(CumAnalSpell)
	Else
		If (aiType == FX_VAGINAL)
			akActor.AddSpell(CumVaginalSpell)
		ElseIf (aiType == FX_ORAL)
			akActor.AddSpell(CumOralSpell)
		ElseIf (aiType == FX_ANAL)
			akActor.AddSpell(CumAnalSpell)
		EndIf
		BeginOverlay(akActor, aiType)
	EndIf
	int handle = ModEvent.Create("SexLabApplyCum")
	ModEvent.PushForm(handle, akActor)
	ModEvent.PushInt(handle, aiType)
	ModEvent.Send(handle)
EndFunction

Function RemoveCumFx(Actor akTarget, int aiType)
	If (aiType == FX_ALL)
		RemoveCumFx(akTarget, FX_VAGINAL)
		RemoveCumFx(akTarget, FX_ANAL)
		RemoveCumFx(akTarget, FX_ORAL)
		return
	EndIf
	int removed = StorageUtil.IntListRemove(akTarget, APPLIED_TEXTURE_LIST, aiType)
	If (removed == 0)
		return
	EndIf
	Bool isFemale = akTarget.GetLeveledActorBase().GetSex() == 1
	String LastEffect = StorageUtil.GetStringValue(akTarget, LAST_APPLIED_TEXTURE_PREFIX + aiType)
	RemovePartOverlay(akTarget, isFemale, LastEffect)
	If (aiType == FX_VAGINAL)
		akTarget.RemoveSpell(CumVaginalSpell)
	ElseIf (aiType == FX_ORAL)
		akTarget.RemoveSpell(CumOralSpell)
	ElseIf (aiType == FX_ANAL)
		akTarget.RemoveSpell(CumAnalSpell)
	EndIf
	StorageUtil.UnsetIntValue(akTarget, ACTIVE_SET_PREFIX + aiType)
	StorageUtil.UnsetIntValue(akTarget, ACTIVE_LAYER_PREFIX + aiType)
	StorageUtil.UnsetFloatValue(akTarget, LAST_APPLIED_TIME_PREFIX + aiType)
	StorageUtil.UnsetStringValue(akTarget, LAST_APPLIED_TEXTURE_PREFIX + aiType)
	If (StorageUtil.IntListCount(akTarget, APPLIED_TEXTURE_LIST) == 0)
		akTarget.RemoveSpell(abCumFX)
	EndIf
	int handle = ModEvent.Create("SexLabClearCum")
	ModEvent.PushForm(handle, akTarget)
	ModEvent.PushInt(handle, aiType)
	ModEvent.Send(handle)
EndFunction

int Function CountCumFx(Actor akActor, int aiType)
	return StorageUtil.CountObjIntValuePrefix(akActor, ACTIVE_LAYER_PREFIX + aiType)
EndFunction

; ------------------------------------------------------- ;
; --- Equipment Functions                             --- ;
; ------------------------------------------------------- ;

; Flag/Clear an item for special strip behavior
Function WriteStrip(Form akExcludeForm, bool abNeverStrip) native global
Function EraseStrip(Form akExcludeForm) native global
Function EraseStripAll() native global
; -1 - Never Strip, 0 - No Info, 1 - Always Strip
int Function CheckStrip(Form akCheckForm) native global

function MakeNoStrip(Form ItemRef)
	WriteStrip(ItemRef, true)
endFunction
function MakeAlwaysStrip(Form ItemRef)
	WriteStrip(ItemRef, false)
endFunction
function ClearStripOverride(Form ItemRef)
	EraseStrip(ItemRef)
endFunction
function ResetStripOverrides()
	EraseStripAll()
endFunction

bool function IsNoStrip(Form ItemRef)
	return CheckStrip(ItemRef) == -1
endFunction
bool function IsAlwaysStrip(Form ItemRef)
	return CheckStrip(ItemRef) == 1
endFunction
bool function IsStrippable(Form ItemRef)
	return !IsNoStrip(ItemRef)
endFunction

Form[] function StripActor(Actor ActorRef, Actor VictimRef = none, bool DoAnimate = true, bool LeadIn = false)
	int[] strips = sslSystemConfig.GetStripForms(ActorRef == VictimRef || SexLabRegistry.GetSex(ActorRef, false) == 1, VictimRef)
	return StripActorImpl(ActorRef, strips[0], strips[1], DoAnimate)
endFunction
Form[] function StripSlots(Actor ActorRef, bool[] Strip, bool DoAnimate = false, bool AllowNudesuit = true)
	If(!ActorRef || Strip.Length < 33)
		return Utility.CreateFormArray(0)
	EndIf
	return StripActorImpl(ActorRef, sslUtility.BoolToBit(Strip), Strip[32], DoAnimate)
EndFunction
Form Function StripSlot(Actor ActorRef, int SlotMask)
	Form ItemRef = ActorRef.GetWornForm(SlotMask)
	If (ItemRef && IsStrippable(ItemRef))
		ActorRef.UnequipItemEX(ItemRef, 0, false)
		return ItemRef
	EndIf
	return none
EndFunction

Function UnstripActor(Actor ActorRef, Form[] Stripped, bool IsVictim = false)
	int i = 0
	While(i < Stripped.Length)
		If(Stripped[i])
 			int hand = StorageUtil.GetIntValue(Stripped[i], "Hand", 0)
 			If(hand)
	 			StorageUtil.UnsetIntValue(Stripped[i], "Hand")
			EndIf
	 		ActorRef.EquipItemEx(Stripped[i], hand, false)
		EndIf
		i += 1
	EndWhile
EndFunction

; ------------------------------------------------------- ;
; --- Actor Validation                                --- ;
; ------------------------------------------------------- ;

Faction property ForbiddenFaction auto

int Function ValidateActorImpl(Actor akActor) native global
int function ValidateActor(Actor ActorRef)
	return ValidateActorImpl(ActorRef)
EndFunction
bool function IsValidActor(Actor ActorRef)
	return ValidateActor(ActorRef) > 0
endFunction

function ForbidActor(Actor ActorRef)
	ActorRef.AddToFaction(ForbiddenFaction)
endFunction
function AllowActor(Actor ActorRef)
	ActorRef.RemoveFromFaction(ForbiddenFaction)
endFunction
bool function IsForbidden(Actor ActorRef)
	return ActorRef.IsInFaction(ForbiddenFaction)
endFunction

; ------------------------------------------------------- ;
; --- Gender Functions                                --- ;
; ------------------------------------------------------- ;

Faction property GenderFaction auto

int[] Function GetSexAll(Actor[] akPositions) global
	int[] ret = Utility.CreateIntArray(akPositions.Length)
	int i = 0
	While (i < akPositions.Length)
		ret[i] = SexLabRegistry.GetSex(akPositions[i], false)
		i += 1
	EndWhile
	return ret
EndFunction

Function TreatAsSex(Actor akActor, int aiSexTag)
	int baseSex = SexLabRegistry.GetSex(akActor, true)
	If (aiSexTag == (baseSex % 3))
		akActor.RemoveFromFaction(GenderFaction)
	Else
		If (baseSex > 3 && aiSexTag == 2)
			aiSexTag = 1
		EndIf
		akActor.SetFactionRank(GenderFaction, aiSexTag)
	EndIf
	int handle = ModEvent.Create("SexLabActorGenderChange")
	If (handle)
		ModEvent.PushForm(handle, akActor)
		ModEvent.PushInt(handle, aiSexTag)
		ModEvent.Send(handle)
	EndIf
EndFunction

Function ClearForcedSex(Actor akActor)
	TreatAsSex(akActor, SexLabRegistry.GetSex(akActor, true))
EndFunction

int[] Function CountSexAll(Actor[] akPositions) global
	int[] ret = new int[5]
	int i = 0
	While (i < akPositions.Length)
		int sex = SexLabRegistry.GetSex(akPositions[i], false)
		ret[sex] = ret[sex] + 1
		i += 1
	EndWhile
	return ret
EndFunction

int Function CountMale(Actor[] akPositions) global
	return CountSexAll(akPositions)[0]
EndFunction
int Function CountFemale(Actor[] akPositions) global
	return CountSexAll(akPositions)[1]
EndFunction
int Function CountFuta(Actor[] akPositions) global
	return CountSexAll(akPositions)[2]
EndFunction
int Function CountCreatures(Actor[] akPositions) global
	int[] count = CountSexAll(akPositions)
	return count[3] + count[4]
EndFunction
int Function CountCrtMale(Actor[] akPositions) global
	return CountSexAll(akPositions)[3]
EndFunction
int Function CountCrtFemale(Actor[] akPositions) global
	return CountSexAll(akPositions)[4]
EndFunction

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

Form[] Function UnequipSlots(Actor akActor, int aiSlots) native global

String Function PickRandomFxSet(int asType) native global
int Function GetFxSetCount(int asType, String asSet) native global

Form[] Function StripActorImpl(Actor akActor, int aiSlots, bool abStripWeapons = true, bool abAnimate = false)
	abAnimate = abAnimate && akActor.GetWornForm(0x4)	; Body armor slot
	If(abAnimate)
		int Gender = akActor.GetLeveledActorBase().GetSex()
		Debug.SendAnimationEvent(akActor, "Arrok_Undress_G" + Gender)
		Utility.Wait(0.6)
	EndIf
	Form[] ret = UnequipSlots(akActor, aiSlots)
	If(abStripWeapons)
		Form RightHand = akActor.GetEquippedObject(1)
		If(RightHand && IsStrippable(RightHand))
			akActor.UnequipItemEX(RightHand, akActor.EquipSlot_RightHand, false)
			ret = PapyrusUtil.PushForm(ret, LeftHand)
			StorageUtil.SetIntValue(RightHand, "Hand", 1)
		EndIf
		Form LeftHand = akActor.GetEquippedObject(0)
		If(LeftHand && IsStrippable(LeftHand))
			akActor.UnequipItemEX(LeftHand, akActor.EquipSlot_LeftHand, false)
			ret = PapyrusUtil.PushForm(ret, LeftHand)
			StorageUtil.SetIntValue(RightHand, "Hand", 2)
		EndIf
	EndIf
	If(abAnimate)
		Utility.Wait(0.4)
	EndIf
	return ret
EndFunction

Function BeginOverlay(Actor akTarget, int aiType)
	bool isFemale = akTarget.GetLeveledActorBase().GetSex() as Bool
	String set = StorageUtil.GetStringValue(akTarget, ACTIVE_SET_PREFIX + aiType, "")
	If (set == "")
		set = PickRandomFxSet(aiType)
		StorageUtil.SetStringValue(akTarget, ACTIVE_SET_PREFIX + aiType, set)
		Log(akTarget + ": Selecting cum fx set; Random " + aiType + " set " + set + " selected for " + akTarget.GetBaseObject().GetName())
	Else
		Log(akTarget + ": Selecting cum fx set; Using " + aiType + " set " + set + " for " + akTarget.GetBaseObject().GetName())
	EndIf
	int layer = StorageUtil.GetIntValue(akTarget, ACTIVE_LAYER_PREFIX + aiType, 0) + 1
	int maxLayer = GetFxSetCount(aiType, set)
	If (layer > maxLayer)
		layer = maxLayer
	EndIf
	If (StorageUtil.SetIntValue(akTarget, ACTIVE_LAYER_PREFIX + aiType, layer) == maxLayer)
		return
	EndIf
	String texturePath = "SexLab/CumFx/" + TypeToString(aiType) + "/" + set + "/" + layer + ".dds"
    ; String akTargetRaceStr = MiscUtil.GetActorRaceEditorID(akTarget)
    ; If (StringUtil.Find(akTargetRaceStr, "UBE") != -1)
    ;     texturePath = "SexLab/CumFx/UBE/" + TypeToString(aiType) + "/" + set + "/" + layer + ".dds"
    ;     Log(akTarget + ": Selecting cum fx set; UBE " + aiType + " set " + set + " selected for " + akTarget.GetBaseObject().GetName())
    ; Endif
	StorageUtil.SetStringValue(akTarget, LAST_APPLIED_TEXTURE_PREFIX + aiType, texturePath)
	StorageUtil.SetFloatValue(akTarget, LAST_APPLIED_TIME_PREFIX + aiType, SexLabUtil.GetCurrentGameRealTime())
	StorageUtil.IntListAdd(akTarget, APPLIED_TEXTURE_LIST, aiType, false)
	String[] parts = GetAreas()
	int i = 0
	While (i < parts.Length)
		String part = parts[i]
		; !(Menu.LimitCumAreas && (part == "Hands" || part == "Feet")) && !part == "Face" || part == "Oral" && part == "Face"
		If (part != "Face" || (part == "Face" && aiType == FX_ORAL))
			Int slot = GetEmptySlot(akTarget, isFemale, part, aiType)
			If slot != -1
				ApplyOverlay(akTarget, isFemale, part, slot, texturePath, set)
			Else
				Log(akTarget + ": Error applying overlay to area: " + part)
			EndIf
		EndIf
		i += 1
	EndWhile
EndFunction

Function ApplyOverlay(Actor akTarget, bool isFemale, String asArea, String asOverlaySlot, String asTexture, String asSet)
	;note on args:
	;Function AddNodeOverrideInt(ObjectReference ref, bool isFemale, string node, int key, int index, int value, bool persist)
	;key 0=Color, 1=??, 2=Gloss,3=specStr, 4/5=Lighting, 6=TextureSet, 7=tintColor, 8=alpha, 9=texture
	NiOverride.AddOverlays(akTarget)
	float alpha = sslSystemConfig.GetSettingFlt("fCumAlpha")
	String node = asArea + " [ovl" + asOverlaySlot + "]"
	NiOverride.AddNodeOverrideString(akTarget, isFemale, node, 9, 0, asTexture, true)
	NiOverride.AddNodeOverrideInt(akTarget, isFemale, node, 7, -1, 0, true)	;tint color
    NiOverride.AddNodeOverrideInt(akTarget, isFemale, node, 0, -1, 0, true)	;color
	NiOverride.AddNodeOverrideFloat(akTarget, isFemale, node, 8, -1, alpha, true)
	NiOverride.AddNodeOverrideFloat(akTarget, isFemale, node, 2, -1, 0.0, true)	;gloss
	NiOverride.AddNodeOverrideFloat(akTarget, isFemale, node, 3, -1, 0.0, true)	;SpecStr
	NiOverride.ApplyNodeOverrides(akTarget)
EndFunction

Function RemovePartOverlay(Actor akTarget, bool isFemale, String LastEffect)
	Log("RemovePartOverlay:" + LastEffect + " started on " + akTarget.GetLeveledActorBase().GetName())
	String[] parts = GetAreas()
	Int i = 0
	While (i < parts.Length)
		Int j = GetNumSlots(parts[i])
		While (j > 0)
			j -= 1
			String Node = parts[i] + " [ovl" + j + "]"
			String TexPath = NiOverride.GetNodeOverrideString(akTarget, isFemale, Node, 9, 0)
			If (TexPath == LastEffect)
				NiOverride.AddNodeOverrideString(akTarget, isFemale, Node, 9, 0, "actors\\character\\overlays\\default.dds", true)
				NiOverride.RemoveNodeOverride(akTarget, isFemale, Node, 9, 0)
				NiOverride.RemoveNodeOverride(akTarget, isFemale, Node, 7, -1)
				NiOverride.RemoveNodeOverride(akTarget, isFemale, Node, 0, -1)
				NiOverride.RemoveNodeOverride(akTarget, isFemale, Node, 8, -1)
				NiOverride.RemoveNodeOverride(akTarget, isFemale, Node, 2, -1)
				NiOverride.RemoveNodeOverride(akTarget, isFemale, Node, 3, -1)
			EndIf
		EndWhile
		i += 1
	EndWhile
EndFunction

int Function GetEmptySlot(Actor akTarget, bool isFemale, String asArea, int aiType)
	String typeStr = TypeToString(aiType)
	int i = GetNumSlots(asArea)
	While (i > 0)
		i -= 1
		String TexPath = NiOverride.GetNodeOverrideString(akTarget, isFemale, asArea + " [ovl" + i + "]", 9, 0)
		Log("GetEmptySlot(): akTarget: " + akTarget.GetBaseObject().GetName() + ". Slot: " + i + ". TexPath: " + TexPath)
		If (TexPath == "" || (StringUtil.Find(TexPath, "SexLab") != -1 && StringUtil.Find(TexPath, typeStr) != -1) || TexPath == "actors\\character\\overlays\\default.dds") 
			Log("GetEmptySlot(): Slot " + i + " chosen for area: " + asArea + " on " + akTarget.GetLeveledActorBase().GetName())
			Return i
		EndIf
	EndWhile
	Log("GetEmptySlot(): Error: Could not find a free slot in area: " + asArea)
	Return -1
EndFunction

String[] Function GetAreas()
	String[] retVal = new String[4]
	retVal[0] = "Face"
	retVal[1] = "Body"
	retVal[2] = "Hands"
	retVal[3] = "Feet"
	return retVal
EndFunction

int Function GetNumSlots(String Area)
	If Area == "Body"
		return NiOverride.GetNumBodyOverlays()
	ElseIf Area == "Face"
		return NiOverride.GetNumFaceOverlays()
	ElseIf Area == "Hands"
		return NiOverride.GetNumHandOverlays()
	Else
		return NiOverride.GetNumFeetOverlays()
	EndIf
EndFunction

String Function TypeToString(int aiType)
	If (aiType == FX_VAGINAL)
		return "Vaginal"
	ElseIf (aiType == FX_ANAL)
		return "Anal"
	ElseIf (aiType == FX_ORAL)
		return "Oral"
	Else
		return "Unknown"
	EndIf
EndFunction

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

bool Function HasVehicle(Actor akActor) native global

Faction property AnimatingFaction hidden
	Faction Function Get()
		return Config.AnimatingFaction
	EndFunction
EndProperty
Weapon property DummyWeapon hidden
	Weapon Function Get()
		return Config.DummyWeapon
	EndFunction
EndProperty
Armor property NudeSuit hidden
	Armor Function Get()
		return Config.NudeSuit
	EndFunction
EndProperty

Spell property CumVaginalOralAnalSpell Hidden
  Spell Function Get()
    return Config.CumVaginalOralAnalSpell
  EndFunction
EndProperty
Spell property CumOralAnalSpell Hidden
  Spell Function Get()
    return Config.CumOralAnalSpell
  EndFunction
EndProperty
Spell property CumVaginalOralSpell Hidden
  Spell Function Get()
    return Config.CumVaginalOralSpell
  EndFunction
EndProperty
Spell property CumVaginalAnalSpell Hidden
  Spell Function Get()
    return Config.CumVaginalAnalSpell
  EndFunction
EndProperty
Spell property Vaginal1Oral1Anal1 Hidden
	Spell Function Get()
		return Config.Vaginal1Oral1Anal1
	EndFunction
EndProperty
Spell property Vaginal2Oral1Anal1 Hidden
	Spell Function Get()
		return Config.Vaginal2Oral1Anal1
	EndFunction
EndProperty
Spell property Vaginal2Oral2Anal1 Hidden
	Spell Function Get()
		return Config.Vaginal2Oral2Anal1
	EndFunction
EndProperty
Spell property Vaginal2Oral1Anal2 Hidden
	Spell Function Get()
		return Config.Vaginal2Oral1Anal2
	EndFunction
EndProperty
Spell property Vaginal1Oral2Anal1 Hidden
	Spell Function Get()
		return Config.Vaginal1Oral2Anal1
	EndFunction
EndProperty
Spell property Vaginal1Oral2Anal2 Hidden
	Spell Function Get()
		return Config.Vaginal1Oral2Anal2
	EndFunction
EndProperty
Spell property Vaginal1Oral1Anal2 Hidden
	Spell Function Get()
		return Config.Vaginal1Oral1Anal2
	EndFunction
EndProperty
Spell property Vaginal2Oral2Anal2 Hidden
	Spell Function Get()
		return Config.Vaginal2Oral2Anal2
	EndFunction
EndProperty
Spell property Oral1Anal1 Hidden
	Spell Function Get()
		return Config.Oral1Anal1
	EndFunction
EndProperty
Spell property Oral2Anal1 Hidden
	Spell Function Get()
		return Config.Oral2Anal1
	EndFunction
EndProperty
Spell property Oral1Anal2 Hidden
	Spell Function Get()
		return Config.Oral1Anal2
	EndFunction
EndProperty
Spell property Oral2Anal2 Hidden
	Spell Function Get()
		return Config.Oral2Anal2
	EndFunction
EndProperty
Spell property Vaginal1Oral1 Hidden
	Spell Function Get()
		return Config.Vaginal1Oral1
	EndFunction
EndProperty
Spell property Vaginal2Oral1 Hidden
	Spell Function Get()
		return Config.Vaginal2Oral1
	EndFunction
EndProperty
Spell property Vaginal1Oral2 Hidden
	Spell Function Get()
		return Config.Vaginal1Oral2
	EndFunction
EndProperty
Spell property Vaginal2Oral2 Hidden
	Spell Function Get()
		return Config.Vaginal2Oral2
	EndFunction
EndProperty
Spell property Vaginal1Anal1 Hidden
	Spell Function Get()
		return Config.Vaginal1Anal1
	EndFunction
EndProperty
Spell property Vaginal2Anal1 Hidden
	Spell Function Get()
		return Config.Vaginal2Anal1
	EndFunction
EndProperty
Spell property Vaginal1Anal2 Hidden
	Spell Function Get()
		return Config.Vaginal1Anal2
	EndFunction
EndProperty
Spell property Vaginal2Anal2 Hidden
	Spell Function Get()
		return Config.Vaginal2Anal2
	EndFunction
EndProperty
Spell property Vaginal1 Hidden
	Spell Function Get()
		return Config.Vaginal1
	EndFunction
EndProperty
Spell property Vaginal2 Hidden
	Spell Function Get()
		return Config.Vaginal2
	EndFunction
EndProperty
Spell property Oral1 Hidden
	Spell Function Get()
		return Config.Oral1
	EndFunction
EndProperty
Spell property Oral2 Hidden
	Spell Function Get()
		return Config.Oral2
	EndFunction
EndProperty
Spell property Anal1 Hidden
	Spell Function Get()
		return Config.Anal1
	EndFunction
EndProperty
Spell property Anal2 Hidden
	Spell Function Get()
		return Config.Anal2
	EndFunction
EndProperty

Keyword property CumOralKeyword Hidden
	Keyword Function Get()
		return Config.CumOralKeyword
	EndFunction
EndProperty
Keyword property CumAnalKeyword Hidden
	Keyword Function Get()
		return Config.CumAnalKeyword
	EndFunction
EndProperty
Keyword property CumVaginalKeyword Hidden
	Keyword Function Get()
		return Config.CumVaginalKeyword
	EndFunction
EndProperty
Keyword property CumOralStackedKeyword Hidden
	Keyword Function Get()
		return Config.CumOralStackedKeyword
	EndFunction
EndProperty
Keyword property CumAnalStackedKeyword Hidden
	Keyword Function Get()
		return Config.CumAnalStackedKeyword
	EndFunction
EndProperty
Keyword property CumVaginalStackedKeyword Hidden
	Keyword Function Get()
		return Config.CumVaginalStackedKeyword
	EndFunction
EndProperty

Furniture property BaseMarker hidden
	Furniture Function Get()
		return Config.BaseMarker
	EndFunction
EndProperty
Package property DoNothing hidden
	Package Function Get()
		return Config.DoNothing
	EndFunction
EndProperty
Keyword property ActorTypeNPC hidden
	Keyword Function Get()
		return Config.ActorTypeNPC
	EndFunction
EndProperty

bool function IsCreature(Actor ActorRef)
	return SexLabRegistry.GetRaceID(ActorRef) > 0
endFunction

int function GetGender(Actor ActorRef)
	if ActorRef
		ActorBase BaseRef = ActorRef.GetLeveledActorBase()
		if sslCreatureAnimationSlots.HasRaceType(BaseRef.GetRace())
			if !Config.UseCreatureGender
				return 2 ; Creature - All Male
			elseIf ActorRef.IsInFaction(GenderFaction)
				return 2 + ActorRef.GetFactionRank(GenderFaction) ; CreatureGender + Override
			else
				return 2 + BaseRef.GetSex() ; CreatureGenders: 2+
			endIf
		elseIf ActorRef.IsInFaction(GenderFaction)
			return ActorRef.GetFactionRank(GenderFaction) ; Override
		else
			return BaseRef.GetSex() ; Default
		endIf
	endIf
	return 0 ; Invalid actor - default to male for compatibility
endFunction

Function TreatAsGender(Actor ActorRef, bool AsFemale)
	If (AsFemale)
		TreatAsSex(ActorRef, 1)
	Else
		TreatAsSex(ActorRef, 0)
	EndIf
EndFunction
function ClearForcedGender(Actor ActorRef)	; Replaced to stay consistent with vocabulary
	ClearForcedSex(ActorRef)
endFunction

function TreatAsMale(Actor ActorRef)
	TreatAsGender(ActorRef, false)
endFunction
function TreatAsFemale(Actor ActorRef)
	TreatAsGender(ActorRef, true)
endFunction

int function GetTrans(Actor ActorRef)
	int configSex = SexLabRegistry.GetSex(ActorRef, true)
	If (configSex != 2 && configSex == SexLabRegistry.GetSex(ActorRef, false))
		; configSex == vanillaSex => No overwrite <=> no "trans"
		return -1
	ElseIf (configSex >= 2)
		; Futa+ has its tag shifted 1 up, since this is a legcay function they need to be shifted down once again
		return configSex - 1
	EndIf
	return configSex
endFunction

int[] function GetTransAll(Actor[] Positions)
	int i = Positions.Length
	int[] Trans = Utility.CreateIntArray(i)
	while i > 0
		i -= 1
		Trans[i] = GetTrans(Positions[i])
	endWhile
	return Trans
endFunction

int[] function TransCount(Actor[] Positions)
	int[] Trans = new int[4]
	int i = Positions.Length
	while i > 0
		i -= 1
		int g = GetTrans(Positions[i])
		if g >= 0 && g < 4
			Trans[g] = Trans[g] + 1
		endIf
	endWhile
	return Trans
endFunction

int[] function GetGendersAll(Actor[] Positions)
	int i = Positions.Length
	int[] Genders = Utility.CreateIntArray(i)
	while i > 0
		i -= 1
		Genders[i] = GetGender(Positions[i])
	endWhile
	return Genders
endFunction

int[] function GenderCount(Actor[] Positions)
	int[] Genders = new int[4]
	int i = Positions.Length
	while i > 0
		i -= 1
		int g = GetGender(Positions[i])
		Genders[g] = Genders[g] + 1
	endWhile
	return Genders
endFunction

int function MaleCount(Actor[] Positions)
	return GenderCount(Positions)[0]
endFunction
int function FemaleCount(Actor[] Positions)
	return GenderCount(Positions)[1]
endFunction
int function CreatureCount(Actor[] Positions)
	int[] Genders = GenderCount(Positions)
	return Genders[2] + Genders[3]
endFunction
int function CreatureMaleCount(Actor[] Positions)
	return GenderCount(Positions)[2]
endFunction
int function CreatureFemaleCount(Actor[] Positions)
	return GenderCount(Positions)[3]
endFunction

string function MakeGenderTag(Actor[] Positions)
	return SexLabUtil.MakeGenderTag(Positions)
endFunction

string function GetGenderTag(int Females = 0, int Males = 0, int Creatures = 0)
	return SexLabUtil.GetGenderTag(Females, Males, Creatures)
endFunction

; A framework shouldnt be "random" and the keyword convention should be established strongly enough to not rely on StorageUtil anymore
bool function ContinueStrip(Form ItemRef, bool DoStrip = true) global
	int t = CheckStrip(ItemRef)
	if t == 1
		return True
	endIf
	return DoStrip && t != -1
endFunction

bool function CanAnimate(Actor ActorRef)
	if !ActorRef
		return false
	endIf
	Race ActorRace  = ActorRef.GetLeveledActorBase().GetRace()
	string RaceName = ActorRace.GetName()+MiscUtil.GetRaceEditorID(ActorRace)
	return !(ActorRace.IsRaceFlagSet(0x00000004) || StringUtil.Find(RaceName, "Moli") != -1 || StringUtil.Find(RaceName, "Child") != -1  || StringUtil.Find(RaceName, "Little") != -1 || StringUtil.Find(RaceName, "117") != -1 || StringUtil.Find(RaceName, "Enfant") != -1 || StringUtil.Find(RaceName, "Teen") != -1 || (StringUtil.Find(RaceName, "Elin") != -1 && ActorRef.GetScale() < 0.92) ||  (StringUtil.Find(RaceName, "Monli") != -1 && ActorRef.GetScale() < 0.92))
endFunction

function ApplyCum(Actor ActorRef, int CumID)
	AddCum(ActorRef, (cumID == 1 || cumID == 4 || cumID == 5 || cumID == 7), (cumID == 2 || cumID == 4 || cumID == 6 || cumID == 7), (cumID == 3 || cumID == 5 || cumID == 6 || cumID == 7))
endFunction

Function AddCum(Actor ActorRef, bool Vaginal = true, bool Oral = true, bool Anal = true)
	If (!Vaginal && !Oral && !Anal)
		return	; Nothing to do
	EndIf
	If (Vaginal)
		AddCumFx(ActorRef, FX_VAGINAL)
	EndIf
	If (Oral)
		AddCumFx(ActorRef, FX_ORAL)
	EndIf
	If (Anal)
		AddCumFx(ActorRef, FX_ANAL)
	EndIf
EndFunction

int Function CountCum(Actor ActorRef, bool Vaginal = true, bool Oral = true, bool Anal = true)
	int retVal = 0
	If Vaginal
		retVal += CountCumFx(ActorRef, FX_VAGINAL)
	EndIf
	If Oral
		retVal += CountCumFx(ActorRef, FX_ORAL)
	EndIf
	If Anal
		retVal += CountCumFx(ActorRef, FX_ANAL)
	EndIf
	return retVal
EndFunction

Function ClearCum(Actor ActorRef)
	RemoveCumFx(ActorRef, FX_ALL)
EndFunction
