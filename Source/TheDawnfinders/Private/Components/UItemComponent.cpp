// Fill out your copyright notice...

#include "Components/UItemComponent.h"

#include "AkGameplayStatics.h"
#include "FrameTypes.h"
#include "Actors/Interactibles/Litter.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Actors/Player/AThrowableObject.h"
#include "Actors/Traps/ATrapBase.h"
#include "Components/UHealthComponent.h"
#include "Components/UInventoryComponent.h"
#include "Components/UStaminaComponent.h"
#include "GameFramework/CustomHUD.h"
#include "Widgets/UMainWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/IPlayer.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Evaluation/Blending/MovieSceneBlendingActuatorID.h"

#include "Perception/AISense_Hearing.h"


UItemComponent::UItemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UItemComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* PlayerActor = GetOwner();
	if (!PlayerActor) return;

	PlayerCharacter = Cast<AAPlayerCharacter>(PlayerActor);

	HealthComponent = PlayerCharacter->HealthComponent;
	InventoryComponent = PlayerCharacter->InventoryComponent;
	StaminaComponent = PlayerCharacter->StaminaComponent;

	InventoryComponent->OnInventoryChange.AddUniqueDynamic(this, &UItemComponent::SetEquippedItem);

	WeaponDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_Weapons.DT_Weapons"));
	if (!WeaponDataTable)
		UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable"));

	WeaponActionsDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_WeaponActions.DT_WeaponActions"));
	if (!WeaponActionsDataTable)
		UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable"));

	WeaponTypeActionsDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_WeaponTypes.DT_WeaponTypes"));
	if (!WeaponActionsDataTable)
		UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable"));
}


void UItemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ShootDelayTimer > 0) {
		ShootDelayTimer -= DeltaTime;
	}

	if (!GetOwner()) return;

	if (AttackInertiaTimer > 0) {
		ManageAttackInertia(DeltaTime);
	}

	ActualiseUseProgress(DeltaTime);

	if (bIsAiming && !bIsReloading) {
		ActualiseAim(DeltaTime);
	}
	else if (bIsReloading) {
		TimerReload -= DeltaTime;
		IPlayerInterface::Execute_ShowProgress(PlayerCharacter, TimerReload);

		if (TimerReload <= 0) {
			CompleteReload();
		}
	}
}


#pragma region Equip

void UItemComponent::SetEquippedItem(const TArray<FInventorySlot>& Slots, int CurrentSlotIndex)
{
	if (EquippedItem.CurrentInfos.ItemData != nullptr && EquippedItem.CurrentInfos.ItemData->ItemType == EItemType::Equipment)
	{
		UnequipWeapon();
	}

	EquippedItem = Slots[CurrentSlotIndex];

	if (EquippedItem.CurrentInfos.ItemData != nullptr && EquippedItem.CurrentInfos.ItemData->ItemType == EItemType::Equipment)
	{
		EquipWeapon();
	}
}

UItemData* UItemComponent::GetEquippedItem()
{
	return EquippedItem.CurrentInfos.ItemData;
}

void UItemComponent::EquipWeapon()
{
	if (GetOwner()->Implements<UPlayerInterface>())
	{
		IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(GetOwner());
		PlayerInterface->SetEquippedMesh_Implementation(EquippedItem.CurrentInfos.ItemData->ItemMesh, EquippedItem.CurrentInfos.ItemData->bIsRangedWeapon);

		CurrentWeaponData = *WeaponDataTable->FindRow<FWeaponInfos>(EquippedItem.CurrentInfos.ItemData->WeaponDataTableRow, " ");
	}
}

void UItemComponent::UnequipWeapon()
{
	if (GetOwner()->Implements<UPlayerInterface>())
	{
		IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(GetOwner());
		PlayerInterface->SetEquippedMesh_Implementation(nullptr, false);
	}
}

#pragma endregion


#pragma region Timers

void UItemComponent::ApplyEffectLogic(EConsumableEffectType EffectType, bool bActivate)
{
	if (!PlayerCharacter) return;

	switch (EffectType)
	{
	case EConsumableEffectType::Adrenaline:
		if (StaminaComponent)
			StaminaComponent->SetStaminaReduced(bActivate);
		UE_LOG(LogTemp,Warning,TEXT("Stamina : %hhd"),StaminaComponent->IsStaminaReduced());
		break;
	case EConsumableEffectType::Protector:
		if (PlayerCharacter->LightComponent)
		{
			bActivate?PlayerCharacter->LightComponent->TurnLightOn():PlayerCharacter->LightComponent->TurnLightOff();
		}
		break;
	case EConsumableEffectType::Poison:
		if (HealthComponent)
		{
			HealthComponent->SetIsPoisoned(bActivate);
		}
	}
}

