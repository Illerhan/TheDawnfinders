// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemData.generated.h"

class AItem;
class AThrowableObject;
class ATrapBase;

UENUM(BlueprintType)
enum class EItemRarity : uint8 {
	Common UMETA(DisplayName = "Common"),
	Rare UMETA(DisplayName = "Rare"),
	Epic UMETA(DisplayName = "Epic"),
	Legendary UMETA(DisplayName = "Legendary")
};

UENUM(BlueprintType)
enum class EItemType : uint8 {
	Equipment UMETA(DisplayName = "Weapon"),
	Valuable UMETA(DisplayName = "Valuable"),
	Consumable UMETA(DisplayName = "Consumable"),
	Currency UMETA(DisplayName = "Currency"),
	Ammo UMETA(DisplayName = "Ammo"),
};

UENUM(BlueprintType)
enum class EConsumableEffectType : uint8 {
	Heal UMETA(DisplayName = "Heal"),
	Revive UMETA (DisplayName = "Revive"),
	PlaceZipline UMETA(DisplayName = "PlaceZipline"),
	ThrowObject UMETA(DisplayName = "ThrowObject"),
	Refile UMETA(DisplayName = "Refile"),
	Inhale UMETA(DisplayName = "Inhale"),
	Adrenaline UMETA(DisplayName = "Adrenaline"),
	Protector UMETA(DisplayName = "Protector"),
	Navigation UMETA(DisplayName = "Navigation"),
	SoundProtect UMETA(DisplayName = "SoundProtect"),
	Antidote UMETA(DisplayName = "Antidote"),
	Poison UMETA(DisplayName="Poison"),
	OpenMap UMETA(DisplayName = "OpenMap"),
	PlaceTrap UMETA(DisplayName = "PlaceTrap"),
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
	int SellValue;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int ItemWeight = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int MaxStackingCapacity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItemRarity ItemRarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UStaticMesh* ItemMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UTexture2D* ItemIcon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSubclassOf<AItem> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int Durability = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float UsedDurabilityMultiplier = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UTexture2D* FirstIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString FirstIconTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString FirstIconText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UTexture2D* SecondIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString SecondIconTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString SecondIconText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UTexture2D* ThirdIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString ThirdIconTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString ThirdIconText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UTexture2D* FourthIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString FourthIconTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString FourthIconText;


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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Consumable")
	bool ImmobiliseOnUse;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable")
	TSubclassOf<class AZiplineInteractible> ZiplineClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable")
	TSubclassOf<class AThrowableObject> ThrowedObjectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable")
	TSubclassOf<class ATrapBase> PlacedTrap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable")
	UAnimMontage* UseConsumableMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consumable")
	UTexture2D* MapSprite;


// Weapon
public :
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	FName WeaponDataTableRow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	bool bIsRangedWeapon;
};
