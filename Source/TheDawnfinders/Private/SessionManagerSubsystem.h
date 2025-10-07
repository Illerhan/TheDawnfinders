// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "FindSessionsCallbackProxyAdvanced.h"
#include "SessionManagerSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, bool, bWasSuccesful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionJoined, bool, bWasSuccesful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionsFound,const TArray<FBlueprintSessionResult>&, Results,bool, bWasSuccesfull);
/**
 * 
 */
UCLASS()
class THEDAWNFINDERS_API USessionManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "SessionManager")
	void CreateAdvancesSession(
		int32 PublicConnection = 4,
		int32 PrivateConnection = 0,
		bool bUseLan = false,
		bool bAllowInvites = true,
		bool bIsDedicatedServer = false,
		bool bIsLobbiesIfAvailable = true
		);

	UFUNCTION(BlueprintCallable, Category = "SessionManager")
	void FindAdvancedSession(
		int32 MaxResults = 20,
		bool bUseLan = false
		);

	UFUNCTION(BlueprintCallable, Category = "SessionManager")
	void JoinAdvancedSession(TArray<FBlueprintSessionResult> SessionResults);

	UFUNCTION(BlueprintCallable, Category = "SessionManager")
	void DestroySession();

	UPROPERTY(BlueprintAssignable, Category = "SessionManager")
	FOnSessionCreated OnSessionCreated;

	UPROPERTY(BlueprintAssignable, Category = "SessionManager")
	FOnSessionsFound OnSessionsFound;

	UPROPERTY(BlueprintAssignable, Category = "SessionManager")
	FOnSessionJoined OnSessionJoined;

private:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnSessionDestroyComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnSessionDestroyCompleteDelegateHandle;
	FDelegateHandle OnFindSessionCompleteDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	IOnlineSessionPtr SessionInterface;

	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

	TSharedPtr<class FOnlineSessionSettings> PendingSessionSettings;
	bool bCreateSessionOnDestroy = false;
	
};
