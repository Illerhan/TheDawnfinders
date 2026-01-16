// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/MovableObjects/Doors.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ADoors::ADoors()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ADoors::StartOpening()
{
    if (!HasAuthority()) return;
    if (!MoveCurve) return;

    bCanMove = false;
    bIsFullyOpen = false;

    if (Timeline.IsReversing())
    {
        Timeline.Play();
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door resuming opening"));
    }
    else if (!Timeline.IsPlaying())
    {
        Timeline.PlayFromStart();
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door opening"));
    }
}

void ADoors::StopOpening()
{
    if (!HasAuthority()) return;
    if (!MoveCurve) return;

    // Si la porte est complètement ouverte et pas en train de bouger
    if (bIsFullyOpen && !Timeline.IsPlaying())
    {
        Timeline.ReverseFromEnd();
        bIsFullyOpen = false;
        bCanMove = false;
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door closing from fully open position (Progress: %f)"), CurrentTimelineProgress);
        return;
    }

    // Si en train d'ouvrir, inverse pour fermer
    if (Timeline.IsPlaying() && !Timeline.IsReversing())
    {
        Timeline.Reverse();
        bIsFullyOpen = false;
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door reversing to close (Progress: %f)"), CurrentTimelineProgress);
        return;
    }

    // Si déjà en train de fermer
    if (Timeline.IsReversing())
    {
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door already closing (Progress: %f)"), CurrentTimelineProgress);
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[SERVER] StopOpening: No action taken (Playing: %d, Reversing: %d, FullyOpen: %d, Progress: %f)"), 
           Timeline.IsPlaying(), Timeline.IsReversing(), bIsFullyOpen, CurrentTimelineProgress);
}

void ADoors::BeginPlay()
{
    Super::BeginPlay();

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

void ADoors::OnTimelineFinished()
{
    // Override pour les portes
    if (Timeline.GetPlaybackPosition() >= 0.99f)
    {
        // Porte complètement ouverte
        bIsFullyOpen = true;
        bCanMove = true;
        CurrentTimelineProgress = 1.0f;
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door fully opened"));
    }
    else
    {
        // Porte complètement fermée
        bIsFullyOpen = false;
        bCanMove = true;
        CurrentTimelineProgress = 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door fully closed"));
    }
}