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
		ItemMesh->SetStaticMesh(ItemData->ItemMesh);
		ItemInfos = FItemInfos(ItemData, ItemData->WeaponDurability);
	}
}


void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShouldLevitate)
	{
		 LevitationTime += DeltaTime;

		float ZOffset = FMath::Sin(LevitationTime * LevitationSpeed) * LevitationAmplitude;

		FVector NewLocation = InitialeLocation + FVector(0, 0, ZOffset);
		SetActorLocation(NewLocation);

		FRotator NewRotation = GetActorRotation();
		NewRotation.Yaw += DeltaTime * 30.f;
		SetActorRotation(NewRotation);
	}

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
	Super::Interact_Implementation(Interactor);

	AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Interactor);
	if (!Player->InventoryComponent->AddNewItem(ItemInfos)) return;

	Destroy();
}

