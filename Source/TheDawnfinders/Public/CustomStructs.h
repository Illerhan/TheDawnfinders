// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "DataAssets/ItemData.h"
#include "CustomStructs.generated.h" 

USTRUCT(BlueprintType)
struct THEDAWNFINDERS_API FInventorySlot {

	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UItemData* ItemData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Quantity;

	FInventorySlot()
		:  ItemData(nullptr), Quantity()
	{
	}
};