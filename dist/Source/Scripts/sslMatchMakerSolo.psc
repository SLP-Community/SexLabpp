Scriptname sslMatchMakerSolo extends ActiveMagicEffect
{A very basic spell effect that starts a SexLab scene in a modern way, but only for one.}

SexLabFramework property SexLab auto
sslMatchMakerMain Property MatchMakerMain Auto

Event OnEffectStart(Actor TargetRef, Actor CasterRef)

    Actor[] sceneActors = new Actor[1]
    sceneActors[0] = CasterRef

	MatchMakerMain.TriggerSex(sceneActors)
EndEvent