// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactibles/Interactible.h"
#include "Components/UPlayerLightComponent.h"
#include "Litter.generated.h"
USTRUCT()

struct FPusherData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector InputVector = FVector::ZeroVector;

	UPROPERTY()
	float LastUdateTime = 0.f;
};

UCLASS()
class THEDAWNFINDERS_API ALitter : public AInteractibleObjects
{
	GENERATED_BODY()

public:
	ALitter();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void StopInteract_Implementation(AActor* Interactor) override;

	UFUNCTION(Server, Reliable)
	void Server_StartPushing(AAPlayerCharacter* Player);

	UFUNCTION(Server, Reliable)
	void Server_EndPushing(AAPlayerCharacter* Player);

	UFUNCTION(Server, Reliable)
	void Server_UpdateInputs(AAPlayerCharacter* Player, FVector Input);

	FVector GetServerVelocity() const { return ServerVelocity; }

protected:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(Replicated)
	FVector ServerVelocity = FVector::ZeroVector;

	UPROPERTY()
	TMap<TWeakObjectPtr<AAPlayerCharacter>, FPusherData> ActivePushers;

	void AttachPlayer(AAPlayerCharacter* Player);
	void DetachPlayer(AAPlayerCharacter* Player);

public:
	UPROPERTY(EditAnywhere)
	float MaxSpeed = 1000.f;

	UPROPERTY(EditAnywhere)
	int32 MaxUsingPlayer = 1;

	UPROPERTY(EditAnywhere)
	float InputTimeout = 0.5f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UPlayerLightComponent* Light;
};
