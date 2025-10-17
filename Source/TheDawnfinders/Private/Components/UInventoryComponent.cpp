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

// ===== REPLICATION =====

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

// === Add items ===

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

	for (int32 i =0; i<InventorySlots.Num(); i++)
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
		else if (Slot.ItemData == NewItem)
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
	InventorySlots=NewSlots;
	BroadcastInventoryChange();

}


// === Throw items ===

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
			if (UPrimitiveComponent* RootComponent = Cast<UPrimitiveComponent>(DroppedItem->GetRootComponent()))
			{
				if (RootComponent->IsSimulatingPhysics())
				{
					FVector ThrowDirection = GetOwner()->GetActorForwardVector() + FVector(0,0,0.5f);
					RootComponent->AddImpulse(ThrowDirection*500.f,NAME_None,true);
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

		CurrentSlot.Quantity--;

		if (CurrentSlot.Quantity <=0)
		{
			CurrentSlot.ItemData = nullptr;
			CurrentSlot.Quantity = 0;
		}

		BroadcastInventoryChange();
	}
}
FInventorySlot UInventoryComponent::GetCurrentSlot()
{
	if (InventorySlots.IsValidIndex(CurrentSlotIndex))
	{
		return InventorySlots[CurrentSlotIndex];
		
	}
	return FInventorySlot();
}

FInventorySlot UInventoryComponent::ChangeCurrentSlot(bool IndexGoUp)
{
	if (!GetOwner()->HasAuthority())
	{
		ServerChangeCurrentSlot(IndexGoUp);
		return GetCurrentSlot();
	}
	ServerChangeCurrentSlot_Implementation(IndexGoUp);
	return GetCurrentSlot();
}

void UInventoryComponent::ServerChangeCurrentSlot_Implementation(bool IndexGoUp)
{
	if (InventorySlots.Num()==0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChangeCurrentSlot: No slots available"));
		return;
	}
	
	int32 OldIndex = CurrentSlotIndex;

	if (IndexGoUp)
	{
		CurrentSlotIndex = (CurrentSlotIndex + 1) % InventorySlots.Num();
	}
	else
	{
		CurrentSlotIndex = (CurrentSlotIndex - 1 + InventorySlots.Num()) % InventorySlots.Num();
	}

	UE_LOG(LogTemp, Log, TEXT("Current slot changed from %d to %d"), OldIndex, CurrentSlotIndex);

	BroadcastInventoryChange();

}


