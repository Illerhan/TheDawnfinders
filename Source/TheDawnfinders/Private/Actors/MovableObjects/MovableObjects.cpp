// Fill out your copyright notice in the Description page of Project Settings.


#include "MovableObjects.h"
// Sets default values



AMovableObjects::AMovableObjects()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	this->bReplicates = true;
	AActor::SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AMovableObjects::BeginPlay()
{
	Super::BeginPlay();

	StartPosition = GetActorLocation();

	if (MoveCurve)
	{
		TimelineProgress.BindUFunction(this, FName("HandleProgress"));
		TimelineFinished.BindUFunction(this, FName("OnTimeLineFinished"));
	}
}

// Called every frame
void AMovableObjects::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (HasAuthority())
		Timeline.TickTimeline(DeltaTime);
	
}

void AMovableObjects::DoMovement()
{
	if (!HasAuthority()) return;
	
	if (MoveCurve)
	{
		Timeline.AddInterpFloat(MoveCurve,TimelineProgress);
		Timeline.SetLooping(false);
		Timeline.SetPlayRate(MoveCurve->FloatCurve.GetLastKey().Time / MovementDuration);
		Timeline.PlayFromStart();
		Timeline.SetTimelineFinishedFunc(TimelineFinished);
		bCanMove = false;

		UE_LOG(LogTemp, Warning, TEXT("[SERVER] DoMovement called"));
	}
}

void AMovableObjects::HandleProgress(float value)
{
	FVector NewPosition = FMath::Lerp(StartPosition, EndPosition, value);
	SetActorLocation(NewPosition);
}

void AMovableObjects::OnTimeLineFinished()
{
	EndPosition = StartPosition;
	StartPosition = GetActorLocation();
	bCanMove  = true;
}
