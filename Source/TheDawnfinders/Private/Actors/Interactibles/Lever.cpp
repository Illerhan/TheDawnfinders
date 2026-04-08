#include "Actors/Interactibles/Lever.h"
#include "Net/UnrealNetwork.h"
#include "Actors/Player/APlayerCharacter.h"


ALever::ALever()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
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

void ALever::DoPlayerAutoMove_Implementation(AAPlayerCharacter* Player)
{

}

bool ALever::GetCanBeUsed_Implementation(AActor* Interactor)
{
    Super::GetCanBeUsed_Implementation(Interactor);

    if(bHasInteractAnim && CurrentInteractActor)
        return false;

    return bCanBeUsed;
}

void ALever::Interact_Implementation(AActor* Interactor)
{
    if (!bCanBeUsed || (LinkedObjects.Num() == 0 && LinkedToggleables.Num() == 0)) return;

    // We need to go to the lever first 
    if (bHasInteractAnim && !CurrentInteractActor) {
        CurrentInteractActor = Interactor;
        AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Interactor);
        DoPlayerAutoMove(Player);
        UE_LOG(LogTemp, Error, TEXT("Start Interact With lever"));
        return;
    }
    else if (bHasInteractAnim && CurrentInteractActor != Interactor) {
        return;
    }

    if (bRequiresHold)
    {
        // Mode HOLD : maintenir pour ouvrir
        StartHoldInteraction(Interactor);
    }
    else
    {
        bIsOn = !bIsOn;

        UE_LOG(LogTemp, Error, TEXT("Interact With lever"));

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

        for (AActor* Actor : LinkedToggleables)
        {
            if (!Actor || !Actor->Implements<UToggleable>()) continue;

            if (IToggleable::Execute_IsActive(Actor))
            {
                IToggleable::Execute_Deactivate(Actor);
                UE_LOG(LogTemp, Warning, TEXT("[SERVER] Toggle: Deactivating %s"), *Actor->GetName());
            }
            else
            {
                IToggleable::Execute_Activate(Actor);
                UE_LOG(LogTemp, Warning, TEXT("[SERVER] Toggle: Activating %s"), *Actor->GetName());
            }
        }

        CurrentInteractActor = nullptr;

        Super::Interact_Implementation(Interactor);
    }
}

void ALever::StartHoldInteraction(AActor* Player)
{
    HoldPlayerCount++;
    if (HoldPlayerCount < HoldPlayerCountNeeded) return;

    // Starts Opening
    if (HoldPlayerCount == HoldPlayerCountNeeded) {
        for (AMovableObjects* const Object : LinkedObjects)
        {
            ADoors* Door = Cast<ADoors>(Object);
            if (Door)
            {
                Door->StartOpening();
                UE_LOG(LogTemp, Warning, TEXT("[SERVER] Hold: Starting door opening"));
            }
        } 
        return;
    }

    // Accelerates the opening
    for (AMovableObjects* const Object : LinkedObjects)
    {
        ADoors* Door = Cast<ADoors>(Object);
        if (Door)
        {
            Door->AddOpeningPlayer();
        }
    }
}

void ALever::StopInteract_Implementation(AActor* Interactor)
{
    if (bRequiresHold)
    {
        StopHoldInteraction(Interactor);
    }
    else {
        //CurrentInteractActor = nullptr;
    }
}

void ALever::StopHoldInteraction(AActor* Player)
{
    if (HoldPlayerCount <= 0) return;
    HoldPlayerCount--;

    // If some players still hold the lever (pause)
    if (HoldPlayerCount > 0 && HoldPlayerCount < HoldPlayerCountNeeded) {
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

    // If enough players remains (change speed)
    if (HoldPlayerCount >= HoldPlayerCountNeeded) {
        for (AMovableObjects* Object : LinkedObjects)
        {
            ADoors* Door = Cast<ADoors>(Object);
            if (Door)
            {
                Door->RemoveOpeningPlayer();
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

void ALever::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ALever, bIsOn);
}
