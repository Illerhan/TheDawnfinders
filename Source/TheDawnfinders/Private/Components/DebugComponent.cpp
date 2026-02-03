// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugComponent.h"

#include "Actors/AItem.h"


// Sets default values for this component's properties
UDebugComponent::UDebugComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UDebugComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UDebugComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UDebugComponent::Server_SpawnDebugItem_Implementation(UItemData* ItemData, FVector Location, FRotator Rotation)
{
	if (!ItemData || !GetWorld()) return;

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	// L'owner du composant (le PlayerController ou Pawn) devient l'Instigator
	SpawnInfo.Instigator = Cast<APawn>(GetOwner());

	AItem* DroppedItem = GetWorld()->SpawnActor<AItem>(
		ItemData->ItemClass,
		Location,
		Rotation,
		SpawnInfo
	);

	if (DroppedItem)
	{
		DroppedItem->ItemData = ItemData;
		DroppedItem->Initialise(FItemInfos(ItemData, ItemData->Durability));
		
		// Setup Mesh, Physique etc...
		if (DroppedItem->ItemMesh && ItemData->ItemMesh)
		{
			DroppedItem->ItemMesh->SetStaticMesh(ItemData->ItemMesh);
			DroppedItem->ItemMesh->SetSimulatePhysics(false);
			DroppedItem->ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		DroppedItem->bShouldLevitate = true;
		
		UE_LOG(LogTemp, Log, TEXT("[DebugComponent] Spawned: %s"), *ItemData->GetName());
	}
}

bool UDebugComponent::Server_SpawnDebugItem_Validate(UItemData* ItemData, FVector Location, FRotator Rotation)
{
	return true;
}

bool UDebugComponent::Server_TravelToMap_Validate(const FString& MapName)
{
	return true; // Tu peux ajouter des checks de sécurité ici
}

void UDebugComponent::Server_TravelToMap_Implementation(const FString& MapName)
{
	UWorld* World = GetWorld();
	if (World)
	{
		// On s'assure que l'URL contient bien les options nécessaires
		FString TravelURL = MapName;
        
		// Si c'est pour du multijoueur, on force souvent le mode Listen
		if (!TravelURL.Contains("?listen"))
		{
			TravelURL += "?listen";
		}

		UE_LOG(LogTemp, Warning, TEXT("[SERVER] ServerTravel requested to: %s"), *TravelURL);

		// bAbsolute = true pour reset complètement l'état du monde (nettoie la mémoire)
		// bShouldSkipGameNotify = false par défaut
		World->ServerTravel(TravelURL, true, false); 
	}
}
