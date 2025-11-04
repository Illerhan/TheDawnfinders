// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UInventoryComponent.h"

#include "Actors/AItem.h"
#include "Net/UnrealNetwork.h"



UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}


void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}



#pragma region Replication

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, InventorySlots);

	DOREPLIFETIME(UInventoryComponent, CurrentSlotIndex);
}


void UInventoryComponent::OnRep_InventorySlots()
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] InventorySlots called by %s"),
				   GetOwner()->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
					   GetOwner()->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT")
				   );

	VerifyCurrentOverloadCount();

	OnInventoryChanging.Broadcast(InventorySlots, CurrentSlotIndex);
}


void UInventoryComponent::OnRep_CurrentSlotIndex()
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] CurrentSlotIndex called by %s"),
				   GetOwner()->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"),
					   GetOwner()->HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT")
				   );
	OnInventoryChanging.Broadcast(InventorySlots, CurrentSlotIndex);
}


void UInventoryComponent::BroadcastInventoryChange()
{
	OnInventoryChanging.Broadcast(InventorySlots,CurrentSlotIndex);
}

#pragma endregion


#pragma region Add / Remove Item

void UInventoryComponent::AddNewItem(UItemData* NewItem)
{
	if (!GetOwner()->HasAuthority())
	{
		ServerAddNewItem(NewItem);
		return;
	}

	ServerAddNewItem_Implementation(NewItem);
}


void UInventoryComponent::ServerAddNewItem_Implementation(UItemData* NewItem)
{
	if (!NewItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddNewItem: NewItem is null"));
		return;
	}

	TArray<FInventorySlot> NewSlots = InventorySlots;;

	bool bItemAdded = false;

	for (int32 i = 0; i < InventorySlots.Num(); i++)
	{
		FInventorySlot& Slot = NewSlots[i];

		if (!Slot.ItemData)
		{
			Slot.ItemData = NewItem;
			Slot.Quantity = 1;
			bItemAdded = true;
			UE_LOG(LogTemp, Log, TEXT("Item added to empty slot %d"), i);
			break;
		}
		else if (Slot.ItemData == NewItem && Slot.Quantity < NewItem->MaxStackingCapacity)
		{
			Slot.Quantity++;
			bItemAdded = true;
			UE_LOG(LogTemp, Log, TEXT("Item quantity increased in slot %d (new quantity: %d)"), i, Slot.Quantity);
			break;
		}
	}

	if (!bItemAdded)
	{
		FInventorySlot NewSlot;
		NewSlot.ItemData = NewItem;
		NewSlot.Quantity = 1;
		NewSlots.Add(NewSlot);
		UE_LOG(LogTemp, Log, TEXT("New slot created for item (total slots: %d)"), NewSlots.Num());
	}
	InventorySlots = NewSlots;

	SortInventory();
}


void UInventoryComponent::RemoveCurrentItem()
{
	if (!GetOwner()->HasAuthority())
	{
		ServerRemoveCurrentItem();
		return;
	}

	ServerRemoveCurrentItem_Implementation();
}


void UInventoryComponent::ServerRemoveCurrentItem_Implementation()
{
	TArray<FInventorySlot> NewSlots = InventorySlots;;

	FInventorySlot& CurrentSlot = NewSlots[CurrentSlotIndex];
	CurrentSlot.Quantity--;
	if (CurrentSlot.Quantity <= 0) CurrentSlot.ItemData = nullptr;

	InventorySlots = NewSlots;

	SortInventory();
}

#pragma endregion


#pragma region Throw Item

void UInventoryComponent::Throw()
{
	if (!GetOwner()->HasAuthority())
	{
		ServerThrow();
		return;
	}
	ServerThrow_Implementation();
}

void UInventoryComponent::ServerThrow_Implementation()
{
	if (!InventorySlots.IsValidIndex(CurrentSlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Throw: Invalid CurrentSlotIndex %d"), CurrentSlotIndex);
		return;
	}

	FInventorySlot& CurrentSlot = InventorySlots[CurrentSlotIndex];

	if (!CurrentSlot.ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Throw: No item in current slot"));
		return;
	}
	if (GetWorld() && GetOwner() && CurrentSlot.ItemData->ItemClass)
	{
		FVector SpawnLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 100.f;
		FRotator SpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Instigator = Cast<APawn>(GetOwner());

		AItem* DroppedItem = GetWorld()->SpawnActor<AItem>(
			CurrentSlot.ItemData->ItemClass,
			SpawnLocation,
			SpawnRotation,
			SpawnInfo
		);

		if (DroppedItem)
		{
			DroppedItem->ItemData = CurrentSlot.ItemData;

			if (DroppedItem->ItemMesh && CurrentSlot.ItemData->ItemMesh)
			{
				DroppedItem->ItemMesh->SetStaticMesh(CurrentSlot.ItemData->ItemMesh);

				DroppedItem->ItemMesh->SetSimulatePhysics(false);
				DroppedItem->ItemMesh->SetEnableGravity(false);
				DroppedItem->ItemMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

				UE_LOG(LogTemp, Log, TEXT("Mesh set for dropped item: %s"), *CurrentSlot.ItemData->ItemMesh->GetName());
			}

			DroppedItem->bShouldLevitate = true;

			if (UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(DroppedItem->GetRootComponent()))
			{
				if (RootComponent->IsSimulatingPhysics())
				{
					FVector ThrowDirection = GetOwner()->GetActorForwardVector() + FVector(0, 0, 0.25f);
					RootComponent->AddImpulse(ThrowDirection * 500.f, NAME_None, true);
				}
				UE_LOG(LogTemp, Log, TEXT("Item thrown from slot %d: %s"), CurrentSlotIndex, *CurrentSlot.ItemData->ItemName);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to spawn item from slot %d"), CurrentSlotIndex);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot spawn item: Missing World, Owner, or ItemClass"));
		}

		RemoveCurrentItem();
	}
}

