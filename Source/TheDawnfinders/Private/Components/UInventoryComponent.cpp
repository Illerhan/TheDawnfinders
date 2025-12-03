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
	CurrentOverloadSlotCount += OverloadSlotBaseCount;

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
	DOREPLIFETIME(UInventoryComponent, Gold);
}


void UInventoryComponent::OnRep_InventorySlots()
{
	VerifyCurrentOverloadCount();

	OnInventoryChange.Broadcast(InventorySlots, CurrentSlotIndex);
}


void UInventoryComponent::OnRep_CurrentSlotIndex()
{
	OnInventoryChange.Broadcast(InventorySlots, CurrentSlotIndex);
}

#pragma endregion


#pragma region Add / Remove Item

bool UInventoryComponent::AddNewItem(UItemData* NewItem)
{
	if (NewItem->ItemType == EItemType::Currency)
	{
		if (!GetOwner()->HasAuthority())
		{
			ServerConsumeItemDirectly(NewItem);
			return true;
		}
		ServerConsumeItemDirectly_Implementation(NewItem);
		return true;
	}
	
	if (!HasRoomForItem(NewItem)) return false;

	if (!GetOwner()->HasAuthority())
	{
		ServerAddNewItem(NewItem);
		return true;
	}

	ServerAddNewItem_Implementation(NewItem);
	return true;
}


void UInventoryComponent::ServerAddNewItem_Implementation(UItemData* NewItem)
{
	if (!NewItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddNewItem: NewItem is null"));
		return;
	}

	for (int32 i = 0; i < InventorySlots.Num(); i++)
	{
		FInventorySlot& Slot = InventorySlots[i];
		
		if (!Slot.ItemData)
		{
			Slot.ItemData = NewItem;
			Slot.Quantity = 1;

			break;
		}
		else if (Slot.ItemData == NewItem && Slot.Quantity < NewItem->MaxStackingCapacity)
		{
			Slot.Quantity++;

			break;
		}
	}

	SortInventory();
	VerifyCurrentOverloadCount();
}


bool UInventoryComponent::HasRoomForItem(UItemData* NewItem)
{
	for (int32 i = 0; i < InventorySlots.Num(); i++)
	{
		FInventorySlot& Slot = InventorySlots[i];

		if (!Slot.ItemData)
		{
			return true;
		}
		else if (Slot.ItemData == NewItem && Slot.Quantity < NewItem->MaxStackingCapacity)
		{
			return true;
		}
	}

	return false;
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
	VerifyCurrentOverloadCount();
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
	if (!InventorySlots.IsValidIndex(CurrentSlotIndex)) return;
	
	FInventorySlot& CurrentSlot = InventorySlots[CurrentSlotIndex];

	if (!CurrentSlot.ItemData) return; 

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
			}

			DroppedItem->bShouldLevitate = true;

			if (UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(DroppedItem->GetRootComponent()))
			{
				if (RootComponent->IsSimulatingPhysics())
				{
					FVector ThrowDirection = GetOwner()->GetActorForwardVector() + FVector(0, 0, 0.25f);
					RootComponent->AddImpulse(ThrowDirection * 500.f, NAME_None, true);
				}
			}
		}

		RemoveCurrentItem();
	}
}

#pragma endregion


#pragma region Sort 

void UInventoryComponent::SortInventory()
{
	SortByCategories();
	SortItems();

	OnInventoryChange.Broadcast(InventorySlots, CurrentSlotIndex);
}

void UInventoryComponent::SortByCategories()
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
}


