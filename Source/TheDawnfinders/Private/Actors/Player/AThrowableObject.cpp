#include "Actors/Player/AThrowableObject.h"
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


void AThrowableObject::Initialise(FVector FinalPos)
{
	StartPos = GetActorLocation();
	EndPos = FinalPos;
	ProgressTimer = 0;
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
		ECC_Visibility,
		FCollisionShape::MakeSphere(EffectRange)
	);

	DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		EffectRange,      // ton rayon
		16,               // nombre de segments
		bHit ? FColor::Red : FColor::Green, // couleur selon collision
		false,            // ne reste pas indéfiniment
		2.0f              // durée en secondes
	);


	// We apply the throwable object effects on the actors in range
	if (!bHit) return;

	for (FHitResult& Hit : HitResults)
	{
		switch (EffectType) {
		case EThrowableEffectType::Explodes :
			if (Hit.GetActor()->GetClass()->ImplementsInterface(UDamageable::StaticClass())) {
				IDamageable* Interface = Cast<IDamageable>(Hit.GetActor());
				Interface->ReceiveDamage_Implementation(EffectPower, this);
			}
			break;

		case EThrowableEffectType::PlayLoudSound :
			break;
		}
	}

	AActor::Destroy();
}
