#include "Actors/Interactibles/AContainer.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Components/UInventoryComponent.h"


AContainer::AContainer() {

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}

void AContainer::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	TArray<UItemData*> ContainerLoot;

	UDataTable* LootDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/LT_ChestLoot.LT_ChestLoot"));
	if (!LootDataTable)
		UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable"));

	FChestSpawn* SpawnData = LootDataTable->FindRow<FChestSpawn>(DataTableRowName, " ");
	int ItemToSpawnCount = FMath::RandRange(SpawnData->MinItemCount, SpawnData->MaxItemCount);

	for (int i = 0; i < ItemToSpawnCount; i++) 
	{
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

		if (!SpawnedData) {
			i--;
			continue;
		}

		ContainerLoot.Add(SpawnedData);
	}

	Multicast_SetupLoot_Implementation(ContainerLoot);
}

void AContainer::Multicast_SetupLoot_Implementation(const TArray<UItemData*>& Items)
{
	for (UItemData* Item : Items) {
		InventoryComponent->AddNewItem(FItemInfos(Item, Item->Durability));
	}
}



void AContainer::Interact_Implementation(AActor* Interactor)
{
	AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Interactor);
	if (!Player) return;

	Player->Client_OpenInteractionUI(EInteractionUI::ContainerInventory, this);
	IPlayerInterface::Execute_Server_AskOwnershipPermission(Interactor, this, Player->GetController());
}

void AContainer::StopInteract_Implementation(AActor* Interactor)
{

}