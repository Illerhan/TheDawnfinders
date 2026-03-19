// Fill out your copyright notice in the Description page of Project Settings.


#include "DangerManager.h"


// Sets default values
ADangerManager::ADangerManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADangerManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADangerManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADangerManager::MuleCalled_Implementation()
{
}

void ADangerManager::IncreaseDangerLevel_Implementation(float DangerCost)
{
}

