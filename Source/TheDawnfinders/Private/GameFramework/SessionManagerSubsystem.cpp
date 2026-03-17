// Fill out your copyright notice in the Description page of Project Settings.

#include "SessionManagerSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineSessionNames.h"

void USessionManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    IOnlineSubsystem* OnlineSubSystem = IOnlineSubsystem::Get();
    if (OnlineSubSystem)
    {
        SessionInterface = OnlineSubSystem->GetSessionInterface();
        ExternalUIInterface = OnlineSubSystem->GetExternalUIInterface();

        if (SessionInterface.IsValid())
        {
            // Bind des callbacks de session standards
            SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &USessionManagerSubsystem::OnCreateSessionComplete);
            SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &USessionManagerSubsystem::OnSessionDestroyComplete);
            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &USessionManagerSubsystem::OnFindSessionsComplete);
            SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &USessionManagerSubsystem::OnJoinSessionComplete);
            
            // IMPORTANT: Callback pour les invitations Steam
            SessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &USessionManagerSubsystem::OnSessionUserInviteAccepted);
            SessionInterface->OnFindFriendSessionCompleteDelegates[0].AddUObject(this, &USessionManagerSubsystem::OnFindFriendSessionComplete);
            
            UE_LOG(LogTemp, Log, TEXT("SessionManagerSubsystem initialized with Steam invite support"));
        }
    }
}

void USessionManagerSubsystem::Deinitialize()
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->OnCreateSessionCompleteDelegates.RemoveAll(this);
        SessionInterface->OnDestroySessionCompleteDelegates.RemoveAll(this);
        SessionInterface->OnFindSessionsCompleteDelegates.RemoveAll(this);
        SessionInterface->OnJoinSessionCompleteDelegates.RemoveAll(this);
        SessionInterface->OnSessionUserInviteAcceptedDelegates.RemoveAll(this);
        SessionInterface->OnFindFriendSessionCompleteDelegates[0].RemoveAll(this);
    }
    
    Super::Deinitialize();
}

void USessionManagerSubsystem::CreateAdvancesSession(int32 PublicConnection, int32 PrivateConnection, bool bUseLan, bool bAllowInvites, bool bIsDedicatedServer, bool bIsLobbiesIfAvailable)
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SessionInterface is Invalid"));
        OnSessionCreated.Broadcast(false);
        return;
    }
    if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
    {
        // Sauvegarde des paramètres pour recréer après destruction
        LastSessionSettings.PublicConnections = PublicConnection;
        LastSessionSettings.PrivateConnections = PrivateConnection;
        LastSessionSettings.bUseLan = bUseLan;
        LastSessionSettings.bAllowInvites = bAllowInvites;
        LastSessionSettings.bIsDedicatedServer = bIsDedicatedServer;
        LastSessionSettings.bIsLobbiesIfAvailable = bIsLobbiesIfAvailable;
        
        bCreateSessionOnDestroy = true;
        SessionInterface->DestroySession(NAME_GameSession);
        return;
    }

    TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());

    // Configuration de base
    SessionSettings->NumPublicConnections = PublicConnection;
    SessionSettings->NumPrivateConnections = PrivateConnection;
    SessionSettings->bIsLANMatch = bUseLan;
    SessionSettings->bShouldAdvertise = true;
    SessionSettings->bAllowJoinInProgress = true;
    SessionSettings->bUsesPresence = true;
    SessionSettings->bIsDedicated = bIsDedicatedServer;
    
    // IMPORTANT: Configuration pour Steam invites
    SessionSettings->bAllowInvites = bAllowInvites;
    SessionSettings->bUseLobbiesIfAvailable = bIsLobbiesIfAvailable;
    SessionSettings->bAllowJoinViaPresence = true;
    SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
    
    // Settings Steam spécifiques pour les invitations
    SessionSettings->Set(SETTING_MAPNAME, FString("Lobby"), EOnlineDataAdvertisementType::ViaOnlineService);
    SessionSettings->Set(SETTING_GAMEMODE, FString("Multiplayer"), EOnlineDataAdvertisementType::ViaOnlineService);

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("LocalPlayer is invalid"));
        OnSessionCreated.Broadcast(false);
        return;
    }

    bool bSuccess = SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings);
    
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create session!"));
        OnSessionCreated.Broadcast(false);
    }
    
}

void USessionManagerSubsystem::FindAdvancedSession(int32 MaxResults, bool bUseLan)
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SessionInterface is Invalid"));
        OnSessionsFound.Broadcast(TArray<FBlueprintSessionResult>(), false);
        return;
    }

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->MaxSearchResults = MaxResults;
    SessionSearch->bIsLanQuery = bUseLan;
    SessionSearch->QuerySettings.Set(SEARCH_KEYWORDS, FString(TEXT("PRESENCESEARCH")), EOnlineComparisonOp::Equals);

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("LocalPlayer is invalid"));
        OnSessionsFound.Broadcast(TArray<FBlueprintSessionResult>(), false);
        return;
    }

    bool bSuccess = SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("SessionSearch failed"));
        OnSessionsFound.Broadcast(TArray<FBlueprintSessionResult>(), false);
    }
}

void USessionManagerSubsystem::JoinAdvancedSession(TArray<FBlueprintSessionResult> SessionResults)
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SessionInterface is Invalid"));
        OnSessionJoined.Broadcast(false);
        return;
    }

    if (SessionResults.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("SessionResults is empty"));
        OnSessionJoined.Broadcast(false);
        return;
    }

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("LocalPlayer is invalid"));
        OnSessionJoined.Broadcast(false);
        return;
    }

    bool bSuccess = SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResults[0].OnlineResult);
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to join session!"));
        OnSessionJoined.Broadcast(false);
    }
}

