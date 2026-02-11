// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/MovableObjects/Doors.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ADoors::ADoors()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ADoors::BeginPlay()
{
    Super::BeginPlay();

    // IMPORTANT: Fixer les positions de départ et fin pour les portes
    // Ne JAMAIS les inverser comme le fait MovableObjects
    StartPosition = GetActorLocation();
    FinalPosition = StartPosition + EndPosition;
    
    UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door initialized - Start: %s, Final: %s"), 
           *StartPosition.ToString(), *FinalPosition.ToString());

    if (MoveCurve)
    {
        Timeline.AddInterpFloat(MoveCurve, TimelineProgress);
        Timeline.SetLooping(false);

        if (MoveCurve->FloatCurve.GetLastKey().Time > 0)
        {
            Timeline.SetPlayRate(MoveCurve->FloatCurve.GetLastKey().Time / MovementDuration);
        }

        FOnTimelineEvent TimeLineFinishedCallback;
        TimeLineFinishedCallback.BindUFunction(this, FName("OnTimelineFinished"));
        Timeline.SetTimelineFinishedFunc(TimeLineFinishedCallback);
    }
}


void ADoors::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}


#pragma region Activable Interface

void ADoors::DoMainAction_Implementation()
{
    CurrentTriggerCount++;
    if (CurrentTriggerCount < NeededTriggerCount) return;

    StartOpening();
}

void ADoors::StopMainAction_Implementation()
{
    if (CurrentTriggerCount == 0) return;
    CurrentTriggerCount--;

    if (CurrentTriggerCount > 0) PauseOpening();
    else StopOpening();
}

#pragma endregion


#pragma region Door Functions

void ADoors::StartOpening()
{
    if (!HasAuthority()) return;
    if (!MoveCurve) return;

    bCanMove = false;

    // On calcule la vitesse normale (positive)
    float ForwardRate = 1.0f;
    if (MoveCurve->FloatCurve.GetLastKey().Time > 0 && MovementDuration > 0)
    {
        ForwardRate = MoveCurve->FloatCurve.GetLastKey().Time / MovementDuration;
    }

    Timeline.SetPlayRate(ForwardRate);


    // La porte était en train de se fermer, on inverse le mouvement immédiatement
    if (Timeline.IsReversing())
    {
        Timeline.Play();
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door reversing to OPEN (was closing)"));
        return;
    }

    if (bIsFullyOpen) {
        Timeline.ReverseFromEnd();
        bIsFullyOpen = false;
        bCanMove = false;

        return;
    }

    // La porte est à l'arrêt (soit fermée, soit au milieu arrêtée)
    if (!Timeline.IsPlaying())
    {
        // Si on est pratiquement au début (Progress < 0.1), on s'assure de partir de 0
        if (CurrentTimelineProgress < 0.1f)
        {
            Timeline.PlayFromStart();
            UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door opening from START"));
        }
        else
        {
            // Sinon on reprend de là où on est
            Timeline.SetPlaybackPosition(CurrentTimelineProgress, false);
            Timeline.Play();
            UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door resuming opening from: %f"), CurrentTimelineProgress);
        }

        bIsFullyOpen = false;
    }
}

void ADoors::AddOpeningPlayer()
{
    CurrentAddedSpeed += SpeedAddedPerAdditionalPlayer;

    float ForwardRate = 1.0f;
    if (MoveCurve->FloatCurve.GetLastKey().Time > 0 && MovementDuration > 0)
    {
        ForwardRate = MoveCurve->FloatCurve.GetLastKey().Time / MovementDuration * CurrentAddedSpeed;
    }

    Timeline.SetPlayRate(ForwardRate);
    UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door opening speed %f"), ForwardRate);
}

void ADoors::RemoveOpeningPlayer()
{
    CurrentAddedSpeed -= SpeedAddedPerAdditionalPlayer;

    float ForwardRate = 1.0f;
    if (MoveCurve->FloatCurve.GetLastKey().Time > 0 && MovementDuration > 0)
    {
        ForwardRate = MoveCurve->FloatCurve.GetLastKey().Time / MovementDuration * CurrentAddedSpeed;
    }

    Timeline.SetPlayRate(ForwardRate );
}


void ADoors::PauseOpening()
{
    if (!HasAuthority()) return;
    if (!MoveCurve) return;

    Timeline.SetPlayRate(0);
}


void ADoors::StopOpening()
{
    if (!HasAuthority()) return;
    if (!MoveCurve) return;

    FVector CurrentPos = GetActorLocation();
    float DistanceFromStart = FVector::Distance(CurrentPos, StartPosition);
    float DistanceFromEnd = FVector::Distance(CurrentPos, FinalPosition);

    UE_LOG(LogTemp, Warning, TEXT("[SERVER] StopOpening - DistFromStart: %f, DistFromEnd: %f, Progress: %f"),
        DistanceFromStart, DistanceFromEnd, CurrentTimelineProgress);

    float ForwardRate = 1.0f;
    if (MoveCurve->FloatCurve.GetLastKey().Time > 0 && MovementDuration > 0)
    {
        ForwardRate = MoveCurve->FloatCurve.GetLastKey().Time / MovementDuration;
    }
    Timeline.SetPlayRate(ForwardRate);

    // Si la porte est complètement ouverte (proche de FinalPosition)
    if ((DistanceFromEnd < 10.0f || bIsFullyOpen) && !Timeline.IsPlaying())
    {
        //Timeline.ReverseFromEnd();
        //bIsFullyOpen = false;
        //bCanMove = false;
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door closing from fully open position"));
        return;
    }

    // Si en train d'ouvrir, inverse pour fermer
    if (Timeline.IsPlaying() && !Timeline.IsReversing())
    {
        Timeline.Reverse();
        bIsFullyOpen = false;
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door reversing to close"));
        return;
    }

    // Si déjà en train de fermer
    if (Timeline.IsReversing())
    {
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door already closing"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[SERVER] StopOpening: No action taken"));
}


void ADoors::OnTimelineFinished()
{
    // Override pour les portes - NE PAS inverser les positions comme MovableObjects
    if (Timeline.GetPlaybackPosition() >= 0.99f)
    {
        // Porte complètement ouverte
        bIsFullyOpen = true;
        bCanMove = true;
        CurrentTimelineProgress = 1.0f;
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door fully opened - Start: %s, Final: %s, Current: %s"), 
               *StartPosition.ToString(), *FinalPosition.ToString(), *GetActorLocation().ToString());
    }
    else
    {
        bIsFullyOpen = false;
        bCanMove = true;
        CurrentTimelineProgress = 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door fully closed - Start: %s, Final: %s, Current: %s"), 
               *StartPosition.ToString(), *FinalPosition.ToString(), *GetActorLocation().ToString());
    }
}

#pragma endregion