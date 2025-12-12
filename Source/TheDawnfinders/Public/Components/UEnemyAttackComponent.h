// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataAssets/EnemyData.h"
#include "UEnemyAttackComponent.generated.h"


UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class THEDAWNFINDERS_API UEnemyAttackComponent : public UActorComponent
{
	GENERATED_BODY()


public:	
	UEnemyAttackComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:
	UFUNCTION(BlueprintCallable)
	void SortPossibleAttacks(TArray<UEnemyAttackData*> PossibleAttacks);

	UFUNCTION(BlueprintCallable)
	UEnemyAttackData* GetCurrentAttack(TArray<AActor*> PlayersAtRange);

	UFUNCTION(BlueprintCallable)
	void ActualiseAttacksCooldowns();

	UPROPERTY(BlueprintReadWrite)
	bool bCanAttack;


protected:
	UFUNCTION()
	bool VerifyTrigger(FEnemyAttackTrigger Trigger, TArray<AActor*> PlayersAtRange);

	UPROPERTY(BlueprintReadOnly)
	TArray<UEnemyAttackData*> SortedPossibleAttacks;

	UPROPERTY(BlueprintReadOnly)
	TArray<int> CurrentAttacksCooldowns;
};
