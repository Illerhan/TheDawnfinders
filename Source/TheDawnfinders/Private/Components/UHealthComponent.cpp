#include "Components/UHealthComponent.h"

#include "AkGameplayStatics.h"
#include "WwiseEventTracking.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/CustomPlayerState.h"
#include "Components/UStaminaComponent.h"
#include "GameFramework/CustomGameMode.h"
#include "GameFramework/CustomPlayerController.h"
#include "GameFramework/CustomHUD.h"
#include "Widgets/UWorldPlayerWidget.h"
#include "Widgets/UWorldHealthBar.h"
#include "Widgets/UMainWidget.h"
#include "Widgets/USpectateWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerStart.h"


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

	PSCustom->SetMaxHealth(MaxHealth);
}


void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (OwnerController && OwnerController->IsLocalPlayerController())
	{
		ApplyCurse(DeltaTime);
	}

	if (GetOwner()->HasAuthority())
	{
		if (bIsPoisoned)
		{
			TakeDamage(PoisonDmg * DeltaTime, EVFXType::Poison);
		}
	}
	else {
		if (OwnerController && OwnerController->IsLocalPlayerController() && bIsPoisoned)
		{
			if (OwnerController && OwnerController->IsLocalPlayerController()) {
				if (!WorldHealthBar) {
					WorldHealthBar = IPlayerInterface::Execute_GetPlayerWidget(GetOwner())->GetHealthBar();
					WorldHealthBar->Setup(3);
				}
				WorldHealthBar->TakeDamage(CurrentHealth / CurrentMaxHealth, true);
			}
		}
	}

	ActualiseHurtPostProcess(DeltaTime);
	ActualisePoisonPostProcess(DeltaTime);

	if (GetOwner()->HasAuthority())
	{
		FallenLoseHP(DeltaTime);
	}
	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UHealthComponent::InitialiseComponent(float MaxHP, float MinMaxHP, float ReviveHP,float FallenSpeed,float DmgPoison, float FallenTime)
{ 
	CurrentHealth = MaxHP;
	CurrentMaxHealth = MaxHP;
	MaxHealth = MaxHP;
	MinimumMaxHP = MinMaxHP;
	MinReviveHP = ReviveHP;
	PoisonDmg = DmgPoison;
	FallenDuration = FallenTime;
	InjureDecreaseSpeed = FallenSpeed;

	// Gestion de la réplication de la santé
	if (!GetOwner()->HasAuthority()) {
		LocalChangeHealth();
		ServerChangeHealth(CurrentHealth);
	}
	else {
		ServerChangeHealth_Implementation(CurrentHealth);
	}
}

void UHealthComponent::ActualiseHurtPostProcess(float DeltaTime)
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!PawnOwner || !PawnOwner->IsLocallyControlled())
	{
		return;
	}

	if (bIsFallen && !bIsDead) {
		CurrentHurtVolumeStrength = FMath::Lerp(CurrentHurtVolumeStrength, PostProcessFallenOpacity, DeltaTime * 1.f);
	}
	else if (bIsDead) {
		CurrentHurtVolumeStrength = FMath::Lerp(CurrentHurtVolumeStrength, 0, DeltaTime * 1.f);
	}
	else {
		CurrentHurtVolumeStrength = FMath::Lerp(CurrentHurtVolumeStrength, FMath::Lerp(0, PostProcessMaxOpacity, 1 - ((CurrentHealth * 1.5f) / CurrentMaxHealth)), DeltaTime * 1.f);
	}
}


#pragma region Main Health Functions