void UItemComponent::OnEffectExpired(EConsumableEffectType EffectType)
{
	ApplyEffectLogic(EffectType, false);

	// 2. Nettoyage de la Map
	if (ActiveEffectsTimers.Contains(EffectType))
	{
		ActiveEffectsTimers.Remove(EffectType);
	}
}

void UItemComponent::Server_StartTimedEffect_Implementation(EConsumableEffectType EffectType, float Duration)
{
	if (Duration <= 0.f) return;

	// 1. On applique l'effet sur le serveur (sera répliqué aux clients via les stats)
	ApplyEffectLogic(EffectType, true);

	// 2. Configuration du Timer avec paramètre
	FTimerDelegate TimerDel;
	TimerDel.BindUObject(this, &UItemComponent::OnEffectExpired, EffectType);

	// On lance le timer. S'il existe déjà, il est réinitialisé (Refresh du buff)
	GetWorld()->GetTimerManager().SetTimer(
		ActiveEffectsTimers.FindOrAdd(EffectType), 
		TimerDel, 
		Duration, 
		false
	);
}

#pragma endregion


#pragma region Use - Main Action

void UItemComponent::DoMainAction()
{
	if (EquippedItem.CurrentInfos.ItemData == nullptr) return;

	if (EquippedItem.CurrentInfos.ItemData->ItemType == EItemType::Consumable)
	{
		if (EquippedItem.CurrentInfos.ItemData->NeededHoldDuration != 0)
		{
			if (EquippedItem.CurrentInfos.ItemData->ConsumableEffectType == EConsumableEffectType::Revive)
			{
				float SearchRadius = 300.f;
				TArray<AAPlayerCharacter*> DeadPlayers = GetNearbyPlayers(SearchRadius, true);
				if (DeadPlayers.Num() == 0) return;

				Ally = DeadPlayers[0]; // On cible le premier mort à portée
			}
			
			if (EquippedItem.CurrentInfos.ItemData->NeededHoldDuration != 0 && !EquippedItem.CurrentInfos.ItemData->ContextualUse)
			{
				ItemUseTimer = EquippedItem.CurrentInfos.ItemData->NeededHoldDuration;
				bIsUsingItem = true;
				
				Multi_PlayHealSound();
				IPlayerInterface::Execute_SetCurrentPlayerState(GetOwner(), EPlayerState::UsingEquipment, false);

				return;
			}
		}
		UseConsumable();
		return;
	}

	if (EquippedItem.CurrentInfos.ItemData->ItemType == EItemType::Equipment) 
	{
		if (EquippedItem.CurrentInfos.ItemData->bIsRangedWeapon && bIsAiming) 
		{
			Shoot();
			return;
		}
		return;
	}
}

void UItemComponent::Multi_PlayHealSound_Implementation()
{
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
				
}

void UItemComponent::ActualiseUseProgress(float DeltaTime)
{
	if (!bIsUsingItem) return;

	if (ItemUseTimer > 0)
	{
		ItemUseTimer -= DeltaTime;

		if (GetOwner()->Implements<UPlayerInterface>())
		{
			IPlayerInterface::Execute_ShowProgress(GetOwner(), ItemUseTimer);
		}
		return;
	}

	UseConsumable();
}

