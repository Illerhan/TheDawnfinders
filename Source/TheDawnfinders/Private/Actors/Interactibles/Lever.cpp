#include "Actors/Interactibles/Lever.h"

ALever::ALever()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ALever::BeginPlay()
{
    HoldPlayerCount = 0;
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
        // Mode HOLD : maintenir pour ouvrir
        StartHoldInteraction(Interactor);
    }
    else
    {
        // Mode TOGGLE : clic pour ouvrir/fermer
        for (AMovableObjects* const Object : LinkedObjects)
        {
            if (!Object) continue;

            ADoors* Door = Cast<ADoors>(Object);
            if (Door)
            {
                // Toggle door: si fermée -> ouvre, si ouverte -> ferme
                if (Door->bIsFullyOpen || Door->GetTimelineProgress() > 0.5f)
                {
                    Door->StopOpening(); // Ferme la porte
                    UE_LOG(LogTemp, Warning, TEXT("[SERVER] Toggle: Closing door %s (Progress: %f, FullyOpen: %d)"), 
                           *Door->GetName(), Door->GetTimelineProgress(), Door->bIsFullyOpen);
                }
                else
                {
                    Door->StartOpening(); // Ouvre la porte
                    UE_LOG(LogTemp, Warning, TEXT("[SERVER] Toggle: Opening door %s"), *Door->GetName());
                }
            }
            else
            {
                // Pour les autres objets movables (non-portes)
                if (!Object->bCanMove) continue;
                
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
            }
        }
        
        Super::Interact_Implementation(Interactor);
    }
}

void ALever::StartHoldInteraction(AActor* Player)
{
    HoldPlayerCount++;
    if (HoldPlayerCount < HoldPlayerCountNeeded) return;

    for (AMovableObjects* const Object : LinkedObjects)
    {
        ADoors* Door = Cast<ADoors>(Object);
        if (Door)
        {
            Door->StartOpening();
            UE_LOG(LogTemp, Warning, TEXT("[SERVER] Hold: Starting door opening"));
        }
    }
}

void ALever::StopInteract_Implementation(AActor* Interactor)
{
    if (bRequiresHold)
    {
        StopHoldInteraction(Interactor);
    }
}

void ALever::StopHoldInteraction(AActor* Player)
{
    if (HoldPlayerCount <= 0) return;
    HoldPlayerCount--;

    // If some players still hold the lever
    if (HoldPlayerCount > 0) {
        for (AMovableObjects* Object : LinkedObjects)
        {
            ADoors* Door = Cast<ADoors>(Object);
            if (Door)
            {
                Door->PauseOpening();
                UE_LOG(LogTemp, Warning, TEXT("[SERVER] Hold: Pause door: %s"), *Door->GetName());
            }
        }
        return;
    }

    // If no players remain
    for (AMovableObjects* Object : LinkedObjects)
    {
        ADoors* Door = Cast<ADoors>(Object);
        if (Door)
        {
            Door->StopOpening();
            UE_LOG(LogTemp, Warning, TEXT("[SERVER] Hold: Closing door: %s"), *Door->GetName());
        }
    }
}