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
String[] _ClimaxTypes
String[] _Sexes

Function AnimationSettings()
	SetCursorFillMode(TOP_TO_BOTTOM)
	AddHeaderOption("$SSL_PlayerSettings")
	AddStateOptionBool("bAutoAdvance", "$SSL_bAutoAdvance")
	AddStateOptionBool("bDisablePlayer", "$SSL_bDisablePlayer")
	AddStateOptionBool("bAutoTFC", "$SSL_bAutoTFC")
	AddStateOptionSlider("fAutoSUCSM", "$SSL_fAutoSUCSM", 5, 1, 20, 1, "{0}")
	AddMenuOptionST("SexSelect_0", "$SSL_PlayerSex", _Sexes[SexLabRegistry.GetSex(PlayerRef, false) % 3])
	If (Config.TargetRef)
		String name = Config.TargetRef.GetLeveledActorBase().GetName()
		AddMenuOptionST("SexSelect_1", "$SSL_{" + name + "}Sex", _Sexes[SexLabRegistry.GetSex(Config.TargetRef, false) % 3])
	Else
		AddTextOption("$SSL_NoTarget", "$SSL_Male", OPTION_FLAG_DISABLED)
	EndIf
	AddHeaderOption("$SSL_ExtraEffects")
	AddMenuOptionST("ClimaxType", "$SSL_ClimaxType", _ClimaxTypes[sslSystemConfig.GetSettingInt("iClimaxType")])
	AddStateOptionBool("bOrgasmEffects", "$SSL_bOrgasmEffects")
	AddStateOptionSlider("fShakeStrength", "$SSL_fShakeStrength", 0.7, 0, 1, 0.05, "{2}%")
	AddStateOptionBool("bUseCum", "$SSL_bUseCum")
	AddStateOptionSlider("fCumTimer", "$SSL_fCumTimer", 120, 0, 1800, 10, "$SSL_Seconds")
	AddStateOptionBool("bUseExpressions", "$SSL_bUseExpressions")
	AddStateOptionBool("bUseLipSync", "$SSL_bUseLipSync")
	AddHeaderOption("$SSL_Lovense")
	bool lovenseFlag = !sslLovense.IsLovenseInstalled()
	AddStateOptionSlider("iLovenseStrength", "$SSL_iLovenseStrength", 10, 0, 20, 1, "{0}", lovenseFlag)
	AddStateOptionSlider("iLovenseStrengthOrgasm", "$SSL_iLovenseStrengthOrgasm", 20, 0, 20, 1, "{0}", lovenseFlag)
	AddStateOptionSlider("fLovenseDurationOrgasm", "$SSL_iLovenseDurationOrgasm", 8, 5, 30, 0.5, "{1}", lovenseFlag)

	SetCursorPosition(1)
	AddHeaderOption("$SSL_Creatures")
	AddStateOptionBool("bAllowCreatures", "$SSL_bAllowCreatures")
	AddStateOptionBool("bCreatureGender", "$SSL_bCreatureGender")
	AddHeaderOption("$SSL_AnimationHandling")
	; COMEBACK: Should prbly delete this entirely as this should always be disabled for 1.5 and always enabled for 1.6
	AddStateOptionBool("bDisableScale", "$SSL_bDisableScale")
	AddMenuOptionST("UseFade","$SSL_UseFade", _FadeOpt[sslSystemConfig.GetSettingInt("iUseFade")])
	AddStateOptionBool("bUndressAnimation", "$SSL_bUndressAnimation")
	AddStateOptionBool("bRedressVictim", "$SSL_bRedressVictim")
	AddStateOptionBool("bDisableTeleport", "$SSL_bDisableTeleport")
	AddStateOptionBool("bShowInMap", "$SSL_bShowInMap")
	AddStateOptionBool("bSetAnimSpeedByEnjoyment", "$SSL_bSetAnimSpeedByEnjoyment", !sslSystemConfig.HasAnimSpeedSE())
	AddMenuOptionST("FurnitureNPC", "$SSL_FurnitureNPC", _NPCFurnOpt[sslSystemConfig.GetSettingInt("iNPCBed")])
	AddMenuOptionST("FurniturePlayer", "$SSL_FurniturePlayer", _PlFurnOpt[sslSystemConfig.GetSettingInt("iAskBed")])
EndFunction

; ------------------------------------------------------- ;
; --- Matchmaker	                                  --- ;
; ------------------------------------------------------- ;

