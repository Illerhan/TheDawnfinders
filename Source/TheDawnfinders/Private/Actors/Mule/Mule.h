// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/UInventoryComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "Mule.generated.h"


class UWorldInteractibleWidget;

UCLASS()
class THEDAWNFINDERS_API AMule : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMule();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mule)
	UBoxComponent* LootCollider;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere,Category = Mule)
	UInventoryComponent* InventoryComponent;
	
	UPROPERTY(BlueprintReadOnly,EditAnywhere)
	UWorldInteractibleWidget* InteractibleWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Widgets")
	UWidgetComponent* InteractibleWidgetComponent;
	
	// Constantes (configurables dans l'éditeur)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Mule)
	float ChargeCooldown = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Mule)
	float CallCooldown = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Mule)
	int32 MaxCharges = 2;

	// State (évolue en runtime)
	UPROPERTY(BlueprintReadWrite, Replicated, Category = Mule)
	int32 CallCharges = 2;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = Mule)
	float CooldownTimer = 0.f;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = Mule)
	float ChargesTimer = 0.f;
			
};
