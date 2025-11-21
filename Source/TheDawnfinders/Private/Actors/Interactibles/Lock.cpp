// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Interactibles/Lock.h"
#include "Actors/MovableObjects/MovableObjects.h"
#include "Actors/Player/APlayerCharacter.h"
#include "DataAssets/ItemData.h"
#include "Interfaces/IPlayer.h"
#include "Kismet/GameplayStatics.h"


ALock::ALock()
{
	
}

void ALock::Tick(float DeltaTime)
{
	if (!bIsInteracting) return;

	PlayerTemp->ShowProgress_Implementation(InteractionTimer);
	InteractionTimer = InteractionTimer - GetWorld()->GetDeltaSeconds();

	if (InteractionTimer <= 0) {
		PlayerTemp->HideProgress_Implementation();
		Unlock();
	}
}


void ALock::Interaction(AAPlayerCharacter* Player)
{
	if (!bCanBeUsed) return;
	if (Player->GetEquippedItem_Implementation() == nullptr) return;
	if (Player->GetEquippedItem_Implementation() != NeededKey) return;

	PlayerTemp = Player;

	bIsInteracting = true;
	InteractionTimer = 2.f;
}

void ALock::StopInteraction(AAPlayerCharacter* Player)
{
	bIsInteracting = false;
	Player->HideProgress_Implementation();
}


void ALock::Unlock()
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
