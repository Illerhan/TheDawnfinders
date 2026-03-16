#include "Widgets/USquadWidget.h"

#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"

#include "GameFramework/CustomPlayerState.h"
#include "GameFramework/CustomGameState.h"


#pragma region Construct / Destruct

void USquadWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BindAllCurrentPlayerStates();
}


void USquadWidget::NativeDestruct()
{
    Super::NativeDestruct();
    bAlreadyBound = false;

    APlayerController* PC = GetOwningPlayer();
    if (!PC || !PC->IsLocalController()) return;

    APlayerState* PS = PC->PlayerState;
    if (!PS) return;

    ACustomGameState* CustomGS = Cast<ACustomGameState>(GetWorld()->GetGameState());
    if (!CustomGS) return;

    ACustomPlayerState* CustomPS = Cast<ACustomPlayerState>(PS);
    if (CustomPS)
    {
        CustomPS->OnInfoChange.RemoveDynamic(this, &USquadWidget::ActualiseSquadInfos);
    }

    //CustomGS->OnPlayerListChanged.RemoveDynamic(this, &USquadWidget::BindNewPlayerState);
}

#pragma endregion


#pragma region Main Functions

// UPDATE ALL SQUAD WIDGETS WHEN PLAYERS INFO CHANGE
void USquadWidget::ActualiseSquadInfos()
{
    APlayerController* LocalPC = GetOwningPlayer();
    if (!LocalPC) return;

    APlayerState* LocalPS = LocalPC->PlayerState;
    AGameStateBase* GS = GetWorld()->GetGameState();
    if (!GS || !LocalPS) return;

    int32 NumPlayers = GS->PlayerArray.Num();

    // IMPORTANT: Ne jamais dépasser le nombre de widgets disponibles
    int32 MaxWidgetsToShow = FMath::Min(NumPlayers, SquadMemberWidgets.Num());

    // Vérification de sécurité
    if (SquadMemberWidgets.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("SquadMemberWidgets array is empty!"));
        return;
    }

    // ---- Cacher TOUS les widgets d'abord ----
    for (int32 i = 0; i < SquadMemberWidgets.Num(); i++)
    {
        if (SquadMemberWidgets[i])
        {
            //SquadMemberWidgets[i]->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    // ---- Update widgets for existing players ----
    int WidgetIndex = 0;

    // Local player first (slot 0)
    for (APlayerState* PS : GS->PlayerArray)
    {
        if (PS == LocalPS)
        {
            if (ACustomPlayerState* CustomPS = Cast<ACustomPlayerState>(PS))
            {
                if (SquadMemberWidgets.IsValidIndex(0) && SquadMemberWidgets[0])
                {
                    SquadMemberWidgets[0]->SetVisibility(ESlateVisibility::HitTestInvisible);
                    SquadMemberWidgets[0]->ActualiseWidget(
                        CustomPS->GetCurrentHealth(),
                        CustomPS->GetCurrentMaxHealth(),
                        CustomPS->GetCurrentStamina(),
                        CustomPS->GetCurrentMaxStamina(),
                        CustomPS->GetMaxHealth(),
                        CustomPS->GetCurrentEquippedItem()
                    );
                }
            }
            WidgetIndex = 1; // Commencer à 1 pour les autres joueurs
            break;
        }
    }

    // Other players (slots 1, 2, 3...)
    for (APlayerState* PS : GS->PlayerArray)
    {
        if (PS == LocalPS) continue;

        // SÉCURITÉ: arrêter si on dépasse le nombre de widgets
        if (WidgetIndex >= SquadMemberWidgets.Num())
        {
            UE_LOG(LogTemp, Warning, TEXT("More players than available widgets! Player limit reached."));
            break;
        }

        if (ACustomPlayerState* CustomPS = Cast<ACustomPlayerState>(PS))
        {
            if (SquadMemberWidgets.IsValidIndex(WidgetIndex) && SquadMemberWidgets[WidgetIndex])
            {
                SquadMemberWidgets[WidgetIndex]->SetVisibility(ESlateVisibility::HitTestInvisible);
                SquadMemberWidgets[WidgetIndex]->Initialise(false);
                SquadMemberWidgets[WidgetIndex]->ActualiseWidget(
                    CustomPS->GetCurrentHealth(),
                    CustomPS->GetCurrentMaxHealth(),
                    CustomPS->GetCurrentStamina(),
                    CustomPS->GetCurrentMaxStamina(),
                    CustomPS->GetMaxHealth(),
                    FInventorySlot()
                );
            }
            WidgetIndex++;
        }
    }
}


// BINDING OF PLAYER STATES
void USquadWidget::BindAllCurrentPlayerStates()
{
    if (bAlreadyBound) return;

    APlayerController* PC = GetOwningPlayer();
    if (!PC || !PC->IsLocalController()) return;

    AGameStateBase* GS = GetWorld()->GetGameState();
    if (!GS || GS->PlayerArray.Num() == 0)
    {
        GetWorld()->GetTimerManager().SetTimer(
            BindDelayTimerHandle,
            this,
            &USquadWidget::BindAllCurrentPlayerStates,
            0.1f,
            false
        );
        return;
    }

    ACustomGameState* CustomGS = Cast<ACustomGameState>(GS);
    if (!CustomGS)
    {
        GetWorld()->GetTimerManager().SetTimer(
            BindDelayTimerHandle,
            this,
            &USquadWidget::BindAllCurrentPlayerStates,
            0.1f,
            false
        );
        return;
    }

    if (!Cast<ACustomPlayerState>(PC->PlayerState)) {
        GetWorld()->GetTimerManager().SetTimer(
            BindDelayTimerHandle,
            this,
            &USquadWidget::BindAllCurrentPlayerStates,
            0.1f,
            false
        );
        return;
    }

    // ---- Bind delegate for new players joining ----
    //CustomGS->OnPlayerListChanged.AddUniqueDynamic(this, &USquadWidget::BindNewPlayerState);

    // ---- Bind delegates for existing players ----
    for (APlayerState* PS : GS->PlayerArray)
    {
        if (!PS) continue;

        ACustomPlayerState* CustomPS = Cast<ACustomPlayerState>(PS);
        if (!CustomPS || !CustomPS->IsActorInitialized()) continue;

        CustomPS->OnInfoChange.AddUniqueDynamic(this, &USquadWidget::ActualiseSquadInfos);
    }

    // ---- Special delegate for local player only ----
    if (ACustomPlayerState* LocalPS = Cast<ACustomPlayerState>(PC->PlayerState))
    {
        LocalPS->OnInfoChangeLocal.BindUFunction(this, "ActualiseSquadInfos");
    }

    bAlreadyBound = true;

    // Appeler l'actualisation une fois
    ActualiseSquadInfos();
}


// WHEN A NEW PLAYER JOINS
void USquadWidget::BindNewPlayerState(APlayerState* State)
{
    if (ACustomPlayerState* CustomPS = Cast<ACustomPlayerState>(State))
    {
        CustomPS->OnInfoChange.AddUniqueDynamic(this, &USquadWidget::ActualiseSquadInfos);
    
        // Juste rafraîchir l'affichage, pas de création de widget
        ActualiseSquadInfos();
    }
}

#pragma endregion


#pragma region Blueprint Implemented

void USquadWidget::AddNewSquadMember_Implementation()
{
    // Géré en Blueprint
}

void USquadWidget::RemoveSquadMember_Implementation()
{
    // Géré en Blueprint
}

#pragma endregion
