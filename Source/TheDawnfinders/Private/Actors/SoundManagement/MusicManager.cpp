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
    bZoneChanged = true;
    EvaluateState();
}

void UMusicManager::ExitZone(AAmbientMusicZone* Zone)
{
    if (CurrentZone != Zone) return;
    CurrentZone = nullptr;
    StopCurrent(CurrentState);
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
        ApplyState(TargetState);
        bZoneChanged = false;
    }
}

void UMusicManager::ApplyState(EMusicState NewState)
{
    
    if (!CurrentZone && NewState != EMusicState::Extraction) return;

    StopCurrent(NewState);
    CurrentState = NewState;

    float newTimer = 0.f;
    float fadeDuration = 1.5f;

    USoundBase* SoundToPlay = nullptr;

    switch (NewState)
    {
    case EMusicState::ZoneCalm:   SoundToPlay = CurrentZone->MusicCalmSound;   break;
    case EMusicState::ZoneCombat: 
        if(CurrentZone->MusicCombatSounds.IsEmpty() )return; 
        SoundToPlay = CurrentZone->MusicCombatSounds[FMath::RandRange(0, CurrentZone->MusicCombatSounds.Num()-1)]; break;
    case EMusicState::Extraction: SoundToPlay = ExtractionSound; break;
    default: return;
    }
    
    if (!SoundToPlay) return;
    
    if (NewState == EMusicState::ZoneCalm && !bZoneChanged) {
        newTimer = FMath::FRandRange(0.f, SoundToPlay->GetDuration());
        fadeDuration = 3.f;
    }

    

    CurrentMusicComponent = UGameplayStatics::CreateSound2D(
        GetGameInstance(),
        SoundToPlay,
        1.f,
        1.f,
        0.f,
        nullptr,
        false,
        true
    );

    CurrentMusicComponent->FadeIn(
        fadeDuration,
        1.f,
        newTimer
    );

    UE_LOG(LogTemp, Warning, TEXT("[MUSIC] State → %d"), (int)NewState);
}

void UMusicManager::StopCurrent(EMusicState NewState)
{
    if (CurrentMusicComponent && CurrentMusicComponent->IsPlaying())
    {
        float fadeDuration = 1.5f;

        if (NewState == EMusicState::ZoneCalm && !bZoneChanged) {
            fadeDuration = 3.f;
        }

        CurrentMusicComponent->FadeOut(fadeDuration, 0.f); // Fade out 1.5s
        CurrentMusicComponent = nullptr;
    }
}