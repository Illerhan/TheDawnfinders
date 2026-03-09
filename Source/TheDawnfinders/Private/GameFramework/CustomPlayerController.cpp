// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomPlayerController.h"

void ACustomPlayerController::Server_RequestSwitchPanel_Implementation(int32 PanelIndex)
{
	// Le serveur broadcast à tout le monde
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ACustomPlayerController* PC = Cast<ACustomPlayerController>(It->Get());
		if (PC) PC->Multicast_SwitchPanel(PanelIndex);
	}
}

void ACustomPlayerController::Multicast_SwitchPanel_Implementation(int32 PanelIndex)
{
	if (!IsLocalController()) return;
	SwitchPanelBP(PanelIndex);
}