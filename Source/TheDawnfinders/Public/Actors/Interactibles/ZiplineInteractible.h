#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactibles/Interactible.h"
#include "ZiplineInteractible.generated.h"

class AAPlayerCharacter;

UCLASS()
class THEDAWNFINDERS_API AZiplineInteractible : public AInteractibleObjects
{
	GENERATED_BODY()

public:
	AZiplineInteractible();

	virtual void Tick(float DeltaTime) override;

	// --- ZIPLINE SETUP ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zipline")
	float LinkRange = 2000.f; // distance max pour se relier

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zipline")
	FVector TeleportOffset = FVector(0, 0, 80);

	UPROPERTY(Replicated)
	AZiplineInteractible* LinkedZipline = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Zipline")
	void TryLinkToNearbyZipline();

	// --- INTERACTION ---
	virtual void Interact_Implementation(AActor* Interactor) override;

protected:
	virtual void BeginPlay() override;

	// --- TRAVEL ---
	UPROPERTY()
	bool bIsTravelling = false;

	UPROPERTY()
	ACharacter* TravellingPlayer = nullptr;

	UPROPERTY()
	FVector StartLocation;

	UPROPERTY()
	FVector EndLocation;

	UPROPERTY()
	float TravelTimer = 0.f;

	UPROPERTY(EditAnywhere, Category = "Zipline")
	float TravelDuration = 1.5f;

	void StartTravel(AActor* Player);
	void UpdateTravel(float DeltaTime);
	void EndTravel();

	// RPC
	UFUNCTION(Server, Reliable)
	void ServerStartTravel(AActor* Player);

	UFUNCTION(Client, Reliable)
	void ClientPlayTravelEffects();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartTravel(AActor* Player, FVector Start, FVector End);
};