// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Player/ANoise.h"
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
	void Initialise(UItemData* Data, AActor* Origin);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DoStartImpulse(FVector Direction, float Strength);

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

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float NoiseRange;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bIsLoud;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	AActor* OriginActor;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<class ANoise> NoiseObject;


protected :
	UPROPERTY()
	float ProgressTimer;

	UPROPERTY(BlueprintReadOnly)
	UItemData* ItemData;
};
