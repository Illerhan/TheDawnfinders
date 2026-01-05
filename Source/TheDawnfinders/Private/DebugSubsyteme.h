// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "TheDawnfinders/DebugWindow.h"
#include "DebugSubsyteme.generated.h"

/**
 * 
 */
#if WITH_EDITOR
UCLASS()
class THEDAWNFINDERS_API UDebugSubsyteme : public UEditorSubsystem
{
	GENERATED_BODY()
	public:
	TSharedPtr<FDebugWindow> DebugWindow;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};
#endif