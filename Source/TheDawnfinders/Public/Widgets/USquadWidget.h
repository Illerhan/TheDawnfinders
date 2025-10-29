// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/USquadMemberWidget.h"
#include "USquadWidget.generated.h"


UCLASS()
class THEDAWNFINDERS_API USquadWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable)
	void ActualiseSquadInfos();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<USquadMemberWidget*> SquadMemberWidgets;

	UFUNCTION(BlueprintCallable)
	void BindAllCurrentPlayerStates();

	UFUNCTION(BlueprintCallable)
	void BindNewPlayerState();
};
