#include "Components/UInteractionComponent.h"

#include "AkAudioDevice.h"
#include "AkGameplayStatics.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Actors/Interactibles/Carriable.h"
#include "Components/BoxComponent.h"
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

	if (bIsHelping && CurrentHelpedTarget) {
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

	if (!PlayerCharacter->IsLocallyControlled()) return;
	if (PlayerCharacter->GetCurrentPlayerState_Implementation() == EPlayerState::Fallen || PlayerCharacter->GetCurrentPlayerState_Implementation() == EPlayerState::Dead) {
		if (IsValid(NearestInteractible)) {
			IInteractible::Execute_UnselectInteractible(NearestInteractible, GetOwner());
		}
		NearestInteractible = nullptr;
		return;
	}

	// If the current interacting object is destroyed
	if (bIsDoingQTE) {
		if (!IsValid(InteractingQTEActor)) {
			InteractingQTEActor = nullptr;
			bIsDoingQTE = false;
			bIsInInteraction = false;
			if (PlayerCharacter->Execute_GetCurrentPlayerState(PlayerCharacter) != EPlayerState::Trapped)
				PlayerCharacter->Execute_SetCurrentPlayerState(PlayerCharacter, EPlayerState::None, false);
		}
	}

	// Nearest Interactible management
	PlayersAtRange = GetNearbyPlayers(150.f, true);

	if ((InteractiblesAtRange.Num() > 0 || PlayersAtRange.Num() > 0) && !bIsInInteraction)
	{
		InteractiblesAtRange.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });

		AActor* Nearest = GetNearestInteractible();
		if (!NearestInteractible || NearestInteractible != Nearest) 
		{
			if (IsValid(NearestInteractible)) {
				IInteractible::Execute_UnselectInteractible(NearestInteractible, GetOwner());
			}

			NearestInteractible = Nearest;
			
			if (IsValid(Nearest)) {
				IInteractible::Execute_SelectInteractible(Nearest, GetOwner());
			}
		}
	}
	else if (IsValid(NearestInteractible) && (!bIsInInteraction || CarriedItem)) 
	{
		IInteractible::Execute_UnselectInteractible(NearestInteractible, GetOwner());
		NearestInteractible = nullptr;
	} 
}

void UInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInteractionComponent, bIsHelping);
	DOREPLIFETIME(UInteractionComponent, HelpTimeRemaining);
	DOREPLIFETIME(UInteractionComponent, CarriedItem);
	DOREPLIFETIME(UInteractionComponent, bIsInInteraction);
	DOREPLIFETIME(UInteractionComponent, NearestInteractible);
	DOREPLIFETIME(UInteractionComponent, CurrentInteractible);
}


#pragma region Interactibles Management

void UInteractionComponent::AddInteractible(AActor* Interactible)
{
	if (!InteractiblesAtRange.Contains(Interactible)) {
		InteractiblesAtRange.Add(Interactible);
	}
}

void UInteractionComponent::RemoveInteractible(AActor* Interactible)
{
	InteractiblesAtRange.Remove(Interactible);

	if (Interactible == InteractingQTEActor) {
		CancelInteraction();
	}

	if (Interactible == CurrentInteractible) {
		StopInteract();
	}
}

AActor* UInteractionComponent::GetNearestInteractible()
{
	float BestDist = FLT_MAX;
	AActor* BestActor = nullptr;
	
	for (AActor* Inter : InteractiblesAtRange)
	{
		if (!IsValid(Inter)) continue;
		if (!IInteractible::Execute_GetCanBeUsedBool(Inter, GetOwner())) continue;
		float Dist = FVector::DistSquared(
			Inter->GetActorLocation(),
			PlayerCharacter->GetActorLocation()
		);

		if (Dist < BestDist)
		{
			BestDist = Dist;
			BestActor = Inter;
		}
	}

	for (AActor* Player : PlayersAtRange)
	{
		if (!IsValid(Player)) continue;
		if (IPlayerInterface::Execute_GetCurrentPlayerState(Player) != EPlayerState::Fallen) continue;

		float Dist = FVector::DistSquared(
			Player->GetActorLocation(),
			PlayerCharacter->GetActorLocation()
		);

		if (Dist < BestDist)
		{
			BestDist = Dist;
			BestActor = Player;
		}
	}

	return BestActor;
}

#pragma endregion


#pragma region Start Interaction