void UItemComponent::UseConsumable()
{
	bIsUsingItem = false;

	if (GetOwner()->Implements<UPlayerInterface>())
	{
		IPlayerInterface::Execute_HideProgress(GetOwner());
		IPlayerInterface::Execute_SetCurrentPlayerState(GetOwner(), EPlayerState::None, false);
	}

	if (EquippedItem.CurrentInfos.ItemData == nullptr) return;

	if (EquippedItem.CurrentInfos.ItemData->UseConsumableMontage)
	{
		if (GetOwner()->HasAuthority())
			PlayerCharacter->ServerPlayMontage_Implementation(EquippedItem.CurrentInfos.ItemData->UseConsumableMontage, 1.0f);

		else
			PlayerCharacter->ServerPlayMontage(EquippedItem.CurrentInfos.ItemData->UseConsumableMontage, 1.0f);
	}

	float Amount = 0;

	switch (EquippedItem.CurrentInfos.ItemData->ConsumableEffectType)
	{
		case EConsumableEffectType::Heal:
			HealthComponent->Heal(EquippedItem.CurrentInfos.ItemData->ConsumableEffectPower);
			InventoryComponent->RemoveCurrentItem();
			break;

		case EConsumableEffectType::Inhale:
			HealthComponent->ChangeCurrentMaxHealth(EquippedItem.CurrentInfos.ItemData->ConsumableEffectPower);
			InventoryComponent->RemoveCurrentItem();
			break;
		
		case EConsumableEffectType::Adrenaline:
			Server_StartTimedEffect(EConsumableEffectType::Adrenaline,EquippedItem.CurrentInfos.ItemData->ConsumableEffectPower);
			InventoryComponent->RemoveCurrentItem();
			break;

		case EConsumableEffectType::Protector:
			Server_StartTimedEffect(EConsumableEffectType::Protector,EquippedItem.CurrentInfos.ItemData->ConsumableEffectPower);
			InventoryComponent->RemoveCurrentItem();
			break;

		case EConsumableEffectType::Antidote:
			HealthComponent->SetIsPoisoned(false);
			InventoryComponent->RemoveCurrentItem();
			break;
	
		case EConsumableEffectType::ThrowObject:
			{
				if (!IsPreviewingThrow) return;

				Server_ThrowItem(EquippedItem.CurrentInfos.ItemData, CurrentThrowDirection);

				InventoryComponent->RemoveCurrentItem();
				StopPreviewThrow();
			}
			break;

		case EConsumableEffectType::Revive:
			if (!PlayerCharacter || !Ally) return;
			if (!PlayerCharacter->HasAuthority())
			{
				ServerRequestRevive(Ally);
			}
			else
			{
				PerformeRevive(Ally);
			}
			break;

		case EConsumableEffectType::Refile:
			if (!PlayerCharacter) return;
			Amount = EquippedItem.CurrentInfos.ItemData->ConsumableEffectPower;
			if (PlayerCharacter->InteractionComponent->GetNearestInteractible())
			{
				ALitter* Litter = Cast<ALitter>(PlayerCharacter->InteractionComponent->GetNearestInteractible());
				if (Litter)
				{
					if (!PlayerCharacter->HasAuthority())
					{
						Litter->LightComponent->Server_RequestFuelUpdate();
					}
					else
					{
						Litter->LightComponent->FuelUpdate();
					}
				}
			}
			InventoryComponent->RemoveCurrentItem();
			break;

		case EConsumableEffectType::PlaceTrap:
		{
			if (!PlayerCharacter) return;
			FVector  SpawnLoc = PlayerCharacter->GetActorLocation() + PlayerCharacter->GetMesh()->GetRightVector() * 80.f + FVector(0, 0, -50.f);
			FRotator SpawnRot = PlayerCharacter->GetActorRotation();
			FActorSpawnParameters Params; Params.Owner = PlayerCharacter; Params.Instigator = PlayerCharacter;
			ATrapBase* NewTrap = GetWorld()->SpawnActor<ATrapBase>(EquippedItem.CurrentInfos.ItemData->PlacedTrap,
				SpawnLoc, SpawnRot, Params);
			if (NewTrap) InventoryComponent->RemoveCurrentItem();
			break;
		}

		case EConsumableEffectType::OpenMap :
			if (!PlayerCharacter) return;
			APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			if (!PC) break;

			AHUD* HUD = PC->GetHUD();
			Cast<ACustomHUD>(HUD)->MainWidget->OpenMap(EquippedItem.CurrentInfos.ItemData->MapSprite);
			break;
	}
}


void UItemComponent::StopMainAction()
{
	if (EquippedItem.CurrentInfos.ItemData == nullptr) return;
	if (EquippedItem.CurrentInfos.ItemData->ItemType == EItemType::Equipment) return;
	if (!bIsUsingItem) return;

	// Throw throwable on release
	if (IsPreviewingThrow) {
		UseConsumable();
	}

	if (GetOwner()->Implements<UPlayerInterface>())
	{
		IPlayerInterface::Execute_RequestStateChange(GetOwner(), EPlayerState::None, false);
		IPlayerInterface::Execute_HideProgress(GetOwner());
	}

	bIsUsingItem = false;

	if (!PlayerCharacter) return;
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC) return;

	AHUD* HUD = PC->GetHUD();
	Cast<ACustomHUD>(HUD)->MainWidget->CloseMap();
}

#pragma endregion


#pragma region Use - Secondary Action

void UItemComponent::DoSecondaryAction()
{
	if (EquippedItem.CurrentInfos.ItemData == nullptr) return;
	if (EquippedItem.CurrentInfos.ItemData->ItemType == EItemType::Valuable) return;
	if (PlayerCharacter->GetCurrentPlayerState_Implementation() == EPlayerState::Dodging) return;

	if (EquippedItem.CurrentInfos.ItemData->ConsumableEffectType == EConsumableEffectType::ThrowObject)
	{
		StartPreviewThrow();
		IPlayerInterface::Execute_SetCurrentPlayerState(PlayerCharacter, EPlayerState::UsingEquipment, false);
		return;
	}

	if (EquippedItem.CurrentInfos.ItemData->ItemType == EItemType::Equipment) 
	{
		if (EquippedItem.CurrentInfos.ItemData->bIsRangedWeapon)
		{
			StartAim();
			return;
		}
		return;
	}
}

