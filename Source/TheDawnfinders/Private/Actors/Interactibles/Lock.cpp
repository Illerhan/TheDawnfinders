// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Interactibles/Lock.h"
#include "Actors/MovableObjects/MovableObjects.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Interfaces/IPlayer.h"
#include "Widgets/UWorldInteractibleWidget.h"
#include "Kismet/GameplayStatics.h"


ALock::ALock()
{
	
}

void ALock::Tick(float DeltaTime)
{
	if (!bIsInteracting) return;

	HoldTimer(DeltaTime);
	Super::Tick(DeltaTime);
}

void ALock::Interact_Implementation(AActor* Interactor)
{
	if (!bCanBeUsed) return;
	if (IPlayerInterface::Execute_GetEquippedItem(Interactor) == nullptr || 
		IPlayerInterface::Execute_GetEquippedItem(Interactor) != NeededKey ) {
		InteractibleWidget->DisplayErrorText("You need a " + NeededKey->ItemName);
		return;
	}

	PlayerTemp = Interactor;

	bIsInteracting = true;
	InteractionTimer = 2.f;
}


void ALock::StopInteract_Implementation(AActor* Interactor)
{
	bIsInteracting = false;
	IPlayerInterface::Execute_HideProgress(Interactor);
}

void ALock::BP_OnInteractionFinished_Implementation()
{
	bIsInteracting = false;

	for (int i = 0; i < LinkedObjects.Num(); i++) {
		LinkedObjects[i]->DoMovement();
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			if (AAPlayerCharacter* PlayerCharacter = Cast<AAPlayerCharacter>(Pawn))
			{
				PlayerCharacter->InventoryComponent->RemoveCurrentItem();
			}
		}
	}

	Destroy();
}
