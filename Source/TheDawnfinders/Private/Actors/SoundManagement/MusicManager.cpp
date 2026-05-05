// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MusicManager.h"
#include "CoreMinimal.h"
#include "AkDynamicSequencePlaylist.h"
#include "AkGameplayStatics.h"
#include "AmbientMusicZone.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/GameInstanceSubsystem.h"

void UMusicManager::EnterZone(AAmbientMusicZone* Zone)
{
    if (CurrentZone == Zone) return;

    CurrentZone = Zone;
    bZoneChanged = true; // ← signal que la zone a changé
    EvaluateState();
}

void UMusicManager::ExitZone(AAmbientMusicZone* Zone)
{
    if (CurrentZone != Zone) return;
    CurrentZone = nullptr;
    StopCurrent();
}

void UMusicManager::OnEnemyAggro()
{
    AggroCount++;
    EvaluateState();
}

void UMusicManager::OnEnemyCalm()
{
    AggroCount = FMath::Max(0, AggroCount - 1);
    EvaluateState();
}

void UMusicManager::TriggerExtraction()
{
    // L'extraction ne peut pas être annulée
    ApplyState(EMusicState::Extraction);
}

void UMusicManager::EvaluateState()
{
    if (CurrentState == EMusicState::Extraction) return;
    if (!CurrentZone) return;

    EMusicState TargetState = AggroCount > 0 
        ? EMusicState::ZoneCombat 
        : EMusicState::ZoneCalm;

    // On force le refresh si la zone a changé, même si le state est identique
    if (TargetState != CurrentState || bZoneChanged)
    {
        bZoneChanged = false;
        ApplyState(TargetState);
    }
}

void UMusicManager::ApplyState(EMusicState NewState)
{
    
    if (!CurrentZone && NewState != EMusicState::Extraction) return;

    StopCurrent();
    CurrentState = NewState;

    USoundBase* SoundToPlay = nullptr;

    switch (NewState)
    {
    case EMusicState::ZoneCalm:   SoundToPlay = CurrentZone->MusicCalmSound;   break;
    case EMusicState::ZoneCombat: SoundToPlay = CurrentZone->MusicCombatSound; break;
    case EMusicState::Extraction: SoundToPlay = ExtractionSound;                break;
    default: return;
    }

    if (!SoundToPlay) return;

    CurrentMusicComponent = UGameplayStatics::SpawnSound2D(
        GetGameInstance(),
        SoundToPlay,
        1.f,  // Volume
        1.f,  // Pitch
        0.f,  // StartTime
        nullptr,
        false, // bPersistAcrossLevelTransitions
        true   // bAutoDestroy
    );

    UE_LOG(LogTemp, Warning, TEXT("[MUSIC] State → %d"), (int)NewState);
}

void UMusicManager::StopCurrent()
{
    if (CurrentMusicComponent && CurrentMusicComponent->IsPlaying())
    {
        CurrentMusicComponent->FadeOut(1.5f, 0.f); // Fade out 1.5s
        CurrentMusicComponent = nullptr;
    }
}