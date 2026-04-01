// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Player/ANoise.h"
#include "GameFramework/PlayerController.h"
#include "CustomPlayerController.generated.h"


class UMuleWidget;
class AMule;

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
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_CallMule(AActor* Actor);
	
	UFUNCTION(Client, Reliable)
	void Client_SetRequestedPanel(int32 TargetPanel);
	
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Server_HoldMule(float DeltaTime);
	
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Server_StopHoldMule();
	
	UFUNCTION(BlueprintCallable)
	void RespawnToCheckpoint();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bIsHolding;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UMuleWidget* MuleWidget;
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category = Mule)
	float NoiseRange;
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category = Mule)
	bool bIsLoud;
		
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mule)
	ANoise* NoiseActor;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<class ANoise> NoiseObject;
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	AMule* Mule;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bIsInTuto;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	AActor* SpawnPoint;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float InputHoldDuration;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float HoldTimer;

};
