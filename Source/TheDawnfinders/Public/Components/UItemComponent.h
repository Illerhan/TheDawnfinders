// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CustomStructs.h"
#include "UItemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemStartUse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemEndUse);

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

// Network
public :
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnItemStartUse OnItemStartUse;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnItemStartUse OnItemEndUse;


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

	UFUNCTION()
	void AttackAnimEnd();


private :
	UFUNCTION()
	void EquipWeapon();

	UFUNCTION()
	void UnequipWeapon();

	UFUNCTION()
	void UseConsumable();

	UFUNCTION()
	void WeaponMainAction();


// Private Variables
private :
	UPROPERTY()
	FInventorySlot EquippedItem;

	UPROPERTY()
	float ItemUseTimer;

	UPROPERTY()
	bool IsUsingItem;

	UPROPERTY()
	bool PressedAttackInput;

	UPROPERTY()
	int ComboIndex;


// Private References
private :
	UPROPERTY()
	UHealthComponent* HealthComponent;

	UPROPERTY()
	UInventoryComponent* InventoryComponent;

	UPROPERTY()
	AAPlayerCharacter* PlayerCharacter;
};
