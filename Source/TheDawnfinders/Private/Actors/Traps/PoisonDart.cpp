// Fill out your copyright notice in the Description page of Project Settings.


#include "PoisonDart.h"

#include "Actors/Projectiles/Darts.h"
#include "Net/UnrealNetwork.h"


// Sets default values
APoisonDart::APoisonDart()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APoisonDart::BeginPlay()
{
	Super::BeginPlay();
	SpawnLocation += GetActorLocation();
	
}

void APoisonDart::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APoisonDart,Cooldown);
	DOREPLIFETIME(APoisonDart,bCooldown);
}

void APoisonDart::DoMainAction_Implementation()
{
	if (bCooldown) return;
	CurrentCooldown = Cooldown;
	UWorld* World = GetWorld();
	FActorSpawnParameters SpawnParam;
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<ADarts>(Dart,SpawnLocation,FRotator::ZeroRotator,SpawnParam);
	UE_LOG(LogTemp,Error,TEXT("DoMainAction_Implementation()"));
} 

// Called every frame
void APoisonDart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (CurrentCooldown<=0)
	{
		bCooldown = false;
	}
	else
	{
		bCooldown = true;
		CurrentCooldown -= DeltaTime;
	}
	
}

