ScriptName SexLabThreadHook extends ReferenceAlias Hidden
{
  Interface Script to manipulate a thread execution during its runtime

  **WHEN TO USE**
  Thread Hooks are blocking, explicit hooks to read and write SexLab threads at runtime, altering their behavior as needed
  Unlike regular. non blocking, Hooks which use mod events the interface here will block the execution of a SL thread, allowing for time sensitive information
  to be injected or altered. The blocking nature of these hooks also makes them potentially harmful to the users experience as they will pause the threads execution
  essentially leaving the user clueless about the current state of their scene, so use them with caution and beware of the amount of time you spend processing information here

  **HOW TO USE**
  1) Create a reference alias object that should receive the incoming events
  2) Create YOUR OWN custom script and have it extend "SexLabThreadHook", attach it to the Formy you created
  3) Copy the "@Interface" functions you see in this script and paste them into the script you just created and implement them
  4) Call the Register() function (in this script) to register your Hook to SexLab to receive incoming hook events, e.g. in an OnInit Event or similar
  5) Youre done \o/. Now every time a SexLab Thread reaches specific key points in its execution, it will call back into the interface functions listed here

  **BEST PRACTICES**
  1) Due to the time sensitive nature of this feature, it is highly recommended to use the locking mechanism written below if you only want to block some specific
      threads (such as only ones the player is involed in)
  2) Optimize for speed. There are certainly moments where execution time is not of utmost importance, but be aware of fellow co-modders and the time the framework itself 
      takes to process. Ideally the player is not aware of your hooks, does not realize additional delays. Hence, the faster your code runs, the better
}

; If you overwrite OnInit make sure to call Parent.OnInit() again
Event OnInit()
  _m = Utility.CreateBoolArray(sslThreadSlots.GetTotalThreadCount(), false)
  If (bAutoegisterOnInit)
    Register()
  EndIf
EndEvent

; ------------------------------------------------------- ;
; --- Interface                                       --- ;
; ------------------------------------------------------- ;
;/
  The following functions are to be implemented/overwritten by your own script
/;

; Called when all of the threads data is set, before the active animation is chosen
Function OnAnimationStarting(SexLabThread akThread)
EndFunction

; Called whenever a new stage is picked, including the very first one
Function OnStageStart(SexLabThread akThread)
EndFunction

; Called whenever a stage ends, including the very last one
Function OnStageEnd(SexLabThread akThread)
EndFunction

; Called once the animation has ended
Function OnAnimationEnd(SexLabThread akThread)
EndFunction

; ------------------------------------------------------- ;
; --- Registration                                    --- ;
; ------------------------------------------------------- ;
;/
  Shorthands to register and unregister the hook
/;

bool Property bAutoegisterOnInit = true Auto
{If this Hook should register itself when it is first initialized. Default: true}

; Register the given Hook to no receive events
; Registration is only recognized for newly started threads. You wont receive events for already running threads
Function Register()
  SexLabUtil.GetAPI().RegisterHook(self)
EndFunction

; Unregister the given Hook to no longer receive events
; Removal of the registration status is recognized for any following animations. You will still receive events for currently running threads
Function Unregister()
  SexLabUtil.GetAPI().UnregisterHook(self)
EndFunction

; ------------------------------------------------------- ;
; --- Lock                                            --- ;
; ------------------------------------------------------- ;
;/
  Simple locking system to skip processing threads you do not care about. If using this, ensure OnInit() has been called
  If you overwrite OnInit() in your implementation, call "Parent.OnInit()" before exiting your OnInit event

  **USAGE**
  Function OnAnimationStarting(SexLabThread akThread)
    bool ignore_thread_events = ...
    SetLocked(ignore_thread_events)

    ...
  EndFunction

  Function OnStageStart(SexLabThread akThread)
    If (Islocked(akThread))
      return
    EndIf

    ...
  EndFunction
/;
bool[] _m

; Return if the calling thread is locked or not
bool Function IsLocked(SexLabThread akThread)
  return _m[akThread.GetThreadID()]
EndFunction

; Set the locked state of this specific thread
Function SetLocked(SexLabThread akThread, bool abLocked)
  _m[akThread.GetThreadID()] = abLocked
EndFunction
