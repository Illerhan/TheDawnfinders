#include "GameFramework/CustomGameMode.h"

#include "CustomPlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/GICustom.h"

void ACustomGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);
	
	if (bNotifySeamlessTravel)
	{
		CallWidgetUpdate();
	}
}

void ACustomGameMode::StartLevelTransition(FString MapName, int32 TargetPanel)
{
	// 1. On boucle sur tous les PlayerControllers connectés
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC)
		{
			ACustomPlayerState* PS = PC->GetPlayerState<ACustomPlayerState>();
			int PlayersAmount = GetNumPlayers();
			OnPreSeamlessTravel(PC, PS, PlayersAmount );
			
			ACustomPlayerController* CPC = Cast<ACustomPlayerController>(PC);		
			if (CPC)
			{
				CPC->Client_SetRequestedPanel(TargetPanel);
			}
		}
	}
	GetWorld()->ServerTravel(MapName);
}

void ACustomGameMode::AddDeadPlayer()
{
	DeadPlayerCount++;
}

void ACustomGameMode::RemoveDeadPlayer()
{
	DeadPlayerCount--;
}
