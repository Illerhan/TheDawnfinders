// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class THEDAWNFINDERS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void InitialiseComponent(float MaxHealth);

	UFUNCTION(BlueprintCallable)
	void Heal(float quantity);

	UFUNCTION(BlueprintCallable)
	void TakeDamage(float quantity);

	UFUNCTION(BlueprintCallable)
	void Die();

	UFUNCTION(Server, Unreliable, BlueprintCallable)
	void ServerChangeHealth(float newHealth);

	UFUNCTION(BlueprintCallable)
	void LocalChangeHealth();

	UPROPERTY(BlueprintReadOnly)
	bool IsDead;

private :

	float CurrentHealth = 100.f;
	float CurrentMaxHealth = 100.f;
};