Function MatchMaker()
	SetCursorFillMode(TOP_TO_BOTTOM)
	AddToggleOptionST("ToggleMatchMaker", "$SSL_ToggleMatchMaker", Config.MatchMaker)
	bool matchMakerDisabled = !sslSystemConfig.GetSettingBool("bMatchMakerActive")
	int flag = DoDisable(matchMakerDisabled)
	AddHeaderOption("$SSL_MatchMakerTagsSettings", flag)
	AddTextOptionST("matchmakerInputTags", "$SSL_InputTags", sslSystemConfig.ParseMMTagString(), flag)
	AddInputOptionST("matchmakerInputRequiredTags", "$SSL_InputRequiredTags", Config.RequiredTags, flag)
	AddInputOptionST("matchmakerInputExcludedTags", "$SSL_InputExcludedTags", Config.ExcludedTags, flag)
	AddInputOptionST("matchmakerInputOptionalTags", "$SSL_InputOptionalTags", Config.OptionalTags, flag)
	AddTextOptionST("TextResetTags", "$SSL_TextResetTags", "$SSL_ResetTagsHere", flag)
	SetCursorPosition(1)
	AddEmptyOption()
	AddHeaderOption("$SSL_MatchMakerActorSettings", flag)
	AddStateOptionBool("bSubmissivePlayer", "$SSL_bSubmissivePlayer", matchMakerDisabled)
	AddStateOptionBool("bSubmissiveTarget", "$SSL_bSubmissiveTarget", matchMakerDisabled)
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
		SetInfoText("$SSL_ToggleMatchMakerHighlight")
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
	AddStateOptionSlider("fVoiceVolume", "$SSL_fVoiceVolume", 1, 0, 1, 0.01, "{2}%")
	AddStateOptionSlider("fSFXVolume", "$SSL_fSFXVolume", 1, 0, 1, 0.01, "{2}%")
	AddStateOptionSlider("fMaleVoiceDelay", "$SSL_fMaleVoiceDelay", 5, 1, 45, 1, "$SSL_Seconds")
	AddStateOptionSlider("fFemaleVoiceDelay", "$SSL_fFemaleVoiceDelay", 4, 1, 45, 1, "$SSL_Seconds")
	AddStateOptionSlider("fSFXDelay", "$SSL_fSFXDelay", 3, 1, 30, 1, "$SSL_Seconds")
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
	AddMenuOptionST("ActiveVoices", "$SSL_ActiveVoices", "$SSL_Race_" + SexLabRegistry.MapRaceKeyToID(_voiceActiveRaceKey))
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
		SetInfoText("$SSL_SelectVoiceCacheHighlight")
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
		SetInfoText("$SSL_SelectVoiceCacheVHighlight")
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
	AddEmptyOption()
	AddEmptyOption()
	AddMenuOptionST("TSModeSelect", "$SSL_TSModeSelect", _stripView[_stripViewIdx])
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
	AddToggleOptionST("enableexpression", "$SSL_Enabled", sslBaseExpression.GetEnabled(_expression[_expressionIdx]))
	AddMenuOptionST("ExpressionScaling", "$SSL_ExpressionScaling", _expressionScales[scalemode], DoDisable(v < 1))
	AddToggleOptionST("expredittag_Normal", "$SSL_ExpressionsNormal", tags.Find("Normal") > -1)
	AddToggleOptionST("expredittag_Victim", "$SSL_ExpressionsVictim", tags.Find("Victim") > -1)
	AddToggleOptionST("expredittag_Aggressor", "$SSL_ExpressionsAggressor", tags.Find("Aggressor") > -1)

	SetCursorPosition(1)
	AddHeaderOption("$SSL_SyncLipsConfig")
	AddTextOptionST("LipsSoundTime", "$SSL_LipsSoundTime", _soundmethod[Config.LipsSoundTime])
	AddStateOptionBool("bLipsFixedValue", "$SSL_bLipsFixedValue")
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
	SetCursorFillMode(TOP_TO_BOTTOM)
	
	AddHeaderOption("Primary Settings")
	AddStateOptionBool("bInternalEnjoymentEnabled", "$SSL_bInternalEnjoymentEnabled")
	AddStateOptionSlider("iInterDetectionStrength", "$SSL_iInterDetectionStrength", 4, 1, 4, 1, "{0}")
	AddStateOptionSlider("fEnjRaiseMultInter", "$SSL_fEnjRaiseMultInter", 1.2, 0, 3, 0.1, "{1}")

	AddHeaderOption("Game Toggles")
	AddStateOptionBool("bGameEnabled", "$SSL_bGameEnabled")
	AddStateOptionBool("bGamePlayerAutoplay", "$SSL_bGamePlayerAutoplay")
	AddStateOptionBool("bGamePlayerVictimAutoplay", "$SSL_bGamePlayerVictimAutoplay")
	AddStateOptionBool("bGameNPCAutoplay", "$SSL_bGameNPCAutoplay")
	AddStateOptionBool("bGameEnjReductionChance", "$SSL_bGameEnjReductionChance")
	AddStateOptionBool("bGameHoldbackWithPartner", "$SSL_bGameHoldbackWithPartner")

	AddHeaderOption("Game Hotkeys")
	AddStateOptionKey("iGameUtilityKey", "$SSL_iGameUtilityKey")
	AddStateOptionKey("iGamePauseKey", "$SSL_iGamePauseKey")
	AddStateOptionKey("iGameRaiseEnjKey", "$SSL_iGameRaiseEnjKey")
	AddStateOptionKey("iGameHoldbackKey", "$SSL_iGameHoldbackKey")
	AddStateOptionKey("iGameSelectNextPos", "$SSL_iGameSelectNextPos")

	AddHeaderOption("Game Modifiers")
	AddStateOptionSlider("iEnjGameStaminaCost", "$SSL_iEnjGameStaminaCost", 10, 0, 50, 1, "{0}")
	AddStateOptionSlider("iEnjGameMagickaCost", "$SSL_iEnjGameMagickaCost", 10, 0, 50, 1, "{0}")
	SetCursorPosition(1)
	AddEmptyOption()
	AddStateOptionSlider("iEdgingRewardType", "$SSL_iEdgingRewardType", 0, 0, 3, 1, "{0}")
	AddStateOptionSlider("iEdgeSpamPunishType", "$SSL_iEdgeSpamPunishType", 0, 0, 3, 1, "{0}")

	AddHeaderOption("General Configs")
	AddStateOptionSlider("iEnjGainOnStageSkip", "$SSL_iEnjGainOnStageSkip", 15, 0, 30, 1, "{0}")
	AddStateOptionBool("bNoStaminaEndsScene", "$SSL_bNoStaminaEndsScene")
	AddStateOptionBool("bMaleOrgasmEndsScene", "$SSL_bMaleOrgasmEndsScene")
	AddStateOptionBool("bDomMustOrgasm", "$SSL_bDomMustOrgasm")
	AddStateOptionBool("bPlayerMustOrgasm", "$SSL_bPlayerMustOrgasm")
	AddStateOptionBool("bHighEnjOrgasmWait", "$SSL_bHighEnjOrgasmWait")
	AddStateOptionSlider("iMaxNoPainOrgasmMale", "$SSL_iMaxNoPainOrgasmMale", 1, 1, 4, 1, "{0}")
	AddStateOptionSlider("iMaxNoPainOrgasmFemale", "$SSL_iMaxNoPainOrgasmFemale", 2, 1, 5, 1, "{0}")
	AddStateOptionSlider("iNoPainRequiredTime", "$SSL_iNoPainRequiredTime", 50, 0, 180, 10, "{0}")
	AddStateOptionSlider("iNoPainRequiredXP", "$SSL_iNoPainRequiredXP", 50, 0, 100, 5, "{0}")

	AddHeaderOption("Misc Multipliers")
	AddStateOptionSlider("fEnjMultVictim", "$SSL_fEnjMultVictim", 0.8, 0, 2, 0.1, "{1}")
	AddStateOptionSlider("fEnjMultAggressor", "$SSL_fEnjMultAggressor", 1.2, 0, 2, 0.1, "{1}")
	AddStateOptionSlider("fEnjMultSub", "$SSL_fEnjMultSub", 0.8, 0, 2, 0.1, "{1}")
	AddStateOptionSlider("fEnjMultDom", "$SSL_fEnjMultDom", 1.2, 0, 2, 0.1, "{1}")
	AddStateOptionSlider("fPainHugePPMult", "$SSL_fPainHugePPMult", 0.5, 0, 2, 0.1, "{1}")
