// Fill out your copyright notice in the Description page of Project Settings.


#include "Lever.h"


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
	if (bCanBeUsed)
	{
		if (LinkedObject->bCanMove)
		{
			UE_LOG(LogTemp, Warning, TEXT("moving %s") , *LinkedObject->GetName());
			LinkedObject->DoMovement();
			Super::Interaction(Player);
		}
	}
}

         