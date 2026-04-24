#include "Actors/Interactibles/AContainer.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Widgets/UWorldInteractibleWidget.h"
#include "Components/UInventoryComponent.h"


AContainer::AContainer() {

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
}

void AContainer::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	GetWorldTimerManager().SetTimer(DelayStartHandle, this, &AContainer::SetupLoot, 0.5f, false);
}

void AContainer::SetupLoot()
{
	TArray<UItemData*> ContainerLoot;

	UDataTable* LootDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/LT_ChestLoot.LT_ChestLoot"));
	if (!LootDataTable)
		UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable"));

	FChestSpawn* SpawnData = LootDataTable->FindRow<FChestSpawn>(DataTableRowName, " ");
	if (!SpawnData) return;

	bAlreadySpawnedIndexes.Init(false, (SpawnData->SpawnableItems.Num()));

	int ItemToSpawnCount = FMath::RandRange(SpawnData->MinItemCount, SpawnData->MaxItemCount);

	for (int i = 0; i < ItemToSpawnCount; i++)
	{
		int PickedPercent = FMath::RandRange(0.f, 100.f);
		int CurrentPercent = 0;
		UItemData* SpawnedData = nullptr;
		int SpawnedIndex = 0;

		for (int j = 0; j < SpawnData->SpawnableItems.Num(); j++) {
			CurrentPercent += SpawnData->SpawnableItems[j].SpawnPercent;

			if (PickedPercent < CurrentPercent) {

				if (bAlreadySpawnedIndexes[j] && SpawnData->SpawnableItems[j].bSpawnOnlyOnce) {
					break;
				}

				SpawnedData = SpawnData->SpawnableItems[j].ItemData;
				SpawnedIndex = j;
				break;
			}
		}

		if (!SpawnedData) {
			i--;
			continue;
		}

		bAlreadySpawnedIndexes[SpawnedIndex] = true;

		int IterationCount = FMath::RandRange(SpawnData->SpawnableItems[SpawnedIndex].MinSpawnCount, SpawnData->SpawnableItems[SpawnedIndex].MaxSpawnCount);
		for (int j = 0; j < IterationCount; j++) {
			ContainerLoot.Add(SpawnedData);
		}
	}

	for (UItemData* Item : ContainerLoot) {
		InventoryComponent->AddNewItem(FItemInfos(Item, Item->Durability), 1);
	}
}

void AContainer::Multicast_SetupLoot_Implementation(const TArray<UItemData*>& Items)
{
	for (UItemData* Item : Items) {
		InventoryComponent->AddNewItem(FItemInfos(Item, Item->Durability));
	}
}

bool AContainer::GetCanBeUsed_Implementation(AActor* Interactor)
{
	if (bHasInteractAnim && CurrentInteractActor && !DidInteractionAnim)
		return false;

	return Super::GetCanBeUsed_Implementation(Interactor);
}

void AContainer::Interact_Implementation(AActor* Interactor)
{
	if (bPlayerIsUsing) {
		return;
	}

	bQTEDone = true;

	// We need to go to the lever first 
	if (bHasInteractAnim && !CurrentInteractActor && !DidInteractionAnim) {
		CurrentInteractActor = Interactor;
		AAPlayerCharacter* PlayerToMove = Cast<AAPlayerCharacter>(Interactor);
		DoPlayerAutoMove(PlayerToMove);
		UE_LOG(LogTemp, Error, TEXT("Start Interact With container"));
		return;
	}
	else if (bHasInteractAnim && CurrentInteractActor != Interactor && !DidInteractionAnim) {
		return;
	}

	bPlayerIsUsing = true;
	DidInteractionAnim = true;

	if (!bIsOpened && OpenedMesh) {
		Multicast_ChangeStaticMesh();
	}

	Player = Cast<AAPlayerCharacter>(Interactor);
	if (!Player) return;

	Player->Client_OpenInteractionUI(EInteractionUI::ContainerInventory, this);
	IPlayerInterface::Execute_Server_AskOwnershipPermission(Interactor, this, Player->GetController());
}

void AContainer::StopInteract_Implementation(AActor* Interactor)
{
	Player = Cast<AAPlayerCharacter>(Interactor);
	if (!Player) return;

	bPlayerIsUsing = false;

	CloseContainerInventory();
}

void AContainer::DoInteractionAnim_Implementation(AActor* Interactor)
{

}

void AContainer::Multicast_ChangeStaticMesh_Implementation()
{
	bIsOpened = true;
	StaticMesh->SetStaticMesh(OpenedMesh);
}


void AContainer::CloseContainerInventory()
{
	if (HasAuthority()) {
		bPlayerIsUsing = false;

		if (InventoryComponent->GetIsEmpty()) {
			DisableInterestPointVFX();
		}

		Player->Client_CloseInteractionUI(EInteractionUI::ContainerInventory, this);
	}
	else {
		Server_CloseContainerInventory();
	}
}

void AContainer::Server_CloseContainerInventory_Implementation()
{
	bPlayerIsUsing = false;

	if (InventoryComponent->GetIsEmpty()) {
		DisableInterestPointVFX();
	}

	Player->Client_CloseInteractionUI(EInteractionUI::ContainerInventory, this);
}

void AContainer::DisableInterestPointVFX_Implementation()
{
	InterestPointVFXComponent->Deactivate();
}

void AContainer::DoPlayerAutoMove_Implementation(AAPlayerCharacter* PlayerToMove)
{
}

