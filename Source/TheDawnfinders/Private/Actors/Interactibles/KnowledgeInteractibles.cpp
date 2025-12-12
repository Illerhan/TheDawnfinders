// Fill out your copyright notice in the Description page of Project Settings.

#include "KnowledgeInteractibles.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Interfaces/IPlayer.h"


// Sets default values
AKnowledgeInteractibles::AKnowledgeInteractibles()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AKnowledgeInteractibles::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AKnowledgeInteractibles::Tick(float DeltaTime)
{
	if (!bIsInteracting) return;

	HoldTimer(DeltaTime);
	
	Super::Tick(DeltaTime);
}

void AKnowledgeInteractibles::Interact_Implementation(AActor* Interactor)
{
	if (!bCanBeUsed) return;

	PlayerTemp = Interactor;
	InteractionTimer = InteractionDuration;
	bIsInteracting = true;
}

void AKnowledgeInteractibles::StopInteract_Implementation(AActor* Interactor)
{
	IPlayerInterface::Execute_HideProgress(Interactor);
	bIsInteracting = false;
}

void AKnowledgeInteractibles::BP_OnInteractionFinished_Implementation()
{
	Cast<AAPlayerCharacter>(PlayerTemp)->InventoryComponent->Server_AddKnowledge_Implementation(KnowledgeAmount);
	bCanBeUsed = false;
	StopInteract_Implementation(PlayerTemp);
}

