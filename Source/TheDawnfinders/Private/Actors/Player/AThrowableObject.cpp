#include "Actors/Player/AThrowableObject.h"
#include "DataAssets/ItemData.h"
#include "Actors/Enemy/ABaseEnemy.h"
#include "Others/BasicEnemyAIController.h"
#include "Interfaces/IDamageable.h"


AThrowableObject::AThrowableObject()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
}


void AThrowableObject::BeginPlay()
{
	Super::BeginPlay();
	
}


void AThrowableObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority()) return;

	ProgressTimer += DeltaTime;

	if (ProgressTimer < ThrowDuration) {
		//ActualisePosition();
	}
	else {
		//Server_DoCollisionEffect();
	}
}


void AThrowableObject::Initialise(UItemData* Data)
{
	ProgressTimer = 0;

	ItemData = Data;
}

void AThrowableObject::DoStartImpulse_Implementation(FVector Direction, float Strength)
{
	
}


void AThrowableObject::ActualisePosition()
{
}

void AThrowableObject::Server_DoCollisionEffect_Implementation()
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

	for (FHitResult& Hit : HitResults)
	{
		if (!Hit.GetActor()) continue;
		if (!Hit.GetActor()->ActorHasTag("Enemy")) continue;

		switch (EffectType) {
		case EThrowableEffectType::Explodes :
			IDamageable::Execute_ReceiveDamage(Hit.GetActor(), ItemData->ConsumableEffectPower, this);
			break;

		case EThrowableEffectType::PlayLoudSound :
			Cast<ABasicEnemyAIController>(Cast<ABaseEnemy>(Hit.GetActor())->GetController())->AddAlertness(ItemData->ConsumableEffectPower, GetActorLocation());
			break;
		}
	}

	AActor::Destroy();
}
