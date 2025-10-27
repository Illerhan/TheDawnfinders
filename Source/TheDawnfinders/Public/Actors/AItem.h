// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactibles/Interactible.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IInteractible.h"
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

	UFUNCTION(BlueprintCallable)
	void Initialise();

	virtual void Interaction(AAPlayerCharacter* Player) override;
};
