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
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<AActor*> PlayersAtRange;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Alertness;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ABaseEnemy* PossessedPawn;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector LastNoiseLocation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EEnemyState EnemyState;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY()
	UAISenseConfig_Hearing* HearingConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UEnemyAttackComponent* EnemyAttackComponent;
};
