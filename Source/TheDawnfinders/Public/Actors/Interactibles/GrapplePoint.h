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
	AGrapplePoint();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;


public :
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	UStaticMeshComponent* GrappleMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple")
	UItemData* RequiredGrappleItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grapple",meta = (MakeEditWidget = true))
	FVector TeleportOffset = FVector(0.f, 0.f, -50.f);

	UPROPERTY()
	bool bIsGrappling = false;
	
	UPROPERTY()
	AActor* GrapplingPlayer = nullptr;
	
	UPROPERTY()
	FVector GrappleStartLocation;

	UPROPERTY()
	FVector GrappleTargetLocation;
	
	UPROPERTY()
	float GrappleElapsedTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "Grapple|SmoothMove")
	float GrappleMoveDuration = 0.75f; 

	void StartSmoothGrapple(AActor* Player);

	void UpdateSmoothGrapple(float DeltaTime);

	virtual void Interact_Implementation(AActor* Interactor) override;


protected:
	UFUNCTION(BlueprintCallable, Category = "Grapple")
	bool HasLineOfSight(AActor* Player) const;

	UFUNCTION(Server, Reliable)
	void ServerTeleportPlayer(AActor* Player);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Grapple")
	void OnGrappleUsed(AActor* Player);

	UFUNCTION(BlueprintImplementableEvent, Category = "Grapple")
	void OnGrappleFailed(AActor* Player, const FString& Reason);

	UFUNCTION(Client, Reliable)
	void ClientPlayGrappleEffects();
};
