#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionManagerSubsystem.h"
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

public:
	// Référence au subsystem de session
	UPROPERTY()
	USessionManagerSubsystem* SessionSubsystem;

	// Bouton pour inviter des amis Steam (à binder dans le designer)
	UPROPERTY(meta = (BindWidget))
	class UButton* InviteFriendsButton;

	// Bouton pour créer une session
	UPROPERTY(meta = (BindWidget))
	class UButton* CreateSessionButton;

	// Texte affichant le nombre de joueurs
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerCountText;

	// Timer pour mettre à jour le compte de joueurs
	FTimerHandle PlayerCountTimerHandle;

private:
	UFUNCTION()
	void OnInviteFriendsClicked();

	UFUNCTION()
	void OnCreateSessionClicked();

	UFUNCTION()
	void OnSessionCreated(bool bWasSuccessful);

	UFUNCTION()
	void OnSessionJoined(bool bWasSuccessful);

	void UpdatePlayerCount();
};
