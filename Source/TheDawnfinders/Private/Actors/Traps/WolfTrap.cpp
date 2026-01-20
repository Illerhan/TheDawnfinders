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
    // IMPORTANT : Par défaut, on ne peut pas se libérer seul
    bCanSelfRelease = false; 
}

void AWolfTrap::BeginPlay()
{
    Super::BeginPlay();
    bDoQTE = true;
    bCanSelfRelease = false; // Sécurité
}

void AWolfTrap::DoTrapAction()
{
    if (!bCanTrap) return;
    
    // 1. Réinitialisation IMMEDIATE des variables AVANT d'appeler Super ou de changer l'état
    CurrentTrappedTime = 0.0f;
    bCanSelfRelease = false; // Le serveur verrouille l'auto-release
    bCanBeUsed = true;
    bCanTrap = false;

    Super::DoTrapAction();
    
    // 2. Immobiliser le joueur
    if (TrappedActor && TrappedActor->Implements<UPlayerInterface>())
    {
        IPlayerInterface::Execute_RequestStateChange(TrappedActor, EPlayerState::Immobilized);
    }
    
    // 3. Mise à jour widget
    if (InteractibleWidget)
    {
        InteractibleWidget->DisplayText("[E] Libérer");
    }
}

void AWolfTrap::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!TrappedActor) return;

    // Calcul du temps seulement sur le serveur pour éviter la triche ou désynchro
    if (HasAuthority()) 
    {
        // Si on ne peut pas encore se libérer
        if (!bCanSelfRelease)
        {
            CurrentTrappedTime += DeltaTime;
            
            // Si le temps est écoulé, on autorise la libération
            if (CurrentTrappedTime >= TimeBeforeSelfRelease)
            {
                bCanSelfRelease = true;
                // La variable bCanSelfRelease est Replicated, donc les clients le sauront bientôt
            }
        }
    }

    // --- MISE A JOUR DU WIDGET (Client Side Visuals) ---
    if (InteractibleWidget)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && TrappedActor)
        {
            bool bIsLocallyTrapped = (TrappedActor == PC->GetPawn());

            if (bIsLocallyTrapped)
            {
                if (bCanSelfRelease)
                {
                    InteractibleWidget->DisplayText("[E] Se libérer (Difficile)");
                }
                else
                {
                    // Affiche le décompte visuel
                    float TimeLeft = FMath::Max(0.0f, TimeBeforeSelfRelease - CurrentTrappedTime);
                    FString WaitMsg = FString::Printf(TEXT("Immobilisé... %.1f"), TimeLeft);
                    InteractibleWidget->DisplayText(WaitMsg);
                }
            }
            else
            {
                InteractibleWidget->DisplayText("[E] Aider l'allié");
            }
        }
    }
}

bool AWolfTrap::GetCanBeUsed_Implementation()
{
    return TrappedActor != nullptr;
}

void AWolfTrap::Interact_Implementation(AActor* Interactor)
{
    if (!TrappedActor || !Interactor) return;

    AAPlayerCharacter* InteractingPlayer = Cast<AAPlayerCharacter>(Interactor);
    bool bAllowedToInteract = false;
    
    // CAS A : Le joueur piégé essaie de se libérer
    if (Interactor == TrappedActor)
    {
        // VERIFICATION STRICTE : Si bCanSelfRelease est false, on refuse TOUT DE SUITE.
        if (bCanSelfRelease)
        {
            bAllowedToInteract = true;
        }
        else
        {
            // Feedback optionnel : "Trop faible pour bouger"
            return; 
        }
    }
    // CAS B : Un allié aide
    else 
    {
        bAllowedToInteract = true;
    }
    
    if (bAllowedToInteract)
    {
        PlayerTemp = InteractingPlayer;
        StartQTE();
    }
}

void AWolfTrap::OnQTESuccess()
{
    Server_ReleaseTrappedActor();
}

void AWolfTrap::OnQTEFailed()
{
    if (InteractibleWidget)
    {
        Server_DisplayErrorMessage("QTE échoué !");
    }
    PlayerTemp = nullptr;
}

void AWolfTrap::Server_ReleaseTrappedActor_Implementation()
{
    if (!TrappedActor) return;

    UE_LOG(LogTemp, Log, TEXT("%s released"), *TrappedActor->GetName());
    
    if (TrappedActor->Implements<UPlayerInterface>())
    {
        // --- CORRECTION CRITIQUE ICI ---
        // Ne met pas "Running", remet l'état par défaut (None)
        IPlayerInterface::Execute_RequestStateChange(TrappedActor, EPlayerState::None);
    }

    Multicast_ReleaseTrappedActor();
    
    TrappedActor = nullptr;
    PlayerTemp = nullptr;
    CurrentTrappedTime = 0.0f;
    bCanSelfRelease = false;
    bCanBeUsed = false;
    // On permet au piège de recapturer plus tard si nécessaire ? Sinon laisser false.
    // bCanTrap = true; // Décommente si le piège est réutilisable
    
    if (InteractibleWidget)
    {
        InteractibleWidget->HideText();
    }
}

void AWolfTrap::Multicast_ReleaseTrappedActor_Implementation()
{
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