// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/CustomPlayerState.h"

#include "../../../../Plugins/Wwise/Source/AkAudio/Classes/BlueprintNodes/PostEventAsync.h"
#include "GameFramework/CustomHUD.h"
#include "Widgets/UMainWidget.h"
#include "Widgets/USquadWidget.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Components/UStaminaComponent.h"
#include "Net/UnrealNetwork.h"



void ACustomPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (GetPlayerController() == nullptr) return;

	for (UAmuletData* Amulet : PossessedAmulets) {
		if (Amulet->AmuletTriggerType == EAmuletTriggerType::Always) {
			Amulet->ApplyEffect(GetPlayerController(), this);
		}
	}
}


#pragma region Health

// CALLED ON THE CLIENT TO ACTUALISE IT'S VALUES INSTANTLY 
void ACustomPlayerState::ActualiseLocalHealth(float current, float max, float fixedMax)
{
	CurrentHealth = current;
	CurrentMaxHealth = max;
	MaxHealth = fixedMax;

	OnCurseInfoChange.Broadcast();
	OnInfoChangeLocal.ExecuteIfBound();
}

// CALLED ON THE SERVER TO ACTUALISE FOR ALL
void ACustomPlayerState::ActualiseHealth(float current, float max, float fixedMax)
{
	CurrentHealth = current;
	CurrentMaxHealth = max;
	MaxHealth = fixedMax;

	OnCurseInfoChange.Broadcast();
	OnRep_StaminaChange();
}

void ACustomPlayerState::SetMaxHealth(float MaxHP)
{
	MaxHealth = MaxHP;

	OnCurseInfoChange.Broadcast();
	OnRep_StaminaChange();
}

void ACustomPlayerState::SetCurrentMaxHealth(float MaxHP)
{
	CurrentMaxHealth = MaxHP;
}

void ACustomPlayerState::StartPoison()
{
	bIsPoisoned = true;

	OnInfoChange.Broadcast();
	OnInfoChangeLocal.ExecuteIfBound();
}

void ACustomPlayerState::EndPoison()
{
	bIsPoisoned = false;

	Multicast_EndPoison();
	OnInfoChangeLocal.ExecuteIfBound();
}

void ACustomPlayerState::Multicast_EndPoison_Implementation()
{
	bIsPoisoned = false;

	OnInfoChangeLocal.ExecuteIfBound();
}

#pragma endregion


#pragma region Stamina

// CALLED ON THE CLIENT TO ACTUALISE IT'S VALUES INSTANTLY 
void ACustomPlayerState::ActualiseLocalStamina(float current, float max)
{
	CurrentStamina = current;
	CurrentMaxStamina = max;

	OnInfoChangeLocal.ExecuteIfBound();
}

// CALLED ON THE SERVER TO ACTUALISE FOR ALL
void ACustomPlayerState::ActualiseStamina(float current, float max)
{
	CurrentStamina = current;
	CurrentMaxStamina = max;

	OnInfoChangeLocal.ExecuteIfBound();
	OnRep_HealthChange();
}

#pragma endregion


#pragma region Others

// CALLED ON THE CLIENT TO ACTUALISE IT'S VALUES INSTANTLY 
void ACustomPlayerState::ActualiseLocalLantern(float current, float max)
{
	LanternPercent = (int)((current / max) * 100);

	OnInfoChangeLocal.ExecuteIfBound();
}


// CALLED ON THE SERVER TO ACTUALISE FOR ALL
void ACustomPlayerState::ActualiseLantern(float current, float max)
{
	LanternPercent = (int)((current / max) * 100);

	OnRep_LanternChange();
}

void ACustomPlayerState::ActualiseEquippedItem(FInventorySlot Current)
{
	CurrentSlot = Current;

	OnInfoChangeLocal.ExecuteIfBound();
}

void ACustomPlayerState::SetIsReady(bool Ready)
{
	bIsReady = Ready;
	if (!HasAuthority()) {
		Server_SetIsReady(Ready);
	}
	else {
		VerifyCountdownLaunch();
	}
}

void ACustomPlayerState::VerifyCountdownLaunch_Implementation()
{

}

void ACustomPlayerState::Server_SetIsReady_Implementation(bool Ready)
{
	bIsReady = Ready;

	VerifyCountdownLaunch();
}

#pragma endregion


#pragma region Amulets

