#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactibles/Interactible.h"
#include "Components/UPlayerLightComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Litter.generated.h"

class AAPlayerCharacter;

USTRUCT()
struct FPusherData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector InputVector = FVector::ZeroVector;

	UPROPERTY()
	float LastUdateTime = 0.f;
};

UCLASS(NotPlaceable)
class THEDAWNFINDERS_API ALitter : public AInteractibleObjects
{
	GENERATED_BODY()

public:
	ALitter();
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void StopInteract_Implementation(AActor* Interactor) override;
	virtual void Tick(float DeltaTime) override;

	// Server RPCs
	UFUNCTION(Server, Reliable)
	void Server_StartPushing(AAPlayerCharacter* Player);

	UFUNCTION(Server, Reliable)
	void Server_EndPushing(AAPlayerCharacter* Player);

	UFUNCTION(Server, Reliable)
	void Server_UpdateInputs(AAPlayerCharacter* Player, FVector Input);

	FVector GetServerVelocity() const { return ServerVelocity; }

protected:

	virtual void BeginPlay() override;
	// Déplacement
	UPROPERTY(Replicated)
	FVector ServerVelocity = FVector::ZeroVector;

	UPROPERTY()
	TMap<TWeakObjectPtr<AAPlayerCharacter>, FPusherData> ActivePushers;

	// Collision physique
	UPROPERTY(EditAnywhere)
	UBoxComponent* CollisionBox;

	// Points d’attache pour les joueurs
	UPROPERTY(VisibleAnywhere)
	TArray<USceneComponent*> CarryPoints;

	// Slots occupés
	UPROPERTY()
	TArray<TWeakObjectPtr<AAPlayerCharacter>> CarrySlots;

	// Fonctions attach/detach
	void AttachPlayer(AAPlayerCharacter* Player);
	void DetachPlayer(AAPlayerCharacter* Player);

	

public:
	UPROPERTY(EditAnywhere)
	float MaxSpeed = 1000.f;

	UPROPERTY(EditAnywhere)
	int32 MaxUsingPlayer = 4;

	UPROPERTY(EditAnywhere)
	float InputTimeout = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerLightComponent* Light;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRange = 2000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxFuel = 1500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FuelRate = 1.f;
	
};
