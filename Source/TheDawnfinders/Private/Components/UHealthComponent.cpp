// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UHealthComponent.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/CustomPlayerState.h"
#include "Net/UnrealNetwork.h"


UHealthComponent::UHealthComponent()
{
	CurrentMaxHealth = 100.0f;
	CurrentHealth = CurrentMaxHealth;
	MaxHealth = 100.0f;
	PrimaryComponentTick.bCanEverTick = true;
	bAllowConcurrentTick = true;
	SetIsReplicatedByDefault(true);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, ProtectionZoneAmount);
	DOREPLIFETIME(UHealthComponent, CurrentHealth);
	DOREPLIFETIME(UHealthComponent, CurrentMaxHealth);
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
	ACustomPlayerState* PSCustom = Cast<ACustomPlayerState>(PC->PlayerState);
	if (!PSCustom) return;
	PSCustom->MaxHealth = MaxHealth;
}

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (GetOwner()->HasAuthority())
	{
		ApplyCurse(DeltaTime);
	}
	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UHealthComponent::InitialiseComponent(float MaxHP)
{
	CurrentHealth = MaxHP;
	CurrentMaxHealth = MaxHP;
	MaxHealth = MaxHP;

	// If is not the server
	if (!GetOwner()->HasAuthority()) {
		LocalChangeHealth();

		ServerChangeHealth(CurrentHealth);
		return;
	}

	// If is server
	ServerChangeHealth_Implementation(CurrentHealth);
}

#pragma region Curse

void UHealthComponent::OnRep_ProtectionZoneAmount()
{
	// You can log for debugging
	UE_LOG(LogTemp, Warning, TEXT("[CLIENT] Protection zone amount replicated: %d for %s"), 
		ProtectionZoneAmount, 
		*GetOwner()->GetName());

	// Optionally update local UI or FX here
	// Example: if (ProtectionZoneAmount > 0) ShowProtectionFX(); else HideProtectionFX();
}

bool UHealthComponent::IsProtectedFromCurse() const
{
	return ProtectionZoneAmount > 0;
}

void UHealthComponent::AddProtectionZone()
{
	if (GetOwner()->HasAuthority())
	{
		ProtectionZoneAmount++;
		// If need to add more logic
		OnRep_ProtectionZoneAmount();
	}
}

void UHealthComponent::RemoveProtectionZone()
{
	if (GetOwner()->HasAuthority() && ProtectionZoneAmount > 0)
	{
		ProtectionZoneAmount--;
		// If need to add more logic
		OnRep_ProtectionZoneAmount();
	}
}


void UHealthComponent::ApplyCurse(float DeltaTime)
{
	// This should ONLY be called on the server now
	if (!GetOwner()->HasAuthority()) return;
	
	if (IsProtectedFromCurse()) return;
	if (CurrentMaxHealth <= MinimumMaxHP) return;

	CurrentMaxHealth -= MaxHealth * CurseRatio * DeltaTime;
	CurrentMaxHealth = FMath::Max(CurrentMaxHealth, MinimumMaxHP);
	
	// Clamp current health if it exceeds new max
	if (CurrentHealth > CurrentMaxHealth)
	{
		CurrentHealth = CurrentMaxHealth;
	}

	ServerChangeHealth_Implementation(CurrentHealth);
	
	UE_LOG(LogTemp, Log, TEXT("[SERVER] Current Health: %f, CurrentMaxHealth: %f, CurrentMaxHealth: %f"),
		CurrentHealth, CurrentMaxHealth, CurrentMaxHealth);
}


#pragma endregion

#pragma region Main Health Functions


void UHealthComponent::TakeDamage(float quantity)
{

	if (!GetOwner()->HasAuthority()) return;
	
	CurrentHealth -= quantity;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0, CurrentMaxHealth);

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

	if (!GetOwner()->HasAuthority()) return;
	
	CurrentHealth += quantity;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0, CurrentMaxHealth);

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
	PSCustom->ActualiseHealth(newHealth, CurrentMaxHealth, MaxHealth);
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
	PSCustom->ActualiseLocalHealth(CurrentHealth, CurrentMaxHealth, MaxHealth);
}

#pragma endregion 