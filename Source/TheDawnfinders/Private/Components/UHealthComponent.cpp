// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UHealthComponent.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/CustomPlayerState.h"
#include "Components/UStaminaComponent.h"
#include "Kismet/GameplayStatics.h"
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

	StaminaComponent = (Cast<AAPlayerCharacter>(PawnOwner))->StaminaComponent;

	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC || !PC->IsLocalController()) return;

	OwnerController = PC;

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

	if (CurseVolume) ActualiseCursePostProcess(DeltaTime);
	

	if (GetOwner()->HasAuthority())
	{
		FallenLoseHP(DeltaTime);
	}
	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UHealthComponent::InitialiseComponent
	(float MaxHP, float MinMaxHP, float ReviveHP,float InjureSpeed, float CurseRate)
{
	CurrentHealth = MaxHP;
	CurrentMaxHealth = MaxHP;
	MaxHealth = MaxHP;
	MinimumMaxHP = MinMaxHP;
	MinReviveHP = ReviveHP;
	InjureDecreaseSpeed = InjureSpeed;
	CurseRatio = CurseRate;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), "PPCurse", FoundActors);

	for (AActor* Actor : FoundActors)
	{
		CurseVolume = (APostProcessVolume*)Actor;
	}

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
	if (IsInvincible) return;

	if (IPlayerInterface::Execute_GetCurrentPlayerState(GetOwner()) == EPlayerState::Blocking)
	{
		StaminaComponent->UseStamina(10.f);
		IPlayerInterface::Execute_DoCameraShake(GetOwner(), 0.5f);
		return;
	}
	
	CurrentHealth = FMath::Clamp(CurrentHealth - quantity, 0.0f, CurrentMaxHealth);

	// Visual effects + Invincibility Frames
	if (IPlayerInterface::Execute_GetCurrentPlayerState(GetOwner()) != EPlayerState::Fallen) {
		StartInvincibilityFrames_Implementation(1.f);

		IPlayerInterface::Execute_DoCameraShake(GetOwner(), 1.f);
		IPlayerInterface::Execute_DoDamagePostProcess(GetOwner(), 1.f);
	}

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
		if (bIsFallen)
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
	if (bIsFallen || bIsDead) return;
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

void UHealthComponent::ActualiseCursePostProcess(float DeltaTime)
{
	if (!OwnerController) return;

	if (IsProtectedFromCurse()) {
		CurseVolume->BlendWeight = FMath::Lerp(CurseVolume->BlendWeight, 0.0f, DeltaTime * 1.5f);
	}
	else {
		CurseVolume->BlendWeight = FMath::Lerp(CurseVolume->BlendWeight, 1.0f, DeltaTime * 1.5f);
	}
}


#pragma endregion


#pragma region Death / Revive

void UHealthComponent::Fallen()
{
	bIsFallen = true;

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
	if (!bIsFallen) return;
	if (!GetOwner()->HasAuthority()) return;

	TakeDamage(InjureDecreaseSpeed * MaxHealth * DeltaTime);
	if (CurrentHealth <= 0)
	{
		Die();
	}
}

void UHealthComponent::Die()
{
	bIsDead = true;

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
	if (!bIsFallen) return;
	CurrentMaxHealth = CurseMaxHealth;
	CurrentHealth = FMath::Clamp(MinReviveHP, MinReviveHP, CurseMaxHealth);
	ServerChangeHealth_Implementation(CurrentHealth);
	bIsFallen = false;
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return;

	// Notify Player 
	if (AAPlayerCharacter* PC = Cast<AAPlayerCharacter>(Owner))
	{
		PC->OnRevive();
		bIsDead = false;
	}
}

#pragma endregion


#pragma region Invincibility

void UHealthComponent::StartInvincibilityFrames_Implementation(float Duration)
{
	if (IsInvincible) return;

	IsInvincible = true;

	GetWorld()->GetTimerManager().SetTimer(
		InvincibilityTimerHandle,              
		this,                       
		&UHealthComponent::EndInvincibilityFrames,
		Duration,
		false                        
	);
}

void UHealthComponent::EndInvincibilityFrames_Implementation()
{
	IsInvincible = false;
}


#pragma endregion


#pragma region Network Functions

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, ProtectionZoneAmount);
	DOREPLIFETIME(UHealthComponent, CurrentHealth);
	DOREPLIFETIME(UHealthComponent, CurrentMaxHealth);
	DOREPLIFETIME(UHealthComponent, bIsDead);
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
