// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemData.generated.h"

class AItem;

UENUM(BlueprintType)
enum class EItemType : uint8 {
	Equipment UMETA(DisplayName = "Weapon"),
	Valuable UMETA(DisplayName = "Valuable"),
	Key UMETA(DisplayName = "Key"),
	Ammo UMETA(DisplayName = "Ammo"),
};

/**
 * 
 */
UCLASS(BlueprintType)
class THEDAWNFINDERS_API UItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public :
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString ItemDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int ItemValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UStaticMesh* ItemMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UTexture2D* ItemIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSubclassOf<AItem> ItemClass;
};
