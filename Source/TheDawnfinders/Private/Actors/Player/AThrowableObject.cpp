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

	if (!bDestroyOnHit) {
		DestroyTimer += DeltaTime;
		if (DestroyTimer >= DestroyDuration) {
			AActor::Destroy();
		}
	}
}


void AThrowableObject::Initialise(UItemData* Data, AActor* Origin)
{
	ProgressTimer = 0;

	OriginActor = Origin;
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

	/*DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		EffectRange,     
		16,               
		bHit ? FColor::Red : FColor::Green, 
		false,        
		2.0f            
	);*/

	Client_DoCollisionEffect();

	for (FHitResult& Hit : HitResults)
	{
		if (!Hit.GetActor()) continue;
		if (!Hit.GetActor()->ActorHasTag("Enemy")) continue;

		switch (EffectType) {
		case EThrowableEffectType::Explodes :
			IDamageable::Execute_ReceiveDamage(Hit.GetActor(), ItemData->ConsumableEffectPower, NULL);
			break;

		case EThrowableEffectType::PlayLoudSound :
			Cast<ABasicEnemyAIController>(Cast<ABaseEnemy>(Hit.GetActor())->GetController())->AddAlertness(ItemData->ConsumableEffectPower, GetActorLocation());
			break;

		case EThrowableEffectType::StunEnemies:
			Cast<ABaseEnemy>(Hit.GetActor())->StunEnemy(EffectDuration);
			break;
		}
	}

	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	Params.Instigator = Cast<APawn>(GetOwner());

	ANoise* Noise = GetWorld()->SpawnActor<ANoise>(NoiseObject, GetActorLocation(), FRotator(0, 0, 0), Params);
	Noise->Radius = NoiseRange;
	Noise->bIsLoud = bIsLoud;
	Noise->NoiseOriginActor = OriginActor;

	if (bPlayVFXOnHit || destroyDelay > 0) return; 

	AActor::Destroy();
}

void AThrowableObject::Client_DoCollisionEffect_Implementation()
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

	/*DrawDebugSphere(
		GetWorld(),
		GetActorLocation(),
		EffectRange,
		16,
		bHit ? FColor::Red : FColor::Green,
		false,
		2.0f
	);*/

	if (bPlayVFXOnHit || destroyDelay > 0) return;

	AActor::Destroy();
}
