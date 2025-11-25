// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DataAssets/EnemyData.h"
#include "Interfaces/IDamageable.h"
#include "ABaseEnemy.generated.h"



UENUM(BlueprintType)
enum class EEnemyState : uint8 {
	Idle UMETA(DisplayName = "Idle"),
	Suspicious UMETA(DisplayName = "Suspicious"),
	Aggressive UMETA(DisplayName = "Aggressive")
};


UCLASS()
class THEDAWNFINDERS_API ABaseEnemy : public ACharacter, public IDamageable
{
	GENERATED_BODY()
	
public:	
	ABaseEnemy();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UEnemyData* EnemyData;

	UFUNCTION(BlueprintCallable)
	void DoAttack(UEnemyAttackData* AttackData);

	UFUNCTION(BlueprintCallable)
	void OnEndAttack(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
	void BP_OnMontageNotifyBegin(FName NotifyName);


// HEALTH
public :
	UPROPERTY(BlueprintReadOnly)
	float CurrentHealth;

	UFUNCTION(BlueprintCallable)
	void Die();

	void ReceiveDamage_Implementation(float Quantity, AActor* Origin);
};
