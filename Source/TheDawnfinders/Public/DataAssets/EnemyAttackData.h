// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CustomStructs.h"
#include "EnemyAttackData.generated.h"



UCLASS(BlueprintType)
class THEDAWNFINDERS_API UEnemyAttackData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public :

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Main")
	float AttackDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Main")
	UAnimMontage* AttackAnimMontage;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Triggers")
	TArray<FEnemyAttackTrigger> AttackTriggers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Triggers")
	int AttackPriority;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Triggers")
	int AttackCooldown;

};
