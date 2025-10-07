// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionManagerSubsystem.h"

void USessionManagerSubsystem::Initialize(FSubsystemCollectionBase& collection)
{

	Super::Initialize(collection);
	IOnlineSubsystem* OnlineSubSystem = IOnlineSubsystem::Get();
	if (OnlineSubSystem)
	{
		SessionInterface = OnlineSubSystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &USessionManagerSubsystem::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &USessionManagerSubsystem::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this,&USessionManagerSubsystem::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this,&USessionManagerSubsystem::OnJoinSessionComplete);
			
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

	auto ExistingSession = SessionInterface->GetNamedSession(TEXT("Session"));
	if (ExistingSession != nullptr)
	{
		bCreateSessionOnDestroy = true;
		SessionInterface->DestroySession(NAME_GameSession);
		return;
	}

	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());

	SessionSettings->NumPublicConnections = PublicConnection;
	SessionSettings->NumPrivateConnections = PrivateConnection;
	SessionSettings->bIsLANMatch = bUseLan;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowInvites = bAllowInvites;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bUseLobbiesIfAvailable = bIsLobbiesIfAvailable;
	SessionSettings->bIsDedicated = bIsDedicatedServer;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("LocalPlayer is invalid"));
		OnSessionCreated.Broadcast(false);
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
		OnSessionCreated.Broadcast(false);
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = MaxResults;
	SessionSearch->bIsLanQuery = bUseLan;

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("LocalPlayer is invalid"));
		OnSessionCreated.Broadcast(false);
		return;
	}

	bool bSuccess = SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(),SessionSearch.ToSharedRef());
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("SessionSearch is invalid"));
		OnSessionJoined.Broadcast(false);
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

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("LocalPlayer is invalid"));
		OnSessionJoined.Broadcast(false);
		return;
	}

	bool bSuccess = SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(),NAME_GameSession,SessionResults[0].OnlineResult);
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

void USessionManagerSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("Session created successfully"));
		// Logique vers le changement de map a mettre ici ( redirection vers le lobby)
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create session!"));
	}
}

void USessionManagerSubsystem::OnSessionDestroyComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("Session destroy successfully"));
		if (bCreateSessionOnDestroy)
		{
			bCreateSessionOnDestroy = false;
			CreateAdvancesSession();
		}
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to destroy session!"));
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

	UE_LOG(LogTemp, Log, TEXT("Found sessions!"));

	TArray<FBlueprintSessionResult> Sessions;
	for (const auto& SearchResult : SessionSearch->SearchResults)
	{
		FBlueprintSessionResult BPResult;
		BPResult.OnlineResult = SearchResult;
		Sessions.Add(BPResult);
	}

	OnSessionsFound.Broadcast(Sessions,true);
}

void USessionManagerSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to join session!"));
		OnSessionJoined.Broadcast(false);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Join session complete"));

	FString ConnectInfo;
	if (SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectInfo))
	{
		UE_LOG(LogTemp, Log, TEXT("Connecting to server: %s"), *ConnectInfo);

		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
			PC->ClientTravel(ConnectInfo,ETravelType::TRAVEL_Absolute);
	}

	OnSessionJoined.Broadcast(true);
}
