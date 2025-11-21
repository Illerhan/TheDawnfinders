// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UStaminaComponent.generated.h"


UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class THEDAWNFINDERS_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStaminaComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


// === MAIN FUNCTIONS ===
public :
	UFUNCTION(BlueprintCallable)
	void UseStamina(float quantity);

	UFUNCTION(BlueprintCallable)
	void ReloadStamina(float quantity);

	UFUNCTION(Server, Unreliable, BlueprintCallable)
	void ServerChangeStamina(float newStamina);

	UFUNCTION(BlueprintCallable)
	void ChangeLocalStamina();


// === OTHERS
public :
	UFUNCTION(BlueprintCallable)
	void InitialiseComponent(float MaxStamina, float ReloadSpd, float ReloadDl);

	UFUNCTION(BlueprintCallable)
	bool VerifyHasStamina();

	UFUNCTION(BlueprintCallable)
	void ActualiseCurrentOverloadCount(int NewCount);


// === PRIVATE PROPERTIES === 
private :
	UPROPERTY()
	int CurrentOverloadCount = 0;

	UPROPERTY()
	float CurrentStamina = 100.f;

	UPROPERTY()
	float CurrentMaxStamina = 100.f;

	UPROPERTY()
	float ReloadSpeed = 10.f;

	UPROPERTY()
	float ReloadDelay = 2.f;

	UPROPERTY()
	float CurrentReloadDelay = 0.f;
};
