#pragma once

#include "CoreMinimal.h"
#include "Actors/Traps/ATrapBase.h"
#include "Interfaces/IInteractible.h"
#include "WolfTrap.generated.h"

class AAPlayerCharacter;

UENUM(BlueprintType)
enum class EWolfTrapState : uint8
{
    Idle            UMETA(DisplayName="Idle"),
    Trapping        UMETA(DisplayName="Trapping"),
    CanSelfRelease  UMETA(DisplayName="Can Self Release"),
    Released        UMETA(DisplayName="Released")
};

UCLASS()
class THEDAWNFINDERS_API AWolfTrap 
    : public ATrapBase
    , public IInteractible
{
    GENERATED_BODY()

public:
    AWolfTrap();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

protected:
    // === TRAP LOGIC ===
    virtual void DoTrapAction() override;

    void ReleasePlayer();

    // === INTERACTION INTERFACE ===
public:
    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual void StopInteract_Implementation(AActor* Interactor) override;
    virtual bool GetCanBeUsed_Implementation() override;
    virtual bool GetQTENeeded_Implementation() override;
    virtual bool ValidateQTE_Implementation() override;

protected:
    // === STATE ===
    UPROPERTY(Replicated, BlueprintReadOnly)
    EWolfTrapState TrapState = EWolfTrapState::Idle;

    UPROPERTY(Replicated, BlueprintReadOnly)
    AAPlayerCharacter* TrappedPlayer = nullptr;

    // === TIMERS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="WolfTrap")
    float SelfReleaseDelay = 5.f;

    float TimeInTrap = 0.f;

    // === INTERNAL ===
    bool IsInteractorAlly(AActor* Interactor) const;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
