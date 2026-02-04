// Fill out your copyright notice in the Description page of Project Settings.


#include "CurseZone.h"

#include "Actors/Player/APlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/UHealthComponent.h"

ACurseZone::ACurseZone()
{
	PrimaryActorTick.bCanEverTick = true;
	
	CurseCollider = CreateDefaultSubobject<USphereComponent>("CurseCollider");
	CurseCollider->SetupAttachment(RootComponent);
	CurseCollider->SetSphereRadius(CurseRadius);
	
}

void ACurseZone::BeginPlay()
{
	Super::BeginPlay();
	if (CurseCollider)
	{
		CurseCollider->OnComponentBeginOverlap.AddDynamic(this, &ACurseZone::OnCurseOverlapBegin);
		CurseCollider->OnComponentEndOverlap.AddDynamic(this, &ACurseZone::OnCurseOverlapEnd);
	}
}

void ACurseZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float CurrentRadius = CurseCollider->GetUnscaledSphereRadius();
	CurseCollider->SetSphereRadius(CurrentRadius + (CurseRate*DeltaTime));

	if (!DestroyAfterTimer) return;

	DestroyTimer -= DeltaTime;
	if (DestroyTimer <= 0) Destroy();
}

void ACurseZone::Initialise(float Radius, float CurseZoneDelay, float Duration)
{
	DestroyAfterTimer = true;
	DestroyTimer = Duration;

	if (HasAuthority()) {
		Multicast_Initialise(Radius, CurseZoneDelay);
	}
}

void ACurseZone::Multicast_Initialise_Implementation(float Radius, float CurseZoneDelay)
{
	CurseCollider->SetSphereRadius(Radius);
	CurseCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorldTimerManager().SetTimer(StartCurseDelayHandle, this, &ACurseZone::InitialiseAfterDelay, CurseZoneDelay, false);
}

void ACurseZone::InitialiseAfterDelay()
{
	CurseCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
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

