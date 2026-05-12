#include "MuleAIController.h"

#include "AkGameplayStatics.h"
#include "Actors/Mule/Mule.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
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
	
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUFunction(this, FName("OnSpawnTimerExpired"), SpawnPosition);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_SpawnObject, TimerDelegate, SpawnDelay, false);
	
	Multi_PlaySound();
	
	FTimerHandle TimerHandle_Travel;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_Travel, [this, Actor]()
	{
		Multi_PlaySound();
	}, 0.5f, false);

	MyMule->SetActorLocation(SpawnPosition);
	MyMule->PlayAppearVFX();

	MyMule->CallCharges--;
	MyMule->CooldownTimer = MyMule->CallCooldown;

	if (MyMule->CallCharges <= 0)
	{
		MyMule->ChargesTimer = MyMule->ChargeCooldown;
	}
}

void AMuleAIController::Multi_PlaySound_Implementation(FVector Position)
{
	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (AudioDevice && MuleTravelSoundID != AK_INVALID_PLAYING_ID)
	{
		AudioDevice->StopPlayingID(MuleTravelSoundID, 300, AkCurveInterpolation_Log1);
		MuleTravelSoundID = AK_INVALID_PLAYING_ID;
	}
	MuleTravelSoundID = UAkGameplayStatics::PostEvent(MuleTravelSound,GetOwner(),0,FOnAkPostEventCallback(), false);
}

void AMuleAIController::OnSpawnTimerExpired(FVector SpawnPos)
{
	MyMule->SetActorLocation(SpawnPos);
	Multi_PlaySound(SpawnPos);

	MyMule->DoAppearMovement();
}

