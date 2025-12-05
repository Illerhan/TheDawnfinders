#include "Actors/Interactibles/Lever.h"


ALever::ALever()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALever::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALever::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ALever::Interact_Implementation(AActor* Interactor)
{
	if (!bCanBeUsed || LinkedObjects.Num() == 0) return;

	if (bRequiresHold)
	{
		StartHoldInteraction(Interactor);
	}

	else
	{
		for (AMovableObjects* const Object : LinkedObjects)
			if (Object && Object->bCanMove)
			{
				bool bReverse = Object->CanReverse();
				UE_LOG(LogTemp, Warning, TEXT("moving %s"), *Object->GetName());
				if (bReverse && Object->bIsMovingForward)
				{
					Object->DoReverseMovement();
				}
				else
				{
					Object->DoMovement();
				}

				Super::Interact_Implementation(Interactor);
			}
	}
}


void ALever::StartHoldInteraction(AActor* Player)
{
	for (AMovableObjects* const Object : LinkedObjects)
	{
		ADoors* Door = Cast<ADoors>(Object);
		if (Door)
		{
			Door->StartOpening();
			UE_LOG(LogTemp, Warning, TEXT("[SERVER] Lever starting door opening"));
		}
	}
}


void ALever::StopInteract_Implementation(AActor* Interactor)
{
	StopHoldInteraction(Interactor);
}


void ALever::StopHoldInteraction(AActor* Player)
{
	for (AMovableObjects* Object : LinkedObjects)
	{
		ADoors* Door = Cast<ADoors>(Object);
		if (Door)
		{
			Door->StopOpening();
			UE_LOG(LogTemp, Warning, TEXT("[SERVER] Closing door: %s"), *Door->GetName());
		}
	}
}
