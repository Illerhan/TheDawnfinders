// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/CustomPlayerState.h"
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
}

void ACustomPlayerState::SetCurrentMaxHealth(float MaxHP)
{
	CurrentMaxHealth = MaxHP;
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

#pragma endregion


#pragma region Network

void ACustomPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACustomPlayerState, CurrentStamina);
	DOREPLIFETIME(ACustomPlayerState, ShopItems);
	DOREPLIFETIME(ACustomPlayerState, SavedGold);
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

void ACustomPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	if (ACustomPlayerState* NewPS = Cast<ACustomPlayerState>(PlayerState))
	{
		NewPS->ShopItems = ShopItems;
	}
	
	
}

void ACustomPlayerState::SaveInventoryBeforeTravel()
{
	APawn* Pawn = GetPawn();
	if (!Pawn) return;

	AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Pawn);
	if (!Player || !Player->InventoryComponent) return;
	
	UInventoryComponent* Inv = Pawn->FindComponentByClass<UInventoryComponent>();
	if (!Inv) return;
	
	SavedGold = Inv->Gold;

}

void ACustomPlayerState::Server_AddShopItem_Implementation(FItemInfos Item)
{
	if (!Item.ItemData) return;
    
	//APawn* Pawn = Cast<APawn>(GetPawn());
	//if (!Pawn) return;
	//UInventoryComponent* Inv = Pawn->FindComponentByClass<UInventoryComponent>();
	//if (!Inv) return;
    
	//if (Inv->Gold < Item.ItemData->Price) return;
	
	int32 TotalRows = 0;
	TMap<UItemData*, int32> TypeCounts;
    
	for (FItemInfos& S : ShopItems)
	{
		if (!S.ItemData) continue;
        
		if (S.ItemData->MaxStackingCapacity <= 1)
		{
			// Non-stackable = 1 row chacun
			TotalRows++;
		}
		else
		{
			// Stackable = 1 row par type
			if (!TypeCounts.Contains(S.ItemData))
				TotalRows++;
			TypeCounts.FindOrAdd(S.ItemData)++;
		}
	}
    
	// Validation
	if (Item.ItemData->MaxStackingCapacity <= 1)
	{
		// Crée toujours une nouvelle row
		if (TotalRows >= 5) return;
	}
	else
	{
		if (TypeCounts.Contains(Item.ItemData))
		{
			// Row existante → vérifier max stack
			if (TypeCounts[Item.ItemData] >= Item.ItemData->MaxStackingCapacity) return;
		}
		else
		{
			// Nouvelle row
			if (TotalRows >= 5) return;
		}
	}
    
	//Inv->Gold -= Item.ItemData->Price;
	//Inv->OnRep_Gold();

	ShopItems.Add(Item);
	OnShopItemsChange.Broadcast();
	OnRep_ShopItems();
}

#pragma endregion
