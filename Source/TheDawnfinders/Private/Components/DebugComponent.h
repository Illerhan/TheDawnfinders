// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DebugComponent.generated.h"

class UItemData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class THEDAWNFINDERS_API UDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UDebugComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SpawnDebugItem(UItemData* ItemData, FVector Location, FRotator Rotation);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TravelToMap(const FString& MapName);
	
	UFUNCTION(Server, Reliable)
	void Server_TeleportToSpawn();
};