EndFunction

; ------------------------------------------------------- ;
; --- Debug & installation							              --- ;
; ------------------------------------------------------- ;

Function RebuildClean()
	SetCursorFillMode(TOP_TO_BOTTOM)
	AddHeaderOption("SexLab v" + GetStringVer() + " by Ashal@LoversLab.com")
	AddStateOptionBool("bDebugMode", "$SSL_bDebugMode")
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

Function AddStateOptionBool(String asOption, String asOptionText, bool abDisable = false)
	AddToggleOptionST(asOption, asOptionText, sslSystemConfig.GetSettingBool(asOption), DoDisable(abDisable))
EndFunction

Function AddStateOptionSlider(String asOption, String asOptionText, float afDefault, float afLow, float afHigh, float afInterval, String asFormat = "{0}", bool abDisable = false)
    float val
    If (StringUtil.GetNthChar(asOption, 0) == "i")
        val = sslSystemConfig.GetSettingInt(asOption) as float
    Else
        val = sslSystemConfig.GetSettingFlt(asOption)
    EndIf
    asOption = asOption + "_" + afDefault + "_" + afLow + "_" + afHigh + "_" + afInterval + "_" + asFormat
    AddSliderOptionST(asOption, asOptionText, val, asFormat, DoDisable(abDisable))
