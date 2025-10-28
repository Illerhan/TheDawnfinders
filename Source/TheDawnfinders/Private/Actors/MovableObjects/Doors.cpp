// Fill out your copyright notice in the Description page of Project Settings.


#include "Doors.h"

#include "Net/UnrealNetwork.h"


// Sets default values
ADoors::ADoors()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ADoors::StartOpening()
{
	if (!HasAuthority()) return;

	if (!MoveCurve) return;

	bCanMove = false;
	bIsFullyOpen = false;

	if (Timeline.IsReversing())
	{
		Timeline.Play();
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door resuming opening"));
	}
	else if (!Timeline.IsPlaying())
	{
		Timeline.PlayFromStart();
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door opening"));
	}
}

void ADoors::StopOpening()
{
	if (!HasAuthority()) return;

	if (!MoveCurve) return;

	if (Timeline.IsPlaying() && !Timeline.IsReversing())
	{
		Timeline.Reverse();
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door closing"));
	}
	else if (!Timeline.IsPlaying() && bIsFullyOpen)
	{
		// Si complètement ouverte et on relâche, ferme
		Timeline.Reverse();
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door closing from open position"));
	}

}

// Called when the game starts or when spawned
void ADoors::BeginPlay()
{
	Super::BeginPlay();

	if (MoveCurve)
	{
		Timeline.AddInterpFloat(MoveCurve,TimelineProgress);
		Timeline.SetLooping(false);

		if (MoveCurve->FloatCurve.GetLastKey().Time > 0)
		{
			Timeline.SetPlayRate(MoveCurve->FloatCurve.GetLastKey().Time / MovementDuration);
		}

		FOnTimelineEvent TimeLineFinishedCallback;
		TimeLineFinishedCallback.BindUFunction(this, FName("OnTimelineFinished"));
		Timeline.SetTimelineFinishedFunc(TimeLineFinishedCallback);
	}
	
}

// Called every frame
void ADoors::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

