// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomGameState.h"

#include "Net/UnrealNetwork.h"



void ACustomGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DOREPLIFETIME_CONDITION_NOTIFY(ACustomGameState, PlayerArray, COND_None,  REPNOTIFY_Always)
}

void ACustomGameState::AddPlayerState(APlayerState* Player)
{
	Super::AddPlayerState(Player);
	OnPlayerListChanged.Broadcast();
}

void ACustomGameState::RemovePlayerState(APlayerState* Player)
{
	Super::RemovePlayerState(Player);
	OnPlayerListChanged.Broadcast();
}
void ACustomGameState::GetPlayers(TArray<APlayerState*>& Players)
{
	for (auto PlayerState : PlayerArray)
		Players.Add(PlayerState);
}