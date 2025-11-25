#include "Components/UInteractionComponent.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Actors/Interactibles/Interactible.h"
#include "Interfaces/IInteractible.h"
#include "Components/UHealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


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

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsReviving || !CurrentReviveTarget) return;

	ReviveTimeRemaining -= DeltaTime;

	// update UI locally
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		IPlayerInterface::Execute_ShowProgress(PlayerCharacter, ReviveTimeRemaining);
	}

	if (ReviveTimeRemaining <= 0.f)
	{
		bIsReviving = false;
		CompleteRevive();
	}
}

void UInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInteractionComponent, bIsReviving);
	DOREPLIFETIME(UInteractionComponent, ReviveTimeRemaining);
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

void UInteractionComponent::OnRep_ReviveState()
{
	if (bIsReviving)
	{
		// start showing progress locally
		IPlayerInterface::Execute_ShowProgress(PlayerCharacter, ReviveTimeRemaining);
	}
	else
	{
		// hide when canceled or completed
		IPlayerInterface::Execute_HideProgress(PlayerCharacter);
	}
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

	bIsReviving = true;
	ReviveTimeRemaining = ReviveDuration;

	// Tell client to show UI
	Client_ShowReviveProgress(ReviveDuration);
	
}

void UInteractionComponent::ServerCancelRevive_Implementation()
{
	bIsReviving = false;
	CurrentReviveTarget = nullptr;

	Client_HideReviveProgress();
}
void UInteractionComponent::Client_ShowReviveProgress_Implementation(float Duration)
{
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		IPlayerInterface::Execute_ShowProgress(PlayerCharacter, Duration);
	}
}

void UInteractionComponent::Client_HideReviveProgress_Implementation()
{
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		IPlayerInterface::Execute_HideProgress(PlayerCharacter);
	}
}


void UInteractionComponent::CompleteRevive()
{
	if (!CurrentReviveTarget) return;

	Client_HideReviveProgress();

	CurrentReviveTarget->HealthComponent->Server_Revive();
	CurrentReviveTarget = nullptr;
}

#pragma endregion