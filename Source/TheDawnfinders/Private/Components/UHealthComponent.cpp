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
	DOREPLIFETIME(UHealthComponent, IsDead);
	DOREPLIFETIME(UHealthComponent, CurseMaxHealth);
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
	
	// UE_LOG(LogTemp, Log, TEXT("[SERVER] Current Health: %f, CurrentMaxHealth: %f, CurrentMaxHealth: %f"),
	// 	CurrentHealth, CurrentMaxHealth, CurrentMaxHealth);
}


#pragma endregion

#pragma region Main Health Functions


void UHealthComponent::TakeDamage(float quantity)
{

	if (!GetOwner()->HasAuthority()) {
		CurrentHealth = FMath::Clamp(CurrentHealth - quantity, 0.0f, CurrentMaxHealth);
		LocalChangeHealth(); 

		Server_TakeDamage(quantity, nullptr);
		return;
	}
	
	CurrentHealth = FMath::Clamp(CurrentHealth - quantity, 0.0f, CurrentMaxHealth);

	// Optionally: call a function to update PlayerState UI on server side
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
	// Security checks (validate origin, invulnerability, etc) before applying
	TakeDamage(quantity); // now the server path executes
}

void UHealthComponent::Fallen()
{
	IsFallen = true;
	UE_LOG(LogTemp, Log, TEXT("La chuuuuuute"));

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return;

	// Notifier le PlayerCharacter
	if (AAPlayerCharacter* PC = Cast<AAPlayerCharacter>(Owner))
	{
		PC->OnFallen();
	}
	CurrentMaxHealth = MaxHealth;
	Heal(MaxHealth);
}

void UHealthComponent::Die()
{
	IsDead = true;
	UE_LOG(LogTemp, Log, TEXT("La Moooooort"));

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return;

	// Notifier le PlayerCharacter
	if (AAPlayerCharacter* PC = Cast<AAPlayerCharacter>(Owner))
	{
		PC->OnDeath();
	}
}

void UHealthComponent::Server_Revive_Implementation()
{
	if (!IsFallen) return;
	CurrentMaxHealth = CurseMaxHealth;
	CurrentHealth = FMath::Clamp(MinReviveHP,MinReviveHP,CurseMaxHealth);
	ServerChangeHealth_Implementation(CurrentHealth);
	IsFallen = false;
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return;

	// Notifier le PlayerCharacter
	if (AAPlayerCharacter* PC = Cast<AAPlayerCharacter>(Owner))
	{
		PC->OnRevive();
		IsDead = false;
	}
	
}

void UHealthComponent::Heal(float quantity)
{

	CurrentHealth += quantity;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0, CurrentMaxHealth);
	
	if (!GetOwner()->HasAuthority())
	{
		ServerChangeHealth(CurrentHealth);
		LocalChangeHealth();
		return;
	};

	// If is server
	ServerChangeHealth_Implementation(CurrentHealth);
}

#pragma endregion

#pragma region Death

void UHealthComponent::FallenLoseHP(float DeltaTime)
{
	if (!IsFallen) return;
	if (!GetOwner()->HasAuthority()) return;

	TakeDamage(InjureDecreaseSpeed * MaxHealth * DeltaTime);
	if (CurrentHealth<=0)
	{
		Die();	
	}
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

void UHealthComponent::OnRep_IsDead()
{
}

void UHealthComponent::OnRep_IsFallen()
{
}

#pragma endregion 
