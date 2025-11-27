// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AmuletData.generated.h"


UENUM(BlueprintType)
enum class EAmuletTriggerType : uint8 
{
	Always UMETA(DisplayName = "ChangeStatistics"),
	OnHealthChange UMETA(DisplayName = "OnHealthChange")
};


UCLASS(Blueprintable)
class THEDAWNFINDERS_API UAmuletData : public UDataAsset
{
	GENERATED_BODY()

public :
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FName AmuletName;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	FName AmuletDescription;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int AmuletPower;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	EAmuletTriggerType AmuletTriggerType;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void VerifyApplyEffect(APlayerController* PC, APlayerState* PS);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ApplyEffect(APlayerController* PC, APlayerState* PS);

	virtual void VerifyApplyEffect_Implementation(APlayerController* PC, APlayerState* PS);
	virtual void ApplyEffect_Implementation(APlayerController* PC, APlayerState* PS);
};
