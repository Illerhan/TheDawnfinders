// Fill out your copyright notice in the Description page of Project Settings.

#include "WolfTrap.h"
#include "Actors/Enemy/ABaseEnemy.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Interfaces/IDamageable.h"
#include "Widgets/ULockpickQTEWidget.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/UWorldInteractibleWidget.h"

AWolfTrap::AWolfTrap()
{
    bCanTrap = true;
}

void AWolfTrap::BeginPlay()
{
    Super::BeginPlay();
    
    // S'assurer que le QTE est activé pour ce piège
    bDoQTE = true;
}

void AWolfTrap::DoTrapAction()
{
    if (!bCanTrap) return;
    
    Super::DoTrapAction();
    
    if (TrappedActor && TrappedActor->Implements<UPlayerInterface>())
    {
        IPlayerInterface::Execute_RequestStateChange(TrappedActor, EPlayerState::Immobilized);
    }
    
    // Réinitialiser les timers
    CurrentTrappedTime = 0.0f;
    bCanSelfRelease = false;
    bCanBeUsed = true;
    bCanTrap = false;
    
    // Afficher le widget d'interaction pour les alliés
    if (InteractibleWidget)
    {
        InteractibleWidget->DisplayText("[E] Libérer");
    }
}

void AWolfTrap::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Si quelqu'un est piégé, incrémenter le timer
    if (TrappedActor && !bCanSelfRelease)
    {
        CurrentTrappedTime += DeltaTime;
        
        // Vérifier si le temps pour se libérer seul est atteint
        if (CurrentTrappedTime >= TimeBeforeSelfRelease)
        {
            bCanSelfRelease = true;
            
            // Mettre à jour le widget pour le joueur piégé
            AAPlayerCharacter* TrappedPlayer = Cast<AAPlayerCharacter>(TrappedActor);
            if (TrappedPlayer && TrappedPlayer->IsLocallyControlled())
            {
                if (InteractibleWidget)
                {
                    InteractibleWidget->DisplayText("[E] Se libérer");
                }
            }
        }
    }
}

void AWolfTrap::Interact_Implementation(AActor* Interactor)
{
    Super::Interact_Implementation(Interactor);
    
    if (!TrappedActor) return;
    
    AAPlayerCharacter* InteractingPlayer = Cast<AAPlayerCharacter>(Interactor);
    if (!InteractingPlayer) return;
    
    // Cas 1: Un allié essaie de libérer le joueur piégé
    if (TrappedActor != Interactor)
    {
        PlayerTemp = InteractingPlayer;
        StartQTE();
    }
    // Cas 2: Le joueur piégé essaie de se libérer (seulement après le délai)
    else if (bCanSelfRelease)
    {
        PlayerTemp = InteractingPlayer;
        StartQTE();
    }
}

void AWolfTrap::OnQTESuccess()
{
    // Appeler la libération sur le serveur
    Server_ReleaseTrappedActor();
}

void AWolfTrap::OnQTEFailed()
{
    // Afficher un message d'erreur
    if (InteractibleWidget)
    {
        Server_DisplayErrorMessage("QTE échoué !");
    }
    
    // Réinitialiser le PlayerTemp
    PlayerTemp = nullptr;
}

void AWolfTrap::Server_ReleaseTrappedActor_Implementation()
{
    if (!TrappedActor) return;
    
    // Libérer le joueur piégé
    if (TrappedActor->Implements<UPlayerInterface>())
    {
        IPlayerInterface::Execute_RequestStateChange(TrappedActor, EPlayerState::None);
    }
    
    // Multicast pour tous les clients
    Multicast_ReleaseTrappedActor();
    
    // Réinitialiser l'état du piège
    TrappedActor = nullptr;
    PlayerTemp = nullptr;
    CurrentTrappedTime = 0.0f;
    bCanSelfRelease = false;
    bCanBeUsed = true;
    
    // Masquer le widget
    if (InteractibleWidget)
    {
        InteractibleWidget->HideText();
    }
}

void AWolfTrap::Multicast_ReleaseTrappedActor_Implementation()
{
    // Effets visuels/sonores de libération (si nécessaire)
    // Par exemple, jouer un son de libération
    
    if (InteractibleWidget)
    {
        InteractibleWidget->HideText();
    }
}

void AWolfTrap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AWolfTrap, TrappedActor);
    DOREPLIFETIME(AWolfTrap, bCanSelfRelease);
    DOREPLIFETIME(AWolfTrap, bCanTrap);
}