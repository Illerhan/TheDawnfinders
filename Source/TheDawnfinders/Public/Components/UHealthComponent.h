// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "UHealthComponent.generated.h"

class UStaminaComponent;


UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class THEDAWNFINDERS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


// === MAIN FUNCTIONS ===
public : 
	UFUNCTION(BlueprintCallable)
	void InitialiseComponent(float MaxHP, float MinMaxHP, float MinReviveHP,float InjureDecreaseSpeed, float CurseRatio);

	UFUNCTION(BlueprintCallable)
	void Heal(float quantity);

	UFUNCTION(BlueprintCallable)
	void TakeDamage(float quantity);
	
	UFUNCTION(Server, Unreliable, BlueprintCallable)
	void Server_TakeDamage(float quantity, AActor* Origin);

	UFUNCTION(Server, Unreliable, BlueprintCallable)
	void ServerChangeHealth(float newHealth);

	UFUNCTION(BlueprintCallable)
	void LocalChangeHealth();


// === CURSE === 
public :
	UFUNCTION(BlueprintCallable)
	bool IsProtectedFromCurse() const;

	UFUNCTION(BlueprintCallable)
	void AddProtectionZone();

	UFUNCTION(BlueprintCallable)
	void RemoveProtectionZone();

	UFUNCTION(BlueprintCallable)
	void ApplyCurse(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void ActualiseCursePostProcess(float DeltaTime);


// === DEATH ===
public : 
	UFUNCTION()
	void Fallen();

	UFUNCTION(BlueprintCallable)
	void FallenLoseHP(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void Die();

	UFUNCTION(Server,Reliable,BlueprintCallable)
	void Server_Revive();


// === INVINCIBILITY ===
public :
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StartInvincibilityFrames(float Duration);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void EndInvincibilityFrames();

	FTimerHandle InvincibilityTimerHandle;


// === NETWORK ===
public :
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_IsDead();

	UFUNCTION()
	void OnRep_IsFallen();
	
	UFUNCTION()
	void OnRep_ProtectionZoneAmount();


// === PUBLIC PROPERTIES ===
public :
	UPROPERTY(Replicated)
	float CurrentHealth = 100.f;

	UPROPERTY()
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_IsDead, BlueprintReadWrite)
	bool bIsDead;
	
	UPROPERTY()
	int CurseZone = 0;


// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY(Replicated)
	float CurrentMaxHealth = 100.f;

	UPROPERTY(Replicated)
	float CurseMaxHealth = 100.f;

	UPROPERTY(BlueprintReadOnly)
	float MinReviveHP = 20.f;

	UPROPERTY(EditAnywhere)
	float InjureDecreaseSpeed = 0.02f;

	UPROPERTY(ReplicatedUsing = OnRep_IsFallen, BlueprintReadWrite)
	bool bIsFallen;
	
	UPROPERTY(ReplicatedUsing = OnRep_ProtectionZoneAmount)
	int32 ProtectionZoneAmount = 0;

	UPROPERTY()
	float MinimumMaxHP = 20.f;

	UPROPERTY()
	float CurseRatio= 1;

	UPROPERTY()
	UStaminaComponent* StaminaComponent;

	UPROPERTY()
	bool IsInvincible;
	
	

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UMaterialInstanceDynamic* CurseMaterial;

	UPROPERTY()
	float CurrentCurseVolumeStrength;

	UPROPERTY()
	AController* OwnerController;
};