void UHealthComponent::TakeDamage(float quantity, EVFXType VFXType)
{
	if (IsInvincible || bIsDead || bIsFallen) return;

	if (VFXType == Blood)
	{
		if (IPlayerInterface::Execute_GetCurrentPlayerState(GetOwner()) == EPlayerState::Blocking)
		{
			StaminaComponent->UseStamina(10.f);
			IPlayerInterface::Execute_DoCameraShake(GetOwner(), 0.6f);
			return;
		}

		// Visual effects + Invincibility Frames
		if (IPlayerInterface::Execute_GetCurrentPlayerState(GetOwner()) != EPlayerState::Fallen) {
			StartInvincibilityFrames_Implementation(0.2f);
			
			IPlayerInterface::Execute_DoCameraShake(GetOwner(), 1.f);
			IPlayerInterface::Execute_DoDamagePostProcess(GetOwner(), 1.f);
		}
	}
	if (OwnerController && OwnerController->IsLocalPlayerController()) {
		if (!WorldHealthBar) {
			WorldHealthBar = IPlayerInterface::Execute_GetPlayerWidget(GetOwner())->GetHealthBar();
			WorldHealthBar->Setup(3);
		}
		WorldHealthBar->TakeDamage((CurrentHealth - quantity) / CurrentMaxHealth, bIsPoisoned);
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - quantity, 0.0f, CurrentMaxHealth);

	// If Client
	if (!GetOwner()->HasAuthority()) 
	{
		LocalChangeHealth(); 
		Server_TakeDamage(quantity, nullptr);
	}
	else {
		if (CurrentHealth <= 0.0f)
		{
			Fallen();
		}

		ServerChangeHealth(CurrentHealth);
	}
}


void UHealthComponent::Server_TakeDamage_Implementation(float quantity, AActor* Origin)
{
	if (IsInvincible || bIsDead || bIsFallen) return;
	TakeDamage(quantity);
}


void UHealthComponent::Heal(float quantity)
{
	CurrentHealth += quantity;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0, CurrentMaxHealth);

	if (OwnerController && OwnerController->IsLocalPlayerController()) {
		if (!WorldHealthBar) {
			WorldHealthBar = IPlayerInterface::Execute_GetPlayerWidget(GetOwner())->GetHealthBar();
			WorldHealthBar->Setup(3);
		}
		WorldHealthBar->Heal(CurrentHealth / CurrentMaxHealth);
	}

	// If client
	if (!GetOwner()->HasAuthority())
	{
		Server_Heal(CurrentHealth);
		LocalChangeHealth();
		return;
	};

	// If server
	Server_Heal_Implementation(CurrentHealth);
}

void UHealthComponent::Server_Heal_Implementation(float quantity)
{
	CurrentHealth += quantity;
	CurrentHealth = FMath::Clamp(CurrentHealth, 0, CurrentMaxHealth);

	if (HealingSoundID)
	{
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (AudioDevice && HealingSoundID != AK_INVALID_PLAYING_ID)
		{
			AudioDevice->StopPlayingID(HealingSoundID);
			HealingSoundID = AK_INVALID_PLAYING_ID; // Reset
		}  
	}
	HealingSoundID = UAkGameplayStatics::PostEvent(HealingSound,GetOwner(),0,FOnAkPostEventCallback(), false);
	
	ServerChangeHealth_Implementation(CurrentHealth);
}

// Called to actualize the player's infos for every other clients
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

void UHealthComponent::RequestMaxHealthChange(float Amount)
{
	if (GetOwner()->HasAuthority())
	{
		// Si on est déjà le serveur, on applique direct
		ChangeCurrentMaxHealth(Amount);
	}
	else
	{
		// Si on est le client, on demande poliment au serveur
		ChangeCurrentMaxHealth_Implementation(Amount);
	}
}

void UHealthComponent::ChangeCurrentMaxHealth_Implementation(float NewValue)
{
	CurrentMaxHealth += NewValue;
	CurrentMaxHealth = FMath::Clamp(CurrentMaxHealth, 0, MaxHealth);
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;

	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC) return;

	if (!PC->PlayerState) return;

	ACustomPlayerState* PSCustom = Cast<ACustomPlayerState>(PC->PlayerState);
	PSCustom->SetCurrentMaxHealth(CurrentMaxHealth);
	CurseMaxHealth = PSCustom->GetCurrentMaxHealth();
	LocalChangeHealth();
	UE_LOG(LogTemp, Warning, TEXT("CurseMaxHealth = %f"), CurseMaxHealth);
}

#pragma endregion


#pragma region Curse

bool UHealthComponent::IsProtectedFromCurse() const
{
	return ProtectionZoneAmount > 0 || CurseZone < 1;
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
	if (IsProtectedFromCurse()) return;
	if (CurrentMaxHealth <= MinimumMaxHP) return;

	CurseMaxHealth = CurrentMaxHealth;
	CurrentMaxHealth -= MaxHealth * CurseRatio * DeltaTime;
	CurrentMaxHealth = FMath::Max(CurrentMaxHealth, MinimumMaxHP);
	CurseMaxHealth = CurrentMaxHealth;

	if (OwnerController && OwnerController->IsLocalPlayerController()) 
	{
		if (!WorldHealthBar) {
			WorldHealthBar = IPlayerInterface::Execute_GetPlayerWidget(GetOwner())->GetHealthBar();
			WorldHealthBar->Setup(3);
		}
		WorldHealthBar->ActualiseCurse(CurseMaxHealth / MaxHealth);
	}

	// Clamp current health if it exceeds new max
	if (CurrentHealth > CurrentMaxHealth)
	{
		CurrentHealth = CurrentMaxHealth;
	}

	ServerChangeHealth_Implementation(CurrentHealth);
}

