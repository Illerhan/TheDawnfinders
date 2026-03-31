
#include "Actors/Player/ANoise.h"

ANoise::ANoise()
{
	PrimaryActorTick.bCanEverTick = true;

	NoiseZone = CreateDefaultSubobject<USphereComponent>(FName("ProtectionZone"));
	NoiseZone->SetupAttachment(RootComponent);
}

void ANoise::BeginPlay()
{
	NoiseZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Super::BeginPlay();
	
}

void ANoise::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	NoiseZone->SetSphereRadius(Radius);

	Timer += DeltaTime;
	if (Timer > 0.05f)
		NoiseZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (bIsConstant) return;

	if (Timer > 0.2f)
		Destroy();
}

