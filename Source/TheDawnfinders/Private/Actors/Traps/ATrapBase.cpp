#include "Actors/Traps/ATrapBase.h"

#include "Actors/Enemy/ABaseEnemy.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


ATrapBase::ATrapBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	TrapCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("TrapCollider"));
	TrapCollider->SetupAttachment(GetRootComponent());
	TrapCollider->SetCollisionResponseToAllChannels(ECR_Overlap);
}


void ATrapBase::BeginPlay()
{
	Super::BeginPlay();

	if (TrapCollider)
	{
		TrapCollider->OnComponentBeginOverlap.AddDynamic(this, &ATrapBase::OnTrapOverlapBegin);
	}

	if (FloatCurve)
	{
		FOnTimelineFloat ProgressFunction{};
		ProgressFunction.BindUFunction(this, FName("HandleFadeProgress"));

		FadeTimeline.AddInterpFloat(FloatCurve, ProgressFunction);

		FadeTimeline.SetLooping(false);
	}
}


void ATrapBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentCooldown > 0.f)
		CurrentCooldown -= DeltaTime;

	FadeTimeline.TickTimeline(DeltaTime);
}


void ATrapBase::OnTrapOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
							   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
							   bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bEnable) return;
	if (CurrentCooldown > 0.f) return;
	if (!OtherActor || OtherActor == this) return;
	TrappedActor = OtherActor;

	// Run trap action (server side)
	if (OtherActor->Implements<UDamageable>())
	{
		DoTrapAction();

		// Multicast sound FX
		Multicast_PlayEffects();

		// Start cooldown
		CurrentCooldown = Cooldown;
	}
}



void ATrapBase::Multicast_PlayEffects_Implementation()
{
	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
	}
}

void ATrapBase::DisableTrap()
{
	if (HasAuthority())
	{
		bEnable = false;
	}
	else
	{
		// If a client calls it, ensure server handles it
		UE_LOG(LogTemp, Warning, TEXT("Client tried to disable trap. Should call on server."));
	}
}

void ATrapBase::OnRep_Enabled()
{
	// effet visuel
}

void ATrapBase::DoTrapAction()
{

}

void ATrapBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATrapBase, bEnable);
}