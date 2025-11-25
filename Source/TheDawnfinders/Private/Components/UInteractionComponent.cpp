#include "Components/UInteractionComponent.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Actors/Interactibles/Interactible.h"
#include "Interfaces/IInteractible.h"
#include "Components/UHealthComponent.h"


UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<AAPlayerCharacter>(GetOwner());
}


#pragma region Interactibles Management

void UInteractionComponent::AddInteractible(AActor* Interactible)
{
	InteractiblesAtRange.Add(Interactible);
}

void UInteractionComponent::RemoveInteractible(AActor* Interactible)
{
	InteractiblesAtRange.Remove(Interactible);
}

AActor* UInteractionComponent::GetNearestInteractible()
{
	float BestDist = FLT_MAX;
	AActor* BestActor = nullptr;

	for (AActor* Inter : InteractiblesAtRange)
	{
		float Dist = FVector::Dist(
			Inter->GetActorLocation(),
			PlayerCharacter->GetActorLocation()
		);

		if (Dist < BestDist)
		{
			BestDist = Dist;
			BestActor = Inter;
		}
	}

	return BestActor;
}

#pragma endregion


#pragma region Start Interaction

void UInteractionComponent::StartInteract()
{
	if (!PlayerCharacter)
		return;

	// We check if there is a player nearby to revive 
	TArray<AAPlayerCharacter*> Fallen = GetNearbyPlayers(150.f, true);

	if (Fallen.Num() > 0)
	{
		AAPlayerCharacter* AllyFound = Fallen[0];
		TryInteractAlly(AllyFound, PlayerCharacter);
		return;
	}

	// If no player to revive was found, we interact with the nearest interactible
	AActor* Nearest = GetNearestInteractible();
	if (!Nearest) return;

	// If is doing QTE
	if (InteractingQTEActor) 
	{
		if (!IInteractible::Execute_ValidateQTE(Nearest)) 
		{
			InteractingQTEActor = nullptr;
			return;
		}

		InteractingQTEActor = nullptr;
		CurrentInteractible = Nearest;

		TryInteract(Nearest, PlayerCharacter);

		return;
	}

	if (IInteractible::Execute_GetQTENeeded(Nearest)) 
	{
		IInteractible::Execute_StartQTE(Nearest);

		InteractingQTEActor = Nearest;
	}
	else 
	{
		CurrentInteractible = Nearest;
		TryInteract(Nearest, PlayerCharacter);
	}
}


void UInteractionComponent::TryInteract(AActor* Interactible, AAPlayerCharacter* Player)
{
	if (!Player || !Player->IsLocallyControlled()) return;
	if (Interactible)
		ServerInteract(Interactible, Player);
}


void UInteractionComponent::ServerInteract_Implementation(AActor* Interactible, AAPlayerCharacter* Player)
{
	if (!Interactible || !IInteractible::Execute_GetCanBeUsed(Interactible))
		return;

	IInteractible::Execute_Interact(Interactible, Player);
}

TArray<AAPlayerCharacter*> UInteractionComponent::GetNearbyPlayers(float Radius, bool bOnlyDead) const
{
	TArray<AAPlayerCharacter*> Result;

	if (!PlayerCharacter) return Result;

	UWorld* World = GetWorld();
	if (!World) return Result;

	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	TArray<FOverlapResult> Overlaps;

	bool Hit = World->OverlapMultiByChannel(
		Overlaps,
		PlayerCharacter->GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		Sphere
	);

	if (!Hit) return Result;

	for (const FOverlapResult& R : Overlaps)
	{
		AAPlayerCharacter* Other = Cast<AAPlayerCharacter>(R.GetActor());
		if (!Other || Other == PlayerCharacter) continue;

		if (bOnlyDead &&
			Other->GetCurrentPlayerState_Implementation() != EPlayerState::Fallen)
			continue;

		Result.Add(Other);
	}

	return Result;
}

#pragma endregion


#pragma region Stop Interaction

void UInteractionComponent::StopInteract()
{
	// Cancel the revive
	ServerCancelRevive();

	// Cancel the interactible interaction
	if (CurrentInteractible)
	{
		ServerStopInteract(CurrentInteractible, PlayerCharacter);
		CurrentInteractible = nullptr;
	}
}

void UInteractionComponent::ServerStopInteract_Implementation(AActor* Interactible, AAPlayerCharacter* Player)
{
	if (!Interactible || !Player) return;
	IInteractible::Execute_StopInteract(Interactible, Player);
}

#pragma endregion


#pragma region Revive

void UInteractionComponent::TryInteractAlly(AAPlayerCharacter* AllyParam, AAPlayerCharacter* Player)
{
	if (!Player || !Player->IsLocallyControlled()) return;

	if (AllyParam)
		ServerStartRevive(AllyParam);
}


void UInteractionComponent::ServerStartRevive_Implementation(AAPlayerCharacter* AllyParam)
{
	if (!AllyParam) return;
	if (AllyParam->GetCurrentPlayerState_Implementation() != EPlayerState::Fallen)
		return;

	CurrentReviveTarget = AllyParam;

	// Timer 2 sec pour relever
	GetWorld()->GetTimerManager().SetTimer(
		ReviveTimer,
		this,
		&UInteractionComponent::CompleteRevive,
		2.0f,
		false
	);
}

void UInteractionComponent::ServerCancelRevive_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(ReviveTimer);
	CurrentReviveTarget = nullptr;
}

void UInteractionComponent::CompleteRevive()
{
	if (CurrentReviveTarget)
	{
		CurrentReviveTarget->SetCurrentPlayerState_Implementation(EPlayerState::None);
		
		if (CurrentReviveTarget->HasAuthority())
		{
			CurrentReviveTarget->HealthComponent->Server_Revive();
			CurrentReviveTarget->SetPlayerSpeed(400.f);
		}
		CurrentReviveTarget = nullptr;
	}
}

#pragma endregion