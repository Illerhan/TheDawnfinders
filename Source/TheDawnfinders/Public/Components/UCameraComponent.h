// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UCameraComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEDAWNFINDERS_API UPlayerCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerCameraComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


// === MAIN FUNCTIONS ===
public : 
	UFUNCTION(BlueprintCallable)
	void InitialiseComponent(USpringArmComponent* SpringArm);

	UFUNCTION(BlueprintCallable)
	void UpdateOffset(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void UpdateDistance(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void StartForcePosition(FVector NewPos, float Dist);

	UFUNCTION(BlueprintCallable)
	void StartAutomaticControl();


// === ACTUALISE INFORMATIONS FUNCTIONS ===
public :
	UFUNCTION(BlueprintCallable)
	void ActualiseEnemiesInfos();

	UFUNCTION(BlueprintCallable)
	void ActualiseEnviroInfos();


// === PARAMETERS ===
protected :
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CameraBaseDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CameraDistanceLerpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CameraOffsetLerpSpeed;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enviro")
	float EnviroRaycastsMaxRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enviro")
	float EnviroDistanceMaxImpact;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enviro")
	float EnviroOffsetMaxImpact;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemies")
	float EnemiesMaxRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemies")
	float EnemiesDistanceMaxImpact;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemies")
	float EnemiesOffsetMaxImpact;
		

// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY()
	FVector CurrentOffset = FVector::ZeroVector;

	UPROPERTY()
	FVector ForcedPosition;

	UPROPERTY()
	float ForcedDist;

	UPROPERTY()
	float CurrentDist;

	UPROPERTY()
	float bIsOnForcedPosition = false;

	UPROPERTY()
	bool bIsInitialised;

	UPROPERTY()
	USpringArmComponent* SpringArmComponent;

	UPROPERTY()
	TArray<AActor*> EnemiesAtRange;

	UPROPERTY()
	AActor* FarestEnemy;

	UPROPERTY()
	TArray<FVector> NearbyWallsLocations;
};
