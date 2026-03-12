// Fill out your copyright notice...

#include "Components/UItemComponent.h"

#include "FrameTypes.h"
#include "Actors/Interactibles/Litter.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Actors/Player/AThrowableObject.h"
#include "Components/UHealthComponent.h"
#include "Components/UInventoryComponent.h"
#include "Components/UStaminaComponent.h"
#include "Kismet/KismetSystemLibrary.h"
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

	if (!GetOwner()) return;

	if (AttackInertiaTimer > 0) {
		ManageAttackInertia(DeltaTime);
	}

	ActualiseUseProgress(DeltaTime);
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
		PlayerInterface->SetEquippedMesh_Implementation(EquippedItem.CurrentInfos.ItemData->ItemMesh);
	}
}

void UItemComponent::UnequipWeapon()
{
	if (GetOwner()->Implements<UPlayerInterface>())
	{
		IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(GetOwner());
		PlayerInterface->SetEquippedMesh_Implementation(nullptr);
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
				
				IPlayerInterface::Execute_SetCurrentPlayerState(GetOwner(), EPlayerState::UsingEquipment);

				return;
			}
		}
		UseConsumable();
		return;
	}
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
		IPlayerInterface::Execute_SetCurrentPlayerState(GetOwner(), EPlayerState::None);
	}

	if (EquippedItem.CurrentInfos.ItemData == nullptr) return;

	if (EquippedItem.CurrentInfos.ItemData->UseConsumableMontage)
	{
		if (GetOwner()->HasAuthority())
			PlayerCharacter->ServerPlayMontage_Implementation(EquippedItem.CurrentInfos.ItemData->UseConsumableMontage, 1.0f);

		else
			PlayerCharacter->ServerPlayMontage(EquippedItem.CurrentInfos.ItemData->UseConsumableMontage, 1.0f);
	}

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

				Server_ThrowItem(EquippedItem.CurrentInfos.ItemData, CurrentThrowPosition);

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
			float Amount = EquippedItem.CurrentInfos.ItemData->ConsumableEffectPower;
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
	}
}


void UItemComponent::StopMainAction()
{
	if (EquippedItem.CurrentInfos.ItemData == nullptr) return;
	if (EquippedItem.CurrentInfos.ItemData->ItemType == EItemType::Equipment) return;

	// Throw throwable on release
	if (IsPreviewingThrow) {
		UseConsumable();
	}

	if (GetOwner()->Implements<UPlayerInterface>())
	{
		IPlayerInterface::Execute_SetCurrentPlayerState(GetOwner(), EPlayerState::None);
		IPlayerInterface::Execute_HideProgress(GetOwner());
	}

	bIsUsingItem = false;
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
		IPlayerInterface::Execute_SetCurrentPlayerState(PlayerCharacter, EPlayerState::UsingEquipment);
		return;
	}

	if (EquippedItem.CurrentInfos.ItemData->ItemType == EItemType::Equipment) {
		IPlayerInterface::Execute_SetCurrentPlayerState(PlayerCharacter, EPlayerState::Blocking);
	}
}

