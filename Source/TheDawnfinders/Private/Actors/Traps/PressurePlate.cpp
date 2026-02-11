// Fill out your copyright notice in the Description page of Project Settings.


#include "PressurePlate.h"

#include "Interfaces/Activable.h"


APressurePlate::APressurePlate()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);
	
	BoxCollider = CreateDefaultSubobject<UBoxComponent>("BoxCollider");
	BoxCollider->SetupAttachment(RootComponent);
}

void APressurePlate::BeginPlay()
{
	Super::BeginPlay();
	
	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &APressurePlate::OnOverlapBegin);
	BoxCollider->OnComponentEndOverlap.AddDynamic(this, &APressurePlate::OnOverlapEnd);
	
}

void APressurePlate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}



void APressurePlate::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	CurrentPlayerCount++;
	if (CurrentPlayerCount < NeededPlayerCount) return;

	for (auto LinkedActor : LinkedActors)
	{
		if (LinkedActor->Implements<UActivable>())
		{
			IActivable::Execute_DoMainAction(LinkedActor);
		}
	}
}

void APressurePlate::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CurrentPlayerCount--;

	for (auto LinkedActor : LinkedActors)
	{
		if (LinkedActor->Implements<UActivable>())
		{
			IActivable::Execute_StopMainAction(LinkedActor);
		}
	}
}

