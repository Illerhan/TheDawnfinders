
#include "Actors/Player/ANoise.h"

ANoise::ANoise()
{
	PrimaryActorTick.bCanEverTick = true;

	NoiseZone = CreateDefaultSubobject<USphereComponent>(FName("ProtectionZone"));
	NoiseZone->SetupAttachment(RootComponent);
}

void ANoise::BeginPlay()
{
	Super::BeginPlay();
	
}

void ANoise::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsConstant) return;

	Timer += DeltaTime;
	if (Timer > 0.05f)
		delete this;
}

