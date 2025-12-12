// Fill out your copyright notice in the Description page of Project Settings.


#include "WolfTrap.h"

#include "Actors/Enemy/ABaseEnemy.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Interfaces/IDamageable.h"


// Sets default values
AWolfTrap::AWolfTrap()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AWolfTrap::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWolfTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWolfTrap::DoTrapAction()
{
	Super::DoTrapAction();
	AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(TrappedActor);
	if (Player)
		if (!Player->HasAuthority())
		{
			Player->Server_OnTrapped();
		}
		else
		{
			Player->OnTrapped();
		}
			
	//ABaseEnemy* Enemy = Cast<ABaseEnemy>(TrappedActor);

}

