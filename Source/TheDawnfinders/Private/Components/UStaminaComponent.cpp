// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UStaminaComponent.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/CustomPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Actors/Player/APlayerCharacter.h"


UStaminaComponent::UStaminaComponent()
{
	CurrentStamina = CurrentMaxStamina;
}


void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;
	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;
	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC || !PC->IsLocalController()) return;

	AAPlayerCharacter* PlayerCharacter = Cast<AAPlayerCharacter>(PawnOwner);
	PlayerCharacter->InventoryComponent->OnOverloadCountChange.AddDynamic(this, &UStaminaComponent::ActualiseCurrentOverloadCount);

	InitialiseComponent(100, 10, 2, 0.1f, 15.f);
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


#pragma region Main Functions

void UStaminaComponent::UseStamina(float quantity)
{
	CurrentStamina -= quantity * (1 + CurrentOverloadCount);
	CurrentStamina = FMath::Clamp(CurrentStamina, 0, CurrentMaxStamina);

	CurrentReloadDelay = ReloadDelay;

	if (!GetOwner()->HasAuthority()) {

		ChangeLocalStamina();

		ServerChangeStamina(CurrentStamina);
		return;
	}

	ServerChangeStamina_Implementation(CurrentStamina);
}

// CALLED IN THE UPDATE AFTER A CERTAIN DELAY
void UStaminaComponent::ReloadStamina(float quantity)
{
	CurrentStamina += quantity;
	CurrentStamina = FMath::Clamp(CurrentStamina, 0, CurrentMaxStamina);

	if (!GetOwner()->HasAuthority()) {
		ChangeLocalStamina();

		ServerChangeStamina(CurrentStamina);
		return;
	}

	ServerChangeStamina_Implementation(CurrentStamina);
}


// CALLED TO CHANGE THE UI INFORMATIONS FOR ALL THE OTHER SQUAD MEMBERS
void UStaminaComponent::ServerChangeStamina_Implementation(float newStamina)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;

	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC) return;

	if (!PC->PlayerState) return;

	ACustomPlayerState* PSCustom = Cast<ACustomPlayerState>(PC->PlayerState);
	PSCustom->ActualiseStamina(newStamina, CurrentMaxStamina);
}


// CALLED TO CHANGE THE UI INFORMATIONS INSTANTLY IN LOCAL
void UStaminaComponent::ChangeLocalStamina()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;

	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC || !PC->IsLocalController()) return;

	if (!PC->PlayerState) return;

	ACustomPlayerState* PSCustom = Cast<ACustomPlayerState>(PC->PlayerState);
	PSCustom->ActualiseLocalStamina(CurrentStamina, CurrentMaxStamina);
}

#pragma endregion


#pragma region Others

bool UStaminaComponent::VerifyHasStamina()
{
	return CurrentStamina > 0;
}


void UStaminaComponent::InitialiseComponent(float MaxStamina, float ReloadSpd, float ReloadDl,float StaminaConsumptionR,float StaminaConsumptionD)
{
	CurrentStamina = MaxStamina;
	CurrentMaxStamina = MaxStamina;
	ReloadSpeed = ReloadSpd;
	ReloadDelay = ReloadDl;
	StaminaConsumptionRun = StaminaConsumptionR;
	StaminaConsumptionDodge = StaminaConsumptionD;

	// If is not server
	if (!GetOwner()->HasAuthority()) {
		ChangeLocalStamina();

		ServerChangeStamina(CurrentStamina);
		return;
	}

	// If is server
	ServerChangeStamina_Implementation(CurrentStamina);
}


void UStaminaComponent::ActualiseCurrentOverloadCount(int NewCount)
{
	CurrentOverloadCount = NewCount;
}

#pragma endregion