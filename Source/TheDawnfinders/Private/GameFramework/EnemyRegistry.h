// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Enemy/ABaseEnemy.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EnemyRegistry.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FEnemyList
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TArray<ABaseEnemy*> Enemies;
};

UCLASS()
class THEDAWNFINDERS_API UEnemyRegistry : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	TMap<int32, FEnemyList> EnemiesByDangerLevel;
		
	UPROPERTY(BlueprintReadWrite)
	TArray<ABaseEnemy*> AllEnemies;
		
	UFUNCTION(BlueprintCallable)
	void RegisterEnemy(ABaseEnemy* Enemy);
	
	UFUNCTION(BlueprintCallable)
	TArray<ABaseEnemy*> GetEnemiesByDangerLevel(int32 DangerLevel) const;
	
	UFUNCTION(BlueprintCallable)
	void UnregisterEnemy(ABaseEnemy* Enemy);
};
