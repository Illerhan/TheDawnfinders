// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AkDynamicSequencePlaylist.h"
#include "AmbientMusicZone.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MusicManager.generated.h"

UENUM(BlueprintType)
enum class EMusicState : uint8
{
	None,
	ZoneCalm,
	ZoneCombat,
	Extraction
};

UCLASS()
class UMusicManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:


	// Appelé par la zone quand le joueur entre/sort
	void EnterZone(AAmbientMusicZone* Zone);
	void ExitZone(AAmbientMusicZone* Zone);

	// Appelé par les ennemis
	UFUNCTION(BlueprintCallable)
	void OnEnemyAggro();
	UFUNCTION(BlueprintCallable)
	void OnEnemyCalm();

	// Appelé par l'objectif d'extraction
	UFUNCTION(BlueprintCallable)
	void TriggerExtraction();

	UFUNCTION(BlueprintCallable)
	void PlayMusic(USoundBase* musicToPlay);

	UFUNCTION(BlueprintCallable)
	void ChangeVolume();
	
	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* ExtractionSound;
	bool bZoneChanged;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio")
	float masterVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio")
	float musicVolume = 1.0f;

private:
	EMusicState CurrentState = EMusicState::None;
	AAmbientMusicZone* CurrentZone = nullptr;
	int32 AggroCount = 0; // Nombre d'ennemis en aggro

	AkPlayingID CurrentMusicID = AK_INVALID_PLAYING_ID;
	UPROPERTY()
	UAudioComponent* CurrentMusicComponent;
	

	void EvaluateState();
	void ApplyState(EMusicState NewState);
	void StopCurrent(EMusicState NewState);
	UFUNCTION(BlueprintCallable)
	void ResetState();
};