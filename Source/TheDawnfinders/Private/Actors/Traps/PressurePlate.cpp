// Fill out your copyright notice in the Description page of Project Settings.


#include "PressurePlate.h"

#include "AkAudioDevice.h"
#include "AkGameplayStatics.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Interfaces/Activable.h"
#include "Net/UnrealNetwork.h"


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

void APressurePlate::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);	
	DOREPLIFETIME(APressurePlate, bIsActive);
}


void APressurePlate::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Cast<AAPlayerCharacter>(OtherActor)) return;
    
	CurrentPlayerCount++;
    
	// Si c'est le premier joueur qui marche dessus
	if (CurrentPlayerCount == 1)
	{
		bool bShouldActivate = true;

		if (ActivationMode == EPlateActivationMode::Toggle)
		{
			// Inverse l'état actuel
			bIsActive = !bIsActive;
			bShouldActivate = bIsActive;
		}
		else if (ActivationMode == EPlateActivationMode::Once)
		{
			if (bIsActive) return; // Déjà fait
			bIsActive = true;
		}
		else // Mode Hold
		{
			bIsActive = true;
		}

		// Exécution de l'action
		ExecutePlateAction(bShouldActivate);
	}
	
}

void APressurePlate::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!Cast<AAPlayerCharacter>(OtherActor)) return;
    
	CurrentPlayerCount--;

	// On ne traite la sortie que si plus personne n'est sur la plaque
	if (CurrentPlayerCount == 0 && ActivationMode == EPlateActivationMode::Hold)
	{
		bIsActive = false;
		ExecutePlateAction(false);
	}
}

void APressurePlate::ExecutePlateAction(bool bActivate)
{
	OnRep_IsActive(); // Pour le visuel/FX
	Multi_PlaySound(); // Son spatialisé synchronisé

	for (AActor* LinkedActor : LinkedActors)
	{
		if (LinkedActor && LinkedActor->Implements<UActivable>())
		{
			if (bActivate)
			{
				IActivable::Execute_DoMainAction(LinkedActor);
			}
			else
			{
				IActivable::Execute_StopMainAction(LinkedActor);
			}
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

