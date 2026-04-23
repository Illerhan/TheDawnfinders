
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int AmmoInMagazine;


	FItemInfos()
	:  ItemData(nullptr), Durability()
	{}

	FItemInfos(UItemData* Data, int Durability)
		: ItemData(Data), Durability(Durability), AmmoInMagazine(0)
	{
	}

	FItemInfos(UItemData* Data, int Durability, int Ammo)
		: ItemData(Data), Durability(Durability), AmmoInMagazine(Ammo)
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRevealed;

	FInventorySlot()
	: CurrentInfos(), Quantity(), bIsOverloadSlot(false), bIsRevealed(false) {}

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


USTRUCT(BlueprintType)
struct FWeaponInfos : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	FName WeaponTypeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	float BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	float PlayerSpeedModifier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main")
	float SneakMultiplier = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
	float StaminaMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
	float AnimsSpeedModifier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"), Category = "Melee")
	float CriticalChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
	float Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
	float Radius = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
	bool DisplayCollisionDebug;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
	EDamageType DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
	float MineDamageMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	float MaxAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	float MinAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	float AimingSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	int MagazineSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	float ReloadDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	float NoiseRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	int NumberOfShots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	float MaxRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	float WalkAimModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	float DelayBetweenShots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	UItemData* NeededAmmo;
};


USTRUCT(BlueprintType)
struct FWeaponTypesData : public FTableRowBase {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged")
	bool IsRangedWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged")
	UAnimMontage* ShootAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
	TArray<FName> LightComboActionNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
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