// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/MovableObjects/MovableObjects.h"
#include "Actors/MovableObjects/Doors.h"
// Sets default values



AMovableObjects::AMovableObjects()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	this->bReplicates = true;
	AActor::SetReplicateMovement(true);

	CurrentTimelineProgress = 0.0f;
	bIsMovingForward = true;
	LastReverseTime = 0.0f;
	ReverseCooldown = 2.f;
}

// Called when the game starts or when spawned
void AMovableObjects::BeginPlay()
{
	Super::BeginPlay();

	StartPosition = GetActorLocation();
	
	EndPosition = GetActorLocation() + EndPosition;
	

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
	
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastReverseTime < ReverseCooldown)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Reverse on cooldown - wait %.2f seconds"), 
			   ReverseCooldown - (CurrentTime - LastReverseTime));
		return;
	}
	
	if (MoveCurve)
	{
		Timeline.AddInterpFloat(MoveCurve,TimelineProgress);
		Timeline.SetLooping(false);
		Timeline.SetPlayRate(MoveCurve->FloatCurve.GetLastKey().Time / MovementDuration);
		if (CurrentTimelineProgress > 0.0f && CurrentTimelineProgress < 1.0f)
		{
			Timeline.SetPlaybackPosition(CurrentTimelineProgress, false);
			Timeline.Play();
			UE_LOG(LogTemp, Warning, TEXT("[SERVER] DoMovement resuming from position: %f"), CurrentTimelineProgress);
		}else
		{
			Timeline.PlayFromStart();
		}
		Timeline.SetTimelineFinishedFunc(TimelineFinished);
		bCanMove = false;
		bIsMovingForward = true;

		LastReverseTime = CurrentTime;
		
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] DoMovement called"));
	}
}

void AMovableObjects::DoReverseMovement()
{
	if (!HasAuthority()) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastReverseTime < ReverseCooldown)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Reverse on cooldown - wait %.2f seconds"), 
			   ReverseCooldown - (CurrentTime - LastReverseTime));
		return;
	}
	
	// Check if we're past the midpoint before allowing reverse
	if (CurrentTimelineProgress < 0.1f || CurrentTimelineProgress > 0.9f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Cannot reverse - not past midpoint (Progress: %f)"), CurrentTimelineProgress);
		return;
	}
    
	if (MoveCurve)
	{
		Timeline.AddInterpFloat(MoveCurve,TimelineProgress);
		Timeline.SetLooping(false);
		Timeline.SetPlayRate(MoveCurve->FloatCurve.GetLastKey().Time / MovementDuration);
		Timeline.Reverse();
		Timeline.SetTimelineFinishedFunc(TimelineFinished);
		bCanMove = false;
		bIsMovingForward = false;

		LastReverseTime = CurrentTime;
		
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] DoReverseMovement called"));
	}
}

void AMovableObjects::HandleProgress(float value)
{
	CurrentTimelineProgress = value;
	bCanMove = CanReverse();
	FVector NewPosition = FMath::Lerp(StartPosition, EndPosition, value);
	SetActorLocation(NewPosition);
}

void AMovableObjects::OnTimeLineFinished()
{
	ADoors* Door = Cast<ADoors>(this);
	if (Door)
	{
		Door->bIsFullyOpen = true;
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door fully opened"));
	}
	else
	{
		CurrentTimelineProgress = bIsMovingForward ? 1.0f : 0.0f;
		EndPosition = StartPosition;
		StartPosition = GetActorLocation();
		
		bCanMove = true;
	}
}

bool AMovableObjects::CanReverse() const
{
	return CurrentTimelineProgress >=0.1f &&  CurrentTimelineProgress <= 0.9f;
}

float AMovableObjects::GetTimelineProgress() const
{
	return CurrentTimelineProgress;
}