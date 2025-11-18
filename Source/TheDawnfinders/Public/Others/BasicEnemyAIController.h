// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BasicEnemyAIController.generated.h"


UCLASS()
class THEDAWNFINDERS_API ABasicEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public :
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<AActor*> PlayersAtRange;
};
