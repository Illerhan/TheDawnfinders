#include "Widgets/LobbyWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"


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
    if (StartGameButton)
    {
        StartGameButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnStartGameClicked);
    }

    UpdateStartGame();
    
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
    UGameplayStatics::OpenLevel(this, FName("Lvl_GymRoom"),false,"listen?");
}

void ULobbyWidget::OnStartGameClicked()
{
    if (!SessionSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("SessionSubsystem is null"));
        return;
    }

    // Vérifier qu'on est bien le serveur
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayerController is null"));
        return;
    }

    // Seul le serveur peut lancer la partie
    if (!PC->HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("Only the host can start the game"));
        return;
    }

    // ServerTravel pour emmener tous les clients avec nous

    this->RemoveFromParent();
    PC->bShowMouseCursor = false;

    
    
    FString MapPath = TEXT("L_Test");
    GetWorld()->ServerTravel(MapPath + "?listen", true);
    
    UE_LOG(LogTemp, Error, TEXT("ServerTravel called to: %s"), *MapPath);
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

void ULobbyWidget::UpdateStartGame()
{
    if (StartGameButton)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        bool bIsServer = PC && PC->HasAuthority();
        StartGameButton->SetVisibility(bIsServer ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}