void ACustomPlayerState::ApplyContextualAmulet(EAmuletTriggerType Trigger)
{
	for (UAmuletData* Amulet : PossessedAmulets) {
		if (Amulet->AmuletTriggerType == Trigger) {
			Amulet->VerifyApplyEffect(GetPlayerController(), this);
		}
	}
}

void ACustomPlayerState::ChangeGold_Implementation(int NewGold)
{

}

#pragma endregion


#pragma region Network

void ACustomPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACustomPlayerState, CurrentStamina);
	DOREPLIFETIME(ACustomPlayerState, CurrentMaxStamina);
	DOREPLIFETIME(ACustomPlayerState, CurrentMaxHealth);
	DOREPLIFETIME(ACustomPlayerState, CurrentHealth);
	DOREPLIFETIME(ACustomPlayerState, ShopItems);
	DOREPLIFETIME(ACustomPlayerState, SavedGold);
	DOREPLIFETIME(ACustomPlayerState, bIsProtected);
}

void ACustomPlayerState::OnRep_StaminaChange()
{
	OnInfoChange.Broadcast();
}

void ACustomPlayerState::OnRep_HealthChange()
{
	OnInfoChange.Broadcast();
}

void ACustomPlayerState::OnRep_LanternChange()
{
	OnInfoChange.Broadcast();
}

void ACustomPlayerState::OverrideWith(APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);
	
	if (ACustomPlayerState* NewPS = Cast<ACustomPlayerState>(PlayerState))
	{
		bIsWoman = NewPS->bIsWoman;
		ShopItems = NewPS->ShopItems;
		SavedGold = NewPS->SavedGold;
		bInSession=NewPS->bInSession;
		PlayerCount = NewPS->PlayerCount;
		InventoryItems = NewPS->InventoryItems;
		StashItems = NewPS->StashItems;
	}
}

void ACustomPlayerState::OnRep_ShopItems()
{
	UE_LOG(LogTemp, Warning, TEXT("OnRep_ShopItems: %d items, %d listeners"), 
		ShopItems.Num(), 
		OnShopItemsChange.IsBound() ? 1 : 0);
	OnShopItemsChange.Broadcast();
}

void ACustomPlayerState::OnRep_Gold()
{
	OnGoldChanged.Broadcast();
}


void ACustomPlayerState::Client_AddShopItemLocally(FItemInfos Item)
{
	if (HasAuthority()) return;
	ShopItems.Add(Item);
	OnShopItemsChange.Broadcast();
}

void ACustomPlayerState::Server_RepairItem_Implementation(int Cost)
{
	SavedGold -= Cost;
}

void ACustomPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	if (ACustomPlayerState* NewPS = Cast<ACustomPlayerState>(PlayerState))
	{
		NewPS->bIsWoman = bIsWoman;
		NewPS->ShopItems = ShopItems;
		NewPS->SavedGold = SavedGold;
		NewPS->bInSession = bInSession;
		NewPS->PlayerCount = PlayerCount;
		NewPS->StashItems = StashItems;
		NewPS->InventoryItems = InventoryItems;
	}
	
	
}

void ACustomPlayerState::Server_SellShopItem_Implementation(UItemData* ItemToSell)
{
	if (!ItemToSell) return;
	int SellPrice = ItemToSell->SellValue;
		
	SavedGold += SellPrice;

}

void ACustomPlayerState::SaveInventoryBeforeTravel()
{
	APawn* Pawn = GetPawn();
	if (!Pawn) return;

	UInventoryComponent* Inv = Pawn->FindComponentByClass<UInventoryComponent>();
	if (!Inv || !Inv->GetOwner()->HasAuthority()) return;
	
	ShopItems.Empty();

	// 3. On transfère les items
	for (const FInventorySlot& Slot : Inv->InventorySlots)
	{
		if (Slot.CurrentInfos.ItemData) 
		{
			ShopItems.Add(Slot.CurrentInfos);
		}
	}
}


void ACustomPlayerState::AddShopItem(FItemInfos Item)
{
	SavedGold -= Item.ItemData->ItemValue;

	OnShopItemsChange.Broadcast();
	OnRep_ShopItems();

	if (!HasAuthority()) {
		Server_AddShopItem(Item);
	}
}

void ACustomPlayerState::Server_AddShopItem_Implementation(FItemInfos Item)
{
	if (!Item.ItemData) return;
	if (SavedGold < Item.ItemData->ItemValue) return;

	SavedGold-=Item.ItemData->ItemValue;
	OnShopItemsChange.Broadcast();
	OnRep_ShopItems();
}

#pragma endregion
