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
	virtual void DoTrapAction(AActor* OtherActor) override;
	virtual void Multicast_PlayTrapAnim_Implementation() override;
	
	UPROPERTY(ReplicatedUsing=OnRep_SpikeAnimValue)
	float SpikeAnimValue = 0.f;
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ApplySpikePosition(float Value);
	
	UFUNCTION()
	virtual void OnRep_SpikeAnimValue();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DoSpikeAnim();
	
	UFUNCTION(BlueprintCallable)
	void SetSpikeAnimValue(float Value);
};
