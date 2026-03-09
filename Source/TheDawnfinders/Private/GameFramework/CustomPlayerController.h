// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CustomPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class THEDAWNFINDERS_API ACustomPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_RequestSwitchPanel(int32 PanelIndex);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_SwitchPanel(int32 PanelIndex);
	
	UFUNCTION(BlueprintImplementableEvent)
	void SwitchPanelBP(int32 PanelIndex);

};