void UItemComponent::StopSecondaryAction()
{
	if (EquippedItem.CurrentInfos.ItemData == nullptr) return;

	if (EquippedItem.CurrentInfos.ItemData->ItemType == EItemType::Equipment) 
	{
		if (bIsAiming)
		{
			StopAim();
			return;
		}
		return;
	}

	if (EquippedItem.CurrentInfos.ItemData->ConsumableEffectType == EConsumableEffectType::ThrowObject)
	{
		StopPreviewThrow();
		IPlayerInterface::Execute_SetCurrentPlayerState(PlayerCharacter, EPlayerState::None, false);
		return;
	}
}

#pragma endregion


#pragma region Throwable Items

void UItemComponent::StartPreviewThrow()
{
	if (IsPreviewingThrow) return;

	IsPreviewingThrow = true;

	ThrowPreviewTimer = 0;
}

void UItemComponent::Server_ThrowItem_Implementation(UItemData* Data, FVector Direc)
{
	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	Params.Instigator = Cast<APawn>(GetOwner());

	AThrowableObject* ThrowedObject =
		GetWorld()->SpawnActor<AThrowableObject>(Data->ThrowedObjectClass,
			PlayerCharacter->ThrowStartPosRef->GetComponentLocation(), FRotator(0.f, 0.f, 0.f), Params);

	if (ThrowedObject)
	{
		ThrowedObject->Initialise(Data, GetOwner());
		ThrowedObject->DoStartImpulse(Direc, ThrowStrength);
	}
}


void UItemComponent::ActualisePreviewThrow(FVector AimInput)
{
	if (!IsPreviewingThrow) return;

	if (EquippedItem.CurrentInfos.ItemData == nullptr || EquippedItem.CurrentInfos.ItemData->ThrowedObjectClass == nullptr)
	{
		StopPreviewThrow();
		return;
	}

	CurrentThrowDirection = FMath::Lerp(CurrentThrowDirection, AimInput + FVector(0, 0, 0.5f), GetWorld()->GetDeltaSeconds() * 5.f);

	AThrowableObject* Throwable = EquippedItem.CurrentInfos.ItemData->ThrowedObjectClass->GetDefaultObject<AThrowableObject>();
	OnThrowPreviewDisplay.Broadcast(CurrentThrowDirection, ThrowStrength);
}


void UItemComponent::StopPreviewThrow()
{
	if (!IsPreviewingThrow) return;

	IsPreviewingThrow = false;
	ThrowPreviewTimer = 0;

	OnThrowHidePreview.Broadcast();
}

#pragma endregion


#pragma region Use Weapon

void UItemComponent::DoLightAttack()
{
	if (EquippedItem.CurrentInfos.ItemData == nullptr) return;
	if (EquippedItem.CurrentInfos.ItemData->ItemType != EItemType::Equipment) return;
	if (EquippedItem.CurrentInfos.ItemData->bIsRangedWeapon) return;

	if (IPlayerInterface::Execute_GetCurrentPlayerState(GetOwner()) == EPlayerState::UsingEquipment)
	{
		PressedAttackInput = true;
		PressedHeavyAttackInput = false;
		return;
	}

	if (!StaminaComponent->VerifyHasStamina()) return;

	AlreadyHitActors.Reset();
	PlayerCharacter->StartAutoLock(5.f);

	FWeaponInfos* WeaponData = WeaponDataTable->FindRow<FWeaponInfos>(EquippedItem.CurrentInfos.ItemData->WeaponDataTableRow, " ");
	FWeaponTypesData* WeaponTypeActions = WeaponTypeActionsDataTable->FindRow<FWeaponTypesData>(WeaponData->WeaponTypeName, " ");

	if (PressedAttackInput)
	{
		PressedAttackInput = false;

		if (++ComboIndex >= WeaponTypeActions->LightComboActionNames.Num())
		{
			ComboIndex = 0;
		}
	}
	else
	{
		ComboIndex = 0;
	}

	CurrentWeaponActionData = *WeaponActionsDataTable->FindRow<FWeaponActionData>(WeaponTypeActions->LightComboActionNames[ComboIndex], " ");

	IPlayerInterface::Execute_PlayAttackMontage(GetOwner(), CurrentWeaponActionData.Animation, WeaponData->AnimsSpeedModifier);
	IPlayerInterface::Execute_SetCurrentPlayerState(GetOwner(), EPlayerState::UsingEquipment, false);

	StaminaComponent->UseStamina(CurrentWeaponActionData.StaminaCost * WeaponData->StaminaMultiplier);
	CurrentAttackDamages = CurrentWeaponActionData.DamageMultiplier * WeaponData->BaseDamage;
}


