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
    bMovingToStep   = false;
    bIsFullyOpen    = false;
    TargetCurvePos  = 0.f;
}

void AIncrementalDoor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime); // Toujours utiliser Super:: au lieu de AActor::

    if (!MoveCurve || !Timeline.IsPlaying())
        return;

    Timeline.TickTimeline(DeltaTime);

    // 🎯 On vérifie si on a atteint ou dépassé la cible actuelle
    if (Timeline.GetPlaybackPosition() >= TargetCurvePos)
    {
        FinishStep();
    }
}

void AIncrementalDoor::StartOpening()
{
    // Sécurités de base
    if (!HasAuthority() || bIsPermanentlyOpen || bIsFullyOpen || !MoveCurve) 
        return;

    // 1. On incrémente le compte à chaque signal reçu
    ActivationCount++;

    if (ActivationCount > NeededTriggerCount)
    {
        ActivationCount = NeededTriggerCount;
    }

    // 2. On calcule la nouvelle cible temporelle de la Timeline
    const float CurveEnd = MoveCurve->FloatCurve.GetLastKey().Time;
    float TargetRatio = (float)ActivationCount / NeededTriggerCount;
    
    TargetCurvePos = TargetRatio * CurveEnd;

    // 3. Vitesse de lecture
    float PlayRate = (MovementDuration > 0.f) ? (CurveEnd / MovementDuration) : 1.f;
    Timeline.SetPlayRate(PlayRate);

    // 4. On lance la lecture (si elle tournait déjà, Play() ne fait que continuer)
    Timeline.Play();
    
    bMovingToStep = true;
    bCanMove      = false;

    Multi_OpeningSound();

    UE_LOG(LogTemp, Warning, TEXT("[IncrementalDoor] '%s' Activation %d/%d - En route vers %.3f"), *GetName(), ActivationCount, NeededTriggerCount, TargetCurvePos);
}

void AIncrementalDoor::FinishStep()
{
    // On snap exactement à la position cible et on met en pause
    Timeline.SetPlaybackPosition(TargetCurvePos, false);
    Timeline.Stop();

    bMovingToStep = false;
    bCanMove      = true;

    UE_LOG(LogTemp, Warning, TEXT("[IncrementalDoor] '%s' Palier %d/%d atteint"), *GetName(), ActivationCount, NeededTriggerCount);

    // Vérification finale si la porte a atteint son dernier palier
    if (ActivationCount >= NeededTriggerCount)
    {
        bIsFullyOpen       = true;
        bIsPermanentlyOpen = bIsExtractionDoor;

        UE_LOG(LogTemp, Warning, TEXT("[IncrementalDoor] '%s' FULLY OPEN"), *GetName());
    }
}

void AIncrementalDoor::StopMainAction_Implementation() 
{
    // Actuellement vide : la porte ne redescend pas si un joueur lâche une plaque.
    // Si tu veux qu'elle redescende un jour, c'est ici qu'il faudra faire un ActivationCount-- 
    // et appeler Timeline.Reverse() avec une logique similaire dans le Tick.
}

void AIncrementalDoor::StopOpening()
{
    UE_LOG(LogTemp, Warning, TEXT("[IncrementalDoor] '%s' StopOpening ignoré"), *GetName());
}