EndFunction

Function AddStateOptionKey(String asOption, String asOptionText, bool abMandatory = false, bool abSkipConflictResolution = false, bool needsRegister = false, bool abDisable = false)
    String asState = asOption
    If (abSkipConflictResolution)
        asState = "S_" + asState
    EndIf
    If (abMandatory)
        asState = "M_" + asState
    EndIf
    If (needsRegister)
        asState = "R_" + asState
    EndIf
	AddKeyMapOptionST(asState, asOptionText, sslSystemConfig.GetSettingInt(asOption))
EndFunction

Event OnSelectST()
	string[] s = PapyrusUtil.StringSplit(GetState(), "_")
	If (s[0] == "Voice")
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
		If(j == -1)		; Never			-> Always
			sslActorLibrary.WriteStrip(item, false)
		ElseIf(j == 1)	; Always		-> Unspecified
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
	ElseIf (s[0] == "expressionprev")
		_phaseIdx -= 2
		ForcePageReset()
	ElseIf (s[0] == "expressionnext")
		_phaseIdx += 2
		ForcePageReset()
	ElseIf (s[0] == "StopCurrentAnimations")
		ShowMessage("$SSL_StopRunningAnimations", false)
		ThreadSlots.StopAll()
	ElseIf (StringUtil.GetNthChar(s[0], 0) == "b")
		bool newValue = !sslSystemConfig.GetSettingBool(s[0])
		sslSystemConfig.SetSettingBool(s[0], newValue)
		SetToggleOptionValueST(newValue)
	Else
		Log("Unrecognized toggle-setting: " + s[0])
	EndIf
EndEvent

Event OnSliderOpenST()
	string[] s = PapyrusUtil.StringSplit(GetState(), "_")
    If (s[0] == "StageTimers")
		int i = s[1] as int
		SetSliderDialogStartValue(sslSystemConfig.GetSettingFltA("fTimers", i))
		SetSliderDialogDefaultValue(15)
		SetSliderDialogRange(3, 180)
		SetSliderDialogInterval(1)
	ElseIf (s[0] == "expredit")
		int i = s[1] as int
		float[] values
		If ((s[2] as int) == 0)
			values = _low
		Else
			values = _high
		EndIf
		SetSliderDialogStartValue(values[i])
		SetSliderDialogDefaultValue(0)
		SetSliderDialogRange(0, 1)
		SetSliderDialogInterval(0.05)
    ElseIf (StringUtil.GetNthChar(s[0], 0) == "i")
        SetSliderDialogStartValue(sslSystemConfig.GetSettingInt(s[0]))
        SetSliderDialogDefaultValue(s[1] as int)
        SetSliderDialogRange(s[2] as int, s[3] as int)
        SetSliderDialogInterval(s[4] as int)
    ElseIf (StringUtil.GetNthChar(s[0], 0) == "f")
        SetSliderDialogStartValue(sslSystemConfig.GetSettingFlt(s[0]))
        SetSliderDialogDefaultValue(s[1] as float)
        SetSliderDialogRange(s[2] as float, s[3] as float)
        SetSliderDialogInterval(s[4] as float)
    Else
        Log("Unknown slider setting: " + s[0])
	EndIf
EndEvent

Event OnSliderAcceptST(float value)
	string[] s = PapyrusUtil.StringSplit(GetState(), "_")
	If (s[0] == "StageTimers")
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
    ElseIf (StringUtil.GetNthChar(s[0], 0) == "i")
        sslSystemConfig.SetSettingInt(s[0], value as int)
		SetSliderOptionValueST(value, s[5])
    ElseIf (StringUtil.GetNthChar(s[0], 0) == "f")
        sslSystemConfig.SetSettingFlt(s[0], value)
		SetSliderOptionValueST(value, s[5])
    Else
        Log("Unknown slider setting: " + s[0])
	EndIf
