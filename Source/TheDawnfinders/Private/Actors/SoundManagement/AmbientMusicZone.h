// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AkAudioEvent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "AmbientMusicZone.generated.h"

UCLASS()
class THEDAWNFINDERS_API AAmbientMusicZone : public AActor
{
	GENERATED_BODY()

public:
	AAmbientMusicZone();

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* MusicCalmSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundBase* MusicCombatSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	int32 Priority = 0;
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* ZoneTrigger;

protected:
	virtual void BeginPlay() override;

private:
	

	UFUNCTION()
	void OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnPlayerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