void UItemComponent::DoHeavyAttack()
{
	if (IPlayerInterface::Execute_GetCurrentPlayerState(GetOwner()) == EPlayerState::Fallen ||
		IPlayerInterface::Execute_GetCurrentPlayerState(GetOwner()) == EPlayerState::Dead) return;

	if (EquippedItem.CurrentInfos.ItemData == nullptr) return;
	if (EquippedItem.CurrentInfos.ItemData->ItemType != EItemType::Equipment) return;

	if (IPlayerInterface::Execute_GetCurrentPlayerState(GetOwner()) == EPlayerState::UsingEquipment)
	{
		PressedAttackInput = false;
		PressedHeavyAttackInput = true;
		return;
	}

	if (!StaminaComponent->VerifyHasStamina()) return;

	AlreadyHitActors.Reset();
	PlayerCharacter->StartAutoLock(5.f);

	FWeaponInfos* WeaponData = WeaponDataTable->FindRow<FWeaponInfos>(EquippedItem.CurrentInfos.ItemData->WeaponDataTableRow, " ");
	FWeaponTypesData* WeaponTypeActions = WeaponTypeActionsDataTable->FindRow<FWeaponTypesData>(WeaponData->WeaponTypeName, " ");

	if (PressedHeavyAttackInput)
	{
		PressedHeavyAttackInput = false;

		if (++ComboIndex >= WeaponTypeActions->HeavyComboActionNames.Num())
		{
			ComboIndex = 0;
		}
	}
	else
	{
		ComboIndex = 0;
	}

	CurrentWeaponActionData = *WeaponActionsDataTable->FindRow<FWeaponActionData>(WeaponTypeActions->HeavyComboActionNames[ComboIndex], " ");

	IPlayerInterface::Execute_PlaySoundOnServer(GetOwner(), "Attack", PlayerCharacter->PlayerConfig->AttackSoundRange, 1.0f, FVector::ZeroVector, true);
	IPlayerInterface::Execute_PlayAttackMontage(GetOwner(), CurrentWeaponActionData.Animation, WeaponData->AnimsSpeedModifier);
	IPlayerInterface::Execute_SetCurrentPlayerState(GetOwner(), EPlayerState::UsingEquipment, false);

	StaminaComponent->UseStamina(CurrentWeaponActionData.StaminaCost * WeaponData->StaminaMultiplier);
	CurrentAttackDamages = CurrentWeaponActionData.DamageMultiplier * WeaponData->BaseDamage;
	
	//PlayerCharacter->SetPlayerSpeed(PlayerCharacter->PlayerConfig->WalkSpeed * WeaponData->PlayerSpeedModifier);
}

void UItemComponent::DoAttackInertia()
{
	AttackInertiaTimer = CurrentWeaponActionData.MoveForceDuration;
	AttackInertiaDuration = CurrentWeaponActionData.MoveForceDuration;
	AttackInertiaForce = CurrentWeaponActionData.MoveForce;

	PlayerCharacter->SetPlayerAcceleration(15000);
}

void UItemComponent::ManageAttackInertia(float DeltaTime)
{
	AttackInertiaTimer -= DeltaTime;
	if (AttackInertiaTimer < 0) {
		return;
	}

	PlayerCharacter->AddMovementInput(PlayerCharacter->GetActorForwardVector(), FMath::Lerp(AttackInertiaForce / 1500.f, 0, 1 - (AttackInertiaTimer / AttackInertiaDuration)), false);
}


void UItemComponent::AttackAnimEnd()
{
	if (!GetOwner()->Implements<UPlayerInterface>()) return;

	//PlayerCharacter->SetPlayerSpeed(PlayerCharacter->PlayerConfig->WalkSpeed);

	IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(GetOwner());
	PlayerInterface->SetCurrentPlayerState_Implementation(EPlayerState::None, false);

	PlayerCharacter->SetPlayerAcceleration(4000);

	PlayerCharacter->StopAutoLock();

	if (PressedAttackInput)
	{
		DoLightAttack();
	}
}

float UItemComponent::GetCurrentAttackDamages()
{
	return CurrentAttackDamages;
}

