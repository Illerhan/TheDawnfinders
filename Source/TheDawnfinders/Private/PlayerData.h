// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerData.generated.h"

/**
 * 
 */
UCLASS()
class THEDAWNFINDERS_API UPlayerData : public UDataAsset
{
	GENERATED_BODY()

public:
	// ---------- MOUVEMENT ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float WalkSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float RunSpeed = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float FallenSpeed = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float WalkRotationRate = 360.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float RunRotationRate  = 900.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float DodgeRotationRate = 1400.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float LowStaminaSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float DodgeStartSpeed = 1400.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float DodgeEndSpeed = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float RotationRate = 720.f;


	// ---------- DODGE ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dodge")
	float DodgeDuration = 0.9f;


	// ---------- FALL DAMAGE -----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall Damage")
	float FallDamageMinYVelocity = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall Damage")
	float FallDamageMaxYVelocity = 100.f;


	// ---------- STAMINA ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stamina")
	float MaxStamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stamina")
	float BlockStaminaDrainPerSecond = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stamina")
	float ReloadSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stamina")
	float ReloadDelay = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stamina")
	float StaminaConsumptionRun =0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stamina")
	float StaminaConsumptionDodge =15.f;

	
	// ---------- HEALTH ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	float MinReviveHP = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	float InjureDecreaseSpeed = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	float MinMaxHP = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	float CurseRatio = 0.02f;


	// ---------- LANTERN ---------- 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lantern")
	float FuelConsumption = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lantern")
	float MaxFuel = 100.f;

	
	// ---------- FRICTION ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float RunFriction = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float WalkFriction = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float DecelerationWalking = 1500.f;


	// ---------- THROW PREVIEW ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Throw")
	float ThrowPreviewScaleMultiplier = 0.01f;


	// ---------- BAGARRE ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fight")
	float AutoLockStrength = 5.f;


	// ---------- SOUND ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	float RunSoundRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	float RunSoundAlertness;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	float DodgeSoundRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	float DodgeSoundAlertness;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	float AttackSoundRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	float AttackSoundAlertness;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	float ShootSoundRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	float ShootSoundAlertness;
};
