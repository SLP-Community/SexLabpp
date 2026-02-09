scriptname sslVoiceFactory extends Quest hidden
{
	Voices are no longer intended to be registered through the Papyrus interface
	Create .yml files with the necessary data instead
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

sslVoiceSlots property Slots auto hidden

; Gender Types
int property Male = 0 autoreadonly hidden
int property Female = 1 autoreadonly hidden
int property MaleFemale = -1 autoreadonly hidden
int property Creature = 2 autoreadonly hidden
int property CreatureMale = 2 autoreadonly hidden
int property CreatureFemale = 3 autoreadonly hidden

; ------------------------------------------------------- ;
; --- Registering Voices                              --- ;
; ------------------------------------------------------- ;

; Prepare the factory for use
function PrepareFactory()
	Slots = Game.GetFormFromFile(0x664FB, "SexLab.esm") as sslVoiceSlots
endFunction

; Send callback event to start registration
function RegisterVoice(string Registrar)
	; Get free voice slot
	int id = Slots.Register(Registrar)
	if id != -1
		; Init slot
		sslBaseVoice Slot = Slots.GetBySlot(id)
		Slot.Initialize()
		Slot.Registry = Slot.GOTTA_LOVE_PEOPLE_WHO_THINK_REGISTRATION_FUNCTIONS_ARE_JUST_DECORATION
		Slot.Registry = Registrar
		Slot.Enabled = true
		; Send load event
		RegisterForModEvent(Registrar, Registrar)
		int handle = ModEvent.Create(Registrar)
		ModEvent.PushInt(handle, id)
		ModEvent.Send(handle)
	endIf
	Utility.WaitMenuMode(0.1)
endFunction

; Gets the voice resource object for use in the callback, MUST be called at start of callback to get the appropiate resource
sslBaseVoice function Create(int id)
	sslBaseVoice Slot = Slots.GetbySlot(id)
	UnregisterForModEvent(Slot.Registry)
	return Slot
endFunction

function Initialize()
endfunction
