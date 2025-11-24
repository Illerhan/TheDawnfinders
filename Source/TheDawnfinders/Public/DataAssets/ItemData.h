// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemData.generated.h"

class AItem;
class AThrowableObject;

UENUM(BlueprintType)
enum class EItemType : uint8 {
	Equipment UMETA(DisplayName = "Weapon"),
	Valuable UMETA(DisplayName = "Valuable"),
	Consumable UMETA(DisplayName = "Consumable"),
	Ammo UMETA(DisplayName = "Ammo"),
};

UENUM(BlueprintType)
enum class EConsumableEffectType : uint8 {
	Heal UMETA(DisplayName = "Heal"),
	Revive UMETA (DisplayName = "Revive"),
	PlaceZipline UMETA(DisplayName = "PlaceZipline"),
	ThrowObject UMETA(DisplayName = "ThrowObject"),
	Refile UMETA(DisplayName = "Refile"),
	None UMETA(DisplayName = "None")
};



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
	int MaxStackingCapacity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UStaticMesh* ItemMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UTexture2D* ItemIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSubclassOf<AItem> ItemClass;


// Consumable
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Consumable")
	EConsumableEffectType ConsumableEffectType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Consumable")
	int ConsumableEffectPower;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Consumable")
	float NeededHoldDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Consumable")
	bool ContextualUse;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable")
	TSubclassOf<class AZiplineInteractible> ZiplineClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable")
	TSubclassOf<class AThrowableObject> ThrowedObjectClass;


// Weapon
public :
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TArray<float> BaseComboDamages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TArray<UAnimMontage*> BaseComboAnims;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TArray<float> HeavyComboDamages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TArray<UAnimMontage*> HeavyComboAnims;
};
