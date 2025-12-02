// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "SoundManager.h"
#include "UObject/Object.h"
#include "GameplaySoundHelper.generated.h"

/**
 * 
 */
UCLASS()
class THEDAWNFINDERS_API UGameplaySoundHelper : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="Sound|Networked")
	static void PlaySoundNetworked(UObject* WorldContextObject, USoundBase* Sound, FVector Location, float Loudness)
	{
		UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
		if(!World) return;

		// Cherche un SoundManager existant
		for(TActorIterator<ASoundManager> It(World); It; ++It)
		{
			It->ServerPlaySound(Sound, Location, Loudness);
			return;
		}
		
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ASoundManager* Manager = World->SpawnActor<ASoundManager>(ASoundManager::StaticClass(), Location, FRotator::ZeroRotator, Params);
		if(Manager)
		{
			Manager->ServerPlaySound(Sound, Location, Loudness);
		}
	}
};