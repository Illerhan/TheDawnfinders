// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Player/APlayerCharacter.h"
#include "Components/UItemComponent.h"
#include "Components/UHealthComponent.h"
#include "Components/UInventoryComponent.h"
#include "Interfaces/IPlayer.h"


UItemComponent::UItemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UItemComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* PlayerActor = GetOwner();
	if (!PlayerActor) return;

	PlayerCharacter = Cast<AAPlayerCharacter>(PlayerActor);

	HealthComponent = PlayerCharacter->HealthComponent;
	InventoryComponent = PlayerCharacter->InventoryComponent;

	InventoryComponent->OnInventoryChanging.AddUniqueDynamic(this, &UItemComponent::SetEquippedItem);
}

void UItemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner()) return;

	// If the player is currently maintaining the use button
	if (!IsUsingItem) return;
	if (ItemUseTimer > 0) 
	{
		ItemUseTimer -= DeltaTime;
		if (GetOwner()->Implements<UPlayerInterface>())
		{
			IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(GetOwner());
			if (PlayerInterface)
			{
				PlayerInterface->ShowProgress_Implementation(ItemUseTimer);
			}
		}
		return;
	}

	UseConsumable();
}


#pragma region Main Action

// CALLED WHEN THE INPUT TO USE THE CURRENT ITEM STARTS
void UItemComponent::DoMainAction()
{
	if (EquippedItem.ItemData == nullptr) return;

	if (EquippedItem.ItemData->ItemType == EItemType::Consumable) 
	{
		if (EquippedItem.ItemData->NeededHoldDuration != 0) {
			ItemUseTimer = EquippedItem.ItemData->NeededHoldDuration;
			IsUsingItem = true;
			return;
		}
		
		UseConsumable();
	}
}

// CALLED TO APPLY THE CONSUMABLE EFFECT
void UItemComponent::UseConsumable()
{
	IsUsingItem = false;
	if (GetOwner()->Implements<UPlayerInterface>())
	{
		IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(GetOwner());
		PlayerInterface->HideProgress_Implementation();
	}

	switch (EquippedItem.ItemData->ConsumableEffectType)
	{
		case EConsumableEffectType::Heal:
			HealthComponent->Heal(EquippedItem.ItemData->ConsumableEffectPower);
			InventoryComponent->RemoveCurrentItem();
			break;

		case EConsumableEffectType::OpenDoor:
			break;

		case EConsumableEffectType::PlaceZipline:
		{
			/*if (PlayerCharacter->HasAuthority())
			{
				ServerUseZiplineItem(slotInfos.ItemData);
				return;
			}
			ServerUseZiplineItem_Implementation(slotInfos.ItemData);
			break;*/
		}
	}
}

// CALLED WHEN THE INPUT TO USE THE CURRENT ITEM ENDS
void UItemComponent::StopMainAction()
{
	if (EquippedItem.ItemData == nullptr) return;

	if (GetOwner()->Implements<UPlayerInterface>())
	{
		IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(GetOwner());
		PlayerInterface->HideProgress_Implementation();
	}

	IsUsingItem = false;
}

#pragma endregion	


#pragma region Seconday Action

void UItemComponent::DoSecondaryAction()
{
	if (EquippedItem.ItemData == nullptr) return;
	if (EquippedItem.ItemData->ItemType == EItemType::Valuable) return;



}

void UItemComponent::StopSecondaryAction()
{
	if (EquippedItem.ItemData == nullptr) return;

}

#pragma endregion


#pragma region Equip / Unequip

void UItemComponent::SetEquippedItem(const TArray<FInventorySlot>& Slots, int CurrentSlotIndex)
{
	if (EquippedItem.ItemData != nullptr && EquippedItem.ItemData->ItemType == EItemType::Equipment) {
		UnequipWeapon();
	}

	EquippedItem = Slots[CurrentSlotIndex];

	if (EquippedItem.ItemData != nullptr && EquippedItem.ItemData->ItemType == EItemType::Equipment) {
		EquipWeapon();
	}
}

void UItemComponent::EquipWeapon()
{

}

void UItemComponent::UnequipWeapon()
{

}

#pragma endregion
