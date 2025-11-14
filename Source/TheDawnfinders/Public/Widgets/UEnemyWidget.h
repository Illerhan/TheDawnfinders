// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UEnemyWidget.generated.h"


UCLASS()
class THEDAWNFINDERS_API UEnemyWidget : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PlaySuspiciousAnim();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PlayAggressiveAnim();
};
