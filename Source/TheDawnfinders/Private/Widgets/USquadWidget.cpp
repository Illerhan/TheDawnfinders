// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/USquadWidget.h"

void USquadWidget::NativeConstruct()
{

}

void USquadWidget::NativeDestruct()
{

}

void USquadWidget::ActualiseSquadInfos()
{
	//TArray<APlayerState> PlayerStates = GetWorld()->GetGameState()->PlayerArray[i];

	for (int i = 0; i < SquadMemberWidgets.Num(); i++) {
		SquadMemberWidgets[i]->ActualiseWidget(10, 10, 100, 100);
	}
}

void USquadWidget::AddSquadMember_Implementation()
{

}
