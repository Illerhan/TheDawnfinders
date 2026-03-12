// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UInventoryComponent.h"
#include "Actors/AItem.h"
#include "GameFramework/CustomPlayerState.h"
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
	DOREPLIFETIME(UInventoryComponent, TreasureSlots);
	DOREPLIFETIME(UInventoryComponent, CurrentSlotIndex);
	DOREPLIFETIME(UInventoryComponent, Gold);
}


void UInventoryComponent::OnRep_InventorySlots()
{
	VerifyCurrentOverloadCount();

	OnInventoryChange.Broadcast(InventorySlots, CurrentSlotIndex);
}

void UInventoryComponent::OnRep_TreasureSlots()
{
	OnTreasureInventoryChange.Broadcast(TreasureSlots);
}


void UInventoryComponent::OnRep_CurrentSlotIndex()
{
	OnInventoryChange.Broadcast(InventorySlots, CurrentSlotIndex);
}

#pragma endregion


#pragma region Add / Remove Item

bool UInventoryComponent::AddNewItem(FItemInfos NewItem, int Quantity)
{
	if (!NewItem.ItemData) return false;

	if (NewItem.ItemData->ItemType == EItemType::Currency)
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
		UE_LOG(LogTemp, Display, TEXT("%s"), *NewItem.ItemData->ItemName);

		ServerAddNewItem(NewItem, Quantity);
		return true;
	}

	ServerAddNewItem_Implementation(NewItem, Quantity);
	return true;
}


void UInventoryComponent::ServerAddNewItem_Implementation(FItemInfos NewItem, int Quantity)
{
	if (!NewItem.ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddNewItem: NewItem is null"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("AddNewItem"));

	bool UseValueableInventory = NewItem.ItemData->ItemType == EItemType::Valuable;

	for (int32 i = 0; i < (UseValueableInventory ? TreasureSlotCount : InventorySlotCount); i++)
	{
		FInventorySlot& Slot = UseValueableInventory ? TreasureSlots[i] : InventorySlots[i];
		
		if (!Slot.CurrentInfos.ItemData)
		{
			Slot.CurrentInfos = NewItem;
			Slot.Quantity = Quantity;
			CurrentWeight += Slot.CurrentInfos.ItemData->ItemWeight;

			break;
		}
		else if (Slot.CurrentInfos.ItemData == NewItem.ItemData && Slot.Quantity < NewItem.ItemData->MaxStackingCapacity)
		{
			Slot.Quantity += Quantity;

			break;
		}
	}

	SortInventory();
	VerifyCurrentOverloadCount();
}

void UInventoryComponent::RestoreShopItems()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return;

	ACustomPlayerState* PS = PC->GetPlayerState<ACustomPlayerState>();
	if (!PS || PS->ShopItems.Num() == 0) return;

	if (PS->SavedGold > 0)
	{
		Server_AddGold_Implementation(PS->SavedGold);
		PS->SavedGold = 0;
	}
	
	bShopItemsRestored = false;
	AddShopItems(PS->ShopItems);
}

void UInventoryComponent::UpdateValuable()
{
	CurrentValue = 0;

	if (TreasureSlots.Num() == 0) return;

	for (const FInventorySlot& Slot : TreasureSlots)
	{
		if (Slot.CurrentInfos.ItemData)
		{
			int32 ValueDuSlot = Slot.CurrentInfos.ItemData->ItemValue * Slot.Quantity;
			CurrentValue += ValueDuSlot;
		}
	}
}

void UInventoryComponent::AddShopItems(TArray<FItemInfos> Items)
{
    for (FItemInfos& Item : Items)
    {
        ServerAddNewItem_Implementation(Item, 1);
    }
}

