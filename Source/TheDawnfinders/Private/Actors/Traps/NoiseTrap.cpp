// Fill out your copyright notice in the Description page of Project Settings.


#include "NoiseTrap.h"

#include "Actors/Player/APlayerCharacter.h"
#include "GameFramework/SoundManager.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ANoiseTrap::ANoiseTrap()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ANoiseTrap::BeginPlay()
{
	Super::BeginPlay();
	AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASoundManager::StaticClass());
	SoundManagerInstance = Cast<ASoundManager>(FoundActor);
	if (!SoundManagerInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("ALitter: Attention, aucun ASoundManager trouvé dans le niveau !"));
	}
	
}

// Called every frame
void ANoiseTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANoiseTrap::DoTrapAction(AActor* OtherActor)
{
	
	Super::DoTrapAction(OtherActor);
	AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor);
	if (!Player) return;
	
	Player->Execute_PlaySoundOnServer(Player,"Distraction",1000,1);
	SoundManagerInstance->MultiPlaySound(Sound,GetActorLocation(),100,1000,true);
	
}