#pragma endregion


#pragma region Others

FInventorySlot UInventoryComponent::GetCurrentSlot()
{
	if (InventorySlots.IsValidIndex(CurrentSlotIndex))
	{
		return InventorySlots[CurrentSlotIndex];

	}
	return FInventorySlot();
}


void UInventoryComponent::VerifyCurrentOverloadCount()
{
	int Current = GetCurrentOverloadCount();

	if (Current == PreviousOverloadCount) return;

	PreviousOverloadCount = Current;
	OnOverloadCountChange.Broadcast(Current);
}


int UInventoryComponent::GetCurrentOverloadCount()
{
	int Count = 0;

	for (int i = InventorySlotCount - 1; i >= InventorySlotCount - OverloadBaseCount; i--) {
		if (InventorySlots[i].ItemData == nullptr) continue;
		if (!InventorySlots[i].IsOverloadSlot) continue;

		Count++;
	}

	return Count;
}


FInventorySlot UInventoryComponent::ChangeCurrentSlot(bool IndexGoUp, int ForcedIndex)
{
	if (!GetOwner()->HasAuthority())
	{
		ServerChangeCurrentSlot(IndexGoUp, ForcedIndex);
		return GetCurrentSlot();
	}
	ServerChangeCurrentSlot_Implementation(IndexGoUp, ForcedIndex);
	return GetCurrentSlot();
}


void UInventoryComponent::ServerChangeCurrentSlot_Implementation(bool IndexGoUp, int ForcedIndex)
{
	if (InventorySlots.Num() == 0) return;
	
	if (ForcedIndex != -1) {
		CurrentSlotIndex = ForcedIndex;
	}
	else
	{
		int32 OldIndex = CurrentSlotIndex;

		if (IndexGoUp)
		{
			CurrentSlotIndex = (CurrentSlotIndex + 1) % InventorySlots.Num();
		}
		else
		{
			CurrentSlotIndex = (CurrentSlotIndex - 1 + InventorySlots.Num()) % InventorySlots.Num();
		}
	}

	BroadcastInventoryChange();
}


void UInventoryComponent::SortInventory()
{
	TArray<FInventorySlot> SortedInventory;

	// Weapons 
	for (int i = 0; i < InventorySlots.Num(); i++) {
		if (InventorySlots[i].ItemData == nullptr) continue;
		if (InventorySlots[i].ItemData->ItemType != EItemType::Equipment) continue;

		SortedInventory.Add(InventorySlots[i]);
	}

	// Consummables
	for (int i = 0; i < InventorySlots.Num(); i++) {
		if (InventorySlots[i].ItemData == nullptr) continue;
		if (InventorySlots[i].ItemData->ItemType != EItemType::Consumable) continue;

		SortedInventory.Add(InventorySlots[i]);
	}

	// Valuables
	for (int i = 0; i < InventorySlots.Num(); i++) {
		if (InventorySlots[i].ItemData == nullptr) continue;
		if (InventorySlots[i].ItemData->ItemType != EItemType::Valuable) continue;

		SortedInventory.Add(InventorySlots[i]);
	}

	// Ammo
	for (int i = 0; i < InventorySlots.Num(); i++) {
		if (InventorySlots[i].ItemData == nullptr) continue;
		if (InventorySlots[i].ItemData->ItemType != EItemType::Ammo) continue;

		SortedInventory.Add(InventorySlots[i]);
	}

	// Empty
	for (int i = 0; i < InventorySlots.Num(); i++) {
		if (InventorySlots[i].ItemData != nullptr) continue;
		SortedInventory.Add(InventorySlots[i]);
	}
	
	
	// We apply the sort
	for (int i = 0; i < SortedInventory.Num(); i++) {
		InventorySlots[i].ItemData = SortedInventory[i].ItemData;
		InventorySlots[i].Quantity = SortedInventory[i].Quantity;
	}

	BroadcastInventoryChange();
}

#pragma endregion
