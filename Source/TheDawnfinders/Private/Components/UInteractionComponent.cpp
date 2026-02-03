#include "Components/UInteractionComponent.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Actors/Interactibles/Carriable.h"
#include "Widgets/ULockpickQTEWidget.h"
#include "Widgets/UQTEMashButtonWidget.h"
#include "Widgets/UWorldPlayerWidget.h"
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

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsDoingQTE) {
		AActor* Nearest = GetNearestInteractible();
		if (Nearest != InteractingQTEActor) {
			InteractingQTEActor = nullptr;
			bIsDoingQTE = false;
			if (PlayerCharacter->Execute_GetCurrentPlayerState(PlayerCharacter) != EPlayerState::Trapped)
			PlayerCharacter->Execute_SetCurrentPlayerState(PlayerCharacter, EPlayerState::None);
		}
	}

	if (!bIsHelping || !CurrentHelpedTarget) return;

	HelpTimeRemaining -= DeltaTime;

	// update UI locally
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		IPlayerInterface::Execute_ShowProgress(PlayerCharacter, HelpTimeRemaining);
	}

	if (HelpTimeRemaining <= 0.f)
	{
		bIsHelping = false;
		CompleteHelp();
	}
}

void UInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInteractionComponent, bIsHelping);
	DOREPLIFETIME(UInteractionComponent, HelpTimeRemaining);
	DOREPLIFETIME(UInteractionComponent,CarriedItem)
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

	// If the player is carrying an heavy object
	if (CarriedItem != nullptr) {
		AActor* Nearest = GetNearestInteractible();
		PutInHeavyItem(Nearest);
		return;
	}

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
	if (bIsDoingQTE)
	{
		if (!CurrentQTEWidget->PressButton()) return;
		
		CurrentQTEWidget = nullptr;
		CurrentInteractible = Nearest;
		if (IPlayerInterface::Execute_GetCurrentPlayerState(PlayerCharacter) != EPlayerState::Trapped)
			IPlayerInterface::Execute_RequestStateChange(PlayerCharacter, EPlayerState::None);
		TryInteract(Nearest, PlayerCharacter);
		bIsDoingQTE = false;

		return;
	}

	// Starts QTE if needed
	if (IInteractible::Execute_GetNeededQTE(Nearest) != EQTEType::NoQTE) 
	{
		if (IPlayerInterface::Execute_GetCurrentPlayerState(PlayerCharacter) != EPlayerState::Trapped)
			IPlayerInterface::Execute_RequestStateChange(PlayerCharacter, EPlayerState::Immobilized);

		switch (IInteractible::Execute_GetNeededQTE(Nearest))
		{
		case EQTEType::SmashButton :
			StartMashButtonQTE(Cast<AInteractibleObjects>(Nearest));
			break;

		case EQTEType::Rotative:
			StartRotativeQTE(Cast<AInteractibleObjects>(Nearest));
			break;
		}

		InteractingQTEActor = Nearest;
		bIsDoingQTE = true;
	}
	else  // No QTE 
	{
		CurrentInteractible = Nearest;
		TryInteract(Nearest, PlayerCharacter);
	}
}


