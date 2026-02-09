ScriptName sslLog Hidden

Function Log(String asMsg, bool abConsole = false) global
  String print = "[SexLab] " + asMsg
  Debug.Trace(print)
  LogImpl(print, abConsole)
EndFunction

Function Error(String asMsg, bool abConsole = false) global
  String print = "[SexLab] " + asMsg
  Debug.TraceStack(print)
  LogImpl(print, abConsole)
EndFunction

Function LogImpl(String asMsg, bool abConsole) global
  bool debugmode = sslSystemConfig.GetSettingBool("bDebugMode")
  If (debugmode)
    If (Debug.OpenUserLog("SexLabDebug"))
      Debug.TraceUser("SexLabDebug", "SexLab Debug/Development Mode Deactivated")
      SexLabUtil.PrintConsole("SexLab Debug/Development Mode Activated")
    EndIf
    Debug.TraceUser("SexLabDebug", asMsg)
  EndIf
  If (debugmode || abConsole)
    SexLabUtil.PrintConsole(asMsg)
  Endif
EndFunction