void USessionManagerSubsystem::DestroySession()
{
    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SessionInterface is Invalid"));
        return;
    }
    
    SessionInterface->DestroySession(NAME_GameSession);
}

void USessionManagerSubsystem::OpenSteamInviteOverlay()
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("[InviteOverlay] No Online Subsystem found."));
        return;
    }

    IOnlineExternalUIPtr ExternalUI = Subsystem->GetExternalUIInterface();
    
    if (!ExternalUIInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("ExternalUIInterface is invalid - Steam overlay not available"));
        return;
    }

    if (!HasActiveSession())
    {
        UE_LOG(LogTemp, Error, TEXT("No active session to invite players to"));
        return;
    }

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("LocalPlayer is invalid"));
        return;
    }

    // Ouvre l'overlay Steam pour inviter des amis
    bool bSuccess = ExternalUIInterface->ShowInviteUI(LocalPlayer->GetControllerId(), NAME_GameSession);
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Steam invite overlay opened successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to open Steam invite overlay"));
    }
}

void USessionManagerSubsystem::JoinSessionViaInvite()
{
    if (!bHasPendingInvite)
    {
        UE_LOG(LogTemp, Warning, TEXT("No pending invite to join"));
        return;
    }

    if (!SessionInterface.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SessionInterface is Invalid"));
        OnSessionJoined.Broadcast(false);
        return;
    }

    const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
    if (!LocalPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("LocalPlayer is invalid"));
        OnSessionJoined.Broadcast(false);
        return;
    }

    bool bSuccess = SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, PendingInviteResult);
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to join session via invite!"));
        OnSessionJoined.Broadcast(false);
    }
    
    bHasPendingInvite = false;
}

bool USessionManagerSubsystem::HasActiveSession() const
{
    if (!SessionInterface.IsValid())
    {
        return false;
    }

    FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
    return Session != nullptr && Session->SessionState == EOnlineSessionState::InProgress;
}

int32 USessionManagerSubsystem::GetCurrentPlayerCount() const
{
    if (!SessionInterface.IsValid())
    {
        return 0;
    }

    FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
    if (Session)
    {
        return Session->RegisteredPlayers.Num();
    }
    
    return 0;
}

// ============================================================================
// CALLBACKS
// ============================================================================

void USessionManagerSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        CurrentSessionName = SessionName;
        UE_LOG(LogTemp, Log, TEXT("Session created successfully - Players can now be invited"));
        OnSessionCreated.Broadcast(true);
        SessionInterface->StartSession(NAME_GameSession);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create session!"));
        OnSessionCreated.Broadcast(false);
    }
}

void USessionManagerSubsystem::OnSessionDestroyComplete(FName SessionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Session destroyed successfully"));
        
        if (bCreateSessionOnDestroy)
        {
            bCreateSessionOnDestroy = false;
            CreateAdvancesSession(
                LastSessionSettings.PublicConnections,
                LastSessionSettings.PrivateConnections,
                LastSessionSettings.bUseLan,
                LastSessionSettings.bAllowInvites,
                LastSessionSettings.bIsDedicatedServer,
                LastSessionSettings.bIsLobbiesIfAvailable
            );
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to destroy session!"));
        bCreateSessionOnDestroy = false;
    }
}

void USessionManagerSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (!bWasSuccessful || !SessionSearch.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find sessions!"));
        OnSessionsFound.Broadcast(TArray<FBlueprintSessionResult>(), false);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Found %d sessions!"), SessionSearch->SearchResults.Num());

    TArray<FBlueprintSessionResult> Sessions;
    for (const auto& SearchResult : SessionSearch->SearchResults)
    {
        FBlueprintSessionResult BPResult;
        BPResult.OnlineResult = SearchResult;
        Sessions.Add(BPResult);
    }

    OnSessionsFound.Broadcast(Sessions, true);
}

void USessionManagerSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (Result != EOnJoinSessionCompleteResult::Success)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to join session!"));
        OnSessionJoined.Broadcast(false);
        OnInviteJoinCompleted.Broadcast(false); // notif échec
        return;
    }

    FString ConnectInfo;
    if (SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectInfo))
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            // Marquer que ce join vient d'une invitation
            // pour que WBP_MainMenu sache quoi faire au PostTravel
            if (bHasPendingInvite)
            {
                bCameFromInvite = true; // nouvelle variable bool dans le .h
            }

            OnSessionJoined.Broadcast(true);
            OnInviteJoinCompleted.Broadcast(true); // notif succès
            PC->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
        }
    }
}

void USessionManagerSubsystem::OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
    if (!bWasSuccessful)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to accept Steam invite"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Steam invite accepted! Attempting to join session..."));

    // Sauvegarde de l'invitation pour traitement
    PendingInviteResult = InviteResult;
    bHasPendingInvite = true;

    // Détruire la session actuelle si elle existe avant de rejoindre
    if (HasActiveSession())
    {
        UE_LOG(LogTemp, Log, TEXT("Destroying current session before joining invite..."));
        DestroySession();
        
        // JoinSessionViaInvite() sera appelé après la destruction
        // Ou vous pouvez l'appeler directement dans OnSessionDestroyComplete
    }
    else
    {
        // Pas de session active, rejoindre directement
        JoinSessionViaInvite();
    }
}

void USessionManagerSubsystem::OnFindFriendSessionComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResults)
{
    if (!bWasSuccessful || SearchResults.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find friend session"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Found friend session, attempting to join..."));

    PendingInviteResult = SearchResults[0];
    bHasPendingInvite = true;
    JoinSessionViaInvite();
}

