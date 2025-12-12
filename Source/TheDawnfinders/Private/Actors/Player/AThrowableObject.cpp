#include "Actors/Player/AThrowableObject.h"
#include "DataAssets/ItemData.h"
#include "Interfaces/IDamageable.h"


AThrowableObject::AThrowableObject()
{
	PrimaryActorTick.bCanEverTick = true;
}


void AThrowableObject::BeginPlay()
{
	Super::BeginPlay();
	
}


void AThrowableObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ProgressTimer += DeltaTime;

	if (ProgressTimer < ThrowDuration) {
		ActualisePosition();
	}
	else {
		DoCollisionEffect();
	}
}


void AThrowableObject::Initialise(FVector FinalPos, UItemData* Data)
{
	StartPos = GetActorLocation();
	EndPos = FinalPos;
	ProgressTimer = 0;

	ItemData = Data;
}


void AThrowableObject::ActualisePosition()
{
	FVector CurrentPos = FMath::Lerp(StartPos, EndPos, ProgressTimer / ThrowDuration);
	float AddedY = FMath::Sin(FMath::Lerp(0, 3.14f, ProgressTimer / ThrowDuration)) * 300.f;

	CurrentPos += FVector(0, 0, AddedY);

	SetActorLocation(CurrentPos);
}

void AThrowableObject::DoCollisionEffect()
{
	// We get the in range actors
	TArray<FHitResult> HitResults;
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,          
		GetActorLocation(),
		GetActorLocation(),
		FQuat::Identity,
		ECC_GameTraceChannel1,
		FCollisionShape::MakeSphere(EffectRange)
	);

	DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		EffectRange,     
		16,               
		bHit ? FColor::Red : FColor::Green, 
		false,        
		2.0f            
	);


	// We apply the throwable object effects on the actors in range
	if (!bHit) return;

	for (FHitResult& Hit : HitResults)
	{
		if (!Hit.GetActor()) continue;
		if (!Hit.GetActor()->ActorHasTag("Enemy")) continue;

		switch (EffectType) {
		case EThrowableEffectType::Explodes :
			IDamageable::Execute_ReceiveDamage(Hit.GetActor(), ItemData->ConsumableEffectPower, this);
			break;

		case EThrowableEffectType::PlayLoudSound :
			break;
		}
	}

	AActor::Destroy();
}
