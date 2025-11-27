// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Components/UHealthComponent.h"
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


// === HEALTH + STAMINA
public :
	UFUNCTION(BlueprintCallable)
	void ActualiseLocalStamina(float current, float max);

	UFUNCTION(BlueprintCallable)
	void ActualiseStamina(float current, float max);

	UFUNCTION(BlueprintCallable)
	void ActualiseLocalHealth(float current, float max, float fixedMax);

	UFUNCTION(BlueprintCallable)
	void ActualiseHealth(float current, float max, float fixedMax);


// === LANTERN ===
public :
	UFUNCTION(BlueprintCallable)
	void ActualiseLocalLantern(float current, float max);

	UFUNCTION(BlueprintCallable)
	void ActualiseLantern(float current, float max);


// === AMULETS
public :
	UFUNCTION(BlueprintCallable)
	void ApplyContextualAmulet(EAmuletTriggerType Trigger);


// === PUBLIC PROPERTIES
public :
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

// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY(BlueprintReadOnly)
	TSet<UAmuletData*> PossessedAmulets;
};
