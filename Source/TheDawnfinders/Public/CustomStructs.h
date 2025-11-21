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
	bool bIsOverloadSlot;

	FInventorySlot()
	:  ItemData(nullptr), Quantity(), bIsOverloadSlot(false)
	{}

	bool operator==(const FInventorySlot& Other) const
	{
		return ItemData == Other.ItemData && Quantity == Other.Quantity && bIsOverloadSlot == Other.bIsOverloadSlot;
	}

	bool operator!=(const FInventorySlot& Other) const
	{
		return !(*this == Other);
	}
};




UENUM(BlueprintType)
enum class EEnemyAttackTriggerType : uint8 {
	None UMETA(DisplayName = "None"),
	DistanceMin UMETA(DisplayName = "DistanceMin"),
	DistanceMax UMETA(DisplayName = "DistanceMax"),
	HealthMin UMETA(DisplayName = "HealthMin"),
	HealthMax UMETA(DisplayName = "HealthMax")
};

USTRUCT(BlueprintType)
struct THEDAWNFINDERS_API FEnemyAttackTrigger {

	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEnemyAttackTriggerType EnemyAttackTriggerType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value;

	FEnemyAttackTrigger()
		: Value()
	{
	}
};