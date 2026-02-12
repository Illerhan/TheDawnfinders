#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/SessionManagerSubsystem.h"
#include "LobbyWidget.generated.h"

/**
 * Widget de lobby avec support des invitations Steam
 */
UCLASS()
class THEDAWNFINDERS_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;


private:
	UFUNCTION()
	void OnInviteFriendsClicked();

	UFUNCTION()
	void OnCreateSessionClicked();
	
	UFUNCTION()
	void OnSessionCreated(bool bWasSuccessful);

	UFUNCTION()
	void OnSessionJoined(bool bWasSuccessful);

// === PROTECTED PROPERTIES ===
protected:
	UPROPERTY()
	USessionManagerSubsystem* SessionSubsystem; 	// Référence au subsystem de session

	UPROPERTY(meta = (BindWidget))
	class UButton* InviteFriendsButton;  	// (à binder dans le designer)

	UPROPERTY(meta = (BindWidget))
	class UButton* CreateSessionButton;
	
	FTimerHandle PlayerCountTimerHandle;
};
