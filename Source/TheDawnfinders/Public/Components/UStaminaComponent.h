// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UStaminaComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChange, float, CurrentStamina, float, MaxStamina);

UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class THEDAWNFINDERS_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStaminaComponent();
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void UseStamina(float quantity);

	UFUNCTION(BlueprintCallable)
	bool VerifyHasStamina();

	UFUNCTION(BlueprintCallable)
	void InitialiseComponent(float MaxStamina, float ReloadSpd, float ReloadDl);

	UFUNCTION(BlueprintCallable)
	void ReloadStamina(float quantity);

	UPROPERTY(BlueprintAssignable)
	FOnStaminaChange OnStaminaChange;

private :

	float CurrentStamina = 100.f;
	float CurrentMaxStamina = 100.f;
	float ReloadSpeed = 10.f;
	float ReloadDelay = 2.f;
	float CurrentReloadDelay = 0.f;
};
