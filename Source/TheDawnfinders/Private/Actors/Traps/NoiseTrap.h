// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Traps/ATrapBase.h"
#include "GameFramework/SoundManager.h"
#include "NoiseTrap.generated.h"

class UAkAudioEvent;

UCLASS()
class THEDAWNFINDERS_API ANoiseTrap : public ATrapBase
{
	GENERATED_BODY()

public:
	ANoiseTrap();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void DoTrapAction(AActor* OtherActor = nullptr) override;

	UPROPERTY()
	ASoundManager* SoundManagerInstance;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UAkAudioEvent* TrapSound;
	
	UPROPERTY()
	int32 TrapSoundID;
};
