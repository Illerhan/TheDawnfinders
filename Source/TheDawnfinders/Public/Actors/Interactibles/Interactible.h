// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractible.h"
#include "Interfaces/IFadeable.h"
#include "Interactible.generated.h"

class ULockpickQTEWidget;
class UWorldInteractibleWidget;


UCLASS()
class THEDAWNFINDERS_API AInteractibleObjects : public AActor, public IInteractible, public IFadeable
{
	GENERATED_BODY()

public:
	AInteractibleObjects();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;


// === INTERACTION INTERFACE ===
public :
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void StopInteract_Implementation(AActor* Interactor) override;
	virtual bool GetCanBeUsed_Implementation() override;
	virtual bool GetQTENeeded_Implementation() override;
	virtual void StartQTE_Implementation() override;
	virtual void StopQTE_Implementation() override;
	virtual bool ValidateQTE_Implementation() override;


// === FADEABLE INTERFACE ===
public :
	virtual void FadeIn_Implementation() override;
	virtual void FadeOut_Implementation() override;


// === MAIN FUNCTIONS ===
public:
	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable, Category = "Interaction")
	void BP_OnInteraction(AAPlayerCharacter* Player);

	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable, Category = "Interaction")
	void BP_OnStopInteraction(AAPlayerCharacter* Player);


// === COMPONENTS ===
public :
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Collision")
	UCapsuleComponent* CapsuleCollider;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Mesh")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Widgets")
	UWidgetComponent* InteractQTEWidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Widgets")
	UWidgetComponent* InteractibleWidgetComponent;


protected :
	UPROPERTY(Replicated)
	float InteractionTimer;

	UPROPERTY(Replicated)
	bool bIsInteracting;

	UPROPERTY(Replicated)
	AActor* PlayerTemp;


public:
	UFUNCTION(BlueprintCallable)
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


// === PROTECTED PROPERTIES
protected :
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDoQTE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float QTESuccessRange;

	UPROPERTY()
	bool bCanBeUsed = true;


// === PROTECTED PROPERTIES ====
protected :
	UPROPERTY()
	ULockpickQTEWidget* InteractQTEWidget;

	UPROPERTY()
	UWorldInteractibleWidget* InteractibleWidget;
};
