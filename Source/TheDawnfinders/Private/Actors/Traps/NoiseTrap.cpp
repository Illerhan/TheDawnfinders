// Fill out your copyright notice in the Description page of Project Settings.


#include "NoiseTrap.h"

#include "AkGameplayStatics.h"
#include "Actors/Player/APlayerCharacter.h"
#include "GameFramework/SoundManager.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ANoiseTrap::ANoiseTrap()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANoiseTrap::BeginPlay()
{
	Super::BeginPlay();
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASoundManager::StaticClass());
	SoundManagerInstance = Cast<ASoundManager>(FoundActor);
	if (!SoundManagerInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("ALitter: Attention, aucun ASoundManager trouvé dans le niveau !"));
	}
	
}

// Called every frame
void ANoiseTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANoiseTrap::DoTrapAction(AActor* OtherActor)
{
	
	Super::DoTrapAction(OtherActor);
	
	if (OtherActor->ActorHasTag("Player")) {
		ANoise* NoiseObj = GetWorld()->SpawnActor<ANoise>(NoiseActor, GetActorLocation(), GetActorRotation());
		NoiseObj->NoiseZone->SetSphereRadius(NoiseRange);
		NoiseObj->NoiseOriginActor = OtherActor;

		SoundManagerInstance->MultiPlaySound(Sound, GetActorLocation(), 100, 1000, true);
	}
}

void ANoiseTrap::Multi_PlaySound_Implementation()
{
	if (TrapSoundID)
	{
		FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
		if (AudioDevice && TrapSoundID != AK_INVALID_PLAYING_ID)
		{
			AudioDevice->StopPlayingID(TrapSoundID);
			TrapSoundID = AK_INVALID_PLAYING_ID; // Reset
		} 
	}
	TrapSoundID = UAkGameplayStatics::PostEvent(TrapSound,this,0,FOnAkPostEventCallback(), false);
}