void UInteractionComponent::TryInteract(AActor* Interactible, AAPlayerCharacter* Player)
{
	if (!Player || !Player->IsLocallyControlled()) return;
	if (!Interactible) return;

	// Server
	if (!GetOwner()->HasAuthority()) {
		ServerInteract(Interactible, Player);
	}
	// Client
	else {
		ServerInteract_Implementation(Interactible, Player);
	}
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

void UInteractionComponent::OnRep_HelpState()
{
	if (bIsHelping)
	{
		// start showing progress locally
		IPlayerInterface::Execute_ShowProgress(PlayerCharacter, HelpTimeRemaining);
	}
	else
	{
		// hide when canceled or completed
		IPlayerInterface::Execute_HideProgress(PlayerCharacter);
	}
}

void UInteractionComponent::StartExternalQTE(AActor* QTEActor)
{
	if (!QTEActor || !PlayerCharacter) return;

	// Marque le QTE comme actif
	InteractingQTEActor = QTEActor;
	bIsDoingQTE = true;

	// Immobilise le joueur (cohérent avec StartInteract)
	if (IPlayerInterface::Execute_GetCurrentPlayerState(PlayerCharacter) != EPlayerState::Trapped)
		IPlayerInterface::Execute_RequestStateChange(PlayerCharacter, EPlayerState::Immobilized);
}

#pragma endregion


#pragma region QTE


void UInteractionComponent::StartRotativeQTE(AInteractibleObjects* Interactible)
{
	ULockpickQTEWidget* RotativeQTE = (IPlayerInterface::Execute_GetPlayerWidget(GetOwner()))->GetQTERotative();
	RotativeQTE->EnterQTE(Interactible->QTESuccessRangeStart, Interactible->QTESuccessRangeEnd, 400.f, Interactible->QTEStepsCount);
	CurrentQTEWidget = RotativeQTE;
}

void UInteractionComponent::StartMashButtonQTE(AInteractibleObjects* Interactible)
{
	UQTEMashButtonWidget* MashQTE = (IPlayerInterface::Execute_GetPlayerWidget(GetOwner()))->GetQTEMashButton();
	MashQTE->StartQTE(Interactible->MashQTEQuantity, Interactible->MashQTEDecresePerSeconds, true);
	CurrentQTEWidget = MashQTE;
}


#pragma endregion


#pragma region Stop Interaction

void UInteractionComponent::StopInteract()
{
	ServerCancelHelp();

	if (CurrentInteractible)
	{
		IInteractible::Execute_StopInteract(CurrentInteractible, PlayerCharacter);

		ServerStopInteract(CurrentInteractible, PlayerCharacter);
		CurrentInteractible = nullptr;
	}
}

void UInteractionComponent::ServerStopInteract_Implementation(AActor* Interactible, AAPlayerCharacter* Player)
{
	if (!Interactible || !Player) return;
	IInteractible::Execute_StopInteract(Interactible, Player);
}

void UInteractionComponent::CancelInteraction()
{
	AActor* Nearest = GetNearestInteractible();
	if (!Nearest) return;

	if (bIsDoingQTE)
	{
		InteractingQTEActor = nullptr;
		bIsDoingQTE = false;

		if (PlayerCharacter->Execute_GetCurrentPlayerState(PlayerCharacter) != EPlayerState::Trapped)
			PlayerCharacter->Execute_SetCurrentPlayerState(PlayerCharacter, EPlayerState::None);
	}
}

#pragma endregion


#pragma region Carry

void UInteractionComponent::StartCarryHeavyItem(ACarriable* Item)
{
	CarriedItem = Item;
	IPlayerInterface::Execute_RequestStateChange(GetOwner(), EPlayerState::Carrying);
}

void UInteractionComponent::PutInHeavyItem(AActor* Target)
{
	if (!GetOwner()->HasAuthority())
	{
		Server_PutInHeavyItem(Target);
		return;
	}
	Server_PutInHeavyItem_Implementation(Target);
}

void UInteractionComponent::Server_PutInHeavyItem_Implementation(AActor* Target)
{
	if (Target == nullptr || CarriedItem == nullptr) return;
	
	if (!Target->IsA(CarriedItem->GetTargetActorType())) return;
	
	CarriedItem->PutInTargetActor(Target);
	
	CarriedItem = nullptr;
	IPlayerInterface::Execute_RequestStateChange(GetOwner(), EPlayerState::None);
}

void UInteractionComponent::EndCarryHeavyItem()
{
	CarriedItem->StopCarry();
	CarriedItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	CarriedItem = nullptr;
	IPlayerInterface::Execute_RequestStateChange(GetOwner(), EPlayerState::None);
}


#pragma endregion


#pragma region Revive

void UInteractionComponent::TryInteractAlly(AAPlayerCharacter* AllyParam, AAPlayerCharacter* Player)
{
	if (!Player || !Player->IsLocallyControlled()) return;

	if (AllyParam)
		ServerStartHelp(AllyParam);
}


void UInteractionComponent::ServerStartHelp_Implementation(AAPlayerCharacter* AllyParam)
{
	if (!AllyParam) return;
	if (AllyParam->GetCurrentPlayerState_Implementation() != EPlayerState::Fallen
		|| PlayerCharacter->GetCurrentPlayerState_Implementation() == EPlayerState::Fallen
		|| PlayerCharacter->GetCurrentPlayerState_Implementation() == EPlayerState::Dead)
		return;

	CurrentHelpedTarget = AllyParam;

	bIsHelping = true;
	HelpTimeRemaining = HelpDuration;

	// Tell client to show UI
	Client_ShowHelpProgress(HelpDuration);
	
}

void UInteractionComponent::ServerCancelHelp_Implementation()
{
	bIsHelping = false;
	CurrentHelpedTarget = nullptr;

	Client_HideHelpProgress();
}
void UInteractionComponent::Client_ShowHelpProgress_Implementation(float Duration)
{
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		IPlayerInterface::Execute_ShowProgress(PlayerCharacter, Duration);
	}
}

void UInteractionComponent::Client_HideHelpProgress_Implementation()
{
	if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		IPlayerInterface::Execute_HideProgress(PlayerCharacter);
	}
}


void UInteractionComponent::CompleteHelp()
{
	if (!CurrentHelpedTarget) return;

	Client_HideHelpProgress();

	CurrentHelpedTarget->HealthComponent->Server_Revive();
	CurrentHelpedTarget = nullptr;
}

#pragma endregion