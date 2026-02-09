ScriptName sslLovense Hidden

bool Function IsLovenseInstalled() global
  return SKSE.GetPluginVersion("SkyrimLovense") > -1 && Lovense.GetConnectedCount() > 0
EndFunction

; ------------------------------------------------------- ;
; --- Start Actions                                   --- ;
; ------------------------------------------------------- ;

Function StartGenitalAction(int aiStrength) global
  String[] toys = Lovense.GetToysByCategory("Genital")
  StartDefaultActions(toys, aiStrength, abStopPrevious = false)
EndFunction

Function StartAnalAction(int aiStrength) global
  String[] toys = Lovense.GetToysByCategory("Anal")
  StartDefaultActions(toys, aiStrength, abStopPrevious = false)
EndFunction

Function StartOrgasmAction(int aiStrength, float duration) global
  String[] toys = new String[1]
  StartDefaultActions(toys, aiStrength, duration, true)
EndFunction

Function StartDefaultActions(String[] toys, int strength, float duration = 0.0, bool abStopPrevious) global
  If (strength <= 0)
    return
  EndIf
  int[] argStrength = new int[1]
  argStrength[0] = strength
  String[] argType = new String[1]
  argType[0] = "All"
  int i = 0
  While (i < toys.Length)
    Lovense.FunctionRequest(argType, argStrength, duration, asToy = toys[i], abStopPrevious = abStopPrevious)
    i += 1
  EndWhile
EndFunction

; ------------------------------------------------------- ;
; --- Stop Actions                                    --- ;
; ------------------------------------------------------- ;

String[] Function RemoveMismatchedCategory(String[] in, String filter) global
  String[] out = Utility.CreateStringArray(in.Length)
  int i = 0
  While (i < in.Length)
    String c = Lovense.GetToyCategory(in[i])
    If (c == filter)
      out[i] = in[i]
    EndIf
    i += 1
  EndWhile
  return PapyrusUtil.RemoveString(out, "")
EndFunction

Function StopGenitalAction(bool abFilterGeneric) global
  String[] toys = Lovense.GetToysByCategory("Genital")
  If (abFilterGeneric)
    toys = RemoveMismatchedCategory(toys, "Genital")
  EndIf
  StopAction(toys)
EndFunction

Function StopAnalAction(bool abFilterGeneric) global
  String[] toys = Lovense.GetToysByCategory("Anal")
  If (abFilterGeneric)
    toys = RemoveMismatchedCategory(toys, "Anal")
  EndIf
  StopAction(toys)
EndFunction

Function StopAction(String[] toys) global
  int i = 0
  While (i < toys.Length)
    Lovense.StopRequest(toys[i])
    i += 1
  EndWhile
EndFunction

Function StopAllActions() global
  Lovense.StopRequest()
EndFunction
