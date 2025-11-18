// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CustomStructs.h"
#include "UItemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemStartUse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemEndUse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnThrowPreviewDisplay, FVector, Position, float, Range);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnThrowHidePreview);

class UHealthComponent;
class UInventoryComponent;
class AAPlayerCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEDAWNFINDERS_API UItemComponent : public UActorComponent
{
	GENERATED_BODY()

public :	
	UItemComponent();

protected :
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	TArray<class AAPlayerCharacter*> GetNearbyPlayers(float Radius, bool bOnlyDead);

public:
	UFUNCTION(Server, Reliable)
	void ServerRequestRevive(AAPlayerCharacter* TargetAlly);
	
	UFUNCTION()
	void PerformeRevive(AAPlayerCharacter* TargetAlly);

	// --------------------
	// Events / Delegates
	// --------------------
public :
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnItemStartUse OnItemStartUse;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnItemEndUse OnItemEndUse;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnThrowPreviewDisplay OnThrowPreviewDisplay;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnThrowHidePreview OnThrowHidePreview;


public :
	UFUNCTION(BlueprintCallable)
	void DoMainAction();

	UFUNCTION(BlueprintCallable)
	void StopMainAction();

	UFUNCTION(BlueprintCallable)
	void DoSecondaryAction();

	UFUNCTION(BlueprintCallable)
	void StopSecondaryAction();

	UFUNCTION(BlueprintCallable)
	void SetEquippedItem(const TArray<FInventorySlot>& Slots, int CurrentSlotIndex);

	UFUNCTION(BlueprintCallable)
	UItemData* GetEquippedItem();

	UFUNCTION()
	void AttackAnimEnd();


private :
	UFUNCTION()
	void EquipWeapon();

	UFUNCTION()
	void UnequipWeapon();

	UFUNCTION()
	void ActualiseUseProgress(float DeltaTime);

	UFUNCTION()
	void UseConsumable();

	UFUNCTION()
	void WeaponMainAction();

	UFUNCTION()
	void StartPreviewThrow();

	UFUNCTION()
	void ActualisePreviewThrow(float DeltaTime);

	UFUNCTION()
	void StopPreviewThrow();


// Private Variables
private :
	UPROPERTY()
	FInventorySlot EquippedItem;

	UPROPERTY()
	float ItemUseTimer = 0.f;

	UPROPERTY()
	bool IsUsingItem = false;

	UPROPERTY()
	float ThrowPreviewTimer = 0.f;

	UPROPERTY()
	bool IsPreviewingThrow = false;

	UPROPERTY()
	bool PressedAttackInput = false;

	UPROPERTY()
	int ComboIndex = 0;


// Private References
private :
	UPROPERTY()
	UHealthComponent* HealthComponent = nullptr;

	UPROPERTY()
	UInventoryComponent* InventoryComponent = nullptr;

	UPROPERTY()
	AAPlayerCharacter* PlayerCharacter = nullptr;

	/** Cible de revive (peut être définie durant le processus de hold) */
	UPROPERTY()
	AAPlayerCharacter* Ally = nullptr;
};
