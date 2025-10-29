// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UStaminaComponent.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/CustomPlayerState.h"
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

	if(CurrentStamina < CurrentMaxStamina)
		ReloadStamina(DeltaTime * ReloadSpeed);
}

void UStaminaComponent::UseStamina(float quantity)
{
	CurrentStamina -= quantity;
	CurrentStamina = FMath::Clamp(CurrentStamina, 0, CurrentMaxStamina);

	CurrentReloadDelay = ReloadDelay;

	if (!GetOwner()->HasAuthority()) {
		ServerChangeStamina(CurrentStamina);
		return;
	}

	ServerChangeStamina_Implementation(CurrentStamina);
}

void UStaminaComponent::ServerChangeStamina_Implementation(float newStamina)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;

	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC) return;

	ACustomPlayerState* PSCustom = Cast<ACustomPlayerState>(PC->PlayerState);
	PSCustom->ActualiseStamina(newStamina, CurrentMaxStamina);
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

	if (!GetOwner()->HasAuthority()) {
		ServerChangeStamina(CurrentStamina);
		return;
	}

	ServerChangeStamina_Implementation(CurrentStamina);
}

void UStaminaComponent::ReloadStamina(float quantity)
{
	CurrentStamina += quantity;
	CurrentStamina = FMath::Clamp(CurrentStamina, 0, CurrentMaxStamina);

	if (!GetOwner()->HasAuthority()) {
		ServerChangeStamina(CurrentStamina);
		return;
	}

	ServerChangeStamina_Implementation(CurrentStamina);
}

