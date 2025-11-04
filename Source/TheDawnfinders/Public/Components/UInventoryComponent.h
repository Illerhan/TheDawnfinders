// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CustomStructs.h"
#include "DataAssets/ItemData.h"
#include "UInventoryComponent.generated.h"



class UItemData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanging, const TArray<FInventorySlot>&, CurrentSlots, int32,
                                             CurrentSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryOpenInput);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryCloseInput);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOverloadCountChange, const int32, newCount);

UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class THEDAWNFINDERS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifeTimeProps) const override;
	virtual bool IsSupportedForNetworking() const override {return true;}
	
	UPROPERTY(ReplicatedUsing=OnRep_InventorySlots,EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FInventorySlot> InventorySlots;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentSlotIndex,BlueprintReadWrite, Category="Inventory")
	int CurrentSlotIndex;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory Parameters")
	int OverloadBaseCount = 3;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory Parameters")
	int InventorySlotCount = 10;
	

// ==== Delegates & Rep Notify ====
public :
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory")
	FOnInventoryChanging OnInventoryChanging;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory")
	FOnInventoryOpenInput OnInventoryOpenInput;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory")
	FOnInventoryCloseInput OnInventoryCloseInput;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory")
	FOnOverloadCountChange OnOverloadCountChange;

	UFUNCTION()
	void OnRep_InventorySlots();

	UFUNCTION()
	void OnRep_CurrentSlotIndex();


// ==== Inventory player action functions ====
public :
	UFUNCTION(BlueprintCallable,Category="Inventory")
	void AddNewItem(UItemData* NewItem);

	UFUNCTION(Server,Reliable,BlueprintCallable,Category = "Inventory")
	void ServerAddNewItem(UItemData* NewItem);


	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveCurrentItem();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory")
	void ServerRemoveCurrentItem();


	UFUNCTION(BlueprintCallable, Category="Inventory")
	void Throw();

	UFUNCTION(Server,Reliable,BlueprintCallable,Category = "Inventory")
	void ServerThrow();


	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventorySlot ChangeCurrentSlot(bool IndexGoUp, int ForcedIndex = -1);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory")
	void ServerChangeCurrentSlot(bool IndexGoUp, int ForcedIndex = -1);


// ==== Others ====
public :
	UFUNCTION()
	void SortInventory();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventorySlot GetCurrentSlot();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SelectSlotByAngle(int angle);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void VerifyCurrentOverloadCount();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int GetCurrentOverloadCount();

protected:
	void BroadcastInventoryChange();

private :
	int PreviousOverloadCount = 0;
};