void UItemComponent::DoAttackCollision()
{
	// only for the owning client
	if (!PlayerCharacter) return;
	if (!PlayerCharacter->GetController()) return;
	if (!PlayerCharacter->GetController()->IsLocalController()) return;

	if (EquippedItem.CurrentInfos.ItemData == nullptr) return;

	IPlayerInterface::Execute_PlaySoundOnServer(GetOwner(), "Attack", PlayerCharacter->PlayerConfig->AttackSoundRange, 0.8, FVector::ZeroVector, false);

	FWeaponInfos* WeaponData = WeaponDataTable->FindRow<FWeaponInfos>(EquippedItem.CurrentInfos.ItemData->WeaponDataTableRow, " ");

	if (!WeaponData) return;

	TArray<FHitResult> Hit;
	FVector FinalCollisionCenter = PlayerCharacter->WeaponCollisionPosRef->GetComponentLocation();
	FVector HalfSize = FVector(WeaponData->Range * 0.5f, WeaponData->Radius * 0.5f, WeaponData->Radius * 0.5f);
	FRotator Rotation = PlayerCharacter->WeaponCollisionPosRef->GetComponentRotation();
	
	bool bHit = UKismetSystemLibrary::BoxTraceMulti(
		this,
		FinalCollisionCenter,
		FinalCollisionCenter,
		HalfSize,
		Rotation,
		UEngineTypes::ConvertToTraceType(ECC_EngineTraceChannel3),
		false,           // trace complex
		TArray<AActor*>(),
		WeaponData->DisplayCollisionDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		Hit,
		true             // ignore self
	);

	if (!bHit) return;
	for (int i = 0; i < Hit.Num(); i++) {
		if (!Hit[i].GetActor()) continue;
		if (!Hit[i].GetActor()->ActorHasTag("Enemy") && !Hit[i].GetActor()->ActorHasTag("Destructible")) continue;
		if (AlreadyHitActors.Contains(Hit[i].GetActor())) continue;

		AlreadyHitActors.Add(Hit[i].GetActor());

		IPlayerInterface::Execute_DoCameraShake(PlayerCharacter, CurrentWeaponActionData.CameraShakeIntensity);

		// We hit an enemy
		if (Hit[i].GetActor()->ActorHasTag("Enemy")) {
			ABaseEnemy* Enemy = Cast<ABaseEnemy>(Hit[i].GetActor());
			Multi_PlayHitSound();
			// Sneak Attack
			float Multiplicator = 1;
			if (Enemy->GetCurrentEnemyState() != EEnemyState::Aggressive) {
				Multiplicator = CurrentWeaponData.SneakMultiplier;
			}
			if (!GetOwner()->HasAuthority())
				Server_ApplyDamagesToEnemy(Enemy, EquippedItem.CurrentInfos.ItemData, CurrentAttackDamages * Multiplicator);
			else
				Server_ApplyDamagesToEnemy_Implementation(Enemy, EquippedItem.CurrentInfos.ItemData, CurrentAttackDamages * Multiplicator);
		}

		// Others
		else {
			if (!GetOwner()->HasAuthority())
				Server_ApplyDamagesToDestructible(Hit[i].GetActor(), EquippedItem.CurrentInfos.ItemData, CurrentAttackDamages);
			else
				Server_ApplyDamagesToDestructible_Implementation(Hit[i].GetActor(), EquippedItem.CurrentInfos.ItemData, CurrentAttackDamages);
		}
	}
}

void UItemComponent::Server_ApplyDamagesToDestructible_Implementation(AActor* Target, UItemData* Data, float BaseDamages)
{
	if (!Target) return;

	float FinalDamage = BaseDamages;
	FWeaponInfos* WeaponData = WeaponDataTable->FindRow<FWeaponInfos>(Data->WeaponDataTableRow, " ");

	if (EquippedItem.CurrentInfos.Durability <= 0) FinalDamage *= Data->UsedDurabilityMultiplier;
	InventoryComponent->UseDurability(1, EquippedItem.CurrentInfos.ItemData);

	FinalDamage *= WeaponData->MineDamageMultiplier;

	IDamageable::Execute_ReceiveDamage(Target, FinalDamage, GetOwner());
}

