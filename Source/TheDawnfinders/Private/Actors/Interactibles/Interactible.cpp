// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Interactibles/Interactible.h"
#include "Widgets/ULockpickQTEWidget.h"
#include "Actors/Player/APlayerCharacter.h"


// Sets default values
AInteractibleObjects::AInteractibleObjects()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	CapsuleCollider = CreateDefaultSubobject<UCapsuleComponent>(FName("SphereCollider"));
	RootComponent = CapsuleCollider;
	CapsuleCollider->SetCollisionResponseToAllChannels(ECR_Overlap);
	CapsuleCollider->SetGenerateOverlapEvents(true);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("StaticMesh"));
	StaticMesh->SetupAttachment(CapsuleCollider);

	InteractQTEWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(FName("LockpickWidget"));
	InteractQTEWidgetComponent->SetupAttachment(CapsuleCollider);
	
	bReplicates = true;
}

void AInteractibleObjects::BeginPlay()
{
	Super::BeginPlay();

	CapsuleCollider->OnComponentBeginOverlap.AddDynamic(this, &AInteractibleObjects::OnOverlapBegin);
	CapsuleCollider->OnComponentEndOverlap.AddDynamic(this, &AInteractibleObjects::OnOverlapEnd);

	InteractQTEWidget = Cast<ULockpickQTEWidget>(InteractQTEWidgetComponent->GetWidget());
}

void AInteractibleObjects::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


#pragma region Colliders 

void AInteractibleObjects::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor);
	if (Player && Player->IsLocallyControlled())
	{
		// This now runs on the client, directly modifying their local array
		Player->AddInteractibleAtRange_Implementation(this);
		UE_LOG(LogTemp, Log, TEXT("Added interactible locally on client"));
	}
}

void AInteractibleObjects::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor);
	if (Player && Player->IsLocallyControlled())
	{
		Player->RemoveInteractibleAtRange_Implementation(this);
		UE_LOG(LogTemp, Log, TEXT("Removed interactible locally on client"));
	}
}

#pragma endregion	


#pragma region Interface

void AInteractibleObjects::Interact_Implementation(AActor* Interact)
{
	if (!bCanBeUsed) return;

	BP_OnInteraction(Cast<AAPlayerCharacter>(Interact));
}

void AInteractibleObjects::StopInteract_Implementation(AActor* Interactor)
{
	if (!bCanBeUsed) return;

	BP_OnStopInteraction(Cast<AAPlayerCharacter>(Interactor));
}

bool AInteractibleObjects::GetCanBeUsed_Implementation()
{
	return bCanBeUsed;
}

bool AInteractibleObjects::GetQTENeeded_Implementation()
{
	return bDoQTE;
}

void AInteractibleObjects::StartQTE_Implementation()
{
	InteractQTEWidget->EnterQTE(QTESuccessRange, 150.f);
}

void AInteractibleObjects::StopQTE_Implementation()
{
	InteractQTEWidget->ExitQTE();
}

void AInteractibleObjects::ValidateQTE_Implementation()
{
	InteractQTEWidget->ValidateQTE();
}

#pragma endregion