EndEvent

Event OnMenuOpenST()
	String[] s = PapyrusUtil.StringSplit(GetState(), "_")
	If (s[0] == "ClimaxType")
		SetMenuDialogStartIndex(sslSystemConfig.GetSettingInt("iClimaxType"))
		SetMenuDialogDefaultIndex(2)
		SetMenuDialogOptions(_ClimaxTypes)
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
	ElseIf (s[0] == "ExpressionScaling")
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
	ElseIf (s[0] == "ActiveVoices")
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
	ElseIf (s[0] == "ExpressionScaling")
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
	ElseIf (s[0] == "ActiveVoices")
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
	EndIf
EndEvent

Event OnKeyMapChangeST(int newKeyCode, string conflictControl, string conflictName)
  String[] s = StringUtil.Split(GetState(), "_")
	int i = 0
	bool mandatory = false
	bool skipConflict = false
    bool needsRegister = false
	If (s[i] == "M")
		i += 1
		mandatory = true
	EndIf
	If (s[i] == "S")
		i += 1
		skipConflict = true
	EndIf
    If (s[i] == "R")
        i += 1
        needsRegister = true
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
  If (needsRegister)
    int oldKeyCode = sslSystemConfig.GetSettingInt(s[i])
    If (oldKeyCode != -1)
      Config.UnregisterForKey(oldKeyCode)
    EndIf
    If (newKeyCode != -1)
      Config.RegisterForKey(newKeyCode)
    EndIf
  EndIf
	sslSystemConfig.SetSettingInt(s[i], newKeyCode)
  SetKeyMapOptionValueST(newKeyCode)
EndEvent

Event OnHighlightST()
	string[] s = PapyrusUtil.StringSplit(GetState(), "_")
	If (s[0] == "Voice")
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
	ElseIf(s[0] == "StrippingW")
		String info = PlayerRef.GetLeveledActorBase().GetName() + " Weapons: "
		info += GetItemName(PlayerRef.GetEquippedWeapon(true), "?") + "(LH), " + GetItemName(PlayerRef.GetEquippedWeapon(false), "?") + "(RH)"
		If (Config.TargetRef)
			info += "\n" + Config.TargetRef.GetLeveledActorBase().GetName() + " Weapons: "
			info += GetItemName(Config.TargetRef.GetEquippedWeapon(true), "?") + "(LH), " + GetItemName(Config.TargetRef.GetEquippedWeapon(false), "?") + "(RH)"
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
	Else
		String tlKey = "$SSL_" + s[0] + "Highlight"
		If SexLabUtil.GetTranslation(tlKey) != ""
			SetInfoText(tlKey)
		EndIf
	EndIf
EndEvent

; ------------------------------------------------------- ;
; --- Player Hotkeys                                  --- ;
; ------------------------------------------------------- ;

function PlayerHotkeys()
	SetCursorFillMode(LEFT_TO_RIGHT)

	AddHeaderOption("$SSL_GlobalHotkeys")
	AddEmptyOption()
	AddStateOptionKey("iTargetActor", "$SSL_iTargetActor", needsRegister = true)
	AddStateOptionKey("iToggleFreeCamera", "$SSL_iToggleFreeCamera", needsRegister = true)
	;AddStateOptionBool("bHideHUD", "$SSL_bHideHUD")

	AddHeaderOption("$SSL_SceneManipulation")
	AddEmptyOption()
	AddStateOptionKey("iKeyUp", "$SSL_iKeyUp", true, true)
	AddStateOptionKey("iKeyExtra2", "$SSL_iKeyExtra2", true, true)
	AddStateOptionKey("iKeyDown", "$SSL_iKeyDown", true, true)
	AddStateOptionKey("iKeyMod", "$SSL_iKeyMod", true, true)
	AddStateOptionKey("iKeyLeft", "$SSL_iKeyLeft", true, true)
	AddStateOptionKey("iKeyReset", "$SSL_iKeyReset", true, true)
	AddStateOptionKey("iKeyRight", "$SSL_iKeyRight", true, true)
	AddStateOptionKey("iKeyEnd", "$SSL_iKeyEnd", true, true)
	AddStateOptionKey("iKeyAdvance", "$SSL_iKeyAdvance", true, true)
	;AddStateOptionKey("iChangeAnimation", "$SSL_iChangeAnimation", true, true)
endFunction

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
