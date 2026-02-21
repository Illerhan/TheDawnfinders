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
	FName Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Main")
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Main")
	float Damages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Main")
	float AlertnessThreshold;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Behavior Parameters")
	float PatrolReachPositionWait = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Behavior Parameters")
	float ListeningStateProba = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Behavior Parameters")
	float ListeningStateDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Behavior Parameters")
	float SuspiciousStateDuration = 10.0f;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hearing")
	float NormalHearingRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hearing")
	float EnterListeningDelay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hearing")
	float ListeningHearingRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hearing")
	float EnterSuspiciousDelay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hearing")
	float SuspiciousHearingRadius;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float NormalSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float SuspiciousSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float AggressiveSpeed;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bagarre")
	TArray<FName> PossibleAttacksRowNames;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bagarre")
	float AtttacksRange = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bagarre")
	float AttacksRadius = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bagarre")
	bool DisplayCollisionDebug = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"), Category = "Bagarre")
	float BluntAbsorption = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"), Category = "Bagarre")
	float PiercingAbsorption = 0;
};
