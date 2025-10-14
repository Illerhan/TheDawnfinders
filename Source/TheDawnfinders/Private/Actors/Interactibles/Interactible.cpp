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
	SphereCollider = CreateDefaultSubobject<USphereComponent>(FName("SphereCollider"));
	RootComponent = SphereCollider;
	SphereCollider->SetCollisionResponseToAllChannels(ECR_Overlap);
}

void AInteractibleObjects::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<AAPlayerCharacter>(OtherActor) != nullptr)
			Cast<AAPlayerCharacter>(OtherActor)->Execute_AddInteractibleAtRange(OtherActor,this);
}

void AInteractibleObjects::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Cast<AAPlayerCharacter>(OtherActor) != nullptr)
		Cast<AAPlayerCharacter>(OtherActor)->Execute_RemoveInteractibleAtRange(OtherActor,this);
}

// Called when the game starts or when spawned
void AInteractibleObjects::BeginPlay()
{
	Super::BeginPlay();
	SphereCollider->OnComponentBeginOverlap.AddDynamic(this,&AInteractibleObjects::OnOverlapBegin);
	SphereCollider->OnComponentEndOverlap.AddDynamic(this,&AInteractibleObjects::OnOverlapEnd);
	
}

void AInteractibleObjects::Interaction_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Interacted with a %s") , *GetName());
}

// Called every frame
void AInteractibleObjects::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
