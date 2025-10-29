// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/USquadWidget.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/CustomPlayerState.h"
#include "GameFramework/CustomGameState.h"
#include "GameFramework/CustomHUD.h"
#include "GameFramework/PlayerState.h"


void USquadWidget::NativeConstruct()
{
	BindAllCurrentPlayerStates();
}

void USquadWidget::NativeDestruct()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC || !PC->IsLocalController()) return;

	APlayerState* PS = PC->PlayerState;
	if (!PS) return;

	ACustomGameState* CustomGS = Cast<ACustomGameState>(GetWorld()->GetGameState());
	if (!CustomGS) return;

	ACustomPlayerState* CustomPS = Cast<ACustomPlayerState>(PS);
	CustomPS->OnInfoChange.RemoveDynamic(this, &USquadWidget::ActualiseSquadInfos);
	CustomGS->OnPlayerListChanged.RemoveDynamic(this, &USquadWidget::BindNewPlayerState);
}



void USquadWidget::ActualiseSquadInfos()
{
	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return;

	if (SquadMemberWidgets.Num() == 0) return;

	if (GetOwningPlayer()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Je suis le SERVEUR (HasAuthority == true)"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Je suis le CLIENT (HasAuthority == false)"));
	}

	for (int i = 0; i < GS->PlayerArray.Num(); i++) {
		APlayerState* PS = GS->PlayerArray[i];
		ACustomPlayerState* CustomPS = Cast<ACustomPlayerState>(PS);

		SquadMemberWidgets[i]->ActualiseWidget(CustomPS->CurrentHealth, CustomPS->CurrentMaxHealth, CustomPS->CurrentStamina, CustomPS->CurrentMaxStamina);
	}
}

void USquadWidget::BindAllCurrentPlayerStates()
{
	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) 
	{ 
		GetWorld()->GetTimerManager().SetTimer(BindDelayTimerHandle, this, &USquadWidget::BindAllCurrentPlayerStates, 0.1f, false);
		return; 
	}

	ACustomGameState* CustomGS = Cast<ACustomGameState>(GS);
	if (!CustomGS)
	{
		GetWorld()->GetTimerManager().SetTimer(BindDelayTimerHandle, this, &USquadWidget::BindAllCurrentPlayerStates, 0.1f, false);
		return;
	}

	if (CustomGS->PlayerArray.Num() == 0)
	{
		GetWorld()->GetTimerManager().SetTimer(BindDelayTimerHandle, this, &USquadWidget::BindAllCurrentPlayerStates, 0.1f, false);
		return;
	}

	CustomGS->OnPlayerListChanged.AddUniqueDynamic(this, &USquadWidget::BindNewPlayerState);

	for (int i = 0; i < GS->PlayerArray.Num(); i++) {
		APlayerState* PS = GS->PlayerArray[i];
		if (!PS) return;

		ACustomPlayerState* CustomPS = Cast<ACustomPlayerState>(PS);
		CustomPS->OnInfoChange.AddUniqueDynamic(this, &USquadWidget::ActualiseSquadInfos);
	}
}

void USquadWidget::BindNewPlayerState()
{
	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return;

	APlayerState* PS = GS->PlayerArray[GS->PlayerArray.Num() - 1];
	if (!PS) return;

	ACustomPlayerState* CustomPS = Cast<ACustomPlayerState>(PS);
	CustomPS->OnInfoChange.AddUniqueDynamic(this, &USquadWidget::ActualiseSquadInfos);
}