void UInventoryComponent::SortItems()
{
	TArray<FInventorySlot> SortedInventory;
	TArray<bool> SortedSlots;
	SortedSlots.Init(false, InventorySlots.Num());

	for (int i = 0; i < InventorySlots.Num(); i++) {
		if (InventorySlots[i].ItemData == nullptr) continue;
		if (SortedSlots[i]) continue;

		// if one stack capacity
		if (InventorySlots[i].ItemData->MaxStackingCapacity <= 1) {
			SortedInventory.Add(InventorySlots[i]);
			continue;
		}

		// We get the other others items of the same type
		TArray<FInventorySlot> SameItemSlots;
		SameItemSlots.Init(InventorySlots[i], 1);

		for (int j = i + 1; j < InventorySlots.Num(); j++) {
			if (InventorySlots[j].ItemData != InventorySlots[i].ItemData) continue;

			SortedSlots[j] = true;
			SameItemSlots.Add(InventorySlots[j]);
		}


		// We try to stack them if possible
		for (int j = 0; j < SameItemSlots.Num(); j++) {
			if (SameItemSlots[j].Quantity == 0) continue;

			// If the slot is already full
			if (SameItemSlots[j].Quantity == SameItemSlots[j].ItemData->MaxStackingCapacity)
			{
				SortedInventory.Add(SameItemSlots[j]);
				continue;
			}

			// We try to stack the two slots
			for (int k = j + 1; k < SameItemSlots.Num(); k++) {
				if (SameItemSlots[j].Quantity == SameItemSlots[j].ItemData->MaxStackingCapacity) break;
				if (SameItemSlots[k].Quantity <= 0) continue;

				int FinalQuantity = SameItemSlots[k].Quantity + SameItemSlots[j].Quantity;
				SameItemSlots[j].Quantity = FMath::Clamp(FinalQuantity, 0, SameItemSlots[j].ItemData->MaxStackingCapacity);
				SameItemSlots[k].Quantity = FMath::Clamp(FinalQuantity - SameItemSlots[j].Quantity, 0, SameItemSlots[j].ItemData->MaxStackingCapacity);
			}

			UE_LOG(LogTemp, Display, TEXT("Stack First Slot = %d"), SameItemSlots[j].Quantity);

			SortedInventory.Add(SameItemSlots[j]);
		}
	}

	// We apply the sort
	for (int i = 0; i < SortedInventory.Num(); i++) {
		InventorySlots[i].ItemData = SortedInventory[i].ItemData;
		InventorySlots[i].Quantity = SortedInventory[i].Quantity;
	}

	for (int i = SortedInventory.Num(); i < InventorySlots.Num(); i++) {
		InventorySlots[i].ItemData = NULL;
		InventorySlots[i].Quantity = 0;
	}
}

#pragma endregion

#pragma region Gold

void UInventoryComponent::Server_AddGold_Implementation(int32 Amount)
{
	Gold+= Amount;
	OnRep_Gold();
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


void UInventoryComponent::SelectSlotByAngle(int angle)
{
	int Index = (angle / 360.f) * InventorySlotCount;

	ChangeCurrentSlot(true, Index);
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

	for (int i = InventorySlotCount - 1; i >= InventorySlotCount - CurrentOverloadSlotCount; i--) {
		if (InventorySlots[i].ItemData == nullptr) continue;
		if (!InventorySlots[i].bIsOverloadSlot) continue;

		Count++;
	}

	return Count;
}

void UInventoryComponent::ChangeOverloadSlotCount(int AmountAdded)
{
	CurrentOverloadSlotCount += AmountAdded;

	ActualiseOverloadedSlots();
}

void UInventoryComponent::ServerConsumeItemDirectly_Implementation(UItemData* Item)
{
	if (!Item) return;
    
	switch (Item->ItemType)
	{
	case EItemType::Currency:
		// Ajouter l'or directement
		Gold += Item->ItemValue;
		OnRep_Gold();
		break;
	
	default:
		break;
	}
}


void UInventoryComponent::Server_AddKnowledge_Implementation(int32 Amount)
{
	Knowledge += Amount;
	OnRep_Knowledge();
}

void UInventoryComponent::OnRep_Gold()
{
	OnInventoryChange.Broadcast(InventorySlots, CurrentSlotIndex);
}

void UInventoryComponent::OnRep_Knowledge()
{
	OnInventoryChange.Broadcast(InventorySlots, CurrentSlotIndex);
}


FInventorySlot UInventoryComponent::ChangeCurrentSlot(bool IndexGoUp, int ForcedIndex)
{
	if (!bIsOpened) return GetCurrentSlot();

	UE_LOG(LogTemp, Display, TEXT("Change Inedx : %d"), CurrentSlotIndex);

	// Client
	if (!GetOwner()->HasAuthority())
	{
		ServerChangeCurrentSlot(IndexGoUp, ForcedIndex);
		return GetCurrentSlot();
	}

	// Server
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
	OnRep_CurrentSlotIndex();
	OnInventoryChange.Broadcast(InventorySlots, CurrentSlotIndex);
}

void UInventoryComponent::ActualiseOverloadedSlots()
{
	if (InventorySlots.Num() == 0) return;

	TArray<FInventorySlot> NewInventorySlots;

	for (int i = 0; i < InventorySlotCount; i++) {
		NewInventorySlots.Add(InventorySlots[i]);

		bool bIsOverloadSlot = (InventorySlotCount - CurrentOverloadSlotCount) <= i;
		NewInventorySlots[i].bIsOverloadSlot = bIsOverloadSlot;
	}

	InventorySlots = NewInventorySlots;
	OnInventoryChange.Broadcast(InventorySlots, CurrentSlotIndex);
}

void UInventoryComponent::OpenInventory()
{
	bIsOpened = true;
	OnInventoryOpenInput.Broadcast();
}

void UInventoryComponent::CloseInventory()
{
	bIsOpened = false;
	OnInventoryCloseInput.Broadcast();
}

#pragma endregion

