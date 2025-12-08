// Fill out your copyright notice in the Description page of Project Settings.


#include "Litter.h"

#include "Actors/Player/APlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ALitter::ALitter()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
}

void ALitter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALitter, ServerVelocity);
}

void ALitter::Interact_Implementation(AActor* Interactor)
{
	Super::Interact_Implementation(Interactor);
	Server_StartPushing(Cast<AAPlayerCharacter>(Interactor));
	
	PlayerTemp = Interactor;
}

void ALitter::StopInteract_Implementation(AActor* Interactor)
{
	Super::StopInteract_Implementation(Interactor);
	Server_EndPushing(Cast<AAPlayerCharacter>(Interactor));
}

// Called every frame
void ALitter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority()) return;

	const float Now = GetWorld()->GetTimeSeconds();

	// Nettoyage des pushers inactifs
	TArray<TWeakObjectPtr<AAPlayerCharacter>> RemoveList;
	for (auto& Pair : ActivePushers)
	{
		if (Now - Pair.Value.LastUdateTime > InputTimeout)
			RemoveList.Add(Pair.Key);
	}

	for (auto& P : RemoveList)
		ActivePushers.Remove(P);

	if (ActivePushers.Num() == 0)
	{
		ServerVelocity = FVector::ZeroVector;
		return;
	}

	FVector Total = FVector::ZeroVector;
	for (auto& Pair : ActivePushers)
		Total += Pair.Value.InputVector;

	FVector Dir = Total.GetSafeNormal();

	float SpeedMultiplier = FMath::Clamp(
		(float)ActivePushers.Num() / (float)MaxUsingPlayer,
		0.f, 1.f
	);

	float FinalSpeed = MaxSpeed * SpeedMultiplier;
	FVector Delta = Dir * FinalSpeed * DeltaTime;

	SetActorLocation(GetActorLocation() + Delta, true);
	ServerVelocity = Dir * FinalSpeed;
}


void ALitter::Server_StartPushing_Implementation(AAPlayerCharacter* Player)
{
	if (!HasAuthority() || !Player) return;

	FPusherData& Data = ActivePushers.FindOrAdd(Player);
	Data.InputVector = FVector::ZeroVector;
	Data.LastUdateTime = GetWorld()->GetTimeSeconds();

	AttachPlayer(Player);
	Player->GetCharacterMovement()->DisableMovement();
	
	Player->Server_SetPushingState(this, true);
}

void ALitter::Server_EndPushing_Implementation(AAPlayerCharacter* Player)
{
	if (!HasAuthority() || !Player) return;

	ActivePushers.Remove(Player);
	DetachPlayer(Player);
	Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	Player->Server_SetPushingState(this, false);
}

void ALitter::Server_UpdateInputs_Implementation(AAPlayerCharacter* Player, FVector Input)
{
	if (!HasAuthority() || !Player) return;

	if (FPusherData* Data = ActivePushers.Find(Player))
	{
		Data->InputVector = Input.GetClampedToMaxSize(1.f);
		Data->LastUdateTime = GetWorld()->GetTimeSeconds();
	}
}

void ALitter::AttachPlayer(AAPlayerCharacter* Player)
{
	if (!Player) return;

	Player->AttachToActor(
		this,
		FAttachmentTransformRules::KeepWorldTransform
	);

	Player->bIsCarrying = true;
	Player->CurrentPushedObject = this;
}

void ALitter::DetachPlayer(AAPlayerCharacter* Player)
{
	if (!Player) return;

	Player->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	Player->bIsCarrying = false;
	Player->CurrentPushedObject = nullptr;
}

