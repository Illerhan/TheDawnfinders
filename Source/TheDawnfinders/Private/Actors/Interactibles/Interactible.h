// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractible.h"
#include "Interactible.generated.h"

UCLASS()
class THEDAWNFINDERS_API AInteractibleObjects : public AActor, public IInteractible
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInteractibleObjects();

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Collision")
	UCapsuleComponent* CapsuleCollider;
	
	UFUNCTION(BlueprintCallable)
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(BlueprintCallable)
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	
	UPROPERTY()
	bool bCanBeUsed = true;
	
	UFUNCTION(BlueprintCallable,Blueprintable,Category="Interactibles")
	virtual void Interaction(AAPlayerCharacter* Player);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void BP_OnInteraction(AAPlayerCharacter* Player);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Interact_Implementation(AActor* Interactor) override;
};
