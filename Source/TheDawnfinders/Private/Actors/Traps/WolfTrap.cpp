#include "WolfTrap.h"

#include "Actors/Player/APlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AWolfTrap::AWolfTrap()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void AWolfTrap::BeginPlay()
{
    Super::BeginPlay();
}

void AWolfTrap::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!HasAuthority())
        return;

    if (TrapState == EWolfTrapState::Trapping)
    {
        TimeInTrap += DeltaTime;

        if (TimeInTrap >= SelfReleaseDelay)
        {
            TrapState = EWolfTrapState::CanSelfRelease;
        }
    }
}

// ================== TRAP ==================

void AWolfTrap::DoTrapAction()
{
    TrappedPlayer = Cast<AAPlayerCharacter>(TrappedActor);
    if (!TrappedPlayer)
        return;

    TrapState = EWolfTrapState::Trapping;
    TimeInTrap = 0.f;

    // Immobilisation (à adapter à ton Character)
    TrappedPlayer->GetCharacterMovement()->DisableMovement();
}

// ================== INTERACTION ==================

void AWolfTrap::Interact_Implementation(AActor* Interactor)
{
    if (!HasAuthority())
        return;

    if (!GetCanBeUsed_Implementation())
        return;

    // Lancer le QTE (déjà géré dans ton système)
}

void AWolfTrap::StopInteract_Implementation(AActor* Interactor)
{
}

bool AWolfTrap::GetCanBeUsed_Implementation()
{
    if (!TrappedPlayer)
        return false;

    AAPlayerCharacter* Interactor = Cast<AAPlayerCharacter>(TrappedPlayer);
    if (!Interactor)
        return false;

    // Alliés → toujours possible
    if (IsInteractorAlly(Interactor))
        return true;

    // Joueur piégé → seulement après délai
    return (Interactor == TrappedPlayer &&
            TrapState == EWolfTrapState::CanSelfRelease);
}

bool AWolfTrap::GetQTENeeded_Implementation()
{
    return true;
}

bool AWolfTrap::ValidateQTE_Implementation()
{
    if (!HasAuthority())
        return false;

    ReleasePlayer();
    return true;
}

// ================== RELEASE ==================

void AWolfTrap::ReleasePlayer()
{
    if (!TrappedPlayer)
        return;

    TrappedPlayer->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

    TrappedPlayer = nullptr;
    TrapState = EWolfTrapState::Released;

    DisableTrap();
}

// ================== UTILS ==================

bool AWolfTrap::IsInteractorAlly(AActor* Interactor) const
{
    return Interactor != TrappedPlayer;
}

// ================== REPLICATION ==================

void AWolfTrap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWolfTrap, TrapState);
    DOREPLIFETIME(AWolfTrap, TrappedPlayer);
}