void UItemComponent::Server_ApplyDamagesToEnemy_Implementation(ABaseEnemy* Enemy, UItemData* Data, float BaseDamages)
{
	if (!Enemy || Enemy->IsInvincible) return;

	float FinalDamage = BaseDamages;
	FWeaponInfos* WeaponData = WeaponDataTable->FindRow<FWeaponInfos>(Data->WeaponDataTableRow, " ");

	// Durability
	if (!EquippedItem.CurrentInfos.ItemData->bIsRangedWeapon) {
		if (EquippedItem.CurrentInfos.Durability <= 0) FinalDamage *= Data->UsedDurabilityMultiplier;
		InventoryComponent->UseDurability(1, EquippedItem.CurrentInfos.ItemData);
	}

	// Enemy Resistances
	switch (WeaponData->DamageType) {
	case EDamageType::Blunt :
		FinalDamage *= 1 - Enemy->EnemyData->BluntAbsorption;
		break;

	case EDamageType::Piercing:
		FinalDamage *= 1 - Enemy->EnemyData->PiercingAbsorption;
		break;
	}

	// Crit
	float CritPercent = FMath::FRandRange(0.f, 100.f);
	if (CritPercent < WeaponData->CriticalChance) {
		FinalDamage *= 3;
	}

	Enemy->ReceiveDamage_Implementation(FinalDamage, GetOwner());
}

#pragma endregion


#pragma region Use Ranged Weapon

void UItemComponent::Multi_PlayHitSound_Implementation()
{
	UAkGameplayStatics::PostEvent(MeleeHitSound,GetOwner(),0,FOnAkPostEventCallback(), false);
}

void UItemComponent::StartAim()
{
	IPlayerInterface::Execute_SetCurrentPlayerState(PlayerCharacter, EPlayerState::UsingEquipment, false);

	CurrentWeaponData = *WeaponDataTable->FindRow<FWeaponInfos>(EquippedItem.CurrentInfos.ItemData->WeaponDataTableRow, " ");

	bIsAiming = true;
	AimCurrentAngle = CurrentWeaponData.MaxAngle;
}

void UItemComponent::StopAim()
{
	if (!bIsAiming) return;
	if (!bIsReloading) IPlayerInterface::Execute_RequestStateChange(PlayerCharacter, EPlayerState::None, false);

	bIsAiming = false;

	HideAimLines();
}

void UItemComponent::Reload()
{
	if (!InventoryComponent->VerifyHasItemInInventory(CurrentWeaponData.NeededAmmo)) return;
	if (CurrentWeaponData.MagazineSize == EquippedItem.CurrentInfos.AmmoInMagazine) return;
	if (!(InventoryComponent->GetCurrentItem()->ItemType == EItemType::Equipment)) return;
	if (!InventoryComponent->GetCurrentItem()->bIsRangedWeapon) return;
	if (bIsReloading) return;

	bIsReloading = true;
	TimerReload = CurrentWeaponData.ReloadDuration;

	IPlayerInterface::Execute_SetCurrentPlayerState(PlayerCharacter, EPlayerState::UsingEquipment, false);

	HideAimLines();
}

void UItemComponent::CompleteReload()
{
	bIsReloading = false;
	InventoryComponent->ReloadGun(EquippedItem.CurrentInfos.ItemData, CurrentWeaponData.NeededAmmo, CurrentWeaponData.MagazineSize);

	IPlayerInterface::Execute_HideProgress(PlayerCharacter);

	if (bIsAiming) ActualiseAimLines();
	else IPlayerInterface::Execute_SetCurrentPlayerState(PlayerCharacter, EPlayerState::None, false);
}

void UItemComponent::CancelReload()
{
	if (!bIsReloading) return;

	IPlayerInterface::Execute_HideProgress(PlayerCharacter);

	if (!bIsAiming) IPlayerInterface::Execute_RequestStateChange(PlayerCharacter, EPlayerState::None, false);

	bIsReloading = false;
	TimerReload = 0;
}

void UItemComponent::ActualiseAim(float DeltaTime)
{
	bool bIsMoving = GetOwner()->GetVelocity().SquaredLength() > 1.0f;

	if (bIsMoving) AimCurrentAngle = FMath::Lerp(AimCurrentAngle, CurrentWeaponData.MinAngle, DeltaTime * CurrentWeaponData.AimingSpeed * CurrentWeaponData.WalkAimModifier);
	else AimCurrentAngle = FMath::Lerp(AimCurrentAngle, CurrentWeaponData.MinAngle, DeltaTime * CurrentWeaponData.AimingSpeed);

	ActualiseAimLines();
}

void UItemComponent::HideAimLines_Implementation()
{
}

void UItemComponent::ActualiseAimLines_Implementation()
{
}

void UItemComponent::DoShootFeedbacks_Implementation(FVector Direction, bool bDoShootLight)
{
}

