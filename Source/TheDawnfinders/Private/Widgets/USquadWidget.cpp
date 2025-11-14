// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/USquadWidget.h"

#include "Components/UHealthComponent.h"
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


// CALLED BY DELEGATES IN PLAYER STATES, ACTUALISE ALL THE SQUAD MEMBERS INFOS
void USquadWidget::ActualiseSquadInfos()
{
	if (SquadMemberWidgets.Num() == 0) return;

	APlayerController* LocalPC = GetOwningPlayer();
	APlayerState* LocalPS = LocalPC->PlayerState;
	AAPlayerCharacter* LocalCharacter = Cast<AAPlayerCharacter>(LocalPC->GetCharacter());
	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return;

	// We actualise the main spot
	for (int i = 0; i < GS->PlayerArray.Num(); i++) {
		APlayerState* PS = GS->PlayerArray[i];
		if (PS != LocalPS) continue;

		ACustomPlayerState* CustomPS = Cast<ACustomPlayerState>(PS);
		SquadMemberWidgets[0]->ActualiseWidget(CustomPS->CurrentHealth, CustomPS->CurrentMaxHealth, CustomPS->CurrentStamina, CustomPS->CurrentMaxStamina,CustomPS->MaxHealth);

		break;
	}
	
	int WidgetIndex = 1;
	// We actualise all the other spots
	for (int i = 0; i < GS->PlayerArray.Num(); i++) {
		APlayerState* PS = GS->PlayerArray[i];
		if (PS == LocalPS) continue;

		ACustomPlayerState* CustomPS = Cast<ACustomPlayerState>(PS);
		SquadMemberWidgets[WidgetIndex++]->ActualiseWidget(CustomPS->CurrentHealth, CustomPS->CurrentMaxHealth, CustomPS->CurrentStamina, CustomPS->CurrentMaxStamina,CustomPS->MaxHealth);
	}
}


// CALLED AT THE START MULTIPLE TIMES UNTIL ALL THE NEEDED ELEMENTS ARE SETUP CORRECTLY
void USquadWidget::BindAllCurrentPlayerStates()
{
	// First we delay until all the needed elements are setup correctly
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

	// Delegate for new players
	CustomGS->OnPlayerListChanged.AddUniqueDynamic(this, &USquadWidget::BindNewPlayerState);

	// Delegate for all current players
	for (int i = 0; i < GS->PlayerArray.Num(); i++) {
		APlayerState* PS = GS->PlayerArray[i];
		if (!PS) return;

		ACustomPlayerState* CustomPS = Cast<ACustomPlayerState>(PS);
		CustomPS->OnInfoChange.AddUniqueDynamic(this, &USquadWidget::ActualiseSquadInfos);

		AddNewSquadMember();
	}

	// Local Delegate
	APlayerController* PC = GetOwningPlayer();
	if (!PC || !PC->IsLocalController()) return;
	
	ACustomPlayerState* PS = Cast<ACustomPlayerState>(PC->PlayerState);
	if (!PS) return;

	PS->OnInfoChangeLocal.BindUFunction(this, "ActualiseSquadInfos");
}


// CALLED WHEN A NEW PLAYER JOINS THE GAME TO BIND THE NEW DELEGATES
void USquadWidget::BindNewPlayerState()
{
	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return;

	APlayerState* PS = GS->PlayerArray[GS->PlayerArray.Num() - 1];
	if (!PS) return;

	ACustomPlayerState* CustomPS = Cast<ACustomPlayerState>(PS);
	CustomPS->OnInfoChange.AddUniqueDynamic(this, &USquadWidget::ActualiseSquadInfos);

	AddNewSquadMember();
}

void USquadWidget::AddNewSquadMember_Implementation()
{

}

void USquadWidget::RemoveSquadMember_Implementation()
{

}
