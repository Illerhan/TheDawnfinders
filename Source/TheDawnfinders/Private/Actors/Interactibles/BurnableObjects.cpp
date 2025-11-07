// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Interactibles/BurnableObjects.h"

#include "Commandlets/GatherTextCommandletBase.h"


// Sets default values
ABurnableObjects::ABurnableObjects()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	LightComponent = CreateDefaultSubobject<UPlayerLightComponent>(FName("AC_Light"));
	if (LightComponent && LightComponent->ProtectionZone)
	{
		LightComponent->ProtectionZone->SetupAttachment(RootComponent);
	}

}

// Called when the game starts or when spawned
void ABurnableObjects::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABurnableObjects::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

