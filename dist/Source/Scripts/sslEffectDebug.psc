Scriptname sslEffectDebug extends ActiveMagicEffect
{
	Old debug-only Matchmaker Script, no longer used
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

SexLabFramework property SexLab auto
sslSystemConfig property Config auto
Actor property PlayerRef auto

sslBenchmark function Benchmark(int Tests = 1, int Iterations = 5000, int Loops = 10, bool UseBaseLoop = false)
	sslBenchmark ret = (Quest.GetQuest("SexLabDev") as sslBenchmark)
	ret.StartBenchmark(Tests, Iterations, Loops, UseBaseLoop)
	return ret
endFunction

event OnEffectStart(Actor TargetRef, Actor CasterRef)
	SexLabUtil.PrintConsole("---- DEBUG EFFECT START ----")
	SexLab.QuickStart(CasterRef, TargetRef)

	Dispel()
endEvent

event OnEffectFinish(Actor TargetRef, Actor CasterRef)
	SexLabUtil.PrintConsole("---- DEBUG EFFECT FINISHED ----")
endEvent

;/-----------------------------------------------\;
;|	Debug Utility Functions                      |;
;\-----------------------------------------------/;

function Log(string log)
	sslLog.Log(log)
endfunction
