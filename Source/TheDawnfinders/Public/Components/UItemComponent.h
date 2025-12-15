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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
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


// === WEAPONS ===
public :
	UFUNCTION(BlueprintCallable)
	void DoLightAttack();

	UFUNCTION(BlueprintCallable)
	void DoHeavyAttack();

	UFUNCTION()
	void AttackAnimEnd();

	UFUNCTION(BlueprintCallable)
	float GetCurrentAttackDamages();

	UFUNCTION(BlueprintCallable)
	void DoAttackCollision();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_ApplyDamagesToEnemy(ABaseEnemy* Enemy, UItemData* Data, float BaseDamages);


// === OTHERS ===
private :
	UFUNCTION()
	void ActualiseUseProgress(float DeltaTime);

	UFUNCTION()
	void UseConsumable();

	UFUNCTION()
	void StartPreviewThrow();

	UFUNCTION(Server, Reliable)
	void Server_ThrowItem(float Progress, UItemData* Data);

	UFUNCTION()
	void ActualisePreviewThrow(float DeltaTime);

	UFUNCTION()
	void StopPreviewThrow();

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


// === PRIVATE PROPERTIES ===
private :
	UPROPERTY()
	FInventorySlot EquippedItem;

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
	UDataTable* WeaponDataTable;

	UPROPERTY()
	UDataTable* WeaponActionsDataTable;

	UPROPERTY()
	UDataTable* WeaponTypeActionsDataTable;


// === PRIVATE REFERENCES ===
private :
	UPROPERTY()
	UHealthComponent* HealthComponent = nullptr;

	UPROPERTY()
	UInventoryComponent* InventoryComponent = nullptr;

	UPROPERTY()
	UStaminaComponent* StaminaComponent = nullptr;

	UPROPERTY()
	AAPlayerCharacter* PlayerCharacter = nullptr;

	UPROPERTY()
	AAPlayerCharacter* Ally = nullptr;
};
