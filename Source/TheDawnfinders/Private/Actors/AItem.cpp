// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/AItem.h"

#include "Actors/Player/APlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/UInventoryComponent.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(BoxCollider);

	if (ItemData && ItemData->ItemMesh)
	ItemMesh->SetStaticMesh(ItemData->ItemMesh);	
}

void AItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}


void AItem::BeginPlay()
{
	Super::BeginPlay();

	InitialeLocation = GetActorLocation();

	if (ItemData && ItemData->ItemMesh)
	{
		ItemMesh->SetVisibility(false);

		if (!ItemData->bIsRangedWeapon) {
			ItemInfos = FItemInfos(ItemData, ItemData->Durability);
			Initialise(ItemInfos);
			return;
		}

		UDataTable* WeaponDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_Weapons.DT_Weapons"));
		if (!WeaponDataTable) {
			UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable"));
			return;
		}

		if (WeaponDataTable->FindRow<FWeaponInfos>(ItemData->WeaponDataTableRow, " ") == nullptr) return;

		FWeaponInfos CurrentWeaponData = *WeaponDataTable->FindRow<FWeaponInfos>(ItemData->WeaponDataTableRow, " ");

		ItemInfos = FItemInfos(ItemData, ItemData->Durability, CurrentWeaponData.MagazineSize);
		Initialise(ItemInfos);
	}
}


void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AItem::Initialise_Implementation(FItemInfos Data)
{
	if (HasAuthority()) {
		Multicast_Initialise(Data);
	}
}

void AItem::Multicast_Initialise_Implementation(FItemInfos Data)
{
	ItemData = Data.ItemData;
	ItemInfos = Data;

	ItemMesh->SetStaticMesh(ItemData->ItemMesh);
	ItemMesh->SetSimulatePhysics(false);
	ItemMesh->SetEnableGravity(false);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	bShouldLevitate = true;
}

void AItem::Interact_Implementation(AActor* Interactor)
{
	if (!IsValid(ItemInfos.ItemData) || !IsValid(ItemData)) {
		Destroy();
		return;
	}

	Super::Interact_Implementation(Interactor);

	AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Interactor);
	if (!Player->InventoryComponent->AddNewItem(ItemInfos)) return;
	
	Player->InteractionComponent->PlayInteractSound();

	Destroy();
}

