// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactibles/Interactible.h"
#include "DataAssets/ItemData.h"
#include "GrapplePoint.generated.h"


UCLASS()
class THEDAWNFINDERS_API AGrapplePoint : public AInteractibleObjects
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGrapplePoint();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	UStaticMeshComponent* GrappleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	UItemData* RequiredGrappleItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	FVector TeleportOffset = FVector(0.f, 0.f, -50.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	float MaxGrappleDistance = 1500.f;

	UPROPERTY()
	bool bIsGrappling = false;
	
	UPROPERTY()
	AAPlayerCharacter* GrapplingPlayer = nullptr;
	
	UPROPERTY()
	FVector GrappleStartLocation;

	UPROPERTY()
	FVector GrappleTargetLocation;
	
	UPROPERTY()
	float GrappleElapsedTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "Grapple|SmoothMove")
	float GrappleMoveDuration = 0.75f; 

	void StartSmoothGrapple(AAPlayerCharacter* Player);

	void UpdateSmoothGrapple(float DeltaTime);

	virtual void Interaction(AAPlayerCharacter* Player) override;

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable, Category = "Grapple")
	bool HasGrappleInInventory(AAPlayerCharacter* Player) const;

	UFUNCTION(BlueprintCallable, Category = "Grapple")
	bool HasLineOfSight(AAPlayerCharacter* Player) const;

	UFUNCTION(Server, Reliable)
	void ServerTeleportPlayer(AAPlayerCharacter* Player);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Grapple")
	void OnGrappleUsed(AAPlayerCharacter* Player);

	UFUNCTION(BlueprintImplementableEvent, Category = "Grapple")
	void OnGrappleFailed(AAPlayerCharacter* Player, const FString& Reason);

	UFUNCTION(Client, Reliable)
	void ClientPlayGrappleEffects();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
