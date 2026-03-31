#include "MuleAIController.h"
#include "Actors/Mule/Mule.h"
#include "Net/UnrealNetwork.h"

AMuleAIController::AMuleAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMuleAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AMuleAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
	if (!HasAuthority()) return;

	AMule* MyMule = Cast<AMule>(GetPawn());
	if (!MyMule) return;

	// Décompte du cooldown entre les calls
	MyMule->CooldownTimer -= DeltaTime;
	MyMule->CooldownTimer = FMath::Clamp(MyMule->CooldownTimer, 0.f, MyMule->CallCooldown);

	// On ne recharge QUE si on est à 0 charge
	if (MyMule->CallCharges > 0) return;

	// Décompte du timer de recharge
	MyMule->ChargesTimer -= DeltaTime;
	MyMule->ChargesTimer = FMath::Clamp(MyMule->ChargesTimer, 0.f, MyMule->ChargeCooldown);

	// Timer écoulé → on redonne une charge
	if (MyMule->ChargesTimer <= 0)
	{
		MyMule->CallCharges++;
		MyMule->ChargesTimer = MyMule->ChargeCooldown;
	}
}

void AMuleAIController::CallMule(AActor* Actor)
{
	if (!HasAuthority()) return;

	AMule* MyMule = Cast<AMule>(GetPawn());
	if (!MyMule) return;

	if (MyMule->CooldownTimer > 0.f || MyMule->CallCharges <= 0) return;
	
	FVector CorrectedForward = Actor->GetActorForwardVector();
    
	FVector SpawnPosition = Actor->GetActorLocation() + (CorrectedForward * SpawnOffset);
	    
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUFunction(this, FName("OnSpawnTimerExpired"), SpawnPosition);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_SpawnObject, TimerDelegate, SpawnDelay, false);
	
	MyMule->CallCharges--;
	MyMule->CooldownTimer = MyMule->CallCooldown;

	if (MyMule->CallCharges <= 0)
	{
		MyMule->ChargesTimer = MyMule->ChargeCooldown;
	}
}

void AMuleAIController::OnSpawnTimerExpired(FVector SpawnPos)
{
	AMule* MyMule = Cast<AMule>(GetPawn());
	if (!MyMule) return;
	MyMule->SetActorLocation(SpawnPos);	
}