void UItemComponent::StopSecondaryAction()
{
	if (EquippedItem.CurrentInfos.ItemData == nullptr) return;

	if (EquippedItem.CurrentInfos.ItemData->ItemType == EItemType::Equipment) {
		IPlayerInterface::Execute_SetCurrentPlayerState(PlayerCharacter, EPlayerState::None);
	}

	if (EquippedItem.CurrentInfos.ItemData->ConsumableEffectType == EConsumableEffectType::ThrowObject)
	{
		StopPreviewThrow();
		IPlayerInterface::Execute_SetCurrentPlayerState(PlayerCharacter, EPlayerState::None);
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

void UItemComponent::Server_ThrowItem_Implementation(UItemData* Data, FVector FinalPosition)
{
	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	Params.Instigator = Cast<APawn>(GetOwner());

	AThrowableObject* ThrowedObject =
		GetWorld()->SpawnActor<AThrowableObject>(Data->ThrowedObjectClass,
			GetOwner()->GetActorLocation(), FRotator(0.f, 0.f, 0.f), Params);

	if (ThrowedObject)
	{
		ThrowedObject->Initialise(FinalPosition, Data);
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

	CurrentThrowPosition = GetOwner()->GetActorLocation() + AimInput * 800.f;

	AThrowableObject* Throwable = EquippedItem.CurrentInfos.ItemData->ThrowedObjectClass->GetDefaultObject<AThrowableObject>();
	OnThrowPreviewDisplay.Broadcast(CurrentThrowPosition, Throwable->EffectRange);
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

	IPlayerInterface::Execute_PlaySoundOnServer(GetOwner(), "Attack", PlayerCharacter->PlayerConfig->AttackSoundRange, 0.8, FVector::ZeroVector);
	IPlayerInterface::Execute_PlayAttackMontage(GetOwner(), CurrentWeaponActionData.Animation, WeaponData->AnimsSpeedModifier);
	IPlayerInterface::Execute_SetCurrentPlayerState(GetOwner(), EPlayerState::UsingEquipment);

	StaminaComponent->UseStamina(CurrentWeaponActionData.StaminaCost * WeaponData->StaminaMultiplier);
	CurrentAttackDamages = CurrentWeaponActionData.DamageMultiplier * WeaponData->BaseDamage;

	//PlayerCharacter->SetPlayerSpeed(PlayerCharacter->PlayerConfig->WalkSpeed * WeaponData->PlayerSpeedModifier);
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

	IPlayerInterface::Execute_PlaySoundOnServer(GetOwner(), "Attack", PlayerCharacter->PlayerConfig->AttackSoundRange, 1.0f, FVector::ZeroVector);
	IPlayerInterface::Execute_PlayAttackMontage(GetOwner(), CurrentWeaponActionData.Animation, WeaponData->AnimsSpeedModifier);
	IPlayerInterface::Execute_SetCurrentPlayerState(GetOwner(), EPlayerState::UsingEquipment);

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
	PlayerInterface->SetCurrentPlayerState_Implementation(EPlayerState::None);

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

	FWeaponInfos* WeaponData = WeaponDataTable->FindRow<FWeaponInfos>(EquippedItem.CurrentInfos.ItemData->WeaponDataTableRow, " ");

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
		if (!Hit[i].GetActor()->ActorHasTag("Enemy")) continue;
		if (AlreadyHitActors.Contains(Hit[i].GetActor())) continue;

		AlreadyHitActors.Add(Hit[i].GetActor());

		IPlayerInterface::Execute_DoCameraShake(PlayerCharacter, CurrentWeaponActionData.CameraShakeIntensity);

		ABaseEnemy* Enemy = Cast<ABaseEnemy>(Hit[i].GetActor());
		if (!Enemy) return;

		if (!GetOwner()->HasAuthority())
			Server_ApplyDamagesToEnemy(Enemy, EquippedItem.CurrentInfos.ItemData, CurrentAttackDamages);

		else
			Server_ApplyDamagesToEnemy_Implementation(Enemy, EquippedItem.CurrentInfos.ItemData, CurrentAttackDamages);
	}
}

void UItemComponent::Server_ApplyDamagesToEnemy_Implementation(ABaseEnemy* Enemy, UItemData* Data, float BaseDamages)
{
	if (!Enemy || Enemy->IsInvincible) return;

	float FinalDamage = BaseDamages;
	FWeaponInfos* WeaponData = WeaponDataTable->FindRow<FWeaponInfos>(Data->WeaponDataTableRow, " ");

	if (EquippedItem.CurrentInfos.Durability <= 0) FinalDamage *= Data->UsedDurabilityMultiplier;
	InventoryComponent->UseDurability(1, EquippedItem.CurrentInfos.ItemData);
	//EquippedItem.CurrentInfos.Durability -= 1;

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


