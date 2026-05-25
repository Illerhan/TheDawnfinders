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
	if (!HasAuthority()) return;

	HoldTimer(DeltaTime);
	Super::Tick(DeltaTime);
}

void ALock::Interact_Implementation(AActor* Interactor)
{
	PlayerTemp = Interactor;

	bIsInteracting = true;
	InteractionTimer = 2.f;

	bPlayerIsUsing = true;

	Cast<AAPlayerCharacter>(Interactor)->StopMovementForDuration(InteractionTimer);
}


void ALock::StopInteract_Implementation(AActor* Interactor)
{
	bIsInteracting = false;
	IPlayerInterface::Execute_HideProgress(Interactor);

	Cast<AAPlayerCharacter>(Interactor)->RestartMovement();

	bPlayerIsUsing = false;
}

void ALock::BP_OnInteractionFinished_Implementation()
{
	bIsInteracting = false;

	for (int i = 0; i < LinkedObjects.Num(); i++) {
		LinkedObjects[i]->DoMovement();
	}

	Super::BP_OnInteractionFinished_Implementation();

	Cast<AAPlayerCharacter>(PlayerTemp)->InteractionComponent->CancelInteraction();

	Destroy();
}
