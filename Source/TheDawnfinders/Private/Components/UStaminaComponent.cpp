// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UStaminaComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"

UStaminaComponent::UStaminaComponent()
{
	CurrentStamina = CurrentMaxStamina;
}

void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentReloadDelay > 0) {
		CurrentReloadDelay -= DeltaTime;
		return;
	}

	ReloadStamina(DeltaTime * ReloadSpeed);
}

void UStaminaComponent::UseStamina(float quantity)
{
	CurrentStamina -= quantity;
	CurrentStamina = FMath::Clamp(CurrentStamina, 0, CurrentMaxStamina);

	CurrentReloadDelay = ReloadDelay;

	OnStaminaChange.Broadcast(CurrentStamina, CurrentMaxStamina);
}

bool UStaminaComponent::VerifyHasStamina()
{
	return CurrentStamina > 0;
}

void UStaminaComponent::InitialiseComponent(float MaxStamina, float ReloadSpd, float ReloadDl)
{
	CurrentStamina = MaxStamina;
	CurrentMaxStamina = MaxStamina;
	ReloadSpeed = ReloadSpd;
	ReloadDelay = ReloadDl;

	OnStaminaChange.Broadcast(CurrentStamina, CurrentMaxStamina);
}

void UStaminaComponent::ReloadStamina(float quantity)
{
	CurrentStamina += quantity;
	CurrentStamina = FMath::Clamp(CurrentStamina, 0, CurrentMaxStamina);

	OnStaminaChange.Broadcast(CurrentStamina, CurrentMaxStamina);
}

