// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AThrowableObject.generated.h"

class UItemData;


UENUM(BlueprintType)
enum class EThrowableEffectType : uint8 {
	Explodes UMETA(DisplayName = "Explodes"),
	PlayLoudSound UMETA(DisplayName = "PlayLoudSound")
};


UCLASS()
class THEDAWNFINDERS_API AThrowableObject : public AActor
{
	GENERATED_BODY()
	
public:	
	AThrowableObject();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void Initialise(FVector FinalPos, UItemData* Data);

	UFUNCTION(BlueprintCallable)
	void ActualisePosition();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_DoCollisionEffect();


public :
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ThrowDuration;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EThrowableEffectType EffectType;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float EffectRange;

protected :
	UPROPERTY()
	FVector StartPos;

	UPROPERTY()
	FVector EndPos;

	UPROPERTY()
	float ProgressTimer;

	UPROPERTY(BlueprintReadOnly)
	UItemData* ItemData;
};
