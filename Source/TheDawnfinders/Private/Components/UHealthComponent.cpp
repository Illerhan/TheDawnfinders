// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UHealthComponent.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/CustomPlayerState.h"
#include "Net/UnrealNetwork.h"


UHealthComponent::UHealthComponent()
{
	CurrentHealth = CurrentMaxHealth;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;
	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;
	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC || !PC->IsLocalController()) return;

	InitialiseComponent(100);
}

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UHealthComponent::InitialiseComponent(float MaxHealth)
{
	CurrentHealth = MaxHealth;
	CurrentMaxHealth = MaxHealth;

	// If is not the server
	if (!GetOwner()->HasAuthority()) {
		LocalChangeHealth();

		ServerChangeHealth(CurrentHealth);
		return;
	}

	// If is server
	ServerChangeHealth_Implementation(CurrentHealth);
}


#pragma region Main Health Functions


void UHealthComponent::TakeDamage(float quantity)
{
	CurrentHealth -= quantity;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0, CurrentMaxHealth);

	// If is not the server
	if (!GetOwner()->HasAuthority()) {
		LocalChangeHealth();

		ServerChangeHealth(CurrentHealth);
		return;
	}

	// If is server
	ServerChangeHealth_Implementation(CurrentHealth);

	if (CurrentHealth <= 0) {
		Die();
	}
}


void UHealthComponent::Die()
{
	IsDead = true;
	UE_LOG(LogTemp, Log, TEXT("La Moooooort"));
}


void UHealthComponent::Heal(float quantity)
{
	CurrentHealth += quantity;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0, CurrentMaxHealth);

	// If is not the server
	if (!GetOwner()->HasAuthority()) {
		LocalChangeHealth();

		ServerChangeHealth(CurrentHealth);
		return;
	}

	// If is server
	ServerChangeHealth_Implementation(CurrentHealth);
}

#pragma endregion


#pragma region Network Functions

// CALLED TO CHANGE THE UI INFORMATIONS FOR ALL THE OTHER SQUAD MEMBERS
void UHealthComponent::ServerChangeHealth_Implementation(float newHealth)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;

	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC) return;

	if (!PC->PlayerState) return;

	ACustomPlayerState* PSCustom = Cast<ACustomPlayerState>(PC->PlayerState);
	PSCustom->ActualiseHealth(newHealth, CurrentMaxHealth);
}


// CALLED TO CHANGE THE UI INFORMATIONS INSTANTLY IN LOCAL
void UHealthComponent::LocalChangeHealth()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;

	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC || !PC->IsLocalController()) return;

	if (!PC->PlayerState) return;

	ACustomPlayerState* PSCustom = Cast<ACustomPlayerState>(PC->PlayerState);
	PSCustom->ActualiseLocalHealth(CurrentHealth, CurrentMaxHealth);
}

#pragma endregion 