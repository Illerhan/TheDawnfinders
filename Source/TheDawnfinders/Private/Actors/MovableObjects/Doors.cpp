// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/MovableObjects/Doors.h"

#include "GameFramework/UDoorRegistry.h"
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
    
    if (bIsExtractionDoor)
    {
        UUDoorRegistry* Registry = GetGameInstance()->GetSubsystem<UUDoorRegistry>();
        if (Registry)
        {
            Registry->RegisterExtractionDoor(this);
        }
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
    if (bIsPermanentlyOpen) 
    {
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door '%s' is permanently open, ignoring close request"), *GetName());
        return;
    }
    
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

void ADoors::OpenPermanently()
{
    if (!HasAuthority()) return;
    if (!MoveCurve) return;
    if (!bIsExtractionDoor) return;

    UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door '%s' opening PERMANENTLY"), *GetName());
    
    bIsPermanentlyOpen = true;
    CurrentTriggerCount = NeededTriggerCount;

    // Force l'ouverture complète
    float ForwardRate = 1.0f;
    if (MoveCurve->FloatCurve.GetLastKey().Time > 0 && MovementDuration > 0)
    {
        ForwardRate = MoveCurve->FloatCurve.GetLastKey().Time / MovementDuration;
    }
    
    Timeline.SetPlayRate(ForwardRate);
    Timeline.PlayFromStart();
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
    Timeline.Stop();
}

void ADoors::StopOpening()
{
    if (!HasAuthority()) return;
    if (!MoveCurve) return;
    if (bIsPermanentlyOpen) return;

    float ForwardRate = MoveCurve->FloatCurve.GetLastKey().Time / MovementDuration;
    Timeline.SetPlayRate(ForwardRate);

    // Si la timeline ne joue pas → on force la fermeture
    if (!Timeline.IsPlaying())
    {
        Timeline.ReverseFromEnd();
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door closing from end"));
        return;
    }

    // Si elle est en train d'ouvrir → on inverse
    if (!Timeline.IsReversing())
    {
        Timeline.Reverse();
        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door reversing to close"));
    }
}


void ADoors::OnTimelineFinished()
{
    bool bOpened = Timeline.GetPlaybackPosition() >= 0.99f;

    if (bOpened)
    {
        bIsFullyOpen = true;
        bCanMove = true;
        CurrentTimelineProgress = 1.0f;

        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door fully opened"));

        if (bAutoCloseWhenFullyOpen && !bIsPermanentlyOpen)
        {
            GetWorldTimerManager().SetTimer(
                AutoCloseTimer,
                this,
                &ADoors::CloseDoor,
                AutoCloseDelay,
                false
            );
        }
    }
    else
    {
        bIsFullyOpen = false;
        bCanMove = true;
        CurrentTimelineProgress = 0.0f;

        UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door fully closed"));
    }
}

void ADoors::CloseDoor()
{
    if (!HasAuthority()) return;
    if (bIsPermanentlyOpen) return;

    float ForwardRate = MoveCurve->FloatCurve.GetLastKey().Time / MovementDuration;
    Timeline.SetPlayRate(ForwardRate);

    Timeline.ReverseFromEnd();

    UE_LOG(LogTemp, Warning, TEXT("[SERVER] Auto closing door"));
}

#pragma endregion