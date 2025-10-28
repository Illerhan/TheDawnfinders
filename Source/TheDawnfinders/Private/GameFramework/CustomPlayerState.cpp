// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/CustomPlayerState.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Components/UStaminaComponent.h"

void ACustomPlayerState::BeginPlay()
{
	APlayerController* PC = GetPlayerController();
	if (!PC) return;
	
	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn) return;
	
	AAPlayerCharacter* PlayerCharacter = Cast<AAPlayerCharacter>(PlayerPawn);
	if (!PlayerCharacter || !PlayerCharacter->StaminaComponent) return;
	
	PlayerCharacter->StaminaComponent->OnStaminaChange.AddUniqueDynamic(this, &ACustomPlayerState::ActualiseStamina);
}

void ACustomPlayerState::ActualiseStamina(float current, float max)
{
	CurrentStamina = current;
	CurrentMaxStamina = max;

	//OnInfoChange.Broadcast(CurrentHealth, CurrentMaxHealth, CurrentStamina, CurrentMaxStamina);
	OnInfoChange.Broadcast();
}

void ACustomPlayerState::ActualiseHealth(float current, float max)
{
	CurrentHealth = current;
	CurrentMaxHealth = max;

	//OnInfoChange.Broadcast(CurrentHealth, CurrentMaxHealth, CurrentStamina, CurrentMaxStamina);
	OnInfoChange.Broadcast();
}
