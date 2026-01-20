// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Traps/ATrapBase.h"
#include "WolfTrap.generated.h"

UCLASS()
class THEDAWNFINDERS_API AWolfTrap : public ATrapBase
{
	GENERATED_BODY()

	virtual void DoTrapAction() override;
};
