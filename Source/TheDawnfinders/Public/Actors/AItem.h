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
	AItem();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
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

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Initialise(UItemData* Data);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Initialise(UItemData* Data);


	virtual void Interact_Implementation(AActor* Interactor) override;

private:
	UPROPERTY()
	float LevitationTime = 0.f;

	UPROPERTY()
	FVector InitialeLocation;
};
