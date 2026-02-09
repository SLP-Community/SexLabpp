Scriptname sslMatchMakerTarget extends ActiveMagicEffect
{A very basic spell effect that starts a SexLab scene in a modern way.}

SexLabFramework property SexLab auto
sslMatchMakerMain Property MatchMakerMain Auto

Event OnEffectStart(Actor TargetRef, Actor CasterRef)

    Actor[] sceneActors = new Actor[2]
    sceneActors[0] = CasterRef
    sceneActors[1] = TargetRef
    
    MatchMakerMain.TriggerSex(sceneActors)
EndEvent