#pragma endregion


#pragma region Poison

void UHealthComponent::StartPoisonEffects_Implementation()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;

	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC) return;

	if (!PC->PlayerState) return;

	ACustomPlayerState* PSCustom = Cast<ACustomPlayerState>(PC->PlayerState);
	PSCustom->StartPoison();
}

void UHealthComponent::EndPoisonEffects_Implementation()
{
	if (OwnerController && OwnerController->IsLocalPlayerController()) {
		if (!WorldHealthBar) {
			WorldHealthBar = IPlayerInterface::Execute_GetPlayerWidget(GetOwner())->GetHealthBar();
			WorldHealthBar->Setup(3);
		}
		WorldHealthBar->TakeDamage((CurrentHealth) / CurrentMaxHealth, false);
	}

	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;

	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC) return;

	if (!PC->PlayerState) return;

	ACustomPlayerState* PSCustom = Cast<ACustomPlayerState>(PC->PlayerState);
	PSCustom->EndPoison();
}

void UHealthComponent::ActualisePoisonPostProcess(float DeltaTime)
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	if (!PawnOwner || !PawnOwner->IsLocallyControlled())
	{
		return;
	}

	if(bIsPoisoned)
		CurrentPoisonVolumeStrength = FMath::Lerp(CurrentPoisonVolumeStrength, 1, DeltaTime * 1.f);

	else
		CurrentPoisonVolumeStrength = FMath::Lerp(CurrentPoisonVolumeStrength, 0, DeltaTime * 1.f);

	//PoisonMaterial->SetScalarParameterValue(TEXT("POISON-GeneralOpacity"), CurrentPoisonVolumeStrength);
}

void UHealthComponent::SetIsPoisoned_Implementation(bool isPoisoned)
{
	if (GetOwner()->HasAuthority())
		bIsPoisoned = isPoisoned;
	else
	{
		SetIsPoisoned_Implementation(isPoisoned);
	}

	if (isPoisoned) {
		StartPoisonEffects();
	}
	else {
		EndPoisonEffects();
	}
}

#pragma endregion


#pragma region Death / Revive


void UHealthComponent::Fallen()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;
	
	FString CurrentLevel = GetWorld()->GetMapName();
	if (CurrentLevel.Contains("Tutorial"))
	{
		AAPlayerCharacter* PC = Cast<AAPlayerCharacter>(GetOwner());
		if (!PC) return;
	
		ACustomPlayerController* Controller = Cast<ACustomPlayerController>(PC->GetController());
		if (!Controller) return;

		Controller->RespawnToCheckpoint();
		Heal(MaxHealth);

		return;
	}

	bIsFallen = true;
	bIsPoisoned = false;

	Multicast_DisplayFallen();
	Multicast_ActualiseFallen(1);
	
	IPlayerInterface::Execute_SetCurrentPlayerState(Owner, EPlayerState::Fallen, true);

	FallenTimer = FallenDuration;
	BreathSoundID = UAkGameplayStatics::PostEvent(FallenBreath,Owner,0,FOnAkPostEventCallback(), false);
	HeartBeatSoundID = UAkGameplayStatics::PostEvent(HeartBeatFallen,Owner,0,FOnAkPostEventCallback(), false);
}

void UHealthComponent::FallenLoseHP(float DeltaTime)
{
	if (!bIsFallen || bIsDead) return;
	if (!GetOwner()->HasAuthority()) return;

	Multicast_ActualiseFallen(FallenTimer / FallenDuration);

	FallenTimer -= DeltaTime;
	if (FallenTimer <= 0)
	{
		Die();
	}
}

