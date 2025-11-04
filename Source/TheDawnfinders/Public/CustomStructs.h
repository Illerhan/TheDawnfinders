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
	int32 Quantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsOverloadSlot;

	FInventorySlot()
	:  ItemData(nullptr), Quantity(), IsOverloadSlot(false)
	{}

	bool operator==(const FInventorySlot& Other) const
	{
		return ItemData == Other.ItemData && Quantity == Other.Quantity && IsOverloadSlot == Other.IsOverloadSlot;
	}

	bool operator!=(const FInventorySlot& Other) const
	{
		return !(*this == Other);
	}
};