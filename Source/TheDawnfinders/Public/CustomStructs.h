// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "DataAssets/ItemData.h"
#include "Engine/DataTable.h"
#include "CustomStructs.generated.h" 


USTRUCT(BlueprintType)
struct THEDAWNFINDERS_API FItemInfos {

	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UItemData* ItemData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Durability;


	FItemInfos()
	:  ItemData(nullptr), Durability()
	{}

	FItemInfos(UItemData* Data, int Durability)
		: ItemData(Data), Durability(Durability)
	{
	}

	bool operator==(const FItemInfos& Other) const
	{
		return ItemData == Other.ItemData && Durability == Other.Durability;
	}

	bool operator!=(const FItemInfos& Other) const
	{
		return !(*this == Other);
	}
};

USTRUCT(BlueprintType)
struct THEDAWNFINDERS_API FInventorySlot {

	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemInfos CurrentInfos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOverloadSlot;

	FInventorySlot()
	: CurrentInfos(), Quantity(), bIsOverloadSlot(false)
	{}

	bool operator==(const FInventorySlot& Other) const
	{
		return CurrentInfos == Other.CurrentInfos && Quantity == Other.Quantity && bIsOverloadSlot == Other.bIsOverloadSlot;
	}

	bool operator!=(const FInventorySlot& Other) const
	{
		return !(*this == Other);
	}
};


UENUM(BlueprintType)
enum class EQTEType : uint8 {
	NoQTE UMETA(DisplayName = "NoQTE"),
	Rotative UMETA(DisplayName = "Rotative"),
	SmashButton UMETA(DisplayName = "Smash Button")
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



UENUM(BlueprintType)
enum class EDamageType : uint8 {
	Piercing UMETA(DisplayName = "Piercing"),
	Blunt UMETA(DisplayName = "Blunt")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8 {
	OneHanded UMETA(DisplayName = "OneHanded"),
	TwoHanded UMETA(DisplayName = "TwoHanded"),
	AtRange UMETA(DisplayName = "AtRange")
};


USTRUCT(BlueprintType)
struct FWeaponInfos : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeaponTypeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StaminaMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayerSpeedModifier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AnimsSpeedModifier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float CriticalChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool DisplayCollisionDebug;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDamageType DamageType;
};


USTRUCT(BlueprintType)
struct FWeaponTypesData : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> LightComboActionNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> HeavyComboActionNames;
};



#pragma region Actions

USTRUCT(BlueprintType)
struct FWeaponActionData : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StaminaCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveForceDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHoldToCharge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* ChargeAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
	float CameraShakeIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
	float CameraShakeDuration;
};


USTRUCT(BlueprintType)
struct FEnemyActionData : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	float DamageMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	UAnimMontage* Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	float MontageSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PreparationSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	float FollowPlayerDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	bool bEnableRootMotion = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
	TArray<FEnemyAttackTrigger> Triggers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
	int AttackPriority;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triggers")
	float AttackCooldown;
};

#pragma endregion


#pragma region Chests Loot

USTRUCT(BlueprintType)
struct FChestSpawnLoot {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "100"))
	float SpawnPercent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UItemData* ItemData;

	FChestSpawnLoot()
		: SpawnPercent(0.f), ItemData(nullptr)
	{
	}
};


USTRUCT(BlueprintType)
struct FChestSpawn : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MinItemCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxItemCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FChestSpawnLoot> SpawnableItems;

	FChestSpawn()
		: MinItemCount(0), MaxItemCount(0)
	{
	}
};

#pragma endregion