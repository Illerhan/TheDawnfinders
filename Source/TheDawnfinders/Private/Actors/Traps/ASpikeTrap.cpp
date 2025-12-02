// Fill out your copyright notice in the Description page of Project Settings.


#include "ASpikeTrap.h"

#include "Interfaces/IDamageable.h"


// Sets default values
AASpikeTrap::AASpikeTrap()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AASpikeTrap::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AASpikeTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AASpikeTrap::DoTrapAction()
{
	if (!TrappedActor) return;

	Super::DoTrapAction();
	IDamageable::Execute_ReceiveDamage(TrappedActor, Damages, this);
	
}