bool UInventoryComponent::HasRoomForItem(FItemInfos NewItem)
{
	if (!NewItem.ItemData) return false;

	if (NewItem.ItemData->ItemType == EItemType::Valuable && TreasureSlotCount > 0) {

		for (int i = 0; i < TreasureSlotCount; i++)
		{
			FInventorySlot& Slot = TreasureSlots[i];

			if (!Slot.CurrentInfos.ItemData)
			{
				return true;
			}
			else if (Slot.CurrentInfos.ItemData == NewItem.ItemData && Slot.Quantity < NewItem.ItemData->MaxStackingCapacity)
			{
				return true;
			}
		}

		return false;
	}

	for (int i = 0; i < InventorySlots.Num(); i++)
	{
		FInventorySlot& Slot = InventorySlots[i];

		if (!Slot.CurrentInfos.ItemData)
		{
			return true;
		}
		else if (Slot.CurrentInfos.ItemData == NewItem.ItemData && Slot.Quantity < NewItem.ItemData->MaxStackingCapacity)
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
	TArray<FInventorySlot> NewSlots = InventorySlots;

	FInventorySlot& CurrentSlot = NewSlots[CurrentSlotIndex];
	CurrentSlot.Quantity--;
	if (CurrentSlot.Quantity <= 0) CurrentSlot.CurrentInfos.ItemData = nullptr;

	InventorySlots = NewSlots;

	SortInventory();
	VerifyCurrentOverloadCount();
}


void UInventoryComponent::RemoveItemAtIndex(int Index, bool bRemoveAll, bool TreasureInventory)
{
	if (!GetOwner()->HasAuthority())
	{
		LocalRemoveItemAtIndex(Index, bRemoveAll, TreasureInventory);
		ServerRemoveItemAtIndex(Index, bRemoveAll, TreasureInventory);

		return;
	}

	ServerRemoveItemAtIndex_Implementation(Index, bRemoveAll, TreasureInventory);
}

void UInventoryComponent::LocalRemoveItemAtIndex(int Index, bool bRemoveAll, bool TreasureInventory)
{
	TArray<FInventorySlot> NewSlots = TreasureInventory ? TreasureSlots : InventorySlots;
	FInventorySlot& CurrentSlot = NewSlots[Index];

	if (!CurrentSlot.CurrentInfos.ItemData) return;

	CurrentWeight -= CurrentSlot.CurrentInfos.ItemData->ItemWeight;

	if (bRemoveAll) CurrentSlot.Quantity = 0;
	else CurrentSlot.Quantity--;

	if (CurrentSlot.Quantity <= 0) CurrentSlot.CurrentInfos.ItemData = nullptr;

	if (TreasureInventory) TreasureSlots = NewSlots;
	else InventorySlots = NewSlots;

	SortInventory();
	VerifyCurrentOverloadCount();
}

void UInventoryComponent::ServerRemoveItemAtIndex_Implementation(int Index, bool bRemoveAll, bool TreasureInventory)
{
	TArray<FInventorySlot> NewSlots = TreasureInventory ? TreasureSlots : InventorySlots;
	FInventorySlot& CurrentSlot = NewSlots[Index];

	if (!CurrentSlot.CurrentInfos.ItemData) return;

	CurrentWeight -= CurrentSlot.CurrentInfos.ItemData->ItemWeight;

	if (bRemoveAll) CurrentSlot.Quantity = 0;
	else CurrentSlot.Quantity--;
	
	if (CurrentSlot.Quantity <= 0) CurrentSlot.CurrentInfos.ItemData = nullptr;

	if(TreasureInventory) TreasureSlots = NewSlots;
	else InventorySlots = NewSlots;

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

	if (!CurrentSlot.CurrentInfos.ItemData) return;

	if (GetWorld() && GetOwner() && CurrentSlot.CurrentInfos.ItemData->ItemClass)
	{
		FVector SpawnLocation = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 100.f;
		FRotator SpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Instigator = Cast<APawn>(GetOwner());

		AItem* DroppedItem = GetWorld()->SpawnActor<AItem>(
			CurrentSlot.CurrentInfos.ItemData->ItemClass,
			SpawnLocation,
			SpawnRotation,
			SpawnInfo
		);

		if (DroppedItem)
		{
			DroppedItem->ItemData = CurrentSlot.CurrentInfos.ItemData;

			DroppedItem->Initialise(CurrentSlot.CurrentInfos);

			if (DroppedItem->ItemMesh && CurrentSlot.CurrentInfos.ItemData->ItemMesh)
			{
				DroppedItem->ItemMesh->SetStaticMesh(CurrentSlot.CurrentInfos.ItemData->ItemMesh);
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

	ActualiseHasTreasures();

	OnInventoryChange.Broadcast(InventorySlots, CurrentSlotIndex);
	if(TreasureSlotCount > 0) OnTreasureInventoryChange.Broadcast(TreasureSlots);
}

void UInventoryComponent::SortByCategories()
{
	TArray<FInventorySlot> SortedInventory;

	// Weapons 
	for (int i = 0; i < InventorySlots.Num(); i++) {
		if (InventorySlots[i].CurrentInfos.ItemData == nullptr) continue;
		if (InventorySlots[i].CurrentInfos.ItemData->ItemType != EItemType::Equipment) continue;

		SortedInventory.Add(InventorySlots[i]);
	}

	// Consummables
	for (int i = 0; i < InventorySlots.Num(); i++) {
		if (InventorySlots[i].CurrentInfos.ItemData == nullptr) continue;
		if (InventorySlots[i].CurrentInfos.ItemData->ItemType != EItemType::Consumable) continue;

		SortedInventory.Add(InventorySlots[i]);
	}

	// Valuables
	for (int i = 0; i < InventorySlots.Num(); i++) {
		if (InventorySlots[i].CurrentInfos.ItemData == nullptr) continue;
		if (InventorySlots[i].CurrentInfos.ItemData->ItemType != EItemType::Valuable) continue;

		SortedInventory.Add(InventorySlots[i]);
	}

	// Ammo
	for (int i = 0; i < InventorySlots.Num(); i++) {
		if (InventorySlots[i].CurrentInfos.ItemData == nullptr) continue;
		if (InventorySlots[i].CurrentInfos.ItemData->ItemType != EItemType::Ammo) continue;

		SortedInventory.Add(InventorySlots[i]);
	}

	// Empty
	for (int i = 0; i < InventorySlots.Num(); i++) {
		if (InventorySlots[i].CurrentInfos.ItemData != nullptr) continue;
		SortedInventory.Add(InventorySlots[i]);
	}


	// We apply the sort
	for (int i = 0; i < SortedInventory.Num(); i++) {
		InventorySlots[i].CurrentInfos.ItemData = SortedInventory[i].CurrentInfos.ItemData;
		InventorySlots[i].Quantity = SortedInventory[i].Quantity;
		InventorySlots[i].CurrentInfos.Durability = SortedInventory[i].CurrentInfos.Durability;
	}
}


void UInventoryComponent::SortItems()
{
	TArray<FInventorySlot> SortedInventory;
	TArray<bool> SortedSlots;
	SortedSlots.Init(false, InventorySlots.Num());

	for (int i = 0; i < InventorySlots.Num(); i++) {
		if (InventorySlots[i].CurrentInfos.ItemData == nullptr) continue;
		if (SortedSlots[i]) continue;

		// if one stack capacity
		if (InventorySlots[i].CurrentInfos.ItemData->MaxStackingCapacity <= 1) {
			SortedInventory.Add(InventorySlots[i]);
			continue;
		}

		// We get the other others items of the same type
		TArray<FInventorySlot> SameItemSlots;
		SameItemSlots.Init(InventorySlots[i], 1);

		for (int j = i + 1; j < InventorySlots.Num(); j++) {
			if (InventorySlots[j].CurrentInfos.ItemData != InventorySlots[i].CurrentInfos.ItemData) continue;

			SortedSlots[j] = true;
			SameItemSlots.Add(InventorySlots[j]);
		}


		// We try to stack them if possible
		for (int j = 0; j < SameItemSlots.Num(); j++) {
			if (SameItemSlots[j].Quantity == 0) continue;

			// If the slot is already full
			if (SameItemSlots[j].Quantity == SameItemSlots[j].CurrentInfos.ItemData->MaxStackingCapacity)
			{
				SortedInventory.Add(SameItemSlots[j]);
				continue;
			}

			// We try to stack the two slots
			for (int k = j + 1; k < SameItemSlots.Num(); k++) {
				if (SameItemSlots[j].Quantity == SameItemSlots[j].CurrentInfos.ItemData->MaxStackingCapacity) break;
				if (SameItemSlots[k].Quantity <= 0) continue;

				int FinalQuantity = SameItemSlots[k].Quantity + SameItemSlots[j].Quantity;
				SameItemSlots[j].Quantity = FMath::Clamp(FinalQuantity, 0, SameItemSlots[j].CurrentInfos.ItemData->MaxStackingCapacity);
				SameItemSlots[k].Quantity = FMath::Clamp(FinalQuantity - SameItemSlots[j].Quantity, 0, SameItemSlots[j].CurrentInfos.ItemData->MaxStackingCapacity);
			}

			UE_LOG(LogTemp, Display, TEXT("Stack First Slot = %d"), SameItemSlots[j].Quantity);

			SortedInventory.Add(SameItemSlots[j]);
		}
	}

	// We apply the sort
	for (int i = 0; i < SortedInventory.Num(); i++) {
		InventorySlots[i].CurrentInfos.ItemData = SortedInventory[i].CurrentInfos.ItemData;
		InventorySlots[i].Quantity = SortedInventory[i].Quantity;
	}

	for (int i = SortedInventory.Num(); i < InventorySlots.Num(); i++) {
		InventorySlots[i].CurrentInfos.ItemData = NULL;
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

UItemData* UInventoryComponent::GetCurrentItem()
{
	if (InventorySlots.IsValidIndex(CurrentSlotIndex))
	{
		return InventorySlots[CurrentSlotIndex].CurrentInfos.ItemData;
	}

	return nullptr;
}


bool UInventoryComponent::VerifyHasItemInInventory(UItemData* Item)
{
	for (int i = 0; i < InventorySlotCount; i++) {
		if (InventorySlots[i].CurrentInfos.ItemData == nullptr) continue;
		if (InventorySlots[i].CurrentInfos.ItemData != Item) continue;

		return true;
	}

	return false;
}

void UInventoryComponent::UseDurability(int UsedDurability, UItemData* ItemToUse)
{
	for (int i = 0; i < InventorySlotCount; i++) {

		if (InventorySlots[i].CurrentInfos.ItemData == nullptr) continue;
		if (InventorySlots[i].CurrentInfos.ItemData != ItemToUse) continue;

		InventorySlots[i].CurrentInfos.Durability -= UsedDurability;

		UE_LOG(LogTemp, Display, TEXT("%f"), InventorySlots[i].CurrentInfos.Durability);

		if (InventorySlots[i].CurrentInfos.Durability <= 0
			&& InventorySlots[i].CurrentInfos.ItemData->ItemType == EItemType::Consumable) {
			RemoveItemAtIndex(i, false);
		}

		OnInventoryChange.Broadcast(InventorySlots, i);

		break;
	}
}


void UInventoryComponent::SelectSlotByAngle(int angle)
{
	int Index = FMath::RoundToInt((angle / 360.f) * InventorySlotCount);
	Index = Index % InventorySlotCount;

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
		if (!InventorySlots.IsValidIndex(i)) continue;
		if (InventorySlots[i].CurrentInfos.ItemData == nullptr) continue;
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

void UInventoryComponent::ServerConsumeItemDirectly_Implementation(FItemInfos Item)
{
	if (!Item.ItemData) return;
    
	switch (Item.ItemData->ItemType)
	{
		case EItemType::Currency:
			// Ajouter l'or directement
			Gold += Item.ItemData->ItemValue;
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


FInventorySlot UInventoryComponent::QuickChange(bool bGoRight)
{
	// Client
	if (!GetOwner()->HasAuthority())
	{
		ServerQuickChange(bGoRight);
		return GetCurrentSlot();
	}

	// Server
	ServerQuickChange_Implementation(bGoRight);
	return GetCurrentSlot();
}

void UInventoryComponent::ServerQuickChange_Implementation(bool bGoRight)
{
	int Added = 1;

	while (Added < InventorySlotCount) {
		int Current = bGoRight ? CurrentSlotIndex + Added : CurrentSlotIndex - Added;
		Added++;

		if (Current < 0) {
			Current += InventorySlotCount;
		}
		else if (Current >= InventorySlotCount) {
			Current -= InventorySlotCount;
		}

		if (InventorySlots[Current].CurrentInfos.ItemData == nullptr) continue;

		CurrentSlotIndex = Current;
		OnRep_CurrentSlotIndex();
		OnInventoryChange.Broadcast(InventorySlots, CurrentSlotIndex);

		break;
	}
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

void UInventoryComponent::ActualiseHasTreasures()
{
	bHasTreasures = false; 
	
	if (TreasureSlots.Num() == 0) return;
	for (int i = 0; i < TreasureSlotCount; i++) {
		if (!TreasureSlots[i].CurrentInfos.ItemData) continue;

		bHasTreasures = true;
		return;
	}
}

void UInventoryComponent::OpenInventory()
{
	if (bIsOpened) return;
	bIsOpened = true;
	OnInventoryOpenInput.Broadcast();
}

void UInventoryComponent::CloseInventory()
{
	if (!bIsOpened) return;
	bIsOpened = false;
	OnInventoryCloseInput.Broadcast();
}

#pragma endregion

