#include "Actors/Interactibles/AChest.h"
#include "Interfaces/IPlayer.h"
#include "Actors/AItem.h"
#include "CustomStructs.h"
#include "Net/UnrealNetwork.h"


AChest::AChest()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AChest::Tick(float DeltaTime)
{
	if (!bIsInteracting) return;

	IPlayerInterface::Execute_ShowProgress(PlayerTemp, InteractionTimer);
	InteractionTimer = InteractionTimer - DeltaTime;

	if (InteractionTimer <= 0) {
		IPlayerInterface::Execute_HideProgress(PlayerTemp);
		
		SpawnLoot();
	}
}



void AChest::Interact_Implementation(AActor* Interactor)
{
	if (!bCanBeUsed) return;

	PlayerTemp = Interactor;
	InteractionTimer = InteractionDuration;
	bIsInteracting = true;
}

void AChest::StopInteract_Implementation(AActor* Interactor)
{
	IPlayerInterface::Execute_HideProgress(Interactor);
	bIsInteracting = false;
}

void AChest::SpawnLoot()
{
	UDataTable* LootDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/LT_ChestLoot.LT_ChestLoot"));
	if (!LootDataTable)
		UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable"));

	FChestSpawn* SpawnData = LootDataTable->FindRow<FChestSpawn>(DataTableRowName, " ");
	int ItemToSpawnCount = FMath::RandRange(SpawnData->MinItemCount, SpawnData->MaxItemCount);

	for (int i = 0; i < ItemToSpawnCount; i++) {
		FVector ItemLocation = GetPossibleSpawnLocation();

		int PickedPercent = FMath::RandRange(0.f, 100.f);
		int CurrentPercent = 0;
		UItemData* SpawnedData = nullptr;

		for (int j = 0; j < SpawnData->SpawnableItems.Num(); j++) {
			CurrentPercent += SpawnData->SpawnableItems[j].SpawnPercent;

			if (PickedPercent < CurrentPercent) {

				SpawnedData = SpawnData->SpawnableItems[j].ItemData;
				break;
			}
		}

		AItem* NewItem = GetWorld()->SpawnActor<AItem>(LootActor, ItemLocation, FRotator());
		NewItem->Initialise(SpawnedData);
	}

	Destroy();
}

FVector AChest::GetPossibleSpawnLocation()
{
	int antiCrashCount = 0;

	while (antiCrashCount++ < 100) {
		FVector Origin = GetActorLocation();
		FVector RandomOffset = FVector(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f), 0.f);
		RandomOffset.Normalize();
		RandomOffset *= FMath::RandRange(-250.f, 250.f);

		FVector FinalPos = Origin + RandomOffset;

		FVector TraceStart = FinalPos + FVector(0, 0, 500);
		FVector TraceEnd = FinalPos - FVector(0, 0, 1000);

		FHitResult Hit;
		FCollisionQueryParams Params;

		if (!GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params)) continue;

		return Hit.ImpactPoint + FVector(0.f, 0.f, 100.f);
	}

	return FVector();
}
