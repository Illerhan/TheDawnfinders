// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/USquadWidget.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/CustomPlayerState.h"
#include "GameFramework/CustomHUD.h"
#include "GameFramework/PlayerState.h"


void USquadWidget::NativeConstruct()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	APlayerState* PS = PC->PlayerState;
	if (!PS) return;

	ACustomPlayerState* CustomPS = Cast<ACustomPlayerState>(PS);
	CustomPS->OnInfoChange.AddUniqueDynamic(this, &USquadWidget::ActualiseSquadInfos);
}

void USquadWidget::NativeDestruct()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	APlayerState* PS = PC->PlayerState;
	if (!PS) return;

	ACustomPlayerState* CustomPS = Cast<ACustomPlayerState>(PS);
	CustomPS->OnInfoChange.RemoveDynamic(this, &USquadWidget::ActualiseSquadInfos);
}


void USquadWidget::ActualiseSquadInfos()
{
	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS)
		return;

	if (SquadMemberWidgets.Num() == 0) return;

	for (int i = 0; i < GS->PlayerArray.Num(); i++) {
		APlayerState* PS = GS->PlayerArray[i];
		ACustomPlayerState* CustomPS = Cast<ACustomPlayerState>(PS);

		UE_LOG(LogTemp, Log, TEXT("%f"), CustomPS->CurrentStamina);

		SquadMemberWidgets[i]->ActualiseWidget(CustomPS->CurrentHealth, CustomPS->CurrentMaxHealth, CustomPS->CurrentStamina, CustomPS->CurrentMaxStamina);
	}
}
