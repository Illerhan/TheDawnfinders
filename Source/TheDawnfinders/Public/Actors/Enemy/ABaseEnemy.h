// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DataAssets/EnemyData.h"
#include "ABaseEnemy.generated.h"



UENUM(BlueprintType)
enum class EEnemyState : uint8 {
	Idle UMETA(DisplayName = "Idle"),
	Suspicious UMETA(DisplayName = "Suspicious"),
	Aggressive UMETA(DisplayName = "Aggressive")
};


UCLASS()
class THEDAWNFINDERS_API ABaseEnemy : public ACharacter
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
};
