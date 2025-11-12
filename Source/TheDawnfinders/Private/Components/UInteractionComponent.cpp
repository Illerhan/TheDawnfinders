// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UInteractionComponent.h"
#include "Net/UnrealNetwork.h"
#include "Actors/Player/APlayerCharacter.h"

#include "Actors/Interactibles/Interactible.h"
#include "Actors/Interactibles/Lever.h"
#include "Actors/Interactibles/ZiplineInteractible.h"


UInteractionComponent::UInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetIsReplicatedByDefault(true);
}


#pragma region Near Interactibles Management

void UInteractionComponent::AddInteractible(AActor* Interactible)
{
    InteractiblesAtRange.Add(Interactible);
}


void UInteractionComponent::RemoveInteractible(AActor* Interactible)
{
    InteractiblesAtRange.Remove(Interactible);
}


AActor* UInteractionComponent::GetNearestInteractible()
{
    float bestDist = FLT_MAX;
    AActor* pickedInteractible = nullptr;

    for (AActor* Interactible : InteractiblesAtRange)
    {
        float currentDist = (Interactible->GetActorLocation() - GetOwner()->GetActorLocation()).Length();
        if (currentDist < bestDist)
        {
            pickedInteractible = Interactible;
            bestDist = currentDist;
        }
    }
    return pickedInteractible;
}

#pragma endregion


#pragma region Start Interaction

void UInteractionComponent::StartInteract()
{
    AActor* NereastInteractible = GetNearestInteractible();
    AInteractibleObjects* Interactible = Cast<AInteractibleObjects>(NereastInteractible);

    if (Interactible && Interactible->bCanBeUsed)
    {
        CurrentInteractible = Interactible;
        ALever* Lever = Cast<ALever>(Interactible);
        if (Lever && Lever->bCanBeUsed)
        {
            TryInteract(Interactible, Cast<AAPlayerCharacter>(GetOwner()));
        }
        else
        {
            TryInteract(Interactible, Cast<AAPlayerCharacter>(GetOwner()));
        }
    }
}


void UInteractionComponent::TryInteract(AInteractibleObjects* InteractibleObject, AAPlayerCharacter* Player)
{
    if (InteractibleObject)
        ServerInteract(InteractibleObject, Player);
}


void UInteractionComponent::ServerInteract_Implementation(AInteractibleObjects* Interactible, AAPlayerCharacter* Player)
{
    if (!Interactible || !Interactible->bCanBeUsed)
        return;

    Interactible->Interaction(Player);
}

#pragma endregion


#pragma region Stop Interaction

void UInteractionComponent::StopInteract()
{
    if (CurrentInteractible)
    {
        ServerStopInteract(CurrentInteractible, Cast<AAPlayerCharacter>(GetOwner()));
        CurrentInteractible = nullptr;
    }
}

void UInteractionComponent::ServerStopInteract_Implementation(AInteractibleObjects* Interactible, AAPlayerCharacter* Player)
{
    if (!Interactible || !Player)
        return;

    Interactible->StopInteraction(Player);
}

#pragma endregion