void UInteractionComponent::StartInteract()
{
	if (!PlayerCharacter) return;
	if (CurrentAnimInteractible) return;
	if (bIsInInteraction && !bIsDoingQTE) return;

	// If the player is carrying an heavy object
	if (CarriedItem != nullptr) {
		AActor* Nearest = GetNearestInteractible();
		ALitter* Litter = Cast<ALitter>(Nearest);

		if (Litter != nullptr)
		{
			if (Litter->LightTrigger->IsOverlappingActor(PlayerCharacter))
			{
				bIsInInteraction = false;
				PutInHeavyItem(Nearest);
				return;
			}
			return;
		}

		return;
	}

	// We check if there is a player nearby to revive
	
	// -> Revive sound integration
	if (PlayersAtRange.Num() > 0)
	{
		bIsInInteraction = true;
		AAPlayerCharacter* AllyFound = PlayersAtRange[0];

		CurrentHelpedTarget = AllyFound;
		TryInteractAlly(AllyFound, PlayerCharacter);
		return;
	}

	// If is doing QTE
	if (bIsDoingQTE)
	{
		if (!CurrentQTEWidget->PressButton()) return;

		IInteractible::Execute_EndQTE(InteractingQTEActor);
		if (!GetOwner()->HasAuthority()) Server_ChangeQTEState(InteractingQTEActor, false);

		CurrentQTEWidget = nullptr;
		bIsInInteraction = false;
		CurrentInteractible = InteractingQTEActor;
		InteractingQTEActor = nullptr;
		if (IPlayerInterface::Execute_GetCurrentPlayerState(PlayerCharacter) != EPlayerState::Trapped)
			IPlayerInterface::Execute_RequestStateChange(PlayerCharacter, EPlayerState::None, true);

		TryInteract(CurrentInteractible, PlayerCharacter);
		bIsDoingQTE = false;

		return;
	}

	// If no player to revive was found, we interact with the nearest interactible
	
	// -> Interaction Sound generique integration
	AActor* Nearest = GetNearestInteractible();
	if (!Nearest) return;
	if (!IInteractible::Execute_GetCanBeUsed(Nearest, GetOwner())) return;

	IInteractible::Execute_UnselectInteractible(Nearest, GetOwner());

	// Starts QTE if needed
	if (IInteractible::Execute_GetNeededQTE(Nearest) != EQTEType::NoQTE && !IInteractible::Execute_GetQTEDone(Nearest))
	{
		if (IPlayerInterface::Execute_GetCurrentPlayerState(PlayerCharacter) != EPlayerState::Trapped)
			IPlayerInterface::Execute_RequestStateChange(PlayerCharacter, EPlayerState::Immobilized, true);

		switch (IInteractible::Execute_GetNeededQTE(Nearest))
		{
		case EQTEType::SmashButton :
			StartMashButtonQTE(Cast<AInteractibleObjects>(Nearest));
			break;

		case EQTEType::Rotative:
			StartRotativeQTE(Cast<AInteractibleObjects>(Nearest));
			break;
		}

		IInteractible::Execute_StartQTE(Nearest);
		if (!GetOwner()->HasAuthority()) Server_ChangeQTEState(Nearest, true);

		InteractingQTEActor = Nearest;
		bIsDoingQTE = true;
		bIsInInteraction = true;
	}
	else  // No QTE 
	{
		
		TryInteract(Nearest, PlayerCharacter);
	}
}


void UInteractionComponent::TryInteract(AActor* Interactible, AAPlayerCharacter* Player)
{
	if (!Player || !Player->IsLocallyControlled()) return;
	if (!Interactible) return;

	if (bIsInInteraction) return;

	if (Cast<AInteractibleObjects>(Interactible)->GetIsInInteractionStateOnInteract()) {
		bIsInInteraction = true;
		CurrentInteractible = Interactible;
	}

	// Client
	if (!GetOwner()->HasAuthority()) {
		ServerInteract(Interactible, Player);
	}
	// Server
	else {
		bWasCrouched = (Player->CurrentState == EPlayerState::Sneaking);

		IInteractible::Execute_Interact(Interactible, Player);
	}
}


void UInteractionComponent::ServerInteract_Implementation(AActor* Interactible, AAPlayerCharacter* Player)
{
	if (!Interactible || !IInteractible::Execute_GetCanBeUsed(Interactible, Player))
		return;

	if (bIsInInteraction) return;

	if (Cast<AInteractibleObjects>(Interactible)->GetIsInInteractionStateOnInteract()) {
		bIsInInteraction = true;
		CurrentInteractible = Interactible;
	}

	bWasCrouched = (Player->CurrentState == EPlayerState::Sneaking);

	IInteractible::Execute_Interact(Interactible, Player);
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
	bIsInInteraction = true;

	// Immobilise le joueur (cohérent avec StartInteract)
	if (IPlayerInterface::Execute_GetCurrentPlayerState(PlayerCharacter) != EPlayerState::Trapped)
		IPlayerInterface::Execute_RequestStateChange(PlayerCharacter, EPlayerState::Immobilized, true);
}

void UInteractionComponent::PlayInteractSound_Implementation()
{
	if (InteractSoundID)
	{
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (AudioDevice && InteractSoundID != AK_INVALID_PLAYING_ID)
		{
			AudioDevice->StopPlayingID(InteractSoundID);
			InteractSoundID = AK_INVALID_PLAYING_ID; // Reset
		}  
	}
	InteractSoundID = UAkGameplayStatics::PostEvent(InteractSound,GetOwner(),0,FOnAkPostEventCallback(), false);
}

#pragma endregion


#pragma region QTE

void UInteractionComponent::StartRotativeQTE(AInteractibleObjects* Interactible)
{
	ULockpickQTEWidget* RotativeQTE = (IPlayerInterface::Execute_GetPlayerWidget(GetOwner()))->GetQTERotative();
	RotativeQTE->EnterQTE(Interactible->QTESuccessRangeStart, Interactible->QTESuccessRangeEnd, 400.f, Interactible->QTEStepsCount);
	CurrentQTEWidget = RotativeQTE;
	RotativeQTE->SetLinkedInteractible(Interactible);
}

void UInteractionComponent::StartMashButtonQTE(AInteractibleObjects* Interactible)
{
	UQTEMashButtonWidget* MashQTE = (IPlayerInterface::Execute_GetPlayerWidget(GetOwner()))->GetQTEMashButton();
	MashQTE->StartQTE(Interactible->MashQTEQuantity, Interactible->MashQTEDecresePerSeconds, true);
	CurrentQTEWidget = MashQTE;
	MashQTE->SetLinkedInteractible(Interactible);
}

void UInteractionComponent::Server_ChangeQTEState_Implementation(AActor* Interactible, bool bStarted)
{
	if(bStarted) IInteractible::Execute_StartQTE(Interactible);
	else IInteractible::Execute_EndQTE(Interactible);
}

#pragma endregion


void UInteractionComponent::DoInteractAnimation(AActor* Target)
{
	bIsInInteraction = true;
	CurrentAnimInteractible = Target;
}

void UInteractionComponent::EndInteractAnimatiopn()
{
	//bIsInInteraction = false;
	//CurrentInteractible = nullptr;
	CurrentAnimInteractible = nullptr;
	//NearestInteractible = nullptr;
}


#pragma region Stop Interaction

void UInteractionComponent::StopInteract()
{
	ServerCancelHelp();

	if (CurrentInteractible && Cast<AInteractibleObjects>(CurrentInteractible)->GetStopInteractOnRelease())
	{
		if (!GetOwner()->HasAuthority()) {
			ServerStopInteract(CurrentInteractible, PlayerCharacter);
		}
		else {
			ServerStopInteract_Implementation(CurrentInteractible, PlayerCharacter);
		}

		IPlayerInterface::Execute_HideProgress(PlayerCharacter);

		NearestInteractible = nullptr;
		CurrentInteractible = nullptr;
		bIsInInteraction = false;
	}
}

void UInteractionComponent::ServerStopInteract_Implementation(AActor* Interactible, AAPlayerCharacter* Player)
{
	if (!Interactible || !Player) return;

	NearestInteractible = nullptr;
	CurrentInteractible = nullptr;
	bIsInInteraction = false;

	IPlayerInterface::Execute_SetCurrentPlayerState(PlayerCharacter, bWasCrouched ? EPlayerState::Sneaking : EPlayerState::None, true);
	IPlayerInterface::Execute_HideProgress(PlayerCharacter);

	IInteractible::Execute_StopInteract(Interactible, Player);
}

void UInteractionComponent::ClientStopInteract_Implementation(AActor* Interactible, AAPlayerCharacter* Player)
{
	if (!Interactible || !Player) return;

	NearestInteractible = nullptr;
	CurrentInteractible = nullptr;
	bIsInInteraction = false;

	IPlayerInterface::Execute_HideProgress(PlayerCharacter);
}

