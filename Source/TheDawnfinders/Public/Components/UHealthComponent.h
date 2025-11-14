// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "UHealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class THEDAWNFINDERS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
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

	UPROPERTY(ReplicatedUsing = OnRep_ProtectionZoneAmount)
	int32 ProtectionZoneAmount = 0;
	
	UFUNCTION()
	void OnRep_ProtectionZoneAmount();

	UFUNCTION(BlueprintCallable)
	bool IsProtectedFromCurse() const;

	UFUNCTION(BlueprintCallable)
	void AddProtectionZone();

	UFUNCTION(BlueprintCallable)
	void RemoveProtectionZone();

	void ApplyCurse(float DeltaTime);

	UPROPERTY(Replicated)
	float CurrentHealth = 100.f;
	UPROPERTY(Replicated)
	float CurrentMaxHealth = 100.f;

private :

	float MaxHealth = 100.f;
	float MinimumMaxHP = 20.f;
	float CurseRatio= 0.02;

};
