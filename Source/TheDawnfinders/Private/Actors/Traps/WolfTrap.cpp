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

void AWolfTrap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWolfTrap, bCanSelfRelease);
    DOREPLIFETIME(AWolfTrap, bCanTrap);
}

bool AWolfTrap::GetQTENeeded_Implementation()
{
    return false; 
}

void AWolfTrap::OnRep_TrappedActor()
{
    // Appeler la version du parent (bonnes pratiques)
    Super::OnRep_TrappedActor();

    CurrentTrappedTime = 0.0f;

    // Si TrappedActor est devenu null (libération), on cache le widget immédiatement
    if (TrappedActor == nullptr)
    {
        if (InteractibleWidget)
        {
            InteractibleWidget->HideText();
        }
    }
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
    bCanSelfRelease = false; 
    bCanBeUsed = true;
    bCanTrap = false;

    Super::DoTrapAction();
    
    // 2. Immobiliser le joueur
    if (TrappedActor && TrappedActor->Implements<UPlayerInterface>())
    {
        IPlayerInterface::Execute_RequestStateChange(TrappedActor, EPlayerState::Trapped);
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

    // Si personne n'est piégé, on sort
    if (!TrappedActor) return;

    // --- LOGIQUE SERVEUR (Le Chef) ---
    if (HasAuthority()) 
    {
        // Le serveur compte le temps et débloque le QTE quand c'est prêt
        if (!bCanSelfRelease)
        {
            CurrentTrappedTime += DeltaTime;
            
            if (CurrentTrappedTime >= TimeBeforeSelfRelease)
            {
                bCanSelfRelease = true; 
                // Dès que cette ligne s'exécute, la valeur 'true' est envoyée aux clients
            }
        }
    }
    // --- LOGIQUE CLIENT (L'Afficheur) ---
    else 
    {
        // Le client incrémente son timer JUSTE pour l'affichage (Barre de progression ou texte)
        // IL NE DOIT JAMAIS TOUCHER A bCanSelfRelease LUI-MÊME !
        if (!bCanSelfRelease) // Tant que le serveur dit "Non", on continue d'attendre
        {
            CurrentTrappedTime += DeltaTime;
        }
    }

    // --- GESTION WIDGET ---
    if (InteractibleWidget)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && TrappedActor == PC->GetPawn()) // Si je suis le piégé
        {
            // Je regarde la variable qui vient du serveur
            if (bCanSelfRelease) 
            {
                InteractibleWidget->DisplayText("[E] Se libérer");
            }
            else
            {
                // J'affiche le temps restant basé sur mon timer local
                float TimeLeft = FMath::Max(0.0f, TimeBeforeSelfRelease - CurrentTrappedTime);
                InteractibleWidget->DisplayText(FString::Printf(TEXT("Bloqué... %.1f"), TimeLeft));
            }
        }
        else // Si je suis un allié
        {
            InteractibleWidget->DisplayText("[E] Aider");
        }
    }
}

bool AWolfTrap::GetCanBeUsed_Implementation()
{
    return bDoQTE;
}

void AWolfTrap::Interact_Implementation(AActor* Interactor)
{
    // 1. Sécurités de base
    if (!TrappedActor || !Interactor) return;

    AAPlayerCharacter* InteractingPlayer = Cast<AAPlayerCharacter>(Interactor);
    if (!InteractingPlayer) return;

    bool bIsTheVictim = false;
    
    if (Interactor == TrappedActor) 
    {
        bIsTheVictim = true;
    }
    else if (InteractingPlayer->CurrentState == EPlayerState::Trapped)
    {
        bIsTheVictim = true;
        UE_LOG(LogTemp, Warning, TEXT("FIX: Pointeur différent mais état Immobilized détecté. C'est bien la victime."));
    }

    if (bIsTheVictim)
    {
        if (bCanSelfRelease)
        {
            PlayerTemp = InteractingPlayer;
            Multicast_StartTrapQTE(InteractingPlayer);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Action bloquée : Le timer n'est pas fini (bCanSelfRelease is false)"));
            
            if (InteractibleWidget) InteractibleWidget->DisplayErrorText("Trop faible pour bouger...");
            return; 
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Allié détecté : Sauvetage autorisé"));
        
        PlayerTemp = InteractingPlayer;
        Multicast_StartTrapQTE(InteractingPlayer);
    }
}

void AWolfTrap::Multicast_StartTrapQTE_Implementation(AAPlayerCharacter* TargetPlayer)
{
    if (!TargetPlayer) return;

    if (TargetPlayer->IsLocallyControlled())
    {
        UInteractionComponent* IC = TargetPlayer->FindComponentByClass<UInteractionComponent>();
        if (!IC) return;
        
        IC->StartExternalQTE(this);
    if (IPlayerInterface::Execute_GetCurrentPlayerState(TargetPlayer)!= EPlayerState::Trapped)
            IPlayerInterface::Execute_RequestStateChange(TargetPlayer, EPlayerState::Immobilized);
        Execute_StartQTE(this);
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
        IPlayerInterface::Execute_RequestStateChange(TrappedActor, EPlayerState::None);
    }

    Multicast_ReleaseTrappedActor();
    
    TrappedActor = nullptr;
    PlayerTemp = nullptr;
    CurrentTrappedTime = 0.0f;
    bCanSelfRelease = false;
    bCanBeUsed = false;
    
    // bCanTrap = true; // Décommenter si le piège est réutilisable
    
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