void UItemComponent::Shoot()
{
	if (bIsReloading) return;
	if (EquippedItem.CurrentInfos.AmmoInMagazine <= 0) {
		Reload();
		return;
	}
	if (ShootDelayTimer > 0) return;

	ShootDelayTimer = CurrentWeaponData.DelayBetweenShots;

	for (int i = 0; i < CurrentWeaponData.NumberOfShots; i++) {
		FVector ShootDir = GetOwner()->GetActorForwardVector();
		float ModificatorAngle = FMath::RandRange(-AimCurrentAngle, AimCurrentAngle);
		ShootDir = ShootDir.RotateAngleAxis(ModificatorAngle, FVector::UpVector);

		DoShootRaycast(ShootDir);
		DoShootFeedbacks(ShootDir, i == 0);
	}

	FWeaponTypesData* WeaponTypeActions = WeaponTypeActionsDataTable->FindRow<FWeaponTypesData>(CurrentWeaponData.WeaponTypeName, " ");
	PlayerCharacter->PlayAttackMontage_Implementation(WeaponTypeActions->ShootAnim, 1.0f);

	IPlayerInterface::Execute_PlaySoundOnServer(GetOwner(), "", CurrentWeaponData.NoiseRange, 1, FVector(0, 0, 0), true);

	AimCurrentAngle = CurrentWeaponData.MaxAngle;
	PlayShootSound();
	InventoryComponent->UseAmmo(1, EquippedItem.CurrentInfos.ItemData);
}

void UItemComponent::DoShootRaycast(FVector Direction)
{
	FHitResult HitResult;

	FVector Start = GetOwner()->GetActorLocation() + FVector(0, 0, 50);
	FVector End = Start + (Direction * CurrentWeaponData.MaxRange);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner()); 

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_GameTraceChannel1,
		Params
	);

	if (!bHit) return;
	if (!HitResult.GetActor()->ActorHasTag("Enemy")) return;

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(HitResult.GetActor());
	float Multiplicator = 1;
	if (Enemy->GetCurrentEnemyState() != EEnemyState::Aggressive) {
		Multiplicator = CurrentWeaponData.SneakMultiplier;
	}

	if (!GetOwner()->HasAuthority())
		Server_ApplyDamagesToEnemy(Enemy, EquippedItem.CurrentInfos.ItemData, CurrentWeaponData.BaseDamage * Multiplicator);
	else
		Server_ApplyDamagesToEnemy_Implementation(Enemy, EquippedItem.CurrentInfos.ItemData, CurrentWeaponData.BaseDamage * Multiplicator);
}

void UItemComponent::PlayShootSound_Implementation()
{
	if (ShootSoundID)
	{
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (AudioDevice && ShootSoundID != AK_INVALID_PLAYING_ID)
		{
			AudioDevice->StopPlayingID(ShootSoundID);
			ShootSoundID = AK_INVALID_PLAYING_ID; // Reset
		}  
	}
	ShootSoundID = UAkGameplayStatics::PostEvent(ShootSound,GetOwner(),0,FOnAkPostEventCallback(), false);
}

#pragma endregion


#pragma region Revive System

// SERVER RPC
void UItemComponent::ServerRequestRevive_Implementation(AAPlayerCharacter* TargetAlly)
{
	PerformeRevive(TargetAlly);
}

// REVIVE ON THE SERVER THE ALLY 
void UItemComponent::PerformeRevive(AAPlayerCharacter* TargetAlly)
{
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority()) return;
	if (!TargetAlly || !TargetAlly->HealthComponent) return;

	if (!TargetAlly->HealthComponent->bIsDead) return;

	float Distance = FVector::Dist(PlayerCharacter->GetActorLocation(), TargetAlly->GetActorLocation());
	if (Distance > 300.f) return;

	TargetAlly->HealthComponent->Server_Revive();
	InventoryComponent->RemoveCurrentItem();
}


TArray<AAPlayerCharacter*> UItemComponent::GetNearbyPlayers(float Radius, bool bOnlyDead)
{
	TArray<AAPlayerCharacter*> Result;

	if (!PlayerCharacter) return Result;

	UWorld* World = GetWorld();
	if (!World) return Result;

	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	TArray<FOverlapResult> Overlaps;

	bool bOverlap = World->OverlapMultiByChannel(
		Overlaps,
		PlayerCharacter->GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		Sphere
	);

	if (!bOverlap) return Result;

	for (const FOverlapResult& Hit : Overlaps)
	{
		AAPlayerCharacter* PC = Cast<AAPlayerCharacter>(Hit.GetActor());
		if (!PC || PC == PlayerCharacter)
			continue;

		if (bOnlyDead)
		{
			UHealthComponent* HC = PC->FindComponentByClass<UHealthComponent>();
			if (!HC || PC->GetCurrentPlayerState_Implementation() != EPlayerState::Dead)
				continue;
		}

		Result.Add(PC);
	}

	return Result;
}
#pragma endregion


