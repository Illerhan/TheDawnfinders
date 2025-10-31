// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CustomPlayerState.h"
#include "GameFramework/GameState.h"
#include "CustomGameState.generated.h"

/**
 * 
 */
UCLASS()
class THEDAWNFINDERS_API ACustomGameState : public AGameStateBase
{
	GENERATED_BODY()

public :
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerListChanged);

	UPROPERTY(BlueprintAssignable, Category = "Custom GameState")
	FOnPlayerListChanged OnPlayerListChanged;

	virtual void AddPlayerState(APlayerState* Player) override;
	virtual void RemovePlayerState(APlayerState* Player) override;
};
