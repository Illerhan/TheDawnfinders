// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UDoorRegistry.generated.h"

class ADoors;
/**
 * 
 */
UCLASS()
class THEDAWNFINDERS_API UUDoorRegistry : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	TArray<TWeakObjectPtr<ADoors>> ExtractDoors;
	
	UFUNCTION(BlueprintCallable)
	void RegisterExtractionDoor(ADoors* Door);
	
	UFUNCTION(BlueprintCallable)
	void OpenAllExtractionDoors();
};
