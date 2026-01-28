// Fill out your copyright notice in the Description page of Project Settings.


#include "CurseZone.h"

#include "Actors/Player/APlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/UHealthComponent.h"


// Sets default values
ACurseZone::ACurseZone()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	CurseCollider = CreateDefaultSubobject<USphereComponent>("CurseCollider");
	CurseCollider->SetupAttachment(RootComponent);
	CurseCollider->SetSphereRadius(CurseRadius);
	
}

// Called when the game starts or when spawned
void ACurseZone::BeginPlay()
{
	Super::BeginPlay();
	if (CurseCollider)
	{
		// On lie l'événement dynamique
		CurseCollider->OnComponentBeginOverlap.AddDynamic(this, &ACurseZone::OnCurseOverlapBegin);
		
		CurseCollider->OnComponentEndOverlap.AddDynamic(this, &ACurseZone::OnCurseOverlapEnd);
	}
}

// Called every frame
void ACurseZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float CurrentRadius = CurseCollider->GetUnscaledSphereRadius();
	CurseCollider->SetSphereRadius(CurrentRadius + (CurseRate*DeltaTime));
	
}

void ACurseZone::OnCurseOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor);
		if (Player)
		{
			// Debug visuel
			//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("DANGER : Tu es DANS la zone !"));

			Player->HealthComponent->CurseZone++;
		}
	}
}

void ACurseZone::OnCurseOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor != this)
	{
		AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor);
		if (Player)
		{
			// Debug visuel
			//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("OUF : Tu es SORTI de la zone."));

			Player->HealthComponent->CurseZone--;
		}
	}
}

