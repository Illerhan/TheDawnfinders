// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/AItem.h"

#include "Actors/Player/APlayerCharacter.h"
#include "Components/UInventoryComponent.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(CapsuleCollider);

	if (ItemData && ItemData->ItemMesh)
	ItemMesh->SetStaticMesh(ItemData->ItemMesh);	
}

void AItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
    
	if (ItemData && ItemData->ItemMesh)
	{
		ItemMesh->SetStaticMesh(ItemData->ItemMesh);
	}
}


void AItem::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Error, TEXT("Static Mesh : %s"),*ItemMesh->GetStaticMesh().GetFullName());
	InitialeLocation = GetActorLocation();
		
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

void AItem::Initialise_Implementation(UItemData* Data)
{
	ItemData = Data;

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
	if (!Player->InventoryComponent->AddNewItem(ItemData)) return;

	Destroy();
}

