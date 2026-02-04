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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float SneakSpeed = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float FallenSpeed = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float CarrySpeed = 250.f;


	// ---------- DODGE ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dodge")
	float DodgeStartSpeed = 1400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dodge")
	float DodgeEndSpeed = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dodge")
	float DodgeInvincibilityDuration = 0.2f;


	// ---------- FALL DAMAGE -----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall Damage")
	float FallDamageMinYVelocity = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fall Damage")
	float FallDamageMaxYVelocity = 100.f;


	// ---------- STAMINA ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stamina")
	float MaxStamina = 100.f;

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
	
	UPROPERTY(EditAnywhere,BLueprintReadOnly, Category="Health")
	float PoisonDmg = 0.05f;


	// ---------- THROW PREVIEW ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Throw")
	float ThrowPreviewScaleMultiplier = 0.01f;


	// ---------- ROTATION ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	float AutoLockStrength = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	float NormalRotationSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	float NormalToForcedSpeed = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
	float ForceRotationSpeed = 5.f;


	// ---------- SOUND ----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	float WalkSoundRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	float WalkSoundAlertness;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	float RunSoundRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	float RunSoundAlertness;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	float SneakSoundRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	float SneakSoundAlertness;

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
