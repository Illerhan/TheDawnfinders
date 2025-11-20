// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFramework/CustomGameMode.h"

#include "GameFramework/GameStateBase.h"

void ACustomGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	
	if (GetWorld())
	{
		Cast<AGameStateBase>(GetWorld()->GetGameState())->PlayerArray.Empty();
	}
	Super::HandleSeamlessTravelPlayer(C);
}
