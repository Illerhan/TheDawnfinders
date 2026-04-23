#include "Actors/MovableObjects/IncrementalDoor.h"

AIncrementalDoor::AIncrementalDoor()
{
    PrimaryActorTick.bCanEverTick = true;
    NeededTriggerCount = 3;
}

void AIncrementalDoor::BeginPlay()
{
    Super::BeginPlay();
    ActivationCount = 0;
    TargetCurvePos  = 0.f;
    bMovingToStep   = false;
}

void AIncrementalDoor::Tick(float DeltaTime)
{
    // On bypass AMovableObjects::Tick qui ferait TickTimeline librement
    // On gère nous-mêmes le tick de la timeline
    AActor::Tick(DeltaTime);

    if (!Timeline.IsPlaying()) return;
    if (!MoveCurve) return;

    if (bMovingToStep)
    {
        float CurrentPos   = Timeline.GetPlaybackPosition();
        float PlayRate     = Timeline.GetPlayRate();
        float Remaining    = TargetCurvePos - CurrentPos;
        float TimeToTarget = (PlayRate > 0.f) ? (Remaining / PlayRate) : 0.f;

        if (DeltaTime >= TimeToTarget)
        {
            // On avance exactement jusqu'au palier, pas un frame de plus
            Timeline.TickTimeline(TimeToTarget);
            Timeline.Stop();

            // Snap précis + déplace l'acteur via le callback HandleProgress
            Timeline.SetPlaybackPosition(TargetCurvePos, true);
            CurrentTimelineProgress = TargetCurvePos;

            bMovingToStep = false;
            bCanMove      = true;

            UE_LOG(LogTemp, Warning,
                TEXT("[IncrementalDoor] '%s' palier %d/%d atteint"),
                *GetName(), ActivationCount, NeededTriggerCount);

            if (ActivationCount >= NeededTriggerCount)
            {
                bIsFullyOpen       = true;
                bIsPermanentlyOpen = bIsExtractionDoor;
            }
        }
        else
        {
            Timeline.TickTimeline(DeltaTime);
        }
    }
    else
    {
        Timeline.TickTimeline(DeltaTime);
    }
}

void AIncrementalDoor::StartOpening()
{
    // Appelé par le Lever directement
    if (!HasAuthority())                       return;
    if (bIsPermanentlyOpen)                    return;
    if (bMovingToStep)                         return; // déjà en mouvement
    if (ActivationCount >= NeededTriggerCount) return; // déjà pleine ouverte
    if (!MoveCurve)                            return;

    ActivationCount++;

    float CurveEnd      = MoveCurve->FloatCurve.GetLastKey().Time;
    float PreviousRatio = (float)(ActivationCount - 1) / (float)NeededTriggerCount;
    float TargetRatio   = (float)ActivationCount       / (float)NeededTriggerCount;

    TargetCurvePos = TargetRatio * CurveEnd;

    float PlayRate = (MovementDuration > 0.f) ? (CurveEnd / MovementDuration) : 1.f;
    Timeline.SetPlayRate(PlayRate);

    // Repart exactement depuis le palier précédent
    Timeline.SetPlaybackPosition(PreviousRatio * CurveEnd, false);
    Timeline.Play();

    bIsMovingForward = true;
    bMovingToStep    = true;
    bCanMove         = false;

    UE_LOG(LogTemp, Warning,
        TEXT("[IncrementalDoor] '%s' levier → activation %d/%d (target curvePos=%.3f)"),
        *GetName(), ActivationCount, NeededTriggerCount, TargetCurvePos);
}

void AIncrementalDoor::StopMainAction_Implementation()
{
    // Les leviers ne referment pas la porte
}
void AIncrementalDoor::StopOpening()
{
    // On ne ferme jamais la porte, les paliers sont permanents
    UE_LOG(LogTemp, Warning,
        TEXT("[IncrementalDoor] '%s' StopOpening ignoré — paliers permanents"),
        *GetName());
}
