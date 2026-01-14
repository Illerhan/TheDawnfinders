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

	OnInfoChangeLocal.ExecuteIfBound();
}

// CALLED ON THE SERVER TO ACTUALISE FOR ALL
void ACustomPlayerState::ActualiseHealth(float current, float max, float fixedMax)
{
	CurrentHealth = current;
	CurrentMaxHealth = max;
	MaxHealth = fixedMax;

	OnRep_StaminaChange();
}

void ACustomPlayerState::SetMaxHealth(float MaxHP)
{
	MaxHealth = MaxHP;
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

#pragma endregion