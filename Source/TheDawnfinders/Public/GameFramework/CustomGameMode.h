#pragma once

#include "CoreMinimal.h"
#include "CustomPlayerState.h"
#include "GameFramework/GameModeBase.h"
#include "CustomGameMode.generated.h"


UCLASS()
class THEDAWNFINDERS_API ACustomGameMode : public AGameModeBase
{
	GENERATED_BODY()

	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Travel")
	void OnPreSeamlessTravel(AController* PC, ACustomPlayerState* PS, int Players);
	
	UFUNCTION(BlueprintCallable, Category = "Travel")
	void StartLevelTransition(FString MapName, int32 PanelIndex);
	
	UFUNCTION(BlueprintCallable, Category = "GameOver")
	void AddDeadPlayer();
	
	UFUNCTION(BlueprintCallable, Category = "GameOver")
	void RemoveDeadPlayer();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "GameOver")
	void CheckAllDead();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameOver")
	int DeadPlayerCount;
	
};
