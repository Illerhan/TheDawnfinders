// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyAttackData.h"
#include "EnemyData.generated.h"



UCLASS(BlueprintType)
class THEDAWNFINDERS_API UEnemyData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public :

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Main")
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Main")
	float NormalSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Main")
	float SuspiciousSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Main")
	float AggressiveSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Main")
	TArray<UEnemyAttackData*> PossibleAttacks;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"), Category = "Main")
	float BluntAbsorption = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"), Category = "Main")
	float PiercingAbsorption = 0;
};
