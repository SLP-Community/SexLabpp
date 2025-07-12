scriptname sslConfigMenu extends SKI_ConfigBase
{
	Skyrim SexLab Mod Configuration Menu
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
Actor property PlayerRef auto
sslSystemConfig property Config auto
sslSystemAlias property SystemAlias auto
sslActorLibrary Property ActorLib Auto
sslThreadSlots Property ThreadSlots Auto

; ------------------------------------------------------- ;
; --- Conmfig Init				                            --- ;
; ------------------------------------------------------- ;

int Function GetVersion()
	return SexLabUtil.GetVersion()
EndFunction
String function GetStringVer()
	return SexLabUtil.GetStringVer()
EndFunction

Event OnVersionUpdate(int version)
EndEvent

Event OnGameReload()
	RegisterForModEvent("SKICP_pageSelected", "OnPageSelected")
	parent.OnGameReload()
EndEvent

Event OnConfigInit()
	Pages = new string[11]
	Pages[0] = "$SSL_SexDiary"
	Pages[1] = "$SSL_AnimationSettings"
	Pages[2] = "$SSL_MatchMaker"
	Pages[3] = "$SSL_SoundSettings"
	Pages[4] = "$SSL_TimersStripping"
	Pages[5] = "$SSL_StripEditor"
	Pages[6] = "$SSL_ToggleAnimations"
	Pages[7] = "$SSL_ExpressionEditor"
	Pages[8] = "$SSL_EnjoymentSettings"
	Pages[9] = "$SSL_PlayerHotkeys"
	Pages[10] = "$SSL_RebuildClean"

	; Animation Settings
	_PlFurnOpt = new String[5]
	_PlFurnOpt[0] = "$SSL_Never"
	_PlFurnOpt[1] = "$SSL_Sometimes"
	_PlFurnOpt[2] = "$SSL_Always"
	_PlFurnOpt[3] = "$SSL_AskAlways"
	_PlFurnOpt[4] = "$SSL_AskNotSub"

	_NPCFurnOpt = new String[3]
	_NPCFurnOpt[0] = "$SSL_Never"
	_NPCFurnOpt[1] = "$SSL_Sometimes"
	_NPCFurnOpt[2] = "$SSL_Always"

	_FadeOpt = new string[3]
	_FadeOpt[0] = "$SSL_Never"
	_FadeOpt[1] = "$SSL_UseBlack"
	_FadeOpt[2] = "$SSL_UseBlur"

	_FilterOpt = new String[3]
	_FilterOpt[0] = "$SSL_Filter_0"		; Loose
	_FilterOpt[1] = "$SSL_Filter_1"		; Standard
	_FilterOpt[2] = "$SSL_Filter_2"		; Strict

	_ClimaxTypes = new String[3]
	_ClimaxTypes[0] = "$SSL_Climax_0"	; Default
	_ClimaxTypes[1] = "$SSL_Climax_1"	; Legacy
	_ClimaxTypes[2] = "$SSL_Climax_2"	; SLSO

	_Sexes = new String[3]
	_Sexes[0] = "$SSL_Male"
	_Sexes[1] = "$SSL_Female"
	_Sexes[2] = "$SSL_Futa"

	; Expression Editor
	_moods = new string[17]
	int i = 0
	While (i < _moods.Length)
		_moods[i] = "$SSL_DialogueMood_" + i
		i += 1
	EndWhile

	_soundmethod = new string[2]
	_soundmethod[0] = "$SSL_Sync"
	_soundmethod[1] = "$SSL_Async"

	_expressionScales = new String[4]
	_expressionScales[0] = "$SSL_ScaleMode_Linear"
	_expressionScales[1] = "$SSL_ScaleMode_Square"
	_expressionScales[2] = "$SSL_ScaleMode_Cubic"
	_expressionScales[3] = "$SSL_ScaleMode_Exponential"

	; Timers & Stripping
	_stripView = new string[2]
	_stripView[0] = "$SSL_DefaultStripping"
	_stripView[1] = "$SSL_DominantStripping"

	; Animation Toggles
	_toggleGroup = new String[3]
	_toggleGroup[0] = "$SSL_Humans"
	_toggleGroup[1] = "$SSL_Creatures"
	_toggleGroup[2] = "$SSL_Everything"

	If (SKSE.GetVersionMinor() < 2)
		Config.DisableScale = true
		Debug.MessageBox("[SexLab]\nYou are using an outdated version of Skyrim and scaling has thus been disabled to prevent crashes.")
	EndIf
EndEvent

Event OnConfigOpen()
	If(PlayerRef.GetLeveledActorBase().GetSex() == 0)
		Pages[0] = "$SSL_SexJournal"
	Else
		Pages[0] = "$SSL_SexDiary"
	EndIf
	_trackedIndex = 0
	_trackedActors = SexLabStatistics.GetAllTrackedUniqueActorsSorted()
	_trackedNames = Utility.CreateStringArray(_trackedActors.Length)
	int i = 0
	While (i < _trackedNames.Length)
		_trackedNames[i] = _trackedActors[i].GetActorBase().GetName()
		i += 1
	EndWhile
	_voiceCacheIndex = 0
	_voiceActiveRaceKey = SexLabRegistry.MapRaceIDToRaceKey(0)
	_voices = sslVoiceSlots.GetAllVoices(_voiceActiveRaceKey)
	_voiceCachedActors = sslVoiceSlots.GetAllCachedUniqueActorsSorted(Config.TargetRef)
	_voiceCachedNames = Utility.CreateStringArray(_voiceCachedActors.Length)
	int n = 0
	While (n < _voiceCachedNames.Length)
		_voiceCachedNames[n] = _voiceCachedActors[n].GetActorBase().GetName()
		n += 1
	EndWhile
	_stripViewIdx = 0
	_playerDisplayAll = false
	_targetDisplayAll = false
	String[] packages = sslAnimationSlots.GetAllPackages()
	_animPack = new String[1]
	_animPack[0] = "$SSL_Everything"
	_animPack = PapyrusUtil.MergeStringArray(_animPack, packages)
	_animPackIdx = 0
	_toggleGroupIdx = 0
	_currentTogglePage = 0
	_expression = sslExpressionSlots.GetAllProfileIDs()
	_expressionIdx = 0
	_phaseidx = 0
EndEvent

Event OnConfigClose()
	ModEvent.Send(ModEvent.Create("SexLabConfigClose"))
endEvent

; ------------------------------------------------------- ;
; --- Config Pages						                        --- ;
; ------------------------------------------------------- ;

Event OnPageReset(string page)
	If (!SystemAlias.IsInstalled)
		InstallMenu()
	ElseIf (Page == "")
		LoadCustomContent("SexLab/logo.dds", 184, 31)
	Else
		UnloadCustomContent()
		If (page == "$SSL_SexDiary" || page == "$SSL_SexJournal")
			SexDiary()
		ElseIf (page == "$SSL_AnimationSettings")
			AnimationSettings()
		ElseIf (page == "$SSL_MatchMaker")
			MatchMaker()
		ElseIf (page == "$SSL_SoundSettings")
			SoundSettings()
		ElseIf (page == "$SSL_TimersStripping")
			TimersStripping()
		ElseIf (page == "$SSL_StripEditor")
			StripEditor()
		ElseIf (page == "$SSL_ToggleAnimations")
			ToggleAnimations()
		ElseIf (page == "$SSL_ExpressionEditor")
			ExpressionEditor()
		ElseIf (page == "$SSL_EnjoymentSettings")
			EnjoymentSettings()
		ElseIf (page == "$SSL_PlayerHotkeys")
			PlayerHotkeys()
		ElseIf (page == "$SSL_RebuildClean")
			RebuildClean()
		EndIf
	EndIf
EndEvent

; ------------------------------------------------------- ;
; --- Sex Diary/Journal Editor                        --- ;
; ------------------------------------------------------- ;

Actor[] _trackedActors
String[] _trackedNames
int _trackedIndex

String Function GetSexualityTitle(Actor ActorRef) global
	int sexuality = SexLabStatistics.GetSexuality(ActorRef)
	If (sexuality == 0)
		return "$SSL_Heterosexual"
	ElseIf (sexuality == 1)
		If (SexLabRegistry.GetSex(ActorRef, true) == 0)
			return "$SSL_Gay"
		Else
			return "$SSL_Lesbian"
		EndIf
	Else
		return "$SSL_Bisexual"
	EndIf
EndFunction

String[] Function StatTitles() global
	String[] StatTitles = new String[7]
	StatTitles[0] = "$SSL_Unskilled"
	StatTitles[1] = "$SSL_Novice"
	StatTitles[2] = "$SSL_Apprentice"
	StatTitles[3] = "$SSL_Journeyman"
	StatTitles[4] = "$SSL_Expert"
	StatTitles[5] = "$SSL_Master"
	StatTitles[6] = "$SSL_GrandMaster"
	return StatTitles
EndFunction

Function SexDiary()
	SetCursorFillMode(LEFT_TO_RIGHT)
	If (_trackedIndex >= _trackedActors.Length)
		_trackedIndex = 0
	EndIf
	Actor it = _trackedActors[_trackedIndex]
	AddMenuOptionST("StatSelectingMenu", "$SSL_StatSelectingMenu", _trackedNames[_trackedIndex])
	AddTextOptionST("ResetTargetStats", "$SSL_Reset{" + _trackedNames[_trackedIndex] + "}Stats", "$SSL_ClickHere")
	AddHeaderOption("$SSL_Statistics")
	AddEmptyOption()

	SetCursorFillMode(TOP_TO_BOTTOM)
	AddTextOption("$SSL_LastTimeInScene", Utility.GameTimeToString(SexLabStatistics.GetStatistic(it, 0)))
	AddTextOption("$SSL_TimeInScenes", sslActorStats.ParseTime(SexLabStatistics.GetStatistic(it, 1) as int))
	String[] xp_titles = StatTitles()
	int i = 2
	While (i < 5)		; XP Statistics
		float value = SexLabStatistics.GetStatistic(it, i)
		int lv = PapyrusUtil.ClampInt(sslActorStats.CalcLevel(value), 0, xp_titles.Length - 1)
		AddTextOption("$SSL_Statistic_" + i, xp_titles[lv])
		i += 1
	EndWhile
	While (i < 9)		; Partner Statistics
		AddTextOption("$SSL_Statistic_" + i, SexLabStatistics.GetStatistic(it, i) as int)
		i += 1
	EndWhile
	SetCursorPosition(5)
	AddTextOptionST("StatChangeSexuality", "$SSL_Sexuality", GetSexualityTitle(it))
	While (i < 16)	; "Times" Statistics
		AddTextOption("$SSL_Statistic_" + i, SexLabStatistics.GetStatistic(it, i) as int)
		i += 1
	EndWhile
EndFunction

State StatSelectingMenu
	Event OnMenuOpenST()
		SetMenuDialogStartIndex(_trackedIndex)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(_trackedNames)
	EndEvent
	Event OnMenuAcceptST(Int aiIndex)
		_trackedIndex = aiIndex
		SetMenuOptionValueST(_trackedNames[_trackedIndex])
		ForcePageReset()
	EndEvent
	Event OnDefaultST()
		_trackedIndex = 0
		SetMenuOptionValueST(_trackedNames[_trackedIndex])
		ForcePageReset()
	EndEvent
	Event OnHighlightST()
		SetInfoText("$SSL_StatSelectingMenuHighlight")
	EndEvent
EndState

State ResetTargetStats
	Event OnSelectST()
		If (!ShowMessage("$SSL_WarnReset{" + _trackedNames[_trackedIndex] + "}Stats"))
			return
		EndIf
		SexLabStatistics.ResetStatistics(_trackedActors[_trackedIndex])
		ForcePageReset()
	EndEvent
	Event OnHighlightST()
		SetInfoText("$SSL_ResetStatHighlight")
	EndEvent
EndState

State StatChangeSexuality
	Event OnSelectST()
		Actor it = _trackedActors[_trackedIndex]
		int sex = SexLabStatistics.GetSexuality(it)
		If (sex == 0)	; Hetero -> Homo
			sslActorStats.SetLegacyStatistic(it, Stats.kSexuality, 25)
		ElseIf (sex == 1)	; Homo -> Bi
			sslActorStats.SetLegacyStatistic(it, Stats.kSexuality, 50)
		Else	; Bi -> Hetero
			sslActorStats.SetLegacyStatistic(it, Stats.kSexuality, 75)
		EndIf
		ForcePageReset()
	EndEvent
	Event OnHighlightST()
		SetInfoText("$SSL_SexualityHighlight")
	EndEvent
EndState

; ------------------------------------------------------- ;
; --- Animation Settings                              --- ;
; ------------------------------------------------------- ;

String[] _PlFurnOpt
String[] _NPCFurnOpt
string[] _FadeOpt
String[] _FilterOpt
String[] _ClimaxTypes
String[] _Sexes

Function AnimationSettings()
	SetCursorFillMode(TOP_TO_BOTTOM)
	AddHeaderOption("$SSL_PlayerSettings")
	AddToggleOptionST("AutoAdvance","$SSL_AutoAdvanceStages", Config.AutoAdvance)
	AddToggleOptionST("DisableSub","$SSL_DisableSubControls", Config.DisablePlayer)
	AddToggleOptionST("AutomaticTFC","$SSL_AutomaticTFC", Config.AutoTFC)
	AddSliderOptionST("AutomaticSUCSM","$SSL_AutomaticSUCSM", Config.AutoSUCSM, "{0}")
	AddMenuOptionST("SexSelect_0", "$SSL_PlayerSex", _Sexes[SexLabRegistry.GetSex(PlayerRef, false) % 3])
	If (Config.TargetRef)
		String name = Config.TargetRef.GetLeveledActorBase().GetName()
		AddMenuOptionST("SexSelect_1", "$SSL_{" + name + "}Sex", _Sexes[SexLabRegistry.GetSex(Config.TargetRef, false) % 3])
	Else
		AddTextOption("$SSL_NoTarget", "$SSL_Male", OPTION_FLAG_DISABLED)
	EndIf
	AddHeaderOption("$SSL_ExtraEffects")
	AddMenuOptionST("ClimaxType", "$SSL_ClimaxType", _ClimaxTypes[sslSystemConfig.GetSettingInt("iClimaxType")])
	AddToggleOptionST("OrgasmEffects","$SSL_OrgasmEffects", Config.OrgasmEffects)
	AddSliderOptionST("ShakeStrength","$SSL_ShakeStrength", (Config.ShakeStrength * 100), "{0}%")
	AddToggleOptionST("UseCum","$SSL_ApplyCumEffects", Config.UseCum)
	AddSliderOptionST("CumEffectTimer","$SSL_CumEffectTimer", Config.CumTimer, "$SSL_Seconds")
	AddToggleOptionST("UseExpressions","$SSL_UseExpressions", Config.UseExpressions)
	AddToggleOptionST("UseLipSync", "$SSL_UseLipSync", Config.UseLipSync)
	AddHeaderOption("$SSL_Lovense")
	int lovenseFlag = DoDisable(!sslLovense.IsLovenseInstalled())
	AddSliderOptionST("LovenseStrength", "$SSL_LovenseStrength", sslSystemConfig.GetSettingInt("iLovenseStrength"), "{0}", lovenseFlag)
	AddSliderOptionST("LovenseStrengthOrgasm", "$SSL_LovenseStrengthOrgasm", sslSystemConfig.GetSettingInt("iLovenseStrengthOrgasm"), "{0}", lovenseFlag)
	AddSliderOptionST("LovenseDurationOrgasm", "$SSL_LovenseDurationOrgasm", sslSystemConfig.GetSettingInt("fLovenseDurationOrgasm"), "{0}", lovenseFlag)

	SetCursorPosition(1)
	AddHeaderOption("$SSL_Creatures")
	AddToggleOptionST("AllowCreatures","$SSL_AllowCreatures", Config.AllowCreatures)
	AddToggleOptionST("UseCreatureGender","$SSL_UseCreatureGender", Config.UseCreatureGender)
	AddHeaderOption("$SSL_AnimationHandling")
	AddToggleOptionST("DisableScale","$SSL_DisableScale", Config.DisableScale)
	; AddMenuOptionST("FilterStrictness", "$SSL_FilterStrictness", _FilterOpt[sslSystemConfig.GetSettingInt("iFilterStrictness")])
	AddMenuOptionST("UseFade","$SSL_UseFade", _FadeOpt[sslSystemConfig.GetSettingInt("iUseFade")])
	AddToggleOptionST("UndressAnimation","$SSL_UndressAnimation", Config.UndressAnimation)
	AddToggleOptionST("RedressVictim","$SSL_VictimsRedress", Config.RedressVictim)
	AddToggleOptionST("DisableTeleport","$SSL_DisableTeleport", Config.DisableTeleport)
	AddToggleOptionST("ShowInMap","$SSL_ShowInMap", Config.ShowInMap)
	AddToggleOptionST("SetAnimSpeedByEnjoyment", "$SSL_SetAnimSpeedByEnjoyment", Config.SetAnimSpeedByEnjoyment, DoDisable(!sslSystemConfig.HasAnimSpeedSE()))
	AddMenuOptionST("FurnitureNPC", "$SSL_FurnitureNPC", _NPCFurnOpt[sslSystemConfig.GetSettingInt("iNPCBed")])
	AddMenuOptionST("FurniturePlayer", "$SSL_FurniturePlayer", _PlFurnOpt[sslSystemConfig.GetSettingInt("iAskBed")])
EndFunction

state DisableScale
	; COMEBACK: Might want to delete this for good since scaling is essential and works relaibly for latest
	event OnSelectST()
		Config.DisableScale = !Config.DisableScale
		SetToggleOptionValueST(Config.DisableScale)
		ForcePageReset()
	endEvent
	event OnDefaultST()
		Config.DisableScale = false
		SetToggleOptionValueST(Config.DisableScale)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoDisableScale")
	endEvent
endState

; ------------------------------------------------------- ;
; --- Matchmaker	                                  --- ;
; ------------------------------------------------------- ;

Function MatchMaker()
	SetCursorFillMode(TOP_TO_BOTTOM)
	int flag = DoDisable(!Config.MatchMaker)
	AddToggleOptionST("ToggleMatchMaker", "$SSL_ToggleMatchMaker", Config.MatchMaker)
	AddHeaderOption("$SSL_MatchMakerTagsSettings", flag)
	AddTextOptionST("matchmakerInputTags", "$SSL_InputTags", sslSystemConfig.ParseMMTagString(), flag)
	AddInputOptionST("matchmakerInputRequiredTags", "$SSL_InputRequiredTags", Config.RequiredTags, flag)
	AddInputOptionST("matchmakerInputExcludedTags", "$SSL_InputExcludedTags", Config.ExcludedTags, flag)
	AddInputOptionST("matchmakerInputOptionalTags", "$SSL_InputOptionalTags", Config.OptionalTags, flag)
	AddTextOptionST("TextResetTags", "$SSL_TextResetTags", "$SSL_ResetTagsHere", flag)
	SetCursorPosition(1)
	AddEmptyOption()
	AddHeaderOption("$SSL_MatchMakerActorSettings", flag)
	AddToggleOptionST("matchmakerToggleSubPlayer", "$SSL_ToggleSubmissivePlayer", Config.SubmissivePlayer, flag)
	AddToggleOptionST("matchmakerToggleSubTarget", "$SSL_ToggleSubmissiveTarget", Config.SubmissiveTarget, flag)
EndFunction

State ToggleMatchMaker
	Event OnSelectST()
		Config.MatchMaker = !Config.MatchMaker
		SetToggleOptionValueST(Config.MatchMaker)
		ForcePageReset()
	EndEvent
	Event OnDefaultST()
		Config.MatchMaker = false
		SetToggleOptionValueST(Config.MatchMaker)
		ForcePageReset()
	EndEvent
	Event OnHighlightST()
		SetInfoText("$SSL_InfoMatchMaker")
	EndEvent
EndState

; ------------------------------------------------------- ;
; --- Sound Settings                                  --- ;
; ------------------------------------------------------- ;

String _voiceActiveRaceKey
String[] _voices
Actor[] _voiceCachedActors
String[] _voiceCachedNames
int _voiceCacheIndex

String Function GetSavedVoice(Actor akActor) global
	String ret = sslVoiceSlots.GetSavedVoice(akActor)
	If (!ret)
		return "$SSL_Random"
	EndIf
	return ret
EndFunction

Function SoundSettings()
	SetCursorFillMode(TOP_TO_BOTTOM)
	If (_voiceCacheIndex >= _voiceCachedNames.Length)
		_voiceCacheIndex = 0
	EndIf
	; Voices & SFX
	AddSliderOptionST("VoiceVolume","$SSL_VoiceVolume", Config.VoiceVolume * 100, "{0}%")
	AddSliderOptionST("SFXVolume","$SSL_SFXVolume", Config.SFXVolume * 100, "{0}%")
	AddSliderOptionST("MaleVoiceDelay","$SSL_MaleVoiceDelay", Config.MaleVoiceDelay, "$SSL_Seconds")
	AddSliderOptionST("FemaleVoiceDelay","$SSL_FemaleVoiceDelay", Config.FemaleVoiceDelay, "$SSL_Seconds")
	AddSliderOptionST("SFXDelay","$SSL_SFXDelay", Config.SFXDelay, "$SSL_Seconds")
	; Cached Voices
	SetCursorPosition(1)
	AddHeaderOption("$SSL_CachedVoices")
	AddMenuOptionST("SelectVoiceCache", "$SSL_SelectVoiceCache", _voiceCachedNames[_voiceCacheIndex])
	AddMenuOptionST("SelectVoiceCacheV", "$SSL_SelectVoiceCacheV", GetSavedVoice(_voiceCachedActors[_voiceCacheIndex]))
	AddEmptyOption()
	AddEmptyOption()
	; Toggle Voices
	SetCursorPosition(10)
	SetCursorFillMode(LEFT_TO_RIGHT)
	AddHeaderOption("$SSL_ToggleVoices")
	AddMenuOptionST("activeVoices", "$SSL_ActiveVoices", "$SSL_Race_" + SexLabRegistry.MapRaceKeyToID(_voiceActiveRaceKey))
	int i = 0
	While (i < _voices.Length)
		AddToggleOptionST("Voice_" + i, sslBaseVoice.GetDisplayName(_voices[i]), sslBaseVoice.GetEnabled(_voices[i]))
		i += 1
	EndWhile
EndFunction

State SelectVoiceCache
	Event OnMenuOpenST()
		SetMenuDialogStartIndex(_voiceCacheIndex)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(_voiceCachedNames)
	EndEvent
	Event OnMenuAcceptST(Int aiIndex)
		_voiceCacheIndex = aiIndex
		ForcePageReset()
	EndEvent
	Event OnDefaultST()
		_voiceCacheIndex = 0
		ForcePageReset()
	EndEvent
	Event OnHighlightST()
		SetInfoText("$SSL_SelectVoiceCacheInfo")
	EndEvent
EndState

State SelectVoiceCacheV
	Event OnMenuOpenST()
		int idx = _voices.Find(GetSavedVoice(_voiceCachedActors[_voiceCacheIndex]))
		If (idx < 0)
			idx = 0
		Endif
		SetMenuDialogStartIndex(idx)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(_voices)
	EndEvent
	Event OnMenuAcceptST(Int aiIndex)
		sslVoiceSlots.StoreVoice(_voiceCachedActors[_voiceCacheIndex], _voices[aiIndex])
		SetMenuOptionValueST(_voices[aiIndex])
	EndEvent
	Event OnHighlightST()
		SetInfoText("$SSL_SelectVoiceCacheVInfo")
	EndEvent
EndState

; ------------------------------------------------------- ;
; --- Timers & Stripping                              --- ;
; ------------------------------------------------------- ;

String[] _stripView
int _stripViewIdx

Function TimersStripping()
	SetCursorFillMode(LEFT_TO_RIGHT)
	; Timers
	AddHeaderOption("$SSL_Timers")
	AddEmptyOption()
	int t = 0
	While (t < 4)
		AddSliderOptionST("StageTimers_" + t, "$SSL_StageTimer_" + t, sslSystemConfig.GetSettingFltA("fTimers", t), "$SSL_Seconds")
		t += 1
	EndWhile
	; Stripping
	AddHeaderOption("$SSL_Stripping")
	AddMenuOptionST("TSModeSelect", "$SSL_View", _stripView[_stripViewIdx])
	AddTextOption("", "$SSL_StrippingFst_" + _stripViewIdx, OPTION_FLAG_DISABLED)
	AddTextOption("", "$SSL_StrippingSnd_" + _stripViewIdx, OPTION_FLAG_DISABLED)
	; iStripForms: 0b[Weapon][Female | Submissive][Aggressive]
	int r1 = _stripViewIdx * 4	; 0 / 4
	int r2 = r1 + 2	; 2 / 6
	AddToggleOptionST("StrippingW_" + (r1 + 1), "$SSL_Weapons", sslSystemConfig.GetSettingIntA("iStripForms", r1 + 1))
	AddToggleOptionST("StrippingW_" + (r2 + 1), "$SSL_Weapons", sslSystemConfig.GetSettingIntA("iStripForms", r2 + 1))
	int i = 0
	While (i < 32)
		int bit = Math.LeftShift(1, i)
		AddToggleOptionST("Stripping_" + r1 + "_" + i, "$SSL_Strip_" + i, Math.LogicalAnd(sslSystemConfig.GetSettingIntA("iStripForms", r1), bit), DoDisable(i >= 30))
		AddToggleOptionST("Stripping_" + r2 + "_" + i, "$SSL_Strip_" + i, Math.LogicalAnd(sslSystemConfig.GetSettingIntA("iStripForms", r2), bit), DoDisable(i >= 30))
		If (i == 13)
			AddHeaderOption("$SSL_ExtraSlots")
			AddHeaderOption("$SSL_ExtraSlots")
		EndIf
		i += 1
	EndWhile
EndFunction

; ------------------------------------------------------- ;
; --- Strip Editor                                    --- ;
; ------------------------------------------------------- ;

Form[] _playerItems
Form[] _targetItems
bool _playerDisplayAll
bool _targetDisplayAll

String Function GetStripState(Form ItemRef)
	int strip = sslActorLibrary.CheckStrip(ItemRef)
	If(strip == 1)
		return "$SSL_AlwaysRemove"
	ElseIf(strip == -1)
		return "$SSL_NeverRemove"
	Else
		return "---"
	EndIf
EndFunction

String Function GetItemName(Form ItemRef, string AltName = "$SSL_Unknown")
	If (!ItemRef)
		return "None"
	EndIf
	String name = ItemRef.GetName()
	If (sslUtility.Trim(name) != "")
		return name
	EndIf
	return AltName
EndFunction

int[] function GetAllMaskSlots(int Mask)
	int i = 30
	int Slot = 0x01
	int[] Output
	while i < 62
		if Math.LogicalAnd(Mask, Slot) == Slot
			Output = PapyrusUtil.PushInt(Output, i)
		endIf
		Slot *= 2
		i += 1
	endWhile
	return Output
endFunction

Function StripEditor()
	SetCursorFillMode(TOP_TO_BOTTOM)
	int n = 0
	While (n < 2)
		Form[] list
		If (n == 0)
			AddHeaderOption("$SSL_Equipment{" + PlayerREf.GetActorBase().GetName() + "}")
			AddToggleOptionST("FullInventory_" + n, "$SSL_FullInventory", _playerDisplayAll)
			_playerItems = sslSystemConfig.GetStrippableItems(PlayerRef, !_playerDisplayAll)
			list = _playerItems
		Else
			SetCursorPosition(1)
			If (!Config.TargetRef)
				AddHeaderOption("$SSL_NoTarget", OPTION_FLAG_DISABLED)
				return
			EndIf
			AddHeaderOption("$SSL_Equipment{" + Config.TargetRef.GetLeveledActorBase().GetName() + "}")
			AddToggleOptionST("FullInventory_" + n, "$SSL_FullInventory", _targetDisplayAll)
			_targetItems = sslSystemConfig.GetStrippableItems(Config.TargetRef, !_targetDisplayAll)
			list = _targetItems
		EndIf
		int MAX_ENTRIES = 62
		int i = 0
		While (i < list.Length && i < MAX_ENTRIES)
			AddTextOptionST("StripFlag_" + n + "_" + i, GetItemName(list[i]), GetStripState(list[i]))
			i += 1
		EndWhile
		n += 1
	EndWhile
EndFunction

; ------------------------------------------------------- ;
; --- Toggle Animations                               --- ;
; ------------------------------------------------------- ;

String[] _animPack
int _animPackIdx
String[] _toggleGroup
int _toggleGroupIdx
String _toggleTags

int Property ANIMS_PER_PAGE = 120 AutoReadOnly Hidden
int _currentTogglePage

Function ToggleAnimations()
	SetCursorFillMode(LEFT_TO_RIGHT)
	String animpack = ""
	If (_animPackIdx != 0)
		animpack = _animPack[_animPackIdx]
	EndIf
	String[] animations = sslAnimationSlots.CreateProxyArray(0, _toggleGroupIdx, _toggleTags, animpack)
	int animpages = 0
	If (animations.Length)
		animpages = Math.Ceiling(animations.Length as float / ANIMS_PER_PAGE as float)
	EndIf
	If (_currentTogglePage >= animpages)
		_currentTogglePage = 0
	EndIf
	AddMenuOptionST("togglepackage", "$SSL_TogglePackage", _animPack[_animPackIdx])
	AddTextOptionST("togglevisible", "$SSL_ToggleVisible", "$SSL_ClickHere")
	AddMenuOptionST("togglecategory", "$SSL_ToggleGroup", _toggleGroup[_toggleGroupIdx])
	AddInputOptionST("toggletags", "$SSL_TagFilter", _toggleTags)
	AddHeaderOption("")
	AddTextOptionST("AnimationTogglePage", "$SSL_Page{" + (_currentTogglePage + 1) + "}{" + animpages + "}", "$SSL_NextPage", DoDisable(animpages <= 1))
	int i = _currentTogglePage * ANIMS_PER_PAGE
	While (i < animations.Length && i < (_currentTogglePage + 1) * ANIMS_PER_PAGE)
		String name = SexLabRegistry.GetSceneName(animations[i])
		AddToggleOptionST("AnimationToggle_" + animations[i], name, SexLabRegistry.IsSceneEnabled(animations[i]))
		i += 1
	EndWhile
EndFunction

; ------------------------------------------------------- ;
; --- Expression Editor                               --- ;
; ------------------------------------------------------- ;

String[] _moods
String[] _soundmethod
String[] _expressionScales
String[] _expression
int _expressionIdx
bool _editFemale
float[] _low
float[] _high
; Legacy Support
int[] _maxphases
int _phaseidx

Function ExpressionEditor()
	SetCursorFillMode(TOP_TO_BOTTOM)
	If (_expressionIdx >= _expression.Length)
		_expressionIdx = 0
	EndIf
	int v = sslBaseExpression.GetVersion(_expression[_expressionIdx])
	If (v == 0)
		_maxphases = sslBaseExpression.GetLevelCounts(_expression[_expressionIdx])
		int maxphase = _maxphases[_editFemale as int]
		If (_phaseIdx >= maxphase)
			_phaseIdx = 0
		EndIf
		_low = sslBaseExpression.GetNthValues(_expression[_expressionIdx], _editFemale, _phaseidx)
		If (maxphase > _phaseidx + 1)
			_high = sslBaseExpression.GetNthValues(_expression[_expressionIdx], _editFemale, _phaseidx + 1)
		EndIf
	Else
		_phaseIdx = 0
		_low	= sslBaseExpression.GetNthValues(_expression[_expressionIdx], _editFemale, 0)
		_high = sslBaseExpression.GetNthValues(_expression[_expressionIdx], _editFemale, 1)
	EndIf
	int scalemode = sslBaseExpression.GetExpressionScaleMode(_expression[_expressionIdx])
	String[] tags = sslBaseExpression.GetExpressionTags(_expression[_expressionIdx])
	AddHeaderOption("$SSL_ExpressionEditor")
	AddMenuOptionST("selectexpression", "$SSL_ModifyingExpression", _expression[_expressionIdx])
	; AddInputOptionST("createexpression", "$SSL_CreateExpression", "")
	AddToggleOptionST("enableexpression", "$SSL_Enabled", sslBaseExpression.GetEnabled(_expression[_expressionIdx]))
	AddMenuOptionST("setexprscaling", "$SSL_ExpressionScaling", _expressionScales[scalemode], DoDisable(v < 1))
	AddToggleOptionST("expredittag_Normal", "$SSL_ExpressionsNormal", tags.Find("Normal") > -1)
	AddToggleOptionST("expredittag_Victim", "$SSL_ExpressionsVictim", tags.Find("Victim") > -1)
	AddToggleOptionST("expredittag_Aggressor", "$SSL_ExpressionsAggressor", tags.Find("Aggressor") > -1)

	SetCursorPosition(1)
	AddHeaderOption("$SSL_SyncLipsConfig")
	AddTextOptionST("LipsSoundTime", "$SSL_LipsSoundTime", _soundmethod[Config.LipsSoundTime])
	AddToggleOptionST("LipsFixedValue", "$SSL_LipsFixedValue", Config.LipsFixedValue)
	AddEmptyOption()
	AddEmptyOption()
	AddToggleOptionST("expressioneditfemale", "$SSL_EditFemale", _editFemale)
	AddTextOptionST("expressiontest", "$SSL_TestOnPlayer", "$SSL_Apply", DoDisable(v < 1))

	int i = 0
	While (i < 2)
		; NOTE: Adjust index here when changing settings above
		int idx = 14 + i
		SetCursorPosition(idx)
		int flag = OPTION_FLAG_NONE
		float[] values
		If (i == 0)
			values = _low
		Else
			If (!_high.Length)
				flag = OPTION_FLAG_DISABLED
			EndIf
			values = _high
		EndIf
		AddHeaderOption("$SSL_EditExpressions_" + i)
		AddMenuOptionST("expredit_" + 30 + "_" + i, "$SSL_MoodType", _moods[values[30] as int], flag)
		AddSliderOptionST("expredit_" + 31 + "_" + i, "$SSL_MoodStrength", values[31], "{2}", flag)
		If (i == 0)
			AddHeaderOption("$SSL_Modifier")
		Else
			AddEmptyOption()
		EndIf
		int MODIFIER_COUNT = 14
		int n = 0
		While (n < MODIFIER_COUNT)
			AddSliderOptionST("expredit_" + n + "_" + i, "$SSL_Modifier_" + n, values[n], "{2}", flag)
			n += 1
		EndWhile
		If (i == 0)
			AddHeaderOption("$SSL_Phoneme")
		Else
			AddEmptyOption()
		EndIf
		int PHONEME_COUNT = 16
		int k = 0
		While (k < PHONEME_COUNT)
			AddSliderOptionST("expredit_" + (n + k) + "_" + i, "$SSL_Phoneme_" + k, values[n + k], "{2}", flag)
			k += 1
		EndWhile
		If (v == 0)
			AddHeaderOption("")
			int maxphase = _maxphases[_editFemale as int]
			If (i == 0)
				AddTextOptionST("expressionprev", "$SSL_PrevPhase", "(" + _phaseIdx + "/" + maxphase + ")", DoDisable(_phaseidx == 0))
			ElseIf (flag != OPTION_FLAG_DISABLED)
				AddTextOptionST("expressionnext", "$SSL_NextPhase", "(" + (_phaseIdx + 1) + "/" + maxphase + ")", DoDisable(maxphase <= _phaseidx + 2))
			EndIf
		EndIf
		i += 1
	EndWhile
EndFunction

Function TestApply(Actor ActorRef)
	string name = ActorRef.GetLeveledActorBase().GetName()
	If (!ShowMessage("$SSL_WarnTestExpression{" + name + "}", true, "$Yes", "$No"))
		return
	EndIf
	bool testLow = ShowMessage("$SSL_WarnTestExpressionLowOrHight", true, "$SSL_Low", "$SSL_High")
	bool testOpenMouth = ShowMessage("$SSL_WarnTestExpressionWithOpenMouth", true, "$Yes", "$No")
	ShowMessage("$SSL_StartTestExpression{" + _expression[_expressionIdx] + "}", false)
	sslLog.Log("Testing Expression: " + _expression[_expressionIdx] + ". Low? " + testlow +", OpenMouth? " + testOpenMouth)
	Utility.Wait(0.1)
	If (ActorRef == PlayerRef)
		Game.ForceThirdPerson()
	EndIf
	If (testOpenMouth)
		sslBaseExpression.OpenMouth(ActorRef)
		Utility.Wait(1.0)
	EndIf
	float str = 0.0
	If (!testLow)
		str = 100.0
	EndIf
	sslBaseExpression.ApplyExpression(_expression[_expressionIdx], ActorRef, str)
	Utility.Wait(0.1)
	Debug.Notification("$SSL_AppliedTestExpression")
	Utility.WaitMenuMode(15.0)
	sslBaseExpression.ClearMFG(ActorRef)
	Debug.Notification("$SSL_RestoredTestExpression")
EndFunction

; ------------------------------------------------------- ;
; --- Enjoyment Settings                              --- ;
; ------------------------------------------------------- ;

Function EnjoymentSettings()
	Config.CacheEnjJsonValues()
	SetCursorFillMode(TOP_TO_BOTTOM)
	
	AddHeaderOption("Primary Settings")
		AddToggleOptionST("InternalEnjoymentEnabled","$SSL_InternalEnjoymentEnabled", Config.InternalEnjoymentEnabled)
		AddSliderOptionST("InterDetectionStrength", "$SSL_InterDetectionStrength", Config.InterDetectionStrength)
		AddSliderOptionST("EnjRaiseMultInter", "$SSL_EnjRaiseMultInter", Config.EnjRaiseMultInter)
	AddHeaderOption("General Configs")
		AddSliderOptionST("EnjGainOnStageSkip", "$SSL_EnjGainOnStageSkip", Config.EnjGainOnStageSkip)
		AddToggleOptionST("NoStaminaEndsScene","$SSL_NoStaminaEndsScene", Config.NoStaminaEndsScene)
		AddToggleOptionST("MaleOrgasmEndsScene","$SSL_MaleOrgasmEndsScene", Config.MaleOrgasmEndsScene)
		AddToggleOptionST("DomMustOrgasm","$SSL_DomMustOrgasm", Config.DomMustOrgasm)
		AddToggleOptionST("PlayerMustOrgasm","$SSL_PlayerMustOrgasm", Config.PlayerMustOrgasm)
		AddToggleOptionST("HighEnjOrgasmWait","$SSL_HighEnjOrgasmWait", Config.HighEnjOrgasmWait)
		AddSliderOptionST("MaxNoPainOrgasmMale","$SSL_MaxNoPainOrgasmMale", Config.MaxNoPainOrgasmMale)
		AddSliderOptionST("MaxNoPainOrgasmFemale", "$SSL_MaxNoPainOrgasmFemale", Config.MaxNoPainOrgasmFemale)
		AddSliderOptionST("NoPainRequiredTime", "$SSL_NoPainRequiredTime", Config.NoPainRequiredTime)
		AddSliderOptionST("NoPainRequiredXP", "$SSL_NoPainRequiredXP", Config.NoPainRequiredXP)
	AddHeaderOption("Misc Multipliers")
		AddSliderOptionST("EnjMultVictim","$SSL_EnjMultVictim", Config.EnjMultVictim)
		AddSliderOptionST("EnjMultAggressor", "$SSL_EnjMultAggressor", Config.EnjMultAggressor)
		AddSliderOptionST("EnjMultSub", "$SSL_EnjMultSub", Config.EnjMultSub)
		AddSliderOptionST("EnjMultDom", "$SSL_EnjMultDom", Config.EnjMultDom)
		AddSliderOptionST("PainHugePPMult", "$SSL_PainHugePPMult", Config.PainHugePPMult)
		
	SetCursorPosition(1)
	
	AddHeaderOption("Game Toggles")
		AddToggleOptionST("GameEnabled","$SSL_GameEnabled", Config.GameEnabled)
		AddToggleOptionST("GamePlayerAutoplay","$SSL_GamePlayerAutoplay", Config.GamePlayerAutoplay)
		AddToggleOptionST("GamePlayerVictimAutoplay","$SSL_GamePlayerVictimAutoplay", Config.GamePlayerVictimAutoplay)
		AddToggleOptionST("GameNPCAutoplay","$SSL_GameNPCAutoplay", Config.GameNPCAutoplay)
		AddToggleOptionST("GameEnjReductionChance","$SSL_GameEnjReductionChance", Config.GameEnjReductionChance)
		AddToggleOptionST("GameHoldbackWithPartner","$SSL_GameHoldbackWithPartner", Config.GameHoldbackWithPartner)
	AddHeaderOption("Game Hotkeys")
		AddKeyMapOptionST("GameUtilityKey", "$SSL_GameUtilityKey", Config.GameUtilityKey)
		AddKeyMapOptionST("GamePauseKey", "$SSL_GamePauseKey", Config.GamePauseKey)
		AddKeyMapOptionST("GameRaiseEnjKey", "$SSL_GameRaiseEnjKey", Config.GameRaiseEnjKey)
		AddKeyMapOptionST("GameHoldbackKey", "$SSL_GameHoldbackKey", Config.GameHoldbackKey)
		AddKeyMapOptionST("GameSelectNextPos", "$SSL_GameSelectNextPos", Config.GameSelectNextPos)
EndFunction

; ------------------------------------------------------- ;
; --- Debug & installation							              --- ;
; ------------------------------------------------------- ;

Function RebuildClean()
	SetCursorFillMode(TOP_TO_BOTTOM)
	AddHeaderOption("SexLab v" + GetStringVer() + " by Ashal@LoversLab.com")
	AddToggleOptionST("DebugMode","$SSL_DebugMode", Config.DebugMode)
	AddToggleOptionST("DebugMode2","$SSL_DebugMode2", Config.DebugMode2)
	AddToggleOptionST("DebugMode3","$SSL_DebugMode3", Config.DebugMode3)
	AddTextOptionST("StopCurrentAnimations","$SSL_StopCurrentAnimations", "$SSL_ClickHere")
	AddTextOptionST("ResetStripOverrides","$SSL_ResetStripOverrides", "$SSL_ClickHere")
	AddTextOptionST("CleanSystem","$SSL_CleanSystem", "$SSL_ClickHere")
	AddTextOptionST("ForceRegisterVoices", "$SSL_ForceRegisterVoices", "$SSL_ClickHere")
	AddHeaderOption("System Requirements")
	SystemCheckOptions()

	SetCursorPosition(1)
	AddHeaderOption("Registry Info")
	; IDEA: Allow clicking on this for more info, custom swf mayhaps?
	AddTextOption("$SSL_Animations", sslSystemConfig.GetAnimationCount(), OPTION_FLAG_DISABLED)
	AddTextOption("$SSL_Voices", sslVoiceSlots.GetAllVoices("").Length, OPTION_FLAG_DISABLED)
	AddTextOption("$SSL_Expressions", sslExpressionSlots.GetAllProfileIDs().Length, OPTION_FLAG_DISABLED)
	AddHeaderOption("$SSL_AvailableStrapons")
	AddTextOptionST("RebuildStraponList","$SSL_RebuildStraponList", "$SSL_ClickHere")
	int i = Config.Strapons.Length
	While i
		i -= 1
		String Name = Config.Strapons[i].GetName()
		AddTextOptionST("toggleStrapon_" + i, Name, "$SSL_Remove")
	EndWhile
EndFunction

Function InstallMenu()
	SetCursorFillMode(TOP_TO_BOTTOM)
	AddHeaderOption("SexLab v" + GetStringVer())
	SystemCheckOptions()

	SetCursorPosition(1)
	AddHeaderOption("$SSL_Installation")
	AddTextOption("$SSL_CurrentlyInstalling", "!")

	While (!SystemAlias.IsInstalled)
		Utility.WaitMenuMode(0.5)
	EndWhile
	ForcePageReset()
EndFunction

Function SystemCheckOptions()
	String[] okOrFail = new String[2]
	okOrFail[0] = "<font color='#FF0000'>X</font>"
	okOrFail[1] = "<font color='#00FF00'>ok</font>"

	AddTextOption("Skyrim Script Extender", okOrFail[Config.CheckSystemPart("SKSE") as int], OPTION_FLAG_DISABLED)
	AddTextOption("SexLab.dll", okOrFail[Config.CheckSystemPart("SexLabP+") as int], OPTION_FLAG_DISABLED)
	AddTextOption("PapyrusUtil.dll", okOrFail[Config.CheckSystemPart("PapyrusUtil") as int], OPTION_FLAG_DISABLED)
	AddTextOption("RaceMenu", okOrFail[Config.CheckSystemPart("NiOverride") as int], OPTION_FLAG_DISABLED)
	AddTextOption("MfgFix NG", okOrFail[Config.CheckSystemPart("MfgFix") as int], OPTION_FLAG_DISABLED)
EndFunction

State ResetStripOverrides
	Event OnSelectST()
		SetOptionFlagsST(OPTION_FLAG_DISABLED)
		SetTextOptionValueST("$SSL_Resetting")		
		ActorLib.ResetStripOverrides()
		ShowMessage("$Done", false)
		SetTextOptionValueST("$SSL_ClickHere")
		SetOptionFlagsST(OPTION_FLAG_NONE)
	EndEvent
EndState

State CleanSystem
	Event OnSelectST()
		If (!ShowMessage("$SSL_WarnCleanSystem"))
			return
		EndIf
		SystemAlias.SetupSystem()
		ModEvent.Send(ModEvent.Create("SexLabReset"))
		Config.CleanSystemFinish.Show()
	EndEvent
EndState

State RebuildStraponList
	Event OnSelectST()
		Config.LoadStrapons()
		If (Config.Strapons.Length > 0)
			ShowMessage("$SSL_FoundStrapon", false)
		Else
			ShowMessage("$SSL_NoStrapons", false)
		EndIf
		ForcePageReset()
	EndEvent
EndState

; ------------------------------------------------------- ;
; --- Mapped State Option Events                      --- ;
; ------------------------------------------------------- ;

Event OnSelectST()
	string[] s = PapyrusUtil.StringSplit(GetState(), "_")
	If (s[0] == "AutoAdvance")
		Config.AutoAdvance = !Config.AutoAdvance
		SetToggleOptionValueST(Config.AutoAdvance)
	ElseIf (s[0] == "DisableSub")
		Config.DisablePlayer = !Config.DisablePlayer
		SetToggleOptionValueST(Config.DisablePlayer)
	ElseIf (s[0] == "AutomaticTFC")
		Config.AutoTFC = !Config.AutoTFC
		SetToggleOptionValueST(Config.AutoTFC)
	ElseIf (s[0] == "OrgasmEffects")
		Config.OrgasmEffects = !Config.OrgasmEffects
		SetToggleOptionValueST(Config.OrgasmEffects)
	ElseIf (s[0] == "UseCum")
		Config.UseCum = !Config.UseCum
		SetToggleOptionValueST(Config.UseCum)
	ElseIf (s[0] == "UseExpressions")
		Config.UseExpressions = !Config.UseExpressions
		SetToggleOptionValueST(Config.UseExpressions)
	ElseIf (s[0] == "UseLipSync")
		Config.UseLipSync = !Config.UseLipSync
		SetToggleOptionValueST(Config.UseLipSync)
	ElseIf (s[0] == "AllowCreatures")
		Config.AllowCreatures = !Config.AllowCreatures
		SetToggleOptionValueST(Config.AllowCreatures)
	ElseIf (s[0] == "UseCreatureGender")
		Config.UseCreatureGender = !Config.UseCreatureGender
		SetToggleOptionValueST(Config.UseCreatureGender)
	ElseIf (s[0] == "UndressAnimation")
		Config.UndressAnimation = !Config.UndressAnimation
		SetToggleOptionValueST(Config.UndressAnimation)
	ElseIf (s[0] == "RedressVictim")
		Config.RedressVictim = !Config.RedressVictim
		SetToggleOptionValueST(Config.RedressVictim)
	ElseIf (s[0] == "DisableTeleport")
		Config.DisableTeleport = !Config.DisableTeleport
		SetToggleOptionValueST(Config.DisableTeleport)
	ElseIf (s[0] == "ShowInMap")
		Config.ShowInMap = !Config.ShowInMap
		SetToggleOptionValueST(Config.ShowInMap)
	ElseIf (s[0] == "SetAnimSpeedByEnjoyment")
		Config.SetAnimSpeedByEnjoyment = !Config.SetAnimSpeedByEnjoyment
		SetToggleOptionValueST(Config.SetAnimSpeedByEnjoyment)
	ElseIf (s[0] == "Voice")
		int idx = s[1] as int
		bool e = sslBaseVoice.GetEnabled(_voices[idx])
		sslBaseVoice.SetEnabled(_voices[idx], !e)
		SetToggleOptionValueST(!e)
	ElseIf(s[0] == "StrippingW")
		int i = s[1] as int
		int value = 1 - sslSystemConfig.GetSettingIntA("iStripForms", i)
		sslSystemConfig.SetSettingIntA("iStripForms", value, i)
		SetToggleOptionValueST(value)
	ElseIf(s[0] == "Stripping")
		int i = s[1] as int
		int n = s[2] as int
		int bit = Math.LeftShift(1, n)
		int value = Math.LogicalXor(sslSystemConfig.GetSettingIntA("iStripForms", i), bit)
		sslSystemConfig.SetSettingIntA("iStripForms", value, i)
    SetToggleOptionValueST(Math.LogicalAnd(value, bit))
	ElseIf (s[0] == "FullInventory")
		If (s[1] as int == 0)
			_playerDisplayAll = !_playerDisplayAll
		Else
			_targetDisplayAll = !_targetDisplayAll
		EndIf
		ForcePageReset()
	ElseIf(s[0] == "StripFlag")
		int n = s[1] as int
		int i = s[2] as int
		Form item
		If (n == 0)
			item = _playerItems[i]
		Else
			item = _targetItems[i]
		EndIf
		int j = sslActorLibrary.CheckStrip(item)
		If(j == -1)			; Never 			-> Always
			sslActorLibrary.WriteStrip(item, false)
		ElseIf(j == 1)	; Always			-> Unspecified
			sslActorLibrary.EraseStrip(item)
		ElseIf(j == 0)	; Unspecified	-> Never
			sslActorLibrary.WriteStrip(item, true)
		EndIf
		SetTextOptionValueST(GetStripState(item))
	ElseIf (s[0] == "AnimationTogglePage")
		_currentTogglePage += 1
		ForcePageReset()
	ElseIf (s[0] == "togglevisible")
		String[] anims = sslAnimationSlots.CreateProxyArray(0, _toggleGroupIdx, _toggleTags, _animPack[_animPackIdx])
		If (!anims.Length)
			return
		EndIf
		bool e = SexLabRegistry.IsSceneEnabled(anims[0])
		int i = 0
		While (i < anims.Length)
			SexLabRegistry.SetSceneEnabled(anims[i], !e)
			i += 1
		EndWhile
		ForcePageReset()
	ElseIf (s[0] == "AnimationToggle")
		String anim = s[1]
		bool e = SexLabRegistry.IsSceneEnabled(anim)
		SexLabRegistry.SetSceneEnabled(anim, !e)
		SetToggleOptionValueST(!e)
	ElseIf (s[0] == "enableexpression")
		bool e = sslBaseExpression.GetEnabled(_expression[_expressionIdx])
		sslBaseExpression.SetEnabled(_expression[_expressionIdx], !e)
		SetToggleOptionValueST(!e)
	ElseIf (s[0] == "expredittag")
		String toggle = s[1]
		String[] tags = sslBaseExpression.GetExpressionTags(_expression[_expressionIdx])
		If (tags.Find(toggle) > -1)
			tags = PapyrusUtil.RemoveString(tags, toggle)
		Else
			tags = PapyrusUtil.PushString(tags, toggle)
		EndIf
		sslBaseExpression.SetExpressionTags(_expression[_expressionIdx], tags)
		SetToggleOptionValueST(tags.Find(toggle) > -1)
	ElseIf (s[0] == "LipsSoundTime")
		Config.LipsSoundTime = 1 - Config.LipsSoundTime
		SetTextOptionValueST(_soundmethod[Config.LipsSoundTime])
	ElseIf (s[0] == "LipsFixedValue")
		Config.LipsFixedValue = !Config.LipsFixedValue
		SetToggleOptionValueST(Config.LipsFixedValue)
	ElseIf (s[0] == "expressioneditfemale")
		_editfemale = !_editfemale
		ForcePageReset()
	ElseIf (s[0] == "expressiontest")
		TestApply(PlayerRef)
	ElseIf (s[0] == "toggleStrapon")
		int i = s[1] as int
		Form[] Output
		Form[] Strapons = Config.Strapons
		int n = Strapons.Length
		while n
			n -= 1
			if n != i
				Output = PapyrusUtil.PushForm(Output, Strapons[n])
			endIf
		endWhile
		Config.Strapons = Output
		ForcePageReset()
	ElseIf (s[0] == "matchmakerInputTags")	; Matchmaker Tags
		ShowMessage(sslSystemConfig.ParseMMTagString(), false, "$Done")
	ElseIf (s[0] == "TextResetTags")
		If (!ShowMessage("$SSL_TagResetAreYouSure"))
			return
		EndIf
		sslSystemConfig.SetSettingStr("sRequiredTags", "")
		sslSystemConfig.SetSettingStr("sOptionalTags", "")
		sslSystemConfig.SetSettingStr("sExcludedTags", "")
		ForcePageReset()
	ElseIf (s[0] == "matchmakerToggleSubPlayer")
		Config.SubmissivePlayer = !Config.SubmissivePlayer
		SetToggleOptionValueST(Config.SubmissivePlayer)
	ElseIf (s[0] == "matchmakerToggleSubTarget")
		Config.SubmissiveTarget = !Config.SubmissiveTarget
		SetToggleOptionValueST(Config.SubmissiveTarget)
	ElseIf (s[0] == "expressionprev")
		_phaseIdx -= 2
		ForcePageReset()
	ElseIf (s[0] == "expressionnext")
		_phaseIdx += 2
		ForcePageReset()
	ElseIf (s[0] == "DebugMode")
		Config.DebugMode = !Config.DebugMode
		SetToggleOptionValueST(Config.DebugMode)
	ElseIf (s[0] == "DebugMode2")
		Config.DebugMode2 = !Config.DebugMode2
		SetToggleOptionValueST(Config.DebugMode2)
	ElseIf (s[0] == "DebugMode3")
		Config.DebugMode3 = !Config.DebugMode3
		SetToggleOptionValueST(Config.DebugMode3)
	ElseIf (s[0] == "StopCurrentAnimations")
		ShowMessage("$SSL_StopRunningAnimations", false)
		ThreadSlots.StopAll()
	; Enjoyment
	ElseIf (s[0] == "InternalEnjoymentEnabled")
		Config.InternalEnjoymentEnabled = !Config.InternalEnjoymentEnabled
		SetToggleOptionValueST(Config.InternalEnjoymentEnabled)
	ElseIf (s[0] == "NoStaminaEndsScene")
		Config.NoStaminaEndsScene = !Config.NoStaminaEndsScene
		SetToggleOptionValueST(Config.NoStaminaEndsScene)
	ElseIf (s[0] == "MaleOrgasmEndsScene")
		Config.MaleOrgasmEndsScene = !Config.MaleOrgasmEndsScene
		SetToggleOptionValueST(Config.MaleOrgasmEndsScene)	
	ElseIf (s[0] == "DomMustOrgasm")
		Config.DomMustOrgasm = !Config.DomMustOrgasm
		SetToggleOptionValueST(Config.DomMustOrgasm)
	ElseIf (s[0] == "PlayerMustOrgasm")
		Config.PlayerMustOrgasm = !Config.PlayerMustOrgasm
		SetToggleOptionValueST(Config.PlayerMustOrgasm)
	ElseIf (s[0] == "HighEnjOrgasmWait")
		Config.HighEnjOrgasmWait = !Config.HighEnjOrgasmWait
		SetToggleOptionValueST(Config.HighEnjOrgasmWait)
	ElseIf (s[0] == "GameEnabled")
		Config.GameEnabled = !Config.GameEnabled
		SetToggleOptionValueST(Config.GameEnabled)
	ElseIf (s[0] == "GamePlayerAutoplay")
		Config.GamePlayerAutoplay = !Config.GamePlayerAutoplay
		SetToggleOptionValueST(Config.GamePlayerAutoplay)
	ElseIf (s[0] == "GamePlayerVictimAutoplay")
		Config.GamePlayerVictimAutoplay = !Config.GamePlayerVictimAutoplay
		SetToggleOptionValueST(Config.GamePlayerVictimAutoplay)
	ElseIf (s[0] == "GameNPCAutoplay")
		Config.GameNPCAutoplay = !Config.GameNPCAutoplay
		SetToggleOptionValueST(Config.GameNPCAutoplay)
	ElseIf (s[0] == "GameEnjReductionChance")
		Config.GameEnjReductionChance = !Config.GameEnjReductionChance
		SetToggleOptionValueST(Config.GameEnjReductionChance)
	EndIf
EndEvent

Event OnSliderOpenST()
	string[] s = PapyrusUtil.StringSplit(GetState(), "_")
	If (s[0] == "AutomaticSUCSM")
		SetSliderDialogStartValue(Config.AutoSUCSM)
		SetSliderDialogDefaultValue(5)
		SetSliderDialogRange(1, 20)
		SetSliderDialogInterval(1)
	ElseIf (s[0] == "ShakeStrength")
		SetSliderDialogStartValue(Config.ShakeStrength * 100)
		SetSliderDialogDefaultValue(70)
		SetSliderDialogRange(0, 100)
		SetSliderDialogInterval(5)
	ElseIf (s[0] == "LovenseStrength")
		SetSliderDialogStartValue(sslSystemConfig.GetSettingInt("iLovenseStrength"))
		SetSliderDialogDefaultValue(10)
		SetSliderDialogRange(0, 20)
		SetSliderDialogInterval(1)
	ElseIf (s[0] == "LovenseStrengthOrgasm")
		SetSliderDialogStartValue(sslSystemConfig.GetSettingInt("iLovenseStrengthOrgasm"))
		SetSliderDialogDefaultValue(20)
		SetSliderDialogRange(0, 20)
		SetSliderDialogInterval(1)
	ElseIf (s[0] == "LovenseDurationOrgasm")
		SetSliderDialogStartValue(sslSystemConfig.GetSettingFlt("fLovenseDurationOrgasm"))
		SetSliderDialogDefaultValue(8)
		SetSliderDialogRange(5, 30)
		SetSliderDialogInterval(0.5)
	ElseIf (s[0] == "CumEffectTimer")
		SetSliderDialogStartValue(Config.CumTimer)
		SetSliderDialogDefaultValue(120)
		SetSliderDialogRange(0, 43200)
		SetSliderDialogInterval(10)
	ElseIf (s[0] == "VoiceVolume")
		SetSliderDialogStartValue(Config.VoiceVolume * 100)
		SetSliderDialogDefaultValue(100)
		SetSliderDialogRange(1, 100)
		SetSliderDialogInterval(1)
	ElseIf (s[0] == "SFXVolume")
		SetSliderDialogStartValue(Config.SFXVolume * 100)
		SetSliderDialogDefaultValue(100)
		SetSliderDialogRange(1, 100)
		SetSliderDialogInterval(1)
	ElseIf (s[0] == "MaleVoiceDelay")
		SetSliderDialogStartValue(Config.MaleVoiceDelay)
		SetSliderDialogDefaultValue(5)
		SetSliderDialogRange(1, 45)
		SetSliderDialogInterval(1)
	ElseIf (s[0] == "FemaleVoiceDelay")
		SetSliderDialogStartValue(Config.FemaleVoiceDelay)
		SetSliderDialogDefaultValue(4)
		SetSliderDialogRange(1, 45)
		SetSliderDialogInterval(1)
	ElseIf (s[0] == "SFXDelay")
		SetSliderDialogStartValue(Config.SFXDelay)
		SetSliderDialogDefaultValue(3)
		SetSliderDialogRange(1, 30)
		SetSliderDialogInterval(1)
	ElseIf (s[0] == "StageTimers")
		int i = s[1] as int
		SetSliderDialogStartValue(sslSystemConfig.GetSettingFltA("fTimers", i))
		SetSliderDialogRange(3, 180)
		SetSliderDialogInterval(1)
		SetSliderDialogDefaultValue(15)
	ElseIf (s[0] == "expredit")
		int i = s[1] as int
		float[] values
		If ((s[2] as int) == 0)
			values = _low
		Else
			values = _high
		EndIf
		SetSliderDialogStartValue(values[i])
		SetSliderDialogRange(0, 1)
		SetSliderDialogInterval(0.05)
		SetSliderDialogDefaultValue(0)
	; Enjoyment
	ElseIf (s[0] == "InterDetectionStrength")
		SetSliderDialogStartValue(Config.InterDetectionStrength)
		SetSliderDialogRange(1, 4)
		SetSliderDialogInterval(1)
		SetSliderDialogDefaultValue(4)
	ElseIf (s[0] == "EnjRaiseMultInter")
		SetSliderDialogStartValue(Config.EnjRaiseMultInter)
		SetSliderDialogRange(0.0, 3.0)
		SetSliderDialogInterval(0.1)
		SetSliderDialogDefaultValue(1.2)
	ElseIf (s[0] == "EnjGainOnStageSkip")
		SetSliderDialogStartValue(Config.EnjGainOnStageSkip)
		SetSliderDialogRange(0, 30)
		SetSliderDialogInterval(1)
		SetSliderDialogDefaultValue(15)
	ElseIf (s[0] == "MaxNoPainOrgasmMale")
		SetSliderDialogStartValue(Config.MaxNoPainOrgasmMale)
		SetSliderDialogRange(1, 5)
		SetSliderDialogInterval(1)
		SetSliderDialogDefaultValue(1)
	ElseIf (s[0] == "MaxNoPainOrgasmFemale")
		SetSliderDialogStartValue(Config.MaxNoPainOrgasmFemale)
		SetSliderDialogRange(1, 5)
		SetSliderDialogInterval(1)
		SetSliderDialogDefaultValue(2)
	ElseIf (s[0] == "NoPainRequiredTime")
		SetSliderDialogStartValue(Config.NoPainRequiredTime)
		SetSliderDialogRange(0, 180)
		SetSliderDialogInterval(10)
		SetSliderDialogDefaultValue(50)
	ElseIf (s[0] == "NoPainRequiredXP")
		SetSliderDialogStartValue(Config.NoPainRequiredXP)
		SetSliderDialogRange(0, 100)
		SetSliderDialogInterval(5)
		SetSliderDialogDefaultValue(50)		
	ElseIf (s[0] == "EnjMultVictim")
		SetSliderDialogStartValue(Config.EnjMultVictim)
		SetSliderDialogRange(0.0, 2.0)
		SetSliderDialogInterval(0.1)
		SetSliderDialogDefaultValue(0.8)
	ElseIf (s[0] == "EnjMultAggressor")
		SetSliderDialogStartValue(Config.EnjMultAggressor)
		SetSliderDialogRange(0.0, 2.0)
		SetSliderDialogInterval(0.1)
		SetSliderDialogDefaultValue(1.2)
	ElseIf (s[0] == "EnjMultSub")
		SetSliderDialogStartValue(Config.EnjMultSub)
		SetSliderDialogRange(0.0, 2.0)
		SetSliderDialogInterval(0.1)
		SetSliderDialogDefaultValue(0.8)
	ElseIf (s[0] == "EnjMultDom")
		SetSliderDialogStartValue(Config.EnjMultDom)
		SetSliderDialogRange(0.0, 2.0)
		SetSliderDialogInterval(0.1)
		SetSliderDialogDefaultValue(1.2)
	ElseIf (s[0] == "PainHugePPMult")
		SetSliderDialogStartValue(Config.PainHugePPMult)
		SetSliderDialogRange(0.0, 2.0)
		SetSliderDialogInterval(0.1)
		SetSliderDialogDefaultValue(0.5)
	EndIf
EndEvent

Event OnSliderAcceptST(float value)
	string[] s = PapyrusUtil.StringSplit(GetState(), "_")
	If (s[0] == "AutomaticSUCSM")
		Config.AutoSUCSM = value
		SetSliderOptionValueST(Config.AutoSUCSM, "{0}")
	ElseIf (s[0] == "ShakeStrength")
		Config.ShakeStrength = (value / 100.0)
		SetSliderOptionValueST(value, "{0}%")
	ElseIf (s[0] == "LovenseStrength")
		sslSystemConfig.SetSettingInt("iLovenseStrength", value as int)
		SetSliderOptionValueST(value, "{0}")
	ElseIf (s[0] == "LovenseStrengthOrgasm")
		sslSystemConfig.SetSettingInt("iLovenseStrengthOrgasm", value as int)
		SetSliderOptionValueST(value, "{0}")
	ElseIf (s[0] == "LovenseDurationOrgasm")
		sslSystemConfig.SetSettingFlt("fLovenseDurationOrgasm", value)
		SetSliderOptionValueST(value, "{1}s")
	ElseIf (s[0] == "CumEffectTimer")
		Config.CumTimer = value
		SetSliderOptionValueST(Config.CumTimer, "$SSL_Seconds")
	ElseIf (s[0] == "VoiceVolume")
		Config.VoiceVolume = (value / 100.0)
		Config.AudioVoice.SetVolume(Config.VoiceVolume)
		SetSliderOptionValueST(value, "{0}%")
	ElseIf (s[0] == "SFXVolume")
		Config.SFXVolume = (value / 100.0)
		Config.AudioSFX.SetVolume(Config.SFXVolume)
		SetSliderOptionValueST(value, "{0}%")
	ElseIf (s[0] == "MaleVoiceDelay")
		Config.MaleVoiceDelay = value
		SetSliderOptionValueST(Config.MaleVoiceDelay, "$SSL_Seconds")
	ElseIf (s[0] == "FemaleVoiceDelay")
		Config.FemaleVoiceDelay = value
		SetSliderOptionValueST(Config.FemaleVoiceDelay, "$SSL_Seconds")
	ElseIf (s[0] == "SFXDelay")
		Config.SFXDelay = value
		SetSliderOptionValueST(Config.SFXDelay, "$SSL_Seconds")
	ElseIf (s[0] == "StageTimers")
		int i = s[1] as int
		sslSystemConfig.SetSettingFltA("fTimers", value, i)
		SetSliderOptionValueST(value, "$SSL_Seconds")
	ElseIf (s[0] == "expredit")
		int i = s[1] as int
		int n = s[2] as int
		float[] values
		If (n == 0)
			values = _low
		Else
			values = _high
		EndIf
		values[i] = value
		sslBaseExpression.SetValues(_expression[_expressionIdx], _editFemale, _phaseidx + n, values)
		SetSliderOptionValueST(value, "{2}")
	; Enjoyment
	ElseIf (s[0] == "InterDetectionStrength")
		Config.InterDetectionStrength = value as int
		SetSliderOptionValueST(Config.InterDetectionStrength)
	ElseIf (s[0] == "EnjRaiseMultInter")
		Config.EnjRaiseMultInter = value
		SetSliderOptionValueST(Config.EnjRaiseMultInter, "{1}")
	ElseIf (s[0] == "EnjGainOnStageSkip")
		Config.EnjGainOnStageSkip = value as int
		SetSliderOptionValueST(Config.EnjGainOnStageSkip)
	ElseIf (s[0] == "MaxNoPainOrgasmMale")
		Config.MaxNoPainOrgasmMale = value as int
		SetSliderOptionValueST(Config.MaxNoPainOrgasmMale)
	ElseIf (s[0] == "MaxNoPainOrgasmFemale")
		Config.MaxNoPainOrgasmFemale = value as int
		SetSliderOptionValueST(Config.MaxNoPainOrgasmFemale)
	ElseIf (s[0] == "NoPainRequiredTime")
		Config.NoPainRequiredTime = value as int
		SetSliderOptionValueST(Config.NoPainRequiredTime)
	ElseIf (s[0] == "NoPainRequiredXP")
		Config.NoPainRequiredXP = value as int
		SetSliderOptionValueST(Config.NoPainRequiredXP)
	ElseIf (s[0] == "EnjMultVictim")
		Config.EnjMultVictim = value
		SetSliderOptionValueST(Config.EnjMultVictim, "{1}")
	ElseIf (s[0] == "EnjMultAggressor")
		Config.EnjMultAggressor = value
		SetSliderOptionValueST(Config.EnjMultAggressor, "{1}")
	ElseIf (s[0] == "EnjMultSub")
		Config.EnjMultSub = value
		SetSliderOptionValueST(Config.EnjMultSub, "{1}")
	ElseIf (s[0] == "EnjMultDom")
		Config.EnjMultDom = value
		SetSliderOptionValueST(Config.EnjMultDom, "{1}")
	ElseIf (s[0] == "PainHugePPMult")
		Config.PainHugePPMult = value
		SetSliderOptionValueST(Config.PainHugePPMult, "{1}")
	EndIf
EndEvent

Event OnMenuOpenST()
	String[] s = PapyrusUtil.StringSplit(GetState(), "_")
	If (s[0] == "ClimaxType")
		SetMenuDialogStartIndex(sslSystemConfig.GetSettingInt("iClimaxType"))
		SetMenuDialogDefaultIndex(2)
		SetMenuDialogOptions(_ClimaxTypes)
	ElseIf (s[0] == "FilterStrictness")
		SetMenuDialogStartIndex(sslSystemConfig.GetSettingInt("iFilterStrictness"))
		SetMenuDialogDefaultIndex(1)
		SetMenuDialogOptions(_FilterOpt)
	ElseIf (s[0] == "SexSelect")
		int sex
		If (s[1] == "0")
			sex = SexLabRegistry.GetSex(PlayerRef, true)
		Else
			sex = SexLabRegistry.GetSex(Config.TargetRef, true)
		EndIf
		String[] options
		If (sex <= 2)	; Human
			options = _Sexes
		Else					; Creature
			options = new String[2]
			options[0] = "$SSL_Male"
			options[1] = "$SSL_Female"
		EndIf
		SetMenuDialogStartIndex(sex % 3)
		SetMenuDialogDefaultIndex(sex % 3)
		SetMenuDialogOptions(options)
	ElseIf (s[0] == "UseFade")
		SetMenuDialogStartIndex(sslSystemConfig.GetSettingInt("iUseFade"))
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(_FadeOpt)
	ElseIf (s[0] == "TSModeSelect")
		SetMenuDialogStartIndex(_stripViewIdx)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(_stripView)
	ElseIf (s[0] == "togglepackage")
		SetMenuDialogStartIndex(_animPackIdx)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(_animPack)
	ElseIf (s[0] == "togglecategory")
		SetMenuDialogStartIndex(_toggleGroupIdx)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(_toggleGroup)
	ElseIf (s[0] == "selectexpression")
		SetMenuDialogStartIndex(_expressionIdx)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(_expression)
	ElseIf (s[0] == "setexprscaling")
		int scalemode = sslBaseExpression.GetExpressionScaleMode(_expression[_expressionIdx])
		SetMenuDialogStartIndex(scalemode)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(_expressionScales)
	ElseIf (s[0] == "expredit")
		int i = s[1] as int
		int n = s[2] as int
		float[] values
		If (n == 0)
			values = _low
		Else
			values = _high
		EndIf
		SetMenuDialogStartIndex(values[i] as int)
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(_moods)
	ElseIf (s[0] == "activeVoices")
		SetMenuDialogStartIndex(SexLabRegistry.MapRaceKeyToId(_voiceActiveRaceKey))
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(SexLabRegistry.GetAllRaceKeys(false))
	ElseIf (s[0] == "FurniturePlayer")
		SetMenuDialogStartIndex(sslSystemConfig.GetSettingInt("iAskBed"))
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(_PlFurnOpt)
	ElseIf (s[0] == "FurnitureNPC")
		SetMenuDialogStartIndex(sslSystemConfig.GetSettingInt("iNPCBed"))
		SetMenuDialogDefaultIndex(0)
		SetMenuDialogOptions(_NPCFurnOpt)
	EndIf
EndEvent

Event OnMenuAcceptST(int aiIndex)
	String[] s = PapyrusUtil.StringSplit(GetState(), "_")
	If (aiIndex < 0)
		return
	EndIf
	If (s[0] == "ClimaxType")
		sslSystemConfig.SetSettingInt("iClimaxType", aiIndex)
		SetMenuOptionValueST(_ClimaxTypes[aiIndex])
	ElseIf (s[0] == "FilterStrictness")
		sslSystemConfig.SetSettingInt("iFilterStrictness", aiIndex)
		SetMenuOptionValueST(_FilterOpt[aiIndex])
	ElseIf (s[0] == "SexSelect")
		If (s[1] == "0")
			ActorLib.TreatAsSex(PlayerRef, aiIndex)
		Else
			ActorLib.TreatAsSex(Config.TargetRef, aiIndex)
		EndIf
		SetMenuOptionValueST(_Sexes[aiIndex])
	ElseIf (s[0] == "UseFade")
		sslSystemConfig.SetSettingInt("iUseFade", aiIndex)
		SetMenuOptionValueST(_FadeOpt[aiIndex])
	ElseIf (s[0] == "TSModeSelect")
		_stripViewIdx = aiIndex
		ForcePageReset()
	ElseIf (s[0] == "togglepackage")
		_animPackIdx = aiIndex
		ForcePageReset()
	ElseIf (s[0] == "togglecategory")
		_toggleGroupIdx = aiIndex
		ForcePageReset()
	ElseIf (s[0] == "selectexpression")
		_expressionIdx = aiIndex
		ForcePageReset()
	ElseIf (s[0] == "setexprscaling")
		sslBaseExpression.SetExpressionScaleMode(_expression[_expressionIdx], aiIndex)
		SetMenuOptionValueST(_expressionScales[aiIndex])
	ElseIf (s[0] == "expredit")
		int i = s[1] as int
		int n = s[2] as int
		float[] values
		If (n == 0)
			values = _low
		Else
			values = _high
		EndIf
		values[i] = aiIndex
		sslBaseExpression.SetValues(_expression[_expressionIdx], _editFemale, _phaseidx + n, values)
		SetMenuOptionValueST(_moods[aiIndex])
	ElseIf (s[0] == "activeVoices")
		_voiceActiveRaceKey = SexLabRegistry.MapRaceIDToRaceKey(aiIndex)
		_voices = sslVoiceSlots.GetAllVoices(_voiceActiveRaceKey)
		ForcePageReset()
	ElseIf (s[0] == "FurniturePlayer")
		sslSystemConfig.SetSettingInt("iAskBed", aiIndex)
		SetMenuOptionValueST(_PlFurnOpt[aiIndex])
	ElseIf (s[0] == "FurnitureNPC")
		sslSystemConfig.SetSettingInt("iAskBedNPC", aiIndex)
		SetMenuOptionValueST(_NpcFurnOpt[aiIndex])
	EndIf
EndEvent

Event OnInputOpenST()
	String[] s = PapyrusUtil.StringSplit(GetState(), "_")
	If (s[0] == "matchmakerInputRequiredTags")
		SetInputDialogStartText(Config.RequiredTags)
	ElseIf (s[0] == "matchmakerInputExcludedTags")
		SetInputDialogStartText(Config.ExcludedTags)
	ElseIf (s[0] == "matchmakerInputOptionalTags")
		SetInputDialogStartText(Config.OptionalTags)
	ElseIf (s[0] == "toggletags")
		SetInputDialogStartText(_toggleTags)
	ElseIf (s[0] == "createexpression")
		SetInputDialogStartText("")
	EndIf
EndEvent

Event OnInputAcceptST(String inputString)
	String[] s = PapyrusUtil.StringSplit(GetState(), "_")
	If (s[0] == "matchmakerInputRequiredTags")
		Config.RequiredTags = inputString
		ForcePageReset()
	ElseIf (s[0] == "matchmakerInputExcludedTags")
		Config.ExcludedTags = inputString
		ForcePageReset()
	ElseIf (s[0] == "matchmakerInputOptionalTags")
		Config.OptionalTags = inputString
		ForcePageReset()
	ElseIf (s[0] == "toggletags")
		_toggleTags = inputString
		ForcePageReset()
	ElseIf (s[0] == "createexpression")
		If (!sslBaseExpression.CreateEmptyProfile(inputString))
			ShowMessage("$SSL_CreateProfileError", false, "$Ok")
			return
		EndIf
		_expression = sslExpressionSlots.GetAllProfileIDs()
		ForcePageReset()
	EndIf
EndEvent

Event OnHighlightST()
	string[] s = PapyrusUtil.StringSplit(GetState(), "_")
	If (s[0] == "AutoAdvance")
		SetInfoText("$SSL_InfoAutoAdvance")
	ElseIf (s[0] == "ClimaxType") 
		SetInfoText("$SSL_ClimaxInfo")
	ElseIf (s[0] == "SexSelect")
		SetInfoText("$SSL_SexSelectInfo")
	ElseIf (s[0] == "UseFade")
		SetInfoText("$SSL_UseFadeInfo")
	ElseIf (s[0] == "FilterStrictness")
		SetInfoText("$SSL_FilterStrictnessInfo")
	ElseIf (s[0] == "DisableSub")
		SetInfoText("$SSL_DisableSubControlsInfo")
	ElseIf (s[0] == "AutomaticTFC")
		SetInfoText("$SSL_InfoAutomaticTFC")
	ElseIf (s[0] == "AutomaticSUCSM")
		SetInfoText("$SSL_InfoAutomaticSUCSM")
	ElseIf (s[0] == "OrgasmEffects")
		SetInfoText("$SSL_InfoOrgasmEffects")
	ElseIf (s[0] == "ShakeStrength")
		SetInfoText("$SSL_InfoShakeStrength")
	ElseIf (s[0] == "UseCum")
		SetInfoText("$SSL_InfoUseCum")
	ElseIf (s[0] == "CumEffectTimer")
		SetInfoText("$SSL_InfoCumTimer")
	ElseIf (s[0] == "UseExpressions")
		SetInfoText("$SSL_InfoUseExpressions")
	ElseIf (s[0] == "UseLipSync")
		SetInfoText("$SSL_InfoUseLipSync")
	ElseIf (s[0] == "LovenseStrength")
		SetInfoText("$SSL_LovenseStrengthHighlight")
	ElseIf (s[0] == "LovenseStrengthOrgasm")
		SetInfoText("$SSL_LovenseStrengthOrgasmHighlight")
	ElseIf (s[0] == "LovenseDurationOrgasm")
		SetInfoText("$SSL_LovenseDurationOrgasmHighlight")
	ElseIf (s[0] == "AllowCreatures")
		SetInfoText("$SSL_InfoAllowCreatures")
	ElseIf (s[0] == "UseCreatureGender")
		SetInfoText("$SSL_InfoUseCreatureGender")
	ElseIf (s[0] == "UndressAnimation")
		SetInfoText("$SSL_InfoUndressAnimation")
	ElseIf (s[0] == "RedressVictim")
		SetInfoText("$SSL_InfoReDressVictim")
	ElseIf (s[0] == "DisableTeleport")
		SetInfoText("$SSL_InfoDisableTeleport")
	ElseIf (s[0] == "ShowInMap")
		SetInfoText("$SSL_InfoShowInMap")
	ElseIf (s[0] == "SetAnimSpeedByEnjoyment")
		SetInfoText("$SSL_InfoSetAnimSpeedByEnjoyment")
	ElseIf (s[0] == "VoiceVolume")
		SetInfoText("$SSL_InfoVoiceVolume")
	ElseIf (s[0] == "SFXVolume")
		SetInfoText("$SSL_InfoSFXVolume")
	ElseIf (s[0] == "MaleVoiceDelay")
		SetInfoText("$SSL_InfoMaleVoiceDelay")
	ElseIf (s[0] == "FemaleVoiceDelay")
		SetInfoText("$SSL_InfoFemaleVoiceDelay")
	ElseIf (s[0] == "SFXDelay")
		SetInfoText("$SSL_InfoSFXDelay")
	ElseIf (s[0] == "Voice")
		int idx = s[1] as int
		String[] tags = sslBaseVoice.GetVoiceTags(_voices[idx])
		SetInfoText("Tags: " + PapyrusUtil.StringJoin(tags, ", "))
	ElseIf(s[0] == "Stripping")
		int i = s[2] as int
		String info = PlayerRef.GetLeveledActorBase().GetName() + " Slot " + (i + 30) + ": "
		info += GetItemName(PlayerRef.GetWornForm(Armor.GetMaskForSlot(i + 30)), "?")
		If (Config.TargetRef)
			info += "\n" + Config.TargetRef.GetLeveledActorBase().GetName() + " Slot " + (i + 30) + ": "
			info += GetItemName(Config.TargetRef.GetWornForm(Armor.GetMaskForSlot(i + 30)), "?")
		EndIf
		SetInfoText(info)
	ElseIf(s[0] == "StripFlag")
		int n = s[1] as int
		int i = s[2] as int
		Form item
		If (n == 0)
			item = _playerItems[i]
		Else
			item = _targetItems[i]
		EndIf
		String InfoText = GetItemName(item, "?")
		Armor ArmorRef = item as Armor
		If(ArmorRef)
			InfoText += "\nArmor Slots: " + GetAllMaskSlots(ArmorRef.GetSlotMask())
		Else
			InfoText += "\nWeapon"
		EndIf
		SetInfoText(InfoText)
	Elseif (s[0] == "AnimationToggle")
		String anim = s[1]
		SetInfoText("Tags: " + PapyrusUtil.StringJoin(SexLabRegistry.GetSceneTags(anim), ", "))
	ElseIf(s[0] == "selectexpression")
		String[] tags = sslBaseExpression.GetExpressionTags(_expression[_expressionIdx])
		SetInfoText("Tags: " + PapyrusUtil.StringJoin(tags, ", "))
	ElseIf (s[0] == "createexpression")
		SetInfoText("$SSL_CreateExpressionInfo")
	ElseIf (s[0] == "LipsSoundTime")
		SetInfoText("$SSL_InfoLipsSoundTime")
	ElseIf (s[0] == "LipsFixedValue")
		SetInfoText("$SSL_InfoLipsFixedValue")
	ElseIf (s[0] == "DebugMode")
		SetInfoText("$SSL_InfoDebugMode")
	ElseIf (s[0] == "DebugMode2")
		SetInfoText("$SSL_InfoDebugMode2")
	ElseIf (s[0] == "DebugMode3")
		SetInfoText("$SSL_InfoDebugMode3")
	ElseIf (s[0] == "setexprscaling")
		SetInfoText("$SSL_ExpressionScalingInfo")
	ElseIf (s[0] == "activeVoices")
		SetInfoText("$SSL_ActiveVoicesHighlight")
	Else
		SetInfoText("$SSL_" + s[0] + "Highlight")
	EndIf
EndEvent

; ------------------------------------------------------- ;
; --- Player Hotkeys                                  --- ;
; ------------------------------------------------------- ;

function PlayerHotkeys()
	SetCursorFillMode(LEFT_TO_RIGHT)

	AddHeaderOption("$SSL_GlobalHotkeys")
	AddEmptyOption()
	AddKeyMapOptionST("TargetActor", "$SSL_TargetActor", Config.TargetActor)
	AddKeyMapOptionST("ToggleFreeCamera", "$SSL_ToggleFreeCamera", Config.ToggleFreeCamera)

	AddHeaderOption("$SSL_SceneManipulation")
	AddEmptyOption()
	AddKeyMapOptionST("M_S_iKeyUp", "$SSL_KeyUp", sslSystemConfig.GetSettingInt("iKeyUp"))
	AddKeyMapOptionST("M_S_iKeyExtra2", "$SSL_KeyExtra2", sslSystemConfig.GetSettingInt("iKeyExtra2"))
	AddKeyMapOptionST("M_S_iKeyDown", "$SSL_KeyDown", sslSystemConfig.GetSettingInt("iKeyDown"))
	AddKeyMapOptionST("M_S_iKeyMod", "$SSL_KeyMod", sslSystemConfig.GetSettingInt("iKeyMod"))
	AddKeyMapOptionST("M_S_iKeyLeft", "$SSL_KeyLeft", sslSystemConfig.GetSettingInt("iKeyLeft"))
	AddKeyMapOptionST("M_S_iKeyReset", "$SSL_KeyReset", sslSystemConfig.GetSettingInt("iKeyReset"))
	AddKeyMapOptionST("M_S_iKeyRight", "$SSL_KeyRight", sslSystemConfig.GetSettingInt("iKeyRight"))
	AddKeyMapOptionST("M_S_iKeyEnd", "$SSL_KeyEnd", sslSystemConfig.GetSettingInt("iKeyEnd"))
	AddKeyMapOptionST("M_S_iKeyAdvance", "$SSL_KeyAdvance", sslSystemConfig.GetSettingInt("iKeyAdvance"))
	; AddKeyMapOptionST("RealignActors","$SSL_RealignActors", Config.RealignActors)
	; AddKeyMapOptionST("EndAnimation", "$SSL_EndAnimation", Config.EndAnimation)
	; AddKeyMapOptionST("AdvanceAnimation", "$SSL_AdvanceAnimationStage", Config.AdvanceAnimation)
	; AddKeyMapOptionST("ChangeAnimation", "$SSL_ChangeAnimationSet", Config.ChangeAnimation)
	; AddKeyMapOptionST("ChangePositions", "$SSL_SwapActorPositions", Config.ChangePositions)
	; AddKeyMapOptionST("MoveSceneLocation", "$SSL_MoveSceneLocation", Config.MoveScene)

	; SetCursorPosition(1)
	; AddHeaderOption("$SSL_AlignmentAdjustments")
	; AddTextOptionST("AdjustTargetStage", "$SSL_AdjustTargetStage", SexLabUtil.StringIfElse(Config.AdjustTargetStage, "$SSL_CurrentStage", "$SSL_AllStages"))
	; AddKeyMapOptionST("AdjustStage", SexLabUtil.StringIfElse(Config.AdjustTargetStage, "$SSL_AdjustAllStages", "$SSL_AdjustStage"), Config.AdjustStage)
	; AddKeyMapOptionST("BackwardsModifier", "$SSL_ReverseDirectionModifier", Config.Backwards)
	; AddKeyMapOptionST("AdjustChange","$SSL_ChangeActorBeingMoved", Config.AdjustChange)
	; AddKeyMapOptionST("AdjustForward","$SSL_MoveActorForwardBackward", Config.AdjustForward)
	; AddKeyMapOptionST("AdjustUpward","$SSL_AdjustPositionUpwardDownward", Config.AdjustUpward)
	; AddKeyMapOptionST("AdjustSideways","$SSL_MoveActorLeftRight", Config.AdjustSideways)
	; AddKeyMapOptionST("AdjustSchlong","$SSL_AdjustSchlong", Config.AdjustSchlong)
	; AddKeyMapOptionST("RotateScene", "$SSL_RotateScene", Config.RotateScene)
	; AddKeyMapOptionST("RestoreOffsets","$SSL_DeleteSavedAdjustments", Config.RestoreOffsets)
endFunction


Event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
  String[] s = StringUtil.Split(GetState(), "_")
	int i = 0
	bool mandatory = false
	bool skipConflict = false
	If (s[i] == "M")
		i += 1
		mandatory = true
	EndIf
	If (s[i] == "S")
		i += 1
		skipConflict = true
	EndIf
  If(newKeyCode == 1 || newKeyCode == 277)
		If (mandatory)
			ShowMessage("$SSL_KeyCannotBeDisabled", false, "$Ok")
			return
		EndIf
    newKeyCode = -1
  EndIf
  If(newKeyCode != -1 && conflictControl != "" && !skipConflict)
    string msg
    If(conflictName != "")
      msg = "$SSL_ConflictControl{" + conflictControl + "}{" + conflictName + "}"
    Else
      msg = "$SSL_ConflictControl{" + conflictControl + "}"
    EndIf
    If(!ShowMessage(msg, true, "$Yes", "$No"))
      return
    EndIf
  EndIf
	sslSystemConfig.SetSettingInt(s[i], newKeyCode)
  SetKeyMapOptionValueST(newKeyCode)
EndEvent

bool function KeyConflict(int newKeyCode, string conflictControl, string conflictName)
	bool continue = true
	if (conflictControl != "")
		string msg
		if (conflictName != "")
			msg = "This key is already mapped to: \n'" + conflictControl + "'\n(" + conflictName + ")\n\nAre you sure you want to continue?"
		else
			msg = "This key is already mapped to: \n'" + conflictControl + "'\n\nAre you sure you want to continue?"
		endIf
		continue = ShowMessage(msg, true, "$Yes", "$No")
	endIf
	return !continue
endFunction

state AdjustStage
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.AdjustStage = newKeyCode
			SetKeyMapOptionValueST(Config.AdjustStage)
		endIf
	endEvent
	event OnDefaultST()
		Config.AdjustStage = 157
		SetKeyMapOptionValueST(Config.AdjustStage)
	endEvent
	event OnHighlightST()
		SetInfoText(SexLabUtil.StringIfElse(Config.AdjustTargetStage, "$SSL_InfoAdjustAllStages", "$SSL_InfoAdjustStage"))
	endEvent
endState
state AdjustChange
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.AdjustChange = newKeyCode
			SetKeyMapOptionValueST(Config.AdjustChange)
		endIf
	endEvent
	event OnDefaultST()
		Config.AdjustChange = 37
		SetKeyMapOptionValueST(Config.AdjustChange)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoAdjustChange")
	endEvent
endState
state AdjustForward
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.AdjustForward = newKeyCode
			SetKeyMapOptionValueST(Config.AdjustForward)
		endIf
	endEvent
	event OnDefaultST()
		Config.AdjustForward = 38
		SetKeyMapOptionValueST(Config.AdjustForward)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoAdjustForward")
	endEvent
endState
state AdjustUpward
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.AdjustUpward = newKeyCode
			SetKeyMapOptionValueST(Config.AdjustUpward)
		endIf
	endEvent
	event OnDefaultST()
		Config.AdjustUpward = 39
		SetKeyMapOptionValueST(Config.AdjustUpward)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoAdjustUpward")
	endEvent
endState
state AdjustSideways
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.AdjustSideways = newKeyCode
			SetKeyMapOptionValueST(Config.AdjustSideways)
		endIf
	endEvent
	event OnDefaultST()
		Config.AdjustSideways = 40
		SetKeyMapOptionValueST(Config.AdjustSideways)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoAdjustSideways")
	endEvent
endState
state AdjustSchlong
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.AdjustSchlong = newKeyCode
			SetKeyMapOptionValueST(Config.AdjustSchlong)
		endIf
	endEvent
	event OnDefaultST()
		Config.AdjustSchlong = 46
		SetKeyMapOptionValueST(Config.AdjustSchlong)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoAdjustSchlong")
	endEvent
endState
state RotateScene
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.RotateScene = newKeyCode
			SetKeyMapOptionValueST(Config.RotateScene)
		endIf
	endEvent
	event OnDefaultST()
		Config.RotateScene = 22
		SetKeyMapOptionValueST(Config.RotateScene)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoRotateScene")
	endEvent
endState
state RestoreOffsets
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.RestoreOffsets = newKeyCode
			SetKeyMapOptionValueST(Config.RestoreOffsets)
		endIf
	endEvent
	event OnDefaultST()
		Config.RestoreOffsets = 12
		SetKeyMapOptionValueST(Config.RestoreOffsets)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoRestoreOffsets")
	endEvent
endState

state RealignActors
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.RealignActors = newKeyCode
			SetKeyMapOptionValueST(Config.RealignActors)
		endIf
	endEvent
	event OnDefaultST()
		Config.RealignActors = 26
		SetKeyMapOptionValueST(Config.RealignActors)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoRealignActors")
	endEvent
endState
state AdvanceAnimation
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.AdvanceAnimation = newKeyCode
			SetKeyMapOptionValueST(Config.AdvanceAnimation)
		endIf
	endEvent
	event OnDefaultST()
		Config.AdvanceAnimation = 57
		SetKeyMapOptionValueST(Config.AdvanceAnimation)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoAdvanceAnimation")
	endEvent
endState
state ChangeAnimation
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.ChangeAnimation = newKeyCode
			SetKeyMapOptionValueST(Config.ChangeAnimation)
		endIf
	endEvent
	event OnDefaultST()
		Config.ChangeAnimation = 24
		SetKeyMapOptionValueST(Config.ChangeAnimation)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoChangeAnimation")
	endEvent
endState
state ChangePositions
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.ChangePositions = newKeyCode
			SetKeyMapOptionValueST(Config.ChangePositions)
		endIf
	endEvent
	event OnDefaultST()
		Config.ChangePositions = 13
		SetKeyMapOptionValueST(Config.ChangePositions)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoChangePositions")
	endEvent
endState
state MoveSceneLocation
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.MoveScene = newKeyCode
			SetKeyMapOptionValueST(Config.MoveScene)
		endIf
	endEvent
	event OnDefaultST()
		Config.MoveScene = 27
		SetKeyMapOptionValueST(Config.MoveScene)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoMoveScene")
	endEvent
endState
state BackwardsModifier
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.Backwards = newKeyCode
			SetKeyMapOptionValueST(Config.Backwards)
		endIf
	endEvent
	event OnDefaultST()
		Config.Backwards = 54
		SetKeyMapOptionValueST(Config.Backwards)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoBackwards")
	endEvent
endState
state EndAnimation
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.EndAnimation = newKeyCode
			SetKeyMapOptionValueST(Config.EndAnimation)
		endIf
	endEvent
	event OnDefaultST()
		Config.EndAnimation = 207
		SetKeyMapOptionValueST(Config.EndAnimation)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoEndAnimation")
	endEvent
endState
state TargetActor
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.UnregisterForKey(Config.TargetActor)
			Config.TargetActor = newKeyCode
			Config.RegisterForKey(Config.TargetActor)
			SetKeyMapOptionValueST(Config.TargetActor)
		endIf
	endEvent
	event OnDefaultST()
		Config.UnregisterForKey(Config.TargetActor)
		Config.TargetActor = 49
		Config.RegisterForKey(Config.TargetActor)
		SetKeyMapOptionValueST(Config.TargetActor)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoTargetActor")
	endEvent
endState
state ToggleFreeCamera
	event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
		if newKeyCode == 1 || !KeyConflict(newKeyCode, conflictControl, conflictName)
			if newKeyCode == 1
				newKeyCode = -1
			endIf
			Config.UnregisterForKey(Config.ToggleFreeCamera)
			Config.ToggleFreeCamera = newKeyCode
			Config.RegisterForKey(Config.ToggleFreeCamera)
			SetKeyMapOptionValueST(Config.ToggleFreeCamera)
		endIf
	endEvent
	event OnDefaultST()
		Config.UnregisterForKey(Config.ToggleFreeCamera)
		Config.ToggleFreeCamera = 81
		Config.RegisterForKey(Config.ToggleFreeCamera)
		SetKeyMapOptionValueST(Config.ToggleFreeCamera)
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoToggleFreeCamera")
	endEvent
endState
state AdjustTargetStage
	event OnSelectST()
		Config.AdjustTargetStage = !Config.AdjustTargetStage
		ForcePageReset()
	endEvent
	event OnDefaultST()
		Config.AdjustTargetStage = false
		SetTextOptionValueST("$SSL_AllStages")
	endEvent
	event OnHighlightST()
		SetInfoText("$SSL_InfoAdjustTargetStage")
	endEvent
endState

State ForceRegisterVoices
  Event OnSelectST()
    ModEvent.Send(ModEvent.Create("SexLabSlotVoices"))
		ModEvent.Send(ModEvent.Create("SexLabSlotExpressions"))
		SetOptionFlagsST(OPTION_FLAG_DISABLED)
  EndEvent
	Event OnHighlightST()
		SetInfoText("$SSL_ForceRegisterVoicesHighlight")
	EndEvent
EndState

; ------------------------------------------------------- ;
; --- Misc Utilities                                  --- ;
; ------------------------------------------------------- ;

int function DoDisable(bool check)
	if check
		return OPTION_FLAG_DISABLED
	endIf
	return OPTION_FLAG_NONE
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

; SexLabFramework property SexLab auto
sslActorStats Property Stats Hidden
  sslActorStats Function Get()
	  return Game.GetFormFromFile(0xD62, "SexLab.esm") as sslActorStats
  EndFunction
EndProperty
sslExpressionSlots Property ExpressionSlots Hidden
	sslExpressionSlots Function Get()
		return Game.GetFormFromFile(0x664FB, "SexLab.esm") as sslExpressionSlots
	EndFunction
EndProperty
sslVoiceSlots property VoiceSlots Hidden
  sslVoiceSlots Function Get()
	  return Game.GetFormFromFile(0x664FB, "SexLab.esm") as sslVoiceSlots
  EndFunction
EndProperty
sslAnimationSlots Property AnimSlots Hidden
	sslAnimationSlots Function Get()
		return Game.GetFormFromFile(0x639DF, "SexLab.esm") as sslAnimationSlots
	EndFunction
EndProperty
sslCreatureAnimationSlots Property CreatureSlots Hidden
	sslCreatureAnimationSlots Function Get()
		return Game.GetFormFromFile(0x664FB, "SexLab.esm") as sslCreatureAnimationSlots
	EndFunction
EndProperty
sslThreadLibrary Property ThreadLib Hidden
	sslThreadLibrary Function Get()
		return Game.GetFormFromFile(0xD62, "SexLab.esm") as sslThreadLibrary
	EndFunction
EndProperty

Function Log(string Log, string Type = "NOTICE")
  If(Type == "FATAL")
    sslLog.Error(Log)
  Else
    sslLog.Log(Log)
  EndIf
EndFunction

string[] function MapOptions()
	return PapyrusUtil.StringSplit(GetState(), "_")
endFunction

function Troubleshoot()
endFunction

int function AddItemToggles(Form[] Items, int ID, int Max)
endFunction

function ToggleExpressions()
endFunction

string[] function PaginationMenu(string BeforePages = "", string AfterPages = "", int CurrentPage)
	string[] Output
	if BeforePages != ""
		Output = PapyrusUtil.PushString(Output, BeforePages)
	endIf
	; if CurrentPage < LastPage
	; 	Output = PapyrusUtil.PushString(Output, "$SSL_NextPage")
	; endIf
	if CurrentPage > 1
		Output = PapyrusUtil.PushString(Output, "$SSL_PrevPage")
	endIf
	if AfterPages != ""
		Output = PapyrusUtil.PushString(Output, AfterPages)
	endIf
	return Output
endfunction

string function GenderLabel(string id)
	if id == "0" || id == "M"
		return "$SSL_Male"
	elseIf id == "1" || id == "F"
		return "$SSL_Female"
	elseIf id >= "2" || id == "C"
		return "$SSL_Creature"
	endIf
	return "$SSL_Unknown"
endFunction

Form[] function GetItems(Actor ActorRef, bool FullInventory = false)
	if FullInventory
		return GetFullInventory(ActorRef)
	else
		return GetEquippedItems(ActorRef)
	endIf
endFunction
Form[] function GetEquippedItems(Actor ActorRef)
	Form[] Output = new Form[34]
	; Weapons
	Form ItemRef
	ItemRef = ActorRef.GetEquippedWeapon(false) ; Right Hand
	if ItemRef && IsToggleable(ItemRef)
		Output[33] = ItemRef
	endIf
	ItemRef = ActorRef.GetEquippedWeapon(true) ; Left Hand
	if ItemRef && ItemRef != Output[33] && IsToggleable(ItemRef)
		Output[32] = ItemRef
	endIf

	; Armor
	int i
	int Slot = 0x01
	while i < 32
		Form WornRef = ActorRef.GetWornForm(Slot)
		if WornRef
			if WornRef as ObjectReference
				WornRef = (WornRef as ObjectReference).GetBaseObject()
			endIf
			if Output.Find(WornRef) == -1 && IsToggleable(WornRef)
				Output[i] = WornRef
			endIf
		endIf
		Slot *= 2
		i    += 1
	endWhile
	return PapyrusUtil.ClearNone(Output)
endFunction
Form[] function GetFullInventory(Actor ActorRef)
	int[] Valid = new int[3]
	Valid[0] = 26 ; kArmor
	Valid[1] = 41 ; kWeapon 
	Valid[2] = 53 ; kLeveledItem
	;/ Valid[3] = 124 ; kOutfit
	Valid[4] = 102 ; kARMA
	Valid[5] = 120 ; kEquipSlot /;

	Form[] Output = GetEquippedItems(ActorRef)
	Form[] Items  = ActorRef.GetContainerForms()
	int n = Output.Length
	int i = Items.Length
	Output = Utility.ResizeFormArray(Output, 126)
	while i && n < 126
		i -= 1
		Form ItemRef = Items[i]
		if ItemRef && Valid.Find(ItemRef.GetType()) != -1
			if ItemRef as ObjectReference
				ItemRef = (ItemRef as ObjectReference).GetBaseObject()
			endIf
			if Output.Find(ItemRef) == -1 && IsToggleable(ItemRef)
				Output[n] = ItemRef
				n += 1
			endIf
		endIf
	endWhile
	return PapyrusUtil.ClearNone(Output)
endFunction

bool function IsToggleable(Form ItemRef)
	return !SexLabUtil.HasKeywordSub(ItemRef, "NoStrip") && !SexLabUtil.HasKeywordSub(ItemRef, "AlwaysStrip")
endFunction

bool[] function GetStripping(int type)
	if _stripViewIdx == 1
		if type == 1
			return Config.StripLeadInFemale
		else
			return Config.StripLeadInMale
		endIf
	elseIf _stripViewIdx == 2
		if type == 1
			return Config.StripVictim
		else
			return Config.StripAggressor
		endIf
	else
		if type == 1
			return Config.StripFemale
		else
			return Config.StripMale
		endIf
	endIf
endFunction

float Function GetDefaultTime(int idx)
	float[] f = new float[15]
	; Default
	f[0] = 15.0		
	f[1] = 20.0
	f[2] = 15.0
	f[3] = 15.0
	f[4] = 9.0
	; lead In
	f[5] = 10.0		
	f[6] = 10.0
	f[7] = 10.0
	f[8] = 8.0
	f[9] = 8.0
	; Aggressive
	f[10] = 20.0	
	f[11] = 15.0
	f[12] = 10.0
	f[13] = 10.0
	f[14] = 4.0
	return f[idx]
EndFunction


float[] function GetTimers()
	if _stripViewIdx == 1
		return Config.StageTimerLeadIn
	elseIf _stripViewIdx == 2
		return Config.StageTimerAggr
	else
		return Config.StageTimer
	endIf
endFunction

; Default Timer Values
float[] function GetTimersDef()
	float[] ret = new float[5]
	if _stripViewIdx == 1
		ret[0] = 10.0
		ret[1] = 10.0
		ret[2] = 10.0
		ret[3] = 8.0
		ret[4] = 8.0
	elseIf _stripViewIdx == 2
		ret[0] = 20.0
		ret[1] = 15.0
		ret[2] = 10.0
		ret[3] = 10.0
		ret[4] = 4.0
	else
		ret[0] = 30.0
		ret[1] = 20.0
		ret[2] = 15.0
		ret[3] = 15.0
		ret[4] = 9.0
	endIf
	return ret
endFunction

function AddAnimationsTag(string Tag)
endFunction

function RemoveAnimationsTag(string Tag)
endFunction

function ToggleAnimationsTag(string Tag)
endFunction

bool function GetToggle(sslBaseAnimation Anim)
	return Anim.Enabled
endFunction

function ResetAllQuests()
endFunction
function ResetQuest(Quest QuestRef)
endFunction