void UHealthComponent::Die()
{
	bIsDead = true;

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return;

	Client_Die();

	Multicast_HideFallen();
	IPlayerInterface::Execute_SetCurrentPlayerState(Owner, EPlayerState::Dead, true);
	
	ACustomGameMode* GM = Cast<ACustomGameMode>(UGameplayStatics::GetGameMode(this));
	if (GM)
	{
		GM->AddDeadPlayer();
		GM->CheckAllDead();
	}
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice && BreathSoundID != AK_INVALID_PLAYING_ID)
	{
		AudioDevice->StopPlayingID(BreathSoundID);
		BreathSoundID = AK_INVALID_PLAYING_ID; // Reset
	}
	if (AudioDevice && HeartBeatSoundID != AK_INVALID_PLAYING_ID)
	{
		AudioDevice->StopPlayingID(HeartBeatSoundID);
		BreathSoundID = AK_INVALID_PLAYING_ID; // Reset
	}
}

void UHealthComponent::Client_Die_Implementation()
{
	if (OwnerController && OwnerController->IsLocalController()) {

		ACustomHUD* HUD = Cast<ACustomHUD>(OwnerController->GetHUD());
		UUSpectateWidget* SpectateWidget = HUD->MainWidget->GetSpectateWidget();
		SpectateWidget->DisplayWidget();

		HUD->MainWidget->EnterSpectate();

		AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OwnerController->GetPawn());
		SpectateWidget->ActualiseInfos(Player->PlayerIndex + 1, false);

		UE_LOG(LogTemp, Display, TEXT("DISPLAY SPECTATE"));
	}
}

void UHealthComponent::Multicast_HideFallen_Implementation()
{
	if (!WorldHealthBar) {
		WorldHealthBar = IPlayerInterface::Execute_GetPlayerWidget(GetOwner())->GetHealthBar();
		WorldHealthBar->Setup(3);
	}
	WorldHealthBar->HideFallenIcon();
}

void UHealthComponent::Multicast_ActualiseFallen_Implementation(float Percent)
{
	if (!WorldHealthBar) {
		WorldHealthBar = IPlayerInterface::Execute_GetPlayerWidget(GetOwner())->GetHealthBar();
		WorldHealthBar->Setup(3);
	}
	WorldHealthBar->ActualiseFallenIcon(Percent);
}

void UHealthComponent::Multicast_DisplayFallen_Implementation()
{
	if (!WorldHealthBar) {
		WorldHealthBar = IPlayerInterface::Execute_GetPlayerWidget(GetOwner())->GetHealthBar();
		WorldHealthBar->Setup(3);
	}
	WorldHealthBar->DisplayFallenIcon();
}

void UHealthComponent::Server_Revive_Implementation()
{
	if (!bIsFallen) return;
	CurrentMaxHealth = CurseMaxHealth;
	CurrentHealth = FMath::Clamp(MinReviveHP, MinReviveHP, CurseMaxHealth);
	ServerChangeHealth_Implementation(CurrentHealth);
	bIsFallen = false;

	if (OwnerController && OwnerController->IsLocalPlayerController() && bIsDead) {

		ACustomHUD* HUD = Cast<ACustomHUD>(OwnerController->GetHUD());
		UUSpectateWidget* SpectateWidget = HUD->MainWidget->GetSpectateWidget();
		SpectateWidget->HideWidget();
	}

	Multicast_HideFallen();
	Client_Revive();

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return;
	
	// Notify Player 
	if (AAPlayerCharacter* PC = Cast<AAPlayerCharacter>(Owner))
	{
		PC->OnRevive();
		bIsDead = false;
		ACustomGameMode* GM = Cast<ACustomGameMode>(UGameplayStatics::GetGameMode(this));
		if (GM)
		{
			GM->RemoveDeadPlayer();
			GM->CheckAllDead();
		}
	}
	
}

void UHealthComponent::Client_Revive_Implementation()
{
	if (OwnerController && OwnerController->IsLocalController()) {

		ACustomHUD* HUD = Cast<ACustomHUD>(OwnerController->GetHUD());
		UUSpectateWidget* SpectateWidget = HUD->MainWidget->GetSpectateWidget();
		SpectateWidget->HideWidget();

		HUD->MainWidget->ExitSpectate();

		UE_LOG(LogTemp, Display, TEXT("DISPLAY SPECTATE"));
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
	DOREPLIFETIME(UHealthComponent, bIsPoisoned);
}

void UHealthComponent::OnRep_IsFallen()
{
}

void UHealthComponent::OnRep_ProtectionZoneAmount()
{
}

#pragma endregion 
