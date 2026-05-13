// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UCameraComponent.generated.h"

class AAPlayerCharacter;


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
	void StartForcePosition(FVector NewPos, float Dist, float LerpDistSpeedOverride, float LerpOffsetSpeedOverride);

	UFUNCTION(BlueprintCallable)
	void StartAutomaticControl();


// === ACTUALISE INFORMATIONS FUNCTIONS ===
public :
	UFUNCTION(BlueprintCallable)
	void ActualiseEnemiesInfos();

	UFUNCTION(BlueprintCallable)
	void ActualiseEnviroInfos();

	UFUNCTION(BlueprintCallable)
	void ActualisePlayerInfos(float DeltaTime);


// === PARAMETERS ===
protected :
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CameraBaseDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CameraDistanceLerpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CameraOffsetLerpSpeed;

	UPROPERTY(BlueprintReadOnly)
	float ForcedDistanceLerpSpeed;

	UPROPERTY(BlueprintReadOnly)
	float ForcedOffsetLerpSpeed;


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


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Actions")
	float PlayerRunDistance = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Actions")
	float PlayerWalkDistance = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Actions")
	float PlayerIdleDistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Actions")
	float PlayerCrouchDistance = -150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Actions")
	float PlayerFallenDistance = -250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Actions")
	float PlayerDeadDistance = -300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Actions")
	float PlayerForceRotationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Actions")
	float PlayerForceRotationDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Actions")
	float PlayerOffsetSpeed = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Actions")
	float PlayerDistanceSpeed = 5.f;
		

// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY()
	AAPlayerCharacter* Player;

	UPROPERTY()
	FVector CurrentTotalOffset = FVector::ZeroVector;

	UPROPERTY()
	FVector CurrentEnviroOffset = FVector::ZeroVector;

	UPROPERTY()
	FVector CurrentPlayerOffset = FVector::ZeroVector;

	UPROPERTY()
	FVector ForcedPosition;

	UPROPERTY()
	float ForcedDist;

	UPROPERTY()
	float CurrentTotalDist;

	UPROPERTY()
	float CurrentEnviroDist;

	UPROPERTY()
	float CurrentPlayerDist = 0;

	UPROPERTY()
	bool bIsOnForcedPosition = false;

	UPROPERTY()
	bool bIsInitialised;

	UPROPERTY()
	float ForcedPositionOffsetProgress;

	UPROPERTY()
	float ForcedPositionDistanceProgress;

	UPROPERTY()
	float StartForcedDist;

	UPROPERTY()
	FVector StartForcedOffset;

	UPROPERTY()
	USpringArmComponent* SpringArmComponent;

	UPROPERTY()
	TArray<AActor*> EnemiesAtRange;

	UPROPERTY()
	AActor* FarestEnemy;

	UPROPERTY()
	TArray<FVector> NearbyWallsLocations;
};
