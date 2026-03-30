#include "Actors/Interactibles/AChest.h"
#include "Interfaces/IPlayer.h"
#include "Actors/AItem.h"
#include "CustomStructs.h"
#include "GameFramework/GameplaySoundHelper.h"


AChest::AChest()
{
	
}

void AChest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsInteracting) return;

	HoldTimer(DeltaTime);
}


void AChest::Interact_Implementation(AActor* Interactor)
{
	if (!bCanBeUsed) return;

	bQTEDone = true;

	PlayerTemp = Interactor;
	InteractionTimer = InteractionDuration;
	bIsInteracting = true;
}

void AChest::StopInteract_Implementation(AActor* Interactor)
{
	IPlayerInterface::Execute_HideProgress(Interactor);
	bIsInteracting = false;
}

void AChest::BP_OnInteractionFinished_Implementation()
{
	if (!HasAuthority()) {
		SpawnLoot();
	}
	else {
		SpawnLoot_Implementation();
	}
}

// Did on the server only
void AChest::SpawnLoot_Implementation()
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

		if (!SpawnedData) continue;

		if (SpawnedData->bIsRangedWeapon) {

			UDataTable* WeaponDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_Weapons.DT_Weapons"));
			if (!WeaponDataTable)
				UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable"));

			FWeaponInfos CurrentWeaponData = *WeaponDataTable->FindRow<FWeaponInfos>(SpawnedData->WeaponDataTableRow, " ");

			AItem* NewItem = GetWorld()->SpawnActor<AItem>(LootActor, ItemLocation, FRotator());
			NewItem->Initialise(FItemInfos(SpawnedData, SpawnedData->Durability, CurrentWeaponData.MagazineSize));
		}
		else {
			AItem* NewItem = GetWorld()->SpawnActor<AItem>(LootActor, ItemLocation, FRotator());
			NewItem->Initialise(FItemInfos(SpawnedData, SpawnedData->Durability));
		}
	}

	// Trapped chest check
	if (FMath::RandRange(0, 100) < TrapProba) {
		DoTrapEffect();
	}

	Destroy();
}

void AChest::DoTrapEffect()
{
	ACurseZone* CurseZone = GetWorld()->SpawnActor<ACurseZone>(TrapCurseZone, GetActorLocation(), FRotator());
	CurseZone->Initialise(TrapRadius, TrapDelay, TrapDuration);
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

		FVector TraceStart = FinalPos + FVector(0, 0, 50);
		FVector TraceEnd = FinalPos - FVector(0, 0, 100);
		
		FHitResult Hit;
		FHitResult Hit2;
		FCollisionQueryParams Params;

		if (!GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params)) continue;
		if (GetWorld()->LineTraceSingleByChannel(Hit2, GetActorLocation() + FVector(0.f, 0.f, 60.f), Hit.ImpactPoint, ECC_Visibility, Params)) {
			continue;
		}


		return Hit.ImpactPoint + FVector(0.f, 0.f, 75.f);
	}

	return FVector();
}
