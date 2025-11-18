// Fill out your copyright notice...

#include "Components/UItemComponent.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Actors/Player/AThrowableObject.h"
#include "Components/UHealthComponent.h"
#include "Components/UInventoryComponent.h"
#include "Interfaces/IPlayer.h"

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

	InventoryComponent->OnInventoryChanging.AddUniqueDynamic(this, &UItemComponent::SetEquippedItem);
}

void UItemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner()) return;

	ActualiseUseProgress(DeltaTime);
	ActualisePreviewThrow(DeltaTime);
}

#pragma region Main Action

void UItemComponent::DoMainAction()
{
	if (EquippedItem.ItemData == nullptr) return;
	if (PlayerCharacter->GetCurrentPlayerState_Implementation() == EPlayerState::Fallen 
		|| PlayerCharacter->GetCurrentPlayerState_Implementation() == EPlayerState::Dead) return;

	if (EquippedItem.ItemData->ItemType == EItemType::Consumable) 
	{
		if (EquippedItem.ItemData->NeededHoldDuration != 0)
		{
			if (EquippedItem.ItemData->ConsumableEffectType == EConsumableEffectType::Revive)
			{
				float SearchRadius = 300.f;
				TArray<AAPlayerCharacter*> DeadPlayers = GetNearbyPlayers(SearchRadius, true);
				if (DeadPlayers.Num() == 0) return;

				Ally = DeadPlayers[0]; // On cible le premier mort à portée
			}
			
			if (EquippedItem.ItemData->NeededHoldDuration != 0 && !EquippedItem.ItemData->ContextualUse)
			{
				ItemUseTimer = EquippedItem.ItemData->NeededHoldDuration;
				IsUsingItem = true;
				return;
			}
		}
		UseConsumable();
		return;
	}

	if (EquippedItem.ItemData->ItemType == EItemType::Equipment)
	{
		WeaponMainAction();
	}
}

void UItemComponent::ActualiseUseProgress(float DeltaTime)
{
	if (!IsUsingItem) return;

	if (ItemUseTimer > 0)
	{
		ItemUseTimer -= DeltaTime;

		if (GetOwner()->Implements<UPlayerInterface>())
		{
			IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(GetOwner());
			if (PlayerInterface)
			{
				PlayerInterface->ShowProgress_Implementation(ItemUseTimer);
			}
		}
		return;
	}

	UseConsumable();
}

