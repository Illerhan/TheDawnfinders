// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundManager.h"

#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"


// Sets default values
ASoundManager::ASoundManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}


// Called when the game starts or when spawned
void ASoundManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASoundManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASoundManager::ServerPlaySound_Implementation(USoundBase* Sound, FVector Location, float Loudness, float Range)
{
	MultiPlaySound(Sound, Location, Loudness);
}


void ASoundManager::MultiPlaySound_Implementation(USoundBase* Sound, FVector Location, float Loudness, float Range)
{
	if(Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, Location);
		UAISense_Hearing::ReportNoiseEvent(this, Location, Loudness, this,Range);
	}
}
