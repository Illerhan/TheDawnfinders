// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CustomPlayerState.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnInfoChange, const float, hp, const float, maxHp, const float, stam, const float, maxStam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInfoChange);

UCLASS()
class THEDAWNFINDERS_API ACustomPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public :
	virtual void BeginPlay() override;

	FOnInfoChange OnInfoChange;

	UFUNCTION(BlueprintCallable)
	void ActualiseStamina(float current, float max);

	UFUNCTION(BlueprintCallable)
	void ActualiseHealth(float current, float max);

	UPROPERTY(ReplicatedUsing = OnRep_StaminaChange)
	float CurrentStamina;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadOnly)
	float CurrentMaxStamina;

	UPROPERTY(BlueprintReadOnly)
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly)
	float CurrentMaxHealth;

	UFUNCTION()
	void OnRep_StaminaChange();
};
