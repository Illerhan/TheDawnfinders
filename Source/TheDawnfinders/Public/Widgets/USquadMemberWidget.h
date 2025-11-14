// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "USquadMemberWidget.generated.h"

/**
 * 
 */
UCLASS()
class THEDAWNFINDERS_API USquadMemberWidget : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActualiseWidget(float currentHealth, float currentMaxHealth, float currentStamina, float maxStamina, float maxHealth);
};
