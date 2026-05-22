#include "MuleAIController.h"

#include "AkGameplayStatics.h"
#include "Actors/Mule/Mule.h"
#include "Actors/Player/APlayerCharacter.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "GameFramework/CustomPlayerController.h"
#include "GameFramework/GameSession.h"
#include "Net/UnrealNetwork.h"

AMuleAIController::AMuleAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMuleAIController::BeginPlay()
{
	Super::BeginPlay();

	MyMule = Cast<AMule>(GetPawn());
	MyMule->SetOwner(GetWorld()->GetFirstPlayerController());
}

void AMuleAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
	if (!HasAuthority()) return;

	MyMule->CooldownTimer -= DeltaTime;
	MyMule->CooldownTimer = FMath::Clamp(MyMule->CooldownTimer, 0.f, MyMule->CallCooldown);

	if (MyMule->CallCharges > 0) return;

	MyMule->ChargesTimer -= DeltaTime;
	MyMule->ChargesTimer = FMath::Clamp(MyMule->ChargesTimer, 0.f, MyMule->ChargeCooldown);

	if (MyMule->ChargesTimer <= 0)
	{
		MyMule->CallCharges++;
		MyMule->ChargesTimer = MyMule->ChargeCooldown;
	}
}

void AMuleAIController::CallMule(AActor* Actor)
{
	if (!HasAuthority()) return;

	if (MyMule->CooldownTimer > 0.f || MyMule->CallCharges <= 0) return;
	
	FVector CorrectedForward = Actor->GetActorForwardVector();
	FVector SpawnPosition = Actor->GetActorLocation() + (CorrectedForward * SpawnOffset);
	
	AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Actor);
	ACustomPlayerController* PC = Cast<ACustomPlayerController>(Player->GetController());
	
	PC->Multicast_PlayTravelSound(SpawnPosition);
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUFunction(this, FName("OnSpawnTimerExpired"), SpawnPosition, PC);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_SpawnObject, TimerDelegate, SpawnDelay, false);
	
	FTimerHandle TimerHandle_Travel;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Travel, [this, Actor, SpawnPosition, PC]()
	{
		PC->Multicast_PlayTravelSound(SpawnPosition);
	}, 1.f, false);

	MyMule->PlayAppearVFX();
	MyMule->SetActorLocation(SpawnPosition);

	MyMule->CallCharges--;
	MyMule->CallCharges--;
	MyMule->CooldownTimer = MyMule->CallCooldown;

	if (MyMule->CallCharges <= 0)
	{
		MyMule->ChargesTimer = MyMule->ChargeCooldown;
	}
}

void AMuleAIController::CallMuleExtraction(AActor* Actor)
{
	if (!HasAuthority()) return;

	if (MyMule->CooldownTimer > 0.f || MyMule->CallCharges <= 0) return;
	
	FVector CorrectedForward = Actor->GetActorForwardVector();
	FVector SpawnPosition = Actor->GetActorLocation() + (CorrectedForward * SpawnOffset);
	
	ACustomPlayerController* PC = Cast<ACustomPlayerController>(GetWorld()->GetFirstPlayerController());
	PC->Multicast_PlayTravelSound(SpawnPosition);
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUFunction(this, FName("OnSpawnTimerExpired"), SpawnPosition, PC);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_SpawnObject, TimerDelegate, SpawnDelay, false);
	
	FTimerHandle TimerHandle_Travel;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Travel, [this, Actor, SpawnPosition, PC]()
	{
		PC->Multicast_PlayTravelSound(SpawnPosition);
	}, 1.f, false);

	MyMule->PlayAppearVFX();
	MyMule->SetActorLocation(SpawnPosition);

	MyMule->CallCharges--;
	MyMule->CallCharges--;
	MyMule->CooldownTimer = MyMule->CallCooldown;

	if (MyMule->CallCharges <= 0)
	{
		MyMule->ChargesTimer = MyMule->ChargeCooldown;
	}
}

void AMuleAIController::OnSpawnTimerExpired(FVector SpawnPos, ACustomPlayerController* Player)
{
	MyMule->SetActorLocation(SpawnPos);
	Player->Multicast_PlayTravelSound(SpawnPos);
	MyMule->DoAppearMovement();
}

