// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CustomStructs.h"
#include "DataAssets/ItemData.h"
#include "UInventoryComponent.generated.h"



class UItemData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChange, const TArray<FInventorySlot>&, CurrentSlots, int32, CurrentSlotIndex);
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
	
	

// ==== DELEGATES + REP NOTIFIES ====
public :
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory")
	FOnInventoryChange OnInventoryChange;

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


// ==== PLAYER ACTIONS ====
public :
	UFUNCTION(BlueprintCallable,Category="Inventory")
	bool AddNewItem(UItemData* NewItem, int Quantity = 1);

	UFUNCTION(Server,Reliable,BlueprintCallable,Category = "Inventory")
	void ServerAddNewItem(UItemData* NewItem, int Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasRoomForItem(UItemData* NewItem);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveCurrentItem();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory")
	void ServerRemoveCurrentItem();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItemAtIndex(int Index, bool bRemoveAll);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory")
	void ServerRemoveItemAtIndex(int Index, bool bRemoveAll);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void Throw();

	UFUNCTION(Server,Reliable,BlueprintCallable,Category = "Inventory")
	void ServerThrow();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventorySlot ChangeCurrentSlot(bool IndexGoUp, int ForcedIndex = -1);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Inventory")
	void ServerChangeCurrentSlot(bool IndexGoUp, int ForcedIndex = -1);


// ==== SORTING ====
public :
	UFUNCTION()
	void SortInventory();

	UFUNCTION()
	void SortByCategories();

	UFUNCTION()
	void SortItems();


// ==== OTHERS ====
public :
	UFUNCTION(BlueprintCallable)
	void ActualiseOverloadedSlots();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OpenInventory();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CloseInventory();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventorySlot GetCurrentSlot();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SelectSlotByAngle(int angle);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void VerifyCurrentOverloadCount();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int GetCurrentOverloadCount();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ChangeOverloadSlotCount(int AmountAdded);

	UFUNCTION(Server, Reliable,BlueprintCallable, Category = "Inventory")
	void ServerConsumeItemDirectly(UItemData* Item);
	


// === PROTECTED PROPERTIES ===
public : 
	UPROPERTY(ReplicatedUsing = OnRep_InventorySlots, EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FInventorySlot> InventorySlots;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentSlotIndex, BlueprintReadWrite, Category = "Inventory")
	int CurrentSlotIndex;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory Parameters")
	int OverloadSlotBaseCount = 3;

	UPROPERTY(BlueprintReadOnly)
	int CurrentOverloadSlotCount;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory Parameters")
	int InventorySlotCount = 10;
	

// === OTHER RESSOURCES ===

	UPROPERTY(ReplicatedUsing = OnRep_Gold, EditAnywhere,BlueprintReadWrite, Category = "Inventory Ressources")
	int Gold;

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


// === PRIVATE PROPERTIES === 
private :
	UPROPERTY()
	int PreviousOverloadCount = 0;

	UPROPERTY()
	bool bIsOpened;
};
