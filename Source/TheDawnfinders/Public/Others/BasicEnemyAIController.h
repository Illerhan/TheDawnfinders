// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Actors/Enemy/ABaseEnemy.h"
#include "BasicEnemyAIController.generated.h"


UCLASS()
class THEDAWNFINDERS_API ABasicEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public :
	UFUNCTION(BlueprintCallable)
	void AddAlertness(float Quantity);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetEnemyState(EEnemyState NewEnemyState);


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
};
