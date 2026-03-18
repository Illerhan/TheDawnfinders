// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "FindSessionsCallbackProxyAdvanced.h"
#include "SessionManagerSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, bool, bWasSuccesful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionJoined, bool, bWasSuccesful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionsFound, const TArray<FBlueprintSessionResult>&, Results, bool, bWasSuccesfull);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInviteJoinResult, bool, bSuccess);


// Structure pour sauvegarder les paramètres de session
USTRUCT()
struct FLastSessionSettingsCache
{
    GENERATED_BODY()

    int32 PublicConnections = 4;
    int32 PrivateConnections = 0;
    bool bUseLan = false;
    bool bAllowInvites = true;
    bool bIsDedicatedServer = false;
    bool bIsLobbiesIfAvailable = false;
};

/**
 * Subsystem de gestion des sessions multijoueur avec support Steam
 */
UCLASS()
class THEDAWNFINDERS_API USessionManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * Crée une session de jeu avancée
     * @param PublicConnection Nombre de connexions publiques
     * @param PrivateConnection Nombre de connexions privées
     * @param bUseLan Utiliser LAN ou en ligne
     * @param bAllowInvites Autoriser les invitations
     * @param bIsDedicatedServer Est-ce un serveur dédié
     * @param bIsLobbiesIfAvailable Utiliser les lobbies Steam si disponible
     */
    UFUNCTION(BlueprintCallable, Category = "SessionManager")
    void CreateAdvancesSession(
       int32 PublicConnection = 4,
       int32 PrivateConnection = 4,
       bool bUseLan = false,
       bool bAllowInvites = true,
       bool bIsDedicatedServer = false,
       bool bIsLobbiesIfAvailable = true
    );

    /**
     * Recherche des sessions disponibles
     * @param MaxResults Nombre maximum de résultats
     * @param bUseLan Rechercher sur LAN ou en ligne
     */
    UFUNCTION(BlueprintCallable, Category = "SessionManager")
    void FindAdvancedSession(
       int32 MaxResults = 20,
       bool bUseLan = false
    );

    /**
     * Rejoint une session à partir des résultats de recherche
     * @param SessionResults Tableau des sessions trouvées
     */
    UFUNCTION(BlueprintCallable, Category = "SessionManager")
    void JoinAdvancedSession(TArray<FBlueprintSessionResult> SessionResults);

    /**
     * Détruit la session actuelle
     */
    UFUNCTION(BlueprintCallable, Category = "SessionManager")
    void DestroySession();

    /**
     * Ouvre l'overlay Steam pour inviter des amis (Steam uniquement)
     * Permet d'inviter des amis Steam à rejoindre la session actuelle
     */
    UFUNCTION(BlueprintCallable, Category = "SessionManager|Steam")
    void OpenSteamInviteOverlay();

    /**
     * Rejoindre une session via une invitation Steam ou un lien
     * Cette fonction est automatiquement appelée quand un joueur accepte une invitation
     */
    UFUNCTION(BlueprintCallable, Category = "SessionManager|Steam")
    void JoinSessionViaInvite();

    /**
     * Vérifie si une session existe actuellement
     * @return true si une session est active
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SessionManager")
    bool HasActiveSession() const;

    /**
     * Obtient le nombre de joueurs dans la session actuelle
     * @return Nombre de joueurs connectés
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SessionManager")
    int32 GetCurrentPlayerCount() const;

    // Delegates pour Blueprint
    UPROPERTY(BlueprintAssignable, Category = "SessionManager")
    FOnSessionCreated OnSessionCreated;

    UPROPERTY(BlueprintAssignable, Category = "SessionManager")
    FOnSessionsFound OnSessionsFound;

    UPROPERTY(BlueprintAssignable, Category = "SessionManager")
    FOnSessionJoined OnSessionJoined;
    
    UPROPERTY(BlueprintAssignable, Category = "Session|Invite")
    FOnInviteJoinResult OnInviteJoinCompleted;
    bool bCameFromInvite;
    UPROPERTY()
    bool bHasPendingInviteJoinResult = false;
    UPROPERTY()
    
    bool PendingInviteJoinSuccess = false;

private:
    // Callbacks des sessions
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnSessionDestroyComplete(FName SessionName, bool bWasSuccessful);
    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    
    // Callbacks spécifiques Steam
    void OnSessionUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);
    void OnFindFriendSessionComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResults);
    UFUNCTION(BlueprintCallable)
    bool ConsumePendingInviteJoinResult(bool& bOutSuccess);

    // Handles des delegates
    FDelegateHandle OnCreateSessionCompleteDelegateHandle;
    FDelegateHandle OnSessionDestroyCompleteDelegateHandle;
    FDelegateHandle OnFindSessionCompleteDelegateHandle;
    FDelegateHandle OnJoinSessionCompleteDelegateHandle;
    FDelegateHandle OnSessionUserInviteAcceptedDelegateHandle;
    FDelegateHandle OnFindFriendSessionCompleteDelegateHandle;

    // Interfaces Online Subsystem
    IOnlineSessionPtr SessionInterface;
    IOnlineExternalUIPtr ExternalUIInterface;

    // Recherche de sessions
    TSharedPtr<class FOnlineSessionSearch> SessionSearch;

    // Sauvegarde des paramètres
    FLastSessionSettingsCache LastSessionSettings;
    bool bCreateSessionOnDestroy = false;

    // Session pending pour les invitations
    FOnlineSessionSearchResult PendingInviteResult;
    bool bHasPendingInvite = false;

    FName CurrentSessionName;
};
