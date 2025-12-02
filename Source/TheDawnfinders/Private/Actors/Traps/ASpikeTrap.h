// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Traps/ATrapBase.h"
#include "ASpikeTrap.generated.h"

UCLASS()
class THEDAWNFINDERS_API AASpikeTrap : public ATrapBase
{
	GENERATED_BODY()

public:
	AASpikeTrap();
	virtual void DoTrapAction() override;
};
