#include "GameFramework/CustomGameMode.h"
#include "GameFramework/GameStateBase.h"


void ACustomGameMode::HandleSeamlessTravelPlayer(AController*& C)
{

	Super::HandleSeamlessTravelPlayer(C);
}

void ACustomGameMode::StartLevelTransition(FString MapName)
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
		}
	}
	GetWorld()->ServerTravel(MapName);
}