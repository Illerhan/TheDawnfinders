// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Actors/Enemy/ABaseEnemy.h"
#include "CustomStructs.h"
#include "UItemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemStartUse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemEndUse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnThrowPreviewDisplay, FVector, Position, float, Range);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnThrowHidePreview);

class UHealthComponent;
class UInventoryComponent;
class UStaminaComponent;
class AAPlayerCharacter;

UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class THEDAWNFINDERS_API UItemComponent : public UActorComponent
{
	GENERATED_BODY()

public :	
	UItemComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


// === EQUIP ===
public:
	UFUNCTION(BlueprintCallable)
	void SetEquippedItem(const TArray<FInventorySlot>& Slots, int CurrentSlotIndex);

	UFUNCTION(BlueprintCallable)
	UItemData* GetEquippedItem();

	UFUNCTION()
	void EquipWeapon();

	UFUNCTION()
	void UnequipWeapon();
	
	
// --- GESTION DES TIMERS ---
	
	TMap<EConsumableEffectType, FTimerHandle> ActiveEffectsTimers;
	
	void ApplyEffectLogic(EConsumableEffectType EffectType, bool bActivate);
	
	UFUNCTION()
	void OnEffectExpired(EConsumableEffectType EffectType);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_StartTimedEffect(EConsumableEffectType EffectType, float Duration);


// === USE ITEMS ===
public :
	UFUNCTION(BlueprintCallable)
	void DoMainAction();

	UFUNCTION(BlueprintCallable)
	void StopMainAction();

	UFUNCTION(BlueprintCallable)
	void DoSecondaryAction();

	UFUNCTION(BlueprintCallable)
	void StopSecondaryAction();


// === MELEE WEAPON ===
public :
	UFUNCTION(BlueprintCallable)
	void DoLightAttack();

	UFUNCTION(BlueprintCallable)
	void DoHeavyAttack();

	UFUNCTION(BlueprintCallable)
	void DoAttackInertia();

	UFUNCTION(BlueprintCallable)
	void ManageAttackInertia(float DeltaTime);

	UFUNCTION()
	void AttackAnimEnd();

	UFUNCTION(BlueprintCallable)
	float GetCurrentAttackDamages();

	UFUNCTION(BlueprintCallable)
	void DoAttackCollision();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_ApplyDamagesToEnemy(ABaseEnemy* Enemy, UItemData* Data, float BaseDamages);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_ApplyDamagesToDestructible(AActor* Target, UItemData* Data, float BaseDamages);


// === RANGED WEAPON ===
public :
	UFUNCTION(BlueprintCallable)
	void StartAim();

	UFUNCTION(BlueprintCallable)
	void StopAim();

	UFUNCTION(BlueprintCallable)
	void ActualiseAim(float DeltaTime);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void HideAimLines();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActualiseAimLines();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DoShootFeedbacks(FVector Direction, bool bDoShootLight);

	UFUNCTION(BlueprintCallable)
	void Shoot();

	UFUNCTION(BlueprintCallable)
	void Reload();

	UFUNCTION(BlueprintCallable)
	void CompleteReload();

	UFUNCTION(BlueprintCallable)
	void CancelReload();

	UFUNCTION(BlueprintCallable)
	void DoShootRaycast(FVector Direction);


// === THROW ===
public :
	UFUNCTION()
	void StartPreviewThrow();

	UFUNCTION(Server, Reliable)
	void Server_ThrowItem(UItemData* Data, FVector FinalPosition);

	UFUNCTION()
	void ActualisePreviewThrow(FVector AimInput);

	UFUNCTION()
	void StopPreviewThrow();


// === OTHERS ===
private :
	UFUNCTION()
	void ActualiseUseProgress(float DeltaTime);

	UFUNCTION()
	void UseConsumable();

	UFUNCTION(Server, Reliable)
	void ServerRequestRevive(AAPlayerCharacter* TargetAlly);

	UFUNCTION()
	void PerformeRevive(AAPlayerCharacter* TargetAlly);

	UFUNCTION()
	TArray<class AAPlayerCharacter*> GetNearbyPlayers(float Radius, bool bOnlyDead);


// === DELEGATES ===
public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnItemStartUse OnItemStartUse;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnItemEndUse OnItemEndUse;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnThrowPreviewDisplay OnThrowPreviewDisplay;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnThrowHidePreview OnThrowHidePreview;


// === GETTERS ===
public :
	bool GetIsPreviewingThrow() { return IsPreviewingThrow; }
	bool GetIsAiming() { return bIsAiming; }


// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY()
	FInventorySlot EquippedItem;

	UPROPERTY()
	TArray<AActor*> AlreadyHitActors;

	UPROPERTY()
	float ItemUseTimer = 0.f;

	UPROPERTY()
	bool bIsUsingItem = false;

	UPROPERTY()
	float ThrowPreviewTimer = 0.f;

	UPROPERTY()
	bool IsPreviewingThrow = false;

	UPROPERTY()
	bool PressedAttackInput = false;

	UPROPERTY()
	bool PressedHeavyAttackInput = false;

	UPROPERTY()
	int ComboIndex = 0;

	UPROPERTY()
	float CurrentAttackDamages = 0;

	UPROPERTY()
	float AttackInertiaTimer = 0;

	UPROPERTY()
	float AttackInertiaDuration = 0;

	UPROPERTY()
	float AttackInertiaForce = 0;

	UPROPERTY()
	FWeaponActionData CurrentWeaponActionData;

	UPROPERTY()
	FVector CurrentThrowDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ThrowStrength;

	UPROPERTY() 
	UDataTable* WeaponDataTable;

	UPROPERTY() 
	UDataTable* WeaponActionsDataTable;

	UPROPERTY() 
	UDataTable* WeaponTypeActionsDataTable;

	UPROPERTY(BlueprintReadWrite)
	float AimCurrentAngle;

	UPROPERTY(BlueprintReadWrite)
	bool bIsAiming;

	UPROPERTY(BlueprintReadWrite)
	bool bIsReloading;

	UPROPERTY(BlueprintReadWrite)
	float TimerReload;

	UPROPERTY(BlueprintReadWrite)
	FWeaponInfos CurrentWeaponData;


// === PRIVATE REFERENCES ===
protected :
	UPROPERTY()
	UHealthComponent* HealthComponent = nullptr;

	UPROPERTY()
	UInventoryComponent* InventoryComponent = nullptr;

	UPROPERTY()
	UStaminaComponent* StaminaComponent = nullptr;

	UPROPERTY(BlueprintReadOnly)
	AAPlayerCharacter* PlayerCharacter = nullptr;

	UPROPERTY()
	AAPlayerCharacter* Ally = nullptr;
};
