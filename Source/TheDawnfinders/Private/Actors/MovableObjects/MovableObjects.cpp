#include "Actors/MovableObjects/MovableObjects.h"
#include "Actors/MovableObjects/Doors.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"


AMovableObjects::AMovableObjects()
{
	PrimaryActorTick.bCanEverTick = true;
	this->bReplicates = true;
	AActor::SetReplicateMovement(true);

	CurrentTimelineProgress = 0.0f;
	bIsMovingForward = true;
	LastReverseTime = 0.0f;
	ReverseCooldown = 2.f;
}

void AMovableObjects::BeginPlay()
{
	Super::BeginPlay();

	StartPosition = GetActorLocation();
	OriginalStart = StartPosition;
	
	FinalPosition = GetActorLocation() + EndPosition;

	if (MoveCurve)
	{
		TimelineProgress.BindUFunction(this, FName("HandleProgress"));
		TimelineFinished.BindUFunction(this, FName("OnTimeLineFinished"));
	}
}

void AMovableObjects::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (HasAuthority())
		Timeline.TickTimeline(DeltaTime);
	
}


#pragma region Activable Interface

void AMovableObjects::DoMainAction_Implementation()
{

}

void AMovableObjects::StopMainAction_Implementation()
{

}

#pragma endregion


void AMovableObjects::DoMovement_Implementation()
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
			CurrentTimelineProgress = 0.0f;
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
	FVector NewPosition = FMath::Lerp(StartPosition, FinalPosition, value);
	SetActorLocation(NewPosition);
}

void AMovableObjects::OnTimeLineFinished()
{
	ADoors* Door = Cast<ADoors>(this);
	if (Door)
	{
		// CORRECTION : On vérifie où la Timeline s'est arrêtée
		// Si on est à la fin (ou presque), la porte est ouverte
		if (Timeline.GetPlaybackPosition() >= 0.99f)
		{
			Door->bIsFullyOpen = true;
			UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door finished OPENING (Fully Open)"));
		}
		// Si on est au début (ou presque), la porte est fermée
		else
		{
			Door->bIsFullyOpen = false;
			UE_LOG(LogTemp, Warning, TEXT("[SERVER] Door finished CLOSING (Closed)"));
		}

		// Important : on libère le mouvement dans les deux cas
		bCanMove = true;
		// On remet la progress bar à jour parfaitement (0 ou 1)
		CurrentTimelineProgress = Timeline.GetPlaybackPosition();
       
		return;
	}
	if (!bIsMovingForward)   
	{
		CurrentTimelineProgress = 0.f;
		bCanMove = true;
		return;
	}
	bCanMove = true;
	
	FinalPosition = StartPosition;
	StartPosition = GetActorLocation();
	
	CurrentTimelineProgress = 1.0f;
	bCanMove = true;

	UE_LOG(LogTemp, Warning, TEXT("[SERVER] Forward movement completed"));
}

bool AMovableObjects::CanReverse() const
{
	return CurrentTimelineProgress >=0.1f &&  CurrentTimelineProgress <= 0.9f;
}

float AMovableObjects::GetTimelineProgress() const
{
	return CurrentTimelineProgress;
}