void UInteractionComponent::CancelInteraction()
{
	if (CurrentAnimInteractible) {
		if (!Cast<AInteractibleObjects>(CurrentAnimInteractible)) return;
		if (Cast<AInteractibleObjects>(CurrentAnimInteractible)->GetAnimationIsActive()) {
			return;
		}
	}

	AActor* Nearest = GetNearestInteractible();
	IPlayerInterface::Execute_HideProgress(PlayerCharacter);

	if (bIsDoingQTE && CurrentQTEWidget)
	{
		CurrentQTEWidget->ExitQTE();

		IInteractible::Execute_EndQTE(InteractingQTEActor);
		if (!GetOwner()->HasAuthority()) Server_ChangeQTEState(InteractingQTEActor, false);

		bIsDoingQTE = false;
		InteractingQTEActor = nullptr;
		bIsInInteraction = false;
		CurrentInteractible = nullptr;
		NearestInteractible = nullptr;

		if (IPlayerInterface::Execute_GetCurrentPlayerState(PlayerCharacter) != EPlayerState::Trapped)
			IPlayerInterface::Execute_RequestStateChange(PlayerCharacter, bWasCrouched ? EPlayerState::Sneaking : EPlayerState::None, true);
	}

	else if (CurrentInteractible && bIsInInteraction) {
		if (!GetOwner()->HasAuthority()) {
			ServerStopInteract(CurrentInteractible, PlayerCharacter);

			//Server_CancelInteraction();
		}
		else {
			IInteractible::Execute_StopInteract(CurrentInteractible, PlayerCharacter);
			ClientStopInteract(CurrentInteractible, PlayerCharacter);

			IPlayerInterface::Execute_RequestStateChange(PlayerCharacter, bWasCrouched ? EPlayerState::Sneaking : EPlayerState::None, true);
		}

		bIsInInteraction = false;
		CurrentInteractible = nullptr;
		NearestInteractible = nullptr;
	}
}

#pragma endregion


#pragma region Carry

void UInteractionComponent::StartCarryHeavyItem(ACarriable* Item)
{
	CarriedItem = Item;
	bIsInInteraction = true;

	IPlayerInterface::Execute_RequestStateChange(GetOwner(), EPlayerState::Carrying, true);
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
	bIsInInteraction = false;

	IPlayerInterface::Execute_RequestStateChange(GetOwner(), EPlayerState::None, true);
}

void UInteractionComponent::EndCarryHeavyItem()
{
	//if (!CarriedItem->bUsed) return;

	CarriedItem->StopCarry();
	CarriedItem->StaticMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	bIsInInteraction = false;
	CarriedItem = nullptr;
	IPlayerInterface::Execute_RequestStateChange(GetOwner(), EPlayerState::None, true);
}


#pragma endregion


#pragma region Revive

TArray<AAPlayerCharacter*> UInteractionComponent::GetNearbyPlayers(float Radius, bool bOnlyFallen) const
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
		ECC_PhysicsBody,
		Sphere
	);

	if (!Hit) return Result;

	for (const FOverlapResult& R : Overlaps)
	{
		AAPlayerCharacter* Other = Cast<AAPlayerCharacter>(R.GetActor());
		if (!Other || Other == PlayerCharacter) continue;
		
		if (bOnlyFallen && Other->GetCurrentPlayerState_Implementation() != EPlayerState::Fallen && Other->GetCurrentPlayerState_Implementation() != EPlayerState::Dead)
			continue;

		if (bOnlyFallen && Other->GetCurrentPlayerState_Implementation() == EPlayerState::Dead) {
			if (!IsValid(PlayerCharacter->InventoryComponent->GetCurrentItem())) continue;
			if (PlayerCharacter->InventoryComponent->GetCurrentItem()->ConsumableEffectType != EConsumableEffectType::Revive) continue;
		}

		Result.Add(Other);
	}

	return Result;
}

void UInteractionComponent::TryInteractAlly(AAPlayerCharacter* AllyParam, AAPlayerCharacter* Player)
{
	if (!Player || !Player->IsLocallyControlled()) return;

	PlayerCharacter->StopMovementForDuration(HelpDuration);

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

	AllyParam->StopMovementForDuration(HelpDuration);
	AllyParam->HealthComponent->StartRevive();

	CurrentHelpedTarget = AllyParam;

	bIsInInteraction = true;
	bIsHelping = true;
	HelpTimeRemaining = HelpDuration;

	// Tell client to show UI
	Client_ShowHelpProgress(HelpDuration);
	
}

void UInteractionComponent::ServerCancelHelp_Implementation()
{
	if (!bIsHelping) return;

	CurrentHelpedTarget->RestartMovement();
	CurrentHelpedTarget->HealthComponent->StopRevive();

	bIsHelping = false;
	bIsInInteraction = false;
	CurrentHelpedTarget = nullptr;

	PlayerCharacter->RestartMovement();

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

		if (!CurrentHelpedTarget) return;
		IInteractible::Execute_UnselectInteractible(CurrentHelpedTarget, GetOwner());
	}
}


void UInteractionComponent::CompleteHelp()
{
	if (!CurrentHelpedTarget) return;

	IInteractible::Execute_UnselectInteractible(CurrentHelpedTarget, GetOwner());
	Client_HideHelpProgress();

	CurrentHelpedTarget->HealthComponent->Server_Revive();
	CurrentHelpedTarget->HealthComponent->StopRevive();
	CurrentHelpedTarget->HealthComponent->Revive();

	bIsHelping = false;
	bIsInInteraction = false;

	CurrentHelpedTarget = nullptr;
	NearestInteractible = nullptr;
}

#pragma endregion