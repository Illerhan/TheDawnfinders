// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Interactibles/Lever.h"

// Sets default values
ALever::ALever()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALever::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALever::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALever::Interaction(AAPlayerCharacter* Player)
{
	if (!bCanBeUsed || LinkedObjects.Num() == 0) return;

	if (bRequiresHold)
	{
		StartHoldInteraction(Player);
	}
	
	else
	{
	for (AMovableObjects* const Object : LinkedObjects)
		if (Object && Object->bCanMove)
		{
			bool bReverse = Object->CanReverse();
			UE_LOG(LogTemp, Warning, TEXT("moving %s") , *Object->GetName());
			if (bReverse && Object->bIsMovingForward)
			{
				Object->DoReverseMovement();
			}
			else
			{
				Object->DoMovement();
			}
			
			Super::Interaction(Player);
		}
	}
}
void ALever::StartHoldInteraction(AAPlayerCharacter* Player)
{
	for (AMovableObjects* const Object : LinkedObjects)
	{
		ADoors* Door = Cast<ADoors>(Object);
		if (Door)
		{
			Door->StartOpening();
			UE_LOG(LogTemp, Warning, TEXT("[SERVER] Lever starting door opening"));
		}
	}
}

void ALever::StopHoldInteraction(AAPlayerCharacter* Player)
{
	for (AMovableObjects* Object : LinkedObjects)
	{
		ADoors* Door = Cast<ADoors>(Object);
		if (Door)
		{
			Door->StopOpening();
			UE_LOG(LogTemp, Warning, TEXT("[SERVER] Closing door: %s"), *Door->GetName());
		}
	}
}
