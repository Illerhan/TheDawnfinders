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
	bReplicates = true;
}


// CALLED ON THE CLIENT TO ACTUALISE IT'S VALUES INSTANTLY 
void ACustomPlayerState::ActualiseLocalStamina(float current, float max)
{
	CurrentStamina = current;
	CurrentMaxStamina = max;

	OnInfoChangeLocal.ExecuteIfBound();
}


// CALLED ON THE CLIENT TO ACTUALISE IT'S VALUES INSTANTLY 
void ACustomPlayerState::ActualiseLocalHealth(float current, float max,float fixedMax)
{
	CurrentHealth = current;
	CurrentMaxHealth = max;
	MaxHealth = fixedMax;

	OnInfoChangeLocal.ExecuteIfBound();
}



// CALLED ON THE SERVER TO ACTUALISE FOR ALL
void ACustomPlayerState::ActualiseStamina(float current, float max)
{
	CurrentStamina = current;
	CurrentMaxStamina = max;

	OnRep_StaminaChange();
}

// CALLED ON THE SERVER TO ACTUALISE FOR ALL
void ACustomPlayerState::ActualiseHealth(float current, float max,float fixedMax)
{
	CurrentHealth = current;
	CurrentMaxHealth = max;
	MaxHealth = fixedMax;

	OnRep_StaminaChange();
}

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
