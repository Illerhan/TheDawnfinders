// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactibles/Interactible.h"
#include "GameFramework/Actor.h"
#include "DataAssets/ItemData.h"
#include "AItem.generated.h"

UCLASS()
class THEDAWNFINDERS_API AItem : public AInteractibleObjects
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();
	virtual void OnConstruction(const FTransform& Transform) override;

	//virtual void Interact_Implementation(AActor* Interactor) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere,meta=(ExposeOnSpawn="true"))
	UItemData* ItemData;

	UPROPERTY(Blueprintable,BlueprintReadWrite,EditAnywhere)
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(EditAnywhere)
	UStaticMesh* ItemMeshAsset;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Levitation")
	bool bShouldLevitate = true;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Levitation")
	float LevitationAmplitude = 20.f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Levitation")
	float LevitationSpeed = 2.f;

	UFUNCTION(BlueprintCallable)
	void Initialise();

	virtual void Interaction(AAPlayerCharacter* Player) override;

private:

	float LevitationTime = 0.f;
	FVector InitialeLocation;
};
