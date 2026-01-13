// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Actors/Enemy/ABaseEnemy.h"
#include "BasicEnemyAIController.generated.h"


class UAIPerceptionComponent;
class UAISenseConfig_Hearing;
class UEnemyAttackComponent;


UCLASS()
class THEDAWNFINDERS_API ABasicEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public :
	ABasicEnemyAIController();
	virtual void Tick(float DeltaTime) override;


public :
	UFUNCTION(BlueprintCallable)
	void AddAlertness(float Quantity);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetEnemyState(EEnemyState NewEnemyState);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void EndAttack();

	UFUNCTION(BlueprintCallable)
	void SetHearingRange(float NewRange);

	UFUNCTION(BlueprintCallable)
	UEnemyAttackComponent* GetEnemyAttackComponent();


protected :
	UPROPERTY(BlueprintReadWrite)
	TArray<AActor*> PlayersAtRange;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AlertnessWaitDuration;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AlertnessDecreaseSpeed;

	UPROPERTY(BlueprintReadWrite)
	float Alertness;

	UPROPERTY(BlueprintReadWrite)
	float AlertnessTimer;

	UPROPERTY(BlueprintReadWrite)
	ABaseEnemy* PossessedPawn;

	UPROPERTY(BlueprintReadWrite)
	FVector LastNoiseLocation;

	UPROPERTY(BlueprintReadWrite)
	EEnemyState EnemyState;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY()
	UAISenseConfig_Hearing* HearingConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UEnemyAttackComponent* EnemyAttackComponent;
};
