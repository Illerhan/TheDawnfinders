// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CustomStructs.h"
#include "DataAssets/ItemData.h"
#include "UInventoryComponent.generated.h"



class UItemData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChange, const TArray<FInventorySlot>&, CurrentSlots, int32, CurrentSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTreasureInventoryChange, const TArray<FInventorySlot>&, CurrentTreasureSlots);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOverloadCountChange, const int32, newCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryOpenInput);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryCloseInput);


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

// === GETTERS ===
public :
	UFUNCTION(BlueprintCallable)
	bool GetHasTreasures() { return bHasTreasures; }
	

// ==== DELEGATES + REP NOTIFIES ====
public :
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory")
	FOnInventoryChange OnInventoryChange;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory")
	FOnTreasureInventoryChange OnTreasureInventoryChange;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory")
	FOnInventoryOpenInput OnInventoryOpenInput;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory")
	FOnInventoryCloseInput OnInventoryCloseInput;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory")
	FOnOverloadCountChange OnOverloadCountChange;

	UFUNCTION()
	void OnRep_InventorySlots();

	UFUNCTION()
	void OnRep_TreasureSlots();

	UFUNCTION()
	void OnRep_CurrentSlotIndex();


// ==== PLAYER ACTIONS ====
public :
	UFUNCTION(BlueprintCallable,Category="Inventory")
	bool AddNewItem(FItemInfos NewItem, int Quantity = 1);

	UFUNCTION(Server,Reliable,BlueprintCallable,Category = "Inventory")
	void ServerAddNewItem(FItemInfos NewItem, int Quantity = 1);
	
	UFUNCTION(BlueprintCallable,Category="Inventory")
	void AddShopItems(TArray<FItemInfos> Items);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasRoomForItem(FItemInfos NewItem);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveCurrentItem();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory")
	void ServerRemoveCurrentItem();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItemAtIndex(int Index, bool bRemoveAll, bool TreasureInventory = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void LocalRemoveItemAtIndex(int Index, bool bRemoveAll, bool TreasureInventory);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory")
	void ServerRemoveItemAtIndex(int Index, bool bRemoveAll, bool TreasureInventory);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void Throw();

	UFUNCTION(Server,Reliable,BlueprintCallable,Category = "Inventory")
	void ServerThrow();


// ==== SORTING ====
public :
	UFUNCTION()
	void SortInventory();

	UFUNCTION()
	void SortByCategories();

	UFUNCTION()
	void SortItems();


// === SELECT SLOT === 
public : 
	UFUNCTION(BlueprintCallable, Category = "Select Slot")
	FInventorySlot QuickChange(bool bGoRight);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Select Slot")
	void ServerQuickChange(bool bGoRight);

	UFUNCTION(BlueprintCallable, Category = "Select Slot")
	FInventorySlot ChangeCurrentSlot(bool IndexGoUp, int ForcedIndex = -1);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Select Slot")
	void ServerChangeCurrentSlot(bool IndexGoUp, int ForcedIndex = -1);

// ==== OTHERS ====
public :
	UFUNCTION(BlueprintCallable)
	void ActualiseOverloadedSlots();

	UFUNCTION(BlueprintCallable)
	void ActualiseHasTreasures();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OpenInventory();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CloseInventory();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventorySlot GetCurrentSlot();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UItemData* GetCurrentItem();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool VerifyHasItemInInventory(UItemData* Item);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseDurability(int NewDurability, UItemData* ItemToUse);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SelectSlotByAngle(int angle);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void VerifyCurrentOverloadCount();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int GetCurrentOverloadCount();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ChangeOverloadSlotCount(int AmountAdded);

	UFUNCTION(Server, Reliable,BlueprintCallable, Category = "Inventory")
	void ServerConsumeItemDirectly(FItemInfos Item);
	
	UFUNCTION(BlueprintCallable)
    void RestoreShopItems();
	
	UFUNCTION(BlueprintCallable)
	void UpdateValuable();
	


// === PROTECTED PROPERTIES ===
public : 
	UPROPERTY(ReplicatedUsing = OnRep_InventorySlots, BlueprintReadWrite, Category = "Inventory")
	TArray<FInventorySlot> InventorySlots;

	UPROPERTY(ReplicatedUsing = OnRep_TreasureSlots, BlueprintReadWrite, Category = "Inventory")
	TArray<FInventorySlot> TreasureSlots;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentSlotIndex, BlueprintReadWrite, Category = "Inventory")
	int CurrentSlotIndex;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory Parameters")
	int OverloadSlotBaseCount = 3;

	UPROPERTY(BlueprintReadOnly)
	int CurrentOverloadSlotCount;

	UPROPERTY()
	bool bHasTreasures;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory Parameters")
	int InventorySlotCount = 10;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory Parameters")
	int TreasureSlotCount = 0;
	
	UPROPERTY(EditAnywhere, Category = "Litter Physics")
	float CurrentWeight = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Litter Physics")
	float SoloMaxWeight = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Litter Physics")
	float DuoMaxWeight = 25.0f;
	

// === OTHER RESSOURCES ===

	UPROPERTY(ReplicatedUsing = OnRep_Gold, EditAnywhere,BlueprintReadWrite, Category = "Inventory Ressources")
	int Gold;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Inventory Ressources")
	int CurrentValue;

	UPROPERTY(ReplicatedUsing = OnRep_Knowledge, Blueprintable,BlueprintReadWrite, Category = "Inventory Ressources")
	int Knowledge;

	UFUNCTION(Server,Reliable,BlueprintCallable,Category = "Inventory")
	void Server_AddGold(int32 Amount);

	UFUNCTION(Server,Reliable,BlueprintCallable,Category = "Inventory")
	void Server_AddKnowledge(int32 Amount);
	
	UFUNCTION()
	void OnRep_Gold();

	UFUNCTION()
	void OnRep_Knowledge();
	
	bool bShopItemsRestored = false;


// === PRIVATE PROPERTIES === 
private :
	UPROPERTY()
	int PreviousOverloadCount = 0;

	UPROPERTY()
	bool bIsOpened;
};
