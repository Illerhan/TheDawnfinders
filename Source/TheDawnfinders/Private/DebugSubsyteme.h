// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TheDawnfinders/DebugWindow.h"
#include "DebugSubsyteme.generated.h"

/**
 * 
 */

UCLASS()
class THEDAWNFINDERS_API UDebugSubsyteme : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	public:
	TSharedPtr<FDebugWindow, ESPMode::ThreadSafe> DebugWindow;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual void Deinitialize() override;
};
