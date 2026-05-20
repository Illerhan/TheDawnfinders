// Fill out your copyright notice in the Description page of Project Settings.


#include "AmbientMusicZone.h"
#include "MusicManager.h"
#include "Components/BoxComponent.h"


AAmbientMusicZone::AAmbientMusicZone()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneTrigger"));
	RootComponent = ZoneTrigger;
	ZoneTrigger->SetCollisionProfileName(TEXT("Trigger"));
}

void AAmbientMusicZone::BeginPlay()
{
	Super::BeginPlay();

	// Musique gérée uniquement côté client
	ZoneTrigger->OnComponentBeginOverlap.AddDynamic(this, &AAmbientMusicZone::OnPlayerEnter);
	ZoneTrigger->OnComponentEndOverlap.AddDynamic(this, &AAmbientMusicZone::OnPlayerExit);
}

void AAmbientMusicZone::OnPlayerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC || !PC->IsLocalController()) return;

	UMusicManager* MM = GetGameInstance()->GetSubsystem<UMusicManager>();
	if (!MM) return;

	MM->EnterZone(this);

}

void AAmbientMusicZone::OnPlayerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}
