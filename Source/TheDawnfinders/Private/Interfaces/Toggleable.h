// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Toggleable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UToggleable : public UInterface
{
	GENERATED_BODY()
};

class THEDAWNFINDERS_API IToggleable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Toggleable")
	void Activate();

	UFUNCTION(BlueprintNativeEvent, Category = "Toggleable")
	void Deactivate();

	UFUNCTION(BlueprintNativeEvent, Category = "Toggleable")
	bool IsActive() const;
};
