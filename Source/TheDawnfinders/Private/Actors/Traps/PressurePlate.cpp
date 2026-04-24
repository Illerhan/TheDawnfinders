// Fill out your copyright notice in the Description page of Project Settings.


#include "PressurePlate.h"

#include "AkAudioDevice.h"
#include "AkGameplayStatics.h"
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
	if (CurrentPlayerCount > 1) return;
	bIsActive = true;
	Multi_PlaySound();
	for (auto LinkedActor : LinkedActors)
	{
		if (LinkedActor->Implements<UActivable>())
		{
			IActivable::Execute_SetPlayerCount(LinkedActor,1);
			if (IActivable::Execute_GetPlayerCount(LinkedActor) >1) continue;
			IActivable::Execute_DoMainAction(LinkedActor);
			
		}
	}
	
}

void APressurePlate::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CurrentPlayerCount--;
	if (CurrentPlayerCount != 0) return;
	bIsActive = false;
	for (auto LinkedActor : LinkedActors)
	{
		if (LinkedActor->Implements<UActivable>())
		{
			IActivable::Execute_SetPlayerCount(LinkedActor,-1);
			if (IActivable::Execute_GetPlayerCount(LinkedActor) <= 0)
			IActivable::Execute_StopMainAction(LinkedActor);
		}
	}
}

void APressurePlate::Multi_PlaySound_Implementation()
{
	if (PlateSoundID)
	{
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (AudioDevice && PlateSoundID != AK_INVALID_PLAYING_ID)
		{
			AudioDevice->StopPlayingID(PlateSoundID);
			PlateSoundID = AK_INVALID_PLAYING_ID; // Reset
		}  
	}
	PlateSoundID = UAkGameplayStatics::PostEvent(PlateSound,this,0,FOnAkPostEventCallback(), false);
	
}

