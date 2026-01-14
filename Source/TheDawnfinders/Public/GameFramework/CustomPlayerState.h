#pragma once

#include "CoreMinimal.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Components/UHealthComponent.h"
#include "CustomStructs.h"
#include "GameFramework/PlayerState.h"
#include "DataAssets/AmuletData.h"
#include "CustomPlayerState.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnInfoChange, const float, hp, const float, maxHp, const float, stam, const float, maxStam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInfoChange);
DECLARE_DYNAMIC_DELEGATE(FOnInfoChangeLocal);


UCLASS()
class THEDAWNFINDERS_API ACustomPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public :
	virtual void BeginPlay() override;


// === REPLICATION ===
public :
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_StaminaChange();

	UFUNCTION()
	void OnRep_HealthChange();

	UFUNCTION()
	void OnRep_LanternChange();


// === DELEGATES
public :
	FOnInfoChange OnInfoChange;
	FOnInfoChangeLocal OnInfoChangeLocal;


// === HEALTH + STAMINA CHANGES
public :
	UFUNCTION(BlueprintCallable)
	void ActualiseLocalStamina(float current, float max);

	UFUNCTION(BlueprintCallable)
	void ActualiseStamina(float current, float max);

	UFUNCTION(BlueprintCallable)
	void ActualiseLocalHealth(float current, float max, float fixedMax);

	UFUNCTION(BlueprintCallable)
	void ActualiseHealth(float current, float max, float fixedMax);

	UFUNCTION(BlueprintCallable)
	void SetMaxHealth(float MaxHP);


// === OTHERS ===
public :
	UFUNCTION(BlueprintCallable)
	void ActualiseLocalLantern(float current, float max);

	UFUNCTION(BlueprintCallable)
	void ActualiseLantern(float current, float max);

	UFUNCTION(BlueprintCallable)
	void ActualiseEquippedItem(FInventorySlot Current);

// === GETTERS ===
public :
	UFUNCTION(BlueprintCallable)
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable)
	float GetCurrentMaxHealth() const { return CurrentMaxHealth; }

	UFUNCTION(BlueprintCallable)
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable)
	float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintCallable)
	float GetCurrentMaxStamina() const { return CurrentMaxStamina; }

	UFUNCTION(BlueprintCallable)
	float GetLanternPercent() const { return LanternPercent; }

	UFUNCTION(BlueprintCallable)
	FInventorySlot GetCurrentEquippedItem() const { return CurrentSlot; }


// === AMULETS ===
public :
	UFUNCTION(BlueprintCallable)
	void ApplyContextualAmulet(EAmuletTriggerType Trigger);


// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY(BlueprintReadOnly)
	TSet<UAmuletData*> PossessedAmulets;

	UPROPERTY(ReplicatedUsing = OnRep_StaminaChange)
	float CurrentStamina;

	UPROPERTY(BlueprintReadOnly, Replicated)
	float CurrentMaxStamina;

	UPROPERTY(ReplicatedUsing = OnRep_HealthChange)
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Replicated)
	float CurrentMaxHealth;

	UPROPERTY(BlueprintReadOnly, Replicated)
	float MaxHealth;

	UPROPERTY(BlueprintReadOnly)
	float LanternPercent = 100;

	UPROPERTY(BlueprintReadOnly)
	FInventorySlot CurrentSlot;
};