void UItemComponent::UseConsumable()
{
	IsUsingItem = false;

	if (GetOwner()->Implements<UPlayerInterface>())
	{
		IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(GetOwner());
		PlayerInterface->HideProgress_Implementation();
	}

	switch (EquippedItem.ItemData->ConsumableEffectType)
	{
		case EConsumableEffectType::Heal:
			HealthComponent->Heal(EquippedItem.ItemData->ConsumableEffectPower);
			InventoryComponent->RemoveCurrentItem();
			break;

		case EConsumableEffectType::PlaceZipline:
			// TODO later
			break;

		case EConsumableEffectType::ThrowObject:
		{
			if (!IsPreviewingThrow) return;

			float Progress = ThrowPreviewTimer / 2.f;
			FVector Pos1 = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 250.f;
			FVector Pos2 = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 800.f;
			FVector FinalPos = FMath::Lerp(Pos1, Pos2, Progress);

			AThrowableObject* ThrowedObject =
				GetWorld()->SpawnActor<AThrowableObject>(EquippedItem.ItemData->ThrowedObjectClass, 
					GetOwner()->GetActorLocation(), FRotator(0.f, 0.f, 0.f));

			if (ThrowedObject)
			{
				ThrowedObject->Initialise(FinalPos);
				InventoryComponent->RemoveCurrentItem();
			}
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
	}
}

void UItemComponent::StopMainAction()
{
	if (EquippedItem.ItemData == nullptr) return;

	if (GetOwner()->Implements<UPlayerInterface>())
	{
		IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(GetOwner());
		PlayerInterface->HideProgress_Implementation();
	}

	IsUsingItem = false;
}

#pragma endregion

#pragma region Secondary Action

void UItemComponent::DoSecondaryAction()
{
	if (EquippedItem.ItemData == nullptr) return;
	if (EquippedItem.ItemData->ItemType == EItemType::Valuable) return;

	if (EquippedItem.ItemData->ItemType == EItemType::Consumable &&
		EquippedItem.ItemData->ConsumableEffectType == EConsumableEffectType::ThrowObject)
	{
		StartPreviewThrow();
	}
}

void UItemComponent::StartPreviewThrow()
{
	if (IsPreviewingThrow) return;

	IsPreviewingThrow = true;
	ThrowPreviewTimer = 0;
}

void UItemComponent::ActualisePreviewThrow(float DeltaTime)
{
	if (!IsPreviewingThrow) return;

	if (EquippedItem.ItemData == nullptr || EquippedItem.ItemData->ThrowedObjectClass == nullptr)
	{
		StopPreviewThrow();
		return;
	}

	ThrowPreviewTimer += DeltaTime;

	float Progress = ThrowPreviewTimer / 2.f;
	FVector Pos1 = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 200.f;
	FVector Pos2 = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 800.f;
	FVector FinalPos = FMath::Lerp(Pos1, Pos2, Progress);

	AThrowableObject* Throwable = EquippedItem.ItemData->ThrowedObjectClass->GetDefaultObject<AThrowableObject>();

	OnThrowPreviewDisplay.Broadcast(FinalPos, Throwable->EffectRange);
}

void UItemComponent::StopPreviewThrow()
{
	if (!IsPreviewingThrow) return;

	IsPreviewingThrow = false;
	ThrowPreviewTimer = 0;

	OnThrowHidePreview.Broadcast();
}

void UItemComponent::StopSecondaryAction()
{
	if (EquippedItem.ItemData == nullptr) return;

	StopPreviewThrow();
}

#pragma endregion

#pragma region Equip / Unequip

void UItemComponent::SetEquippedItem(const TArray<FInventorySlot>& Slots, int CurrentSlotIndex)
{
	if (EquippedItem.ItemData != nullptr && EquippedItem.ItemData->ItemType == EItemType::Equipment)
	{
		UnequipWeapon();
	}

	EquippedItem = Slots[CurrentSlotIndex];

	if (EquippedItem.ItemData != nullptr && EquippedItem.ItemData->ItemType == EItemType::Equipment)
	{
		EquipWeapon();
	}
}

UItemData* UItemComponent::GetEquippedItem()
{
	return EquippedItem.ItemData;
}

void UItemComponent::EquipWeapon()
{
	if (GetOwner()->Implements<UPlayerInterface>())
	{
		IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(GetOwner());
		PlayerInterface->SetEquippedMesh_Implementation(EquippedItem.ItemData->ItemMesh);
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

#pragma region Use Weapon

void UItemComponent::WeaponMainAction()
{
	if (!GetOwner()->Implements<UPlayerInterface>()) return;

	IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(GetOwner());

	if (PlayerInterface->GetCurrentPlayerState_Implementation() == EPlayerState::UsingEquipment)
	{
		PressedAttackInput = true;
		return;
	}

	if (PressedAttackInput)
	{
		PressedAttackInput = false;

		if (++ComboIndex >= EquippedItem.ItemData->BaseComboAnims.Num())
		{
			ComboIndex = 0;
		}
	}
	else
	{
		ComboIndex = 0;
	}

	PlayerInterface->PlayAttackMontage_Implementation(EquippedItem.ItemData->BaseComboAnims[ComboIndex]);
	PlayerInterface->SetCurrentPlayerState_Implementation(EPlayerState::UsingEquipment);
}

void UItemComponent::AttackAnimEnd()
{
	if (!GetOwner()->Implements<UPlayerInterface>()) return;

	IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(GetOwner());
	PlayerInterface->SetCurrentPlayerState_Implementation(EPlayerState::None);

	if (PressedAttackInput)
	{
		WeaponMainAction();
	}
}

#pragma endregion

#pragma region Revive System

// SERVER RPC
void UItemComponent::ServerRequestRevive_Implementation(AAPlayerCharacter* TargetAlly)
{
	PerformeRevive(TargetAlly);
}

// Réalise réellement le revive côté serveur
void UItemComponent::PerformeRevive(AAPlayerCharacter* TargetAlly)
{
	if (!PlayerCharacter || !PlayerCharacter->HasAuthority()) return;
	if (!TargetAlly || !TargetAlly->HealthComponent) return;

	if (!TargetAlly->HealthComponent->IsDead) return;

	
	float Distance = FVector::Dist(PlayerCharacter->GetActorLocation(), TargetAlly->GetActorLocation());
	if (Distance > 300.f) return;

	// Effectuer la résurrection
	TargetAlly->HealthComponent->Server_Revive();
	InventoryComponent->RemoveCurrentItem();
}
TArray<AAPlayerCharacter*> UItemComponent::GetNearbyPlayers(float Radius, bool bOnlyDead)
{
	TArray<AAPlayerCharacter*> Result;

	if (!PlayerCharacter)
		return Result;

	UWorld* World = GetWorld();
	if (!World)
		return Result;

	// Paramètres de la sphère
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	TArray<FOverlapResult> Overlaps;

	bool bOverlap = World->OverlapMultiByChannel(
		Overlaps,
		PlayerCharacter->GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		Sphere
	);

	if (!bOverlap)
		return Result;

	for (const FOverlapResult& Hit : Overlaps)
	{
		AAPlayerCharacter* PC = Cast<AAPlayerCharacter>(Hit.GetActor());
		if (!PC || PC == PlayerCharacter)
			continue;

		// Filtrer selon l'état "mort"
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
