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
	UFUNCTION(BlueprintCallable, Category="Sound|Networked", meta=(WorldContext="WorldContextObject"))
	static void PlaySoundNetworked(UObject* WorldContextObject, USoundBase* Sound, FVector Location, float Loudness, float Range, bool bNeedNoise);
};