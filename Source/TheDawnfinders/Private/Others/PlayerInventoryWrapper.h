// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomStructs.h"
#include "PlayerInventoryWrapper.generated.h"

USTRUCT(BlueprintType)
struct FPlayerInventoryWrapper
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TArray<FInventorySlot> InventorySlots;
};

