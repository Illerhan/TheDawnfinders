// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAssets/AmuletData.h"
#include "OverloadAmulet.generated.h"

UCLASS(Blueprintable)
class THEDAWNFINDERS_API UOverloadAmulet : public UAmuletData
{
	GENERATED_BODY()

public :
	void VerifyApplyEffect_Implementation(APlayerController* PC, APlayerState* PS) override;
	void ApplyEffect_Implementation(APlayerController* PC, APlayerState* PS) override;
};
