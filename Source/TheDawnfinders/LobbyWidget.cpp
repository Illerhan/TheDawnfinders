// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void ULobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Récupération du subsystem
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance)
    {
        SessionSubsystem = GameInstance->GetSubsystem<USessionManagerSubsystem>();
        
        if (SessionSubsystem)
        {
            // Bind des delegates
            SessionSubsystem->OnSessionCreated.AddDynamic(this, &ULobbyWidget::OnSessionCreated);
            SessionSubsystem->OnSessionJoined.AddDynamic(this, &ULobbyWidget::OnSessionJoined);
        }
    }

    // Bind des boutons
    if (InviteFriendsButton)
    {
        InviteFriendsButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnInviteFriendsClicked);
    }

    if (CreateSessionButton)
    {
        CreateSessionButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnCreateSessionClicked);
    }

    // Timer pour mettre à jour le compte de joueurs toutes les secondes
    GetWorld()->GetTimerManager().SetTimer(
        PlayerCountTimerHandle,
        this,
        &ULobbyWidget::UpdatePlayerCount,
        1.0f,
        true
    );
}

void ULobbyWidget::NativeDestruct()
{
    if (SessionSubsystem)
    {
        SessionSubsystem->OnSessionCreated.RemoveDynamic(this, &ULobbyWidget::OnSessionCreated);
        SessionSubsystem->OnSessionJoined.RemoveDynamic(this, &ULobbyWidget::OnSessionJoined);
    }

    GetWorld()->GetTimerManager().ClearTimer(PlayerCountTimerHandle);

    Super::NativeDestruct();
}

void ULobbyWidget::OnInviteFriendsClicked()
{
    if (!SessionSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("SessionSubsystem is null"));
        return;
    }

    if (!SessionSubsystem->HasActiveSession())
    {
        UE_LOG(LogTemp, Warning, TEXT("No active session - Create a session first!"));
        // Vous pouvez afficher un message à l'utilisateur ici
        return;
    }

    // Ouvre l'overlay Steam pour inviter des amis
    SessionSubsystem->OpenSteamInviteOverlay();
}

void ULobbyWidget::OnCreateSessionClicked()
{
    if (!SessionSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("SessionSubsystem is null"));
        return;
    }

    // Crée une session avec invitations activées
    SessionSubsystem->CreateAdvancesSession(
        4,      // 4 joueurs publics
        4,      // 0 joueurs privés
        false,  // Pas LAN
        true,   // Invitations activées
        false,  // Pas de serveur dédié
        true    // Utiliser les lobbies Steam
    );
}

void ULobbyWidget::OnSessionCreated(bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Warning, TEXT(">>> OnCreateSessionComplete called for session at %f"),GetWorld()->GetTimeSeconds());
        
        // Activer le bouton d'invitation
        if (InviteFriendsButton)
        {
            InviteFriendsButton->SetIsEnabled(true);
        }

        // Vous pouvez changer de map vers le lobby ici
        // UGameplayStatics::OpenLevel(this, FName("LobbyMap"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create session"));
        // Afficher un message d'erreur à l'utilisateur
    }
}

void ULobbyWidget::OnSessionJoined(bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully joined session via invite!"));
        // Le ClientTravel est déjà géré dans le subsystem
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to join session"));
    }
}

void ULobbyWidget::UpdatePlayerCount()
{
    if (!SessionSubsystem || !PlayerCountText)
    {
        return;
    }

    if (SessionSubsystem->HasActiveSession())
    {
        int32 PlayerCount = SessionSubsystem->GetCurrentPlayerCount();
        FString CountText = FString::Printf(TEXT("Players: %d/4"), PlayerCount);
        PlayerCountText->SetText(FText::FromString(CountText));
    }
    else
    {
        PlayerCountText->SetText(FText::FromString(TEXT("No Active Session")));
    }
}
