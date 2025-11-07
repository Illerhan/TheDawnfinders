// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactible.h"

#include "Actors/Player/APlayerCharacter.h"
#include "GameFramework/GameSession.h"
#include "Interfaces/IPlayer.h"

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
	
	bReplicates = true;
}

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

void AInteractibleObjects::Interaction(AAPlayerCharacter* Player)
{
	BP_OnInteraction(Player);
}

// Called when the game starts or when spawned
void AInteractibleObjects::BeginPlay()
{
	Super::BeginPlay();
	CapsuleCollider->OnComponentBeginOverlap.AddDynamic(this,&AInteractibleObjects::OnOverlapBegin);
	CapsuleCollider->OnComponentEndOverlap.AddDynamic(this,&AInteractibleObjects::OnOverlapEnd);
	
}

void AInteractibleObjects::Interact_Implementation(AActor* Interact)
{
	
	AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Interact);
	if (Player)
		Player->InteractionComponent->TryInteract(this,Player);
	UE_LOG(LogTemp, Warning, TEXT("Interacted with a %s") , *GetName());
}

// Called every frame
void AInteractibleObjects::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
