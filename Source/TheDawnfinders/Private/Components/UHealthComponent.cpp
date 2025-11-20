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

	if (GetOwner()->HasAuthority())
	{
		FallenLoseHP(DeltaTime);
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


#pragma region Main Health Functions

void UHealthComponent::TakeDamage(float quantity)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - quantity, 0.0f, CurrentMaxHealth);

	// If Client
	if (!GetOwner()->HasAuthority()) 
	{
		LocalChangeHealth(); 
		Server_TakeDamage(quantity, nullptr);
		return;
	}
	
	// If Server
	ServerChangeHealth(CurrentHealth);

	if (CurrentHealth <= 0.0f)
	{
		if (IsFallen)
		{
			Die();
			return;
		}
		Fallen();
	}
}


void UHealthComponent::Server_TakeDamage_Implementation(float quantity, AActor* Origin)
{
	TakeDamage(quantity);
}


void UHealthComponent::Heal(float quantity)
{
	CurrentHealth += quantity;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0, CurrentMaxHealth);

	// If client
	if (!GetOwner()->HasAuthority())
	{
		ServerChangeHealth(CurrentHealth);
		LocalChangeHealth();
		return;
	};

	// If server
	ServerChangeHealth_Implementation(CurrentHealth);
}


// Called to actualise the player's infos for every other clients
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


// Called to change the UI informations instantly in local
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


#pragma region Curse

bool UHealthComponent::IsProtectedFromCurse() const
{
	return ProtectionZoneAmount > 0;
}


void UHealthComponent::AddProtectionZone()
{
	if (GetOwner()->HasAuthority())
	{
		ProtectionZoneAmount++;
		OnRep_ProtectionZoneAmount();
	}
}

void UHealthComponent::RemoveProtectionZone()
{
	if (GetOwner()->HasAuthority() && ProtectionZoneAmount > 0)
	{
		ProtectionZoneAmount--;
		OnRep_ProtectionZoneAmount();
	}
}


void UHealthComponent::ApplyCurse(float DeltaTime)
{
	if (IsFallen || IsDead) return;
	if (!GetOwner()->HasAuthority()) return;
	if (IsProtectedFromCurse()) return;
	if (CurrentMaxHealth <= MinimumMaxHP) return;

	CurrentMaxHealth -= MaxHealth * CurseRatio * DeltaTime;
	CurrentMaxHealth = FMath::Max(CurrentMaxHealth, MinimumMaxHP);
	CurseMaxHealth = CurrentMaxHealth;

	// Clamp current health if it exceeds new max
	if (CurrentHealth > CurrentMaxHealth)
	{
		CurrentHealth = CurrentMaxHealth;
	}

	ServerChangeHealth_Implementation(CurrentHealth);
}


#pragma endregion


#pragma region Death / Revive

void UHealthComponent::Fallen()
{
	IsFallen = true;

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return;

	// Notify Player 
	if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Owner))
	{
		Player->OnFallen();
	}

	CurrentMaxHealth = MaxHealth;
	Heal(MaxHealth);
}

void UHealthComponent::FallenLoseHP(float DeltaTime)
{
	if (!IsFallen) return;
	if (!GetOwner()->HasAuthority()) return;

	TakeDamage(InjureDecreaseSpeed * MaxHealth * DeltaTime);
	if (CurrentHealth <= 0)
	{
		Die();
	}
}

void UHealthComponent::Die()
{
	IsDead = true;

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return;

	// Notify Player 
	if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Owner))
	{
		Player->OnDeath();
	}
}

void UHealthComponent::Server_Revive_Implementation()
{
	if (!IsFallen) return;
	CurrentMaxHealth = CurseMaxHealth;
	CurrentHealth = FMath::Clamp(MinReviveHP, MinReviveHP, CurseMaxHealth);
	ServerChangeHealth_Implementation(CurrentHealth);
	IsFallen = false;
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return;

	// Notify Player 
	if (AAPlayerCharacter* PC = Cast<AAPlayerCharacter>(Owner))
	{
		PC->OnRevive();
		IsDead = false;
	}
}

#pragma endregion


#pragma region Network Functions

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, ProtectionZoneAmount);
	DOREPLIFETIME(UHealthComponent, CurrentHealth);
	DOREPLIFETIME(UHealthComponent, CurrentMaxHealth);
	DOREPLIFETIME(UHealthComponent, IsDead);
	DOREPLIFETIME(UHealthComponent, CurseMaxHealth);
}

void UHealthComponent::OnRep_IsDead()
{
}

void UHealthComponent::OnRep_IsFallen()
{
}

void UHealthComponent::OnRep_ProtectionZoneAmount()
{
}


#pragma endregion 
