// Fill out your copyright notice in the Description page of Project Settings.
#include "Actors/Traps/ATrapBase.h"
#include "AssetTypeActions/AssetDefinition_SoundBase.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATrapBase::ATrapBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	TrapCollider =  CreateDefaultSubobject<UBoxComponent>(TEXT("TrapCollider"));
	RootComponent = TrapCollider;
	
	TrapCollider->SetCollisionResponseToAllChannels(ECR_Overlap);
}

void ATrapBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						   bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		UE_LOG(LogTemp, Warning, TEXT("Overlap avec: %s"), *OtherActor->GetName());
		UGameplayStatics::PlaySoundAtLocation(this,Sound,GetActorLocation());
		DoTrapAction();
	}
}

// Called when the game starts or when spawned
void ATrapBase::BeginPlay()
{
	Super::BeginPlay();

	TrapCollider->OnComponentBeginOverlap.AddDynamic(this, &ATrapBase::OnOverlapBegin);
}

// Called every frame
void ATrapBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

