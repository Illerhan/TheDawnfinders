// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/UInventoryComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "Mule.generated.h"


class ADangerManager;
class UWorldInteractibleWidget;

UCLASS()
class THEDAWNFINDERS_API AMule : public ACharacter
{
	GENERATED_BODY()

public:
	AMule();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mule)
	UBoxComponent* LootCollider;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category = Mule)
	UInventoryComponent* InventoryComponent;
	
	UPROPERTY(BlueprintReadWrite)
	UWorldInteractibleWidget* InteractibleWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Widgets")
	UWidgetComponent* InteractibleWidgetComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Mule)
	float ChargeCooldown = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Mule)
	float CallCooldown = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Mule)
	int32 MaxCharges = 1;

	// State (évolue en runtime)
	UPROPERTY(BlueprintReadWrite, Replicated, Category = Mule)
	int32 CallCharges = 1;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = Mule)
	float CooldownTimer = 0.f;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = Mule)
	float ChargesTimer = 0.f;
	
	UPROPERTY(BlueprintReadWrite, Category = Mule)
	ADangerManager* DangerManager;
			
};
