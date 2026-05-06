#pragma once

#include "CoreMinimal.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Components/UHealthComponent.h"
#include "CustomStructs.h"
#include "GameFramework/PlayerState.h"
#include "DataAssets/AmuletData.h"
#include "CustomPlayerState.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnInfoChange, const float, hp, const float, maxHp, const float, stam, const float, maxStam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInfoChange);
DECLARE_DYNAMIC_DELEGATE(FOnInfoChangeLocal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCurseRatioChange);



UCLASS()
class THEDAWNFINDERS_API ACustomPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public :
	virtual void BeginPlay() override;


// === REPLICATION ===
public :
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_StaminaChange();

	UFUNCTION()
	void OnRep_HealthChange();

	UFUNCTION()
	void OnRep_LanternChange();

//  === SHOP ==
	UPROPERTY(ReplicatedUsing = OnRep_ShopItems,Blueprintable,BlueprintReadWrite)
	TArray<FItemInfos> ShopItems;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShopItemsChange);
	
	UPROPERTY(BlueprintAssignable)
	FOnShopItemsChange OnShopItemsChange;

	UFUNCTION(BlueprintCallable)
	void AddShopItem(FItemInfos Item);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_AddShopItem(FItemInfos Item);
	
	UFUNCTION(BlueprintCallable)
	void Client_AddShopItemLocally(FItemInfos Item);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_RepairItem(int Cos);
	

	UFUNCTION()
	void OnRep_ShopItems();
	
	virtual void CopyProperties(APlayerState* PlayerState) override;
	
	UFUNCTION(BlueprintCallable)
	void SaveInventoryBeforeTravel();
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SellShopItem(UItemData* ItemToSell);


// === DELEGATES ===
public :
	UPROPERTY(BlueprintAssignable)
	FOnInfoChange OnInfoChange;

	FOnInfoChangeLocal OnInfoChangeLocal;

	UPROPERTY(BlueprintAssignable)
	FOnCurseRatioChange OnCurseInfoChange;


// === HEALTH + STAMINA CHANGES
public :
	UFUNCTION(BlueprintCallable)
	void ActualiseLocalStamina(float current, float max);

	UFUNCTION(BlueprintCallable)
	void ActualiseStamina(float current, float max);

	UFUNCTION(BlueprintCallable)
	void ActualiseLocalHealth(float current, float max, float fixedMax);

	UFUNCTION(BlueprintCallable)
	void ActualiseHealth(float current, float max, float fixedMax);

	UFUNCTION(BlueprintCallable)
	void SetMaxHealth(float MaxHP);
	
	UFUNCTION(BlueprintCallable)
	void SetCurrentMaxHealth(float MaxHP);

	UFUNCTION(BlueprintCallable)
	void StartPoison();

	UFUNCTION(BlueprintCallable)
	void EndPoison();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EndPoison();


// === OTHERS ===
public :
	UFUNCTION(BlueprintCallable)
	void ActualiseLocalLantern(float current, float max);

	UFUNCTION(BlueprintCallable)
	void ActualiseLantern(float current, float max);

	UFUNCTION(BlueprintCallable)
	void ActualiseEquippedItem(FInventorySlot Current);

	UFUNCTION(BlueprintCallable)
	void SetIsReady(bool Ready);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SetIsReady(bool Ready);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void VerifyCountdownLaunch();

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsReady = false;


// === GETTERS ===
public :
	UFUNCTION(BlueprintCallable)
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable)
	float GetCurrentMaxHealth() const { return CurrentMaxHealth; }

	UFUNCTION(BlueprintCallable)
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintCallable)
	bool GetIsPoisoned() const { return bIsPoisoned; }

	UFUNCTION(BlueprintCallable)
	float GetCurseRatio() const { return 1 - (CurrentMaxHealth / MaxHealth); }

	UFUNCTION(BlueprintCallable)
	float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintCallable)
	float GetCurrentMaxStamina() const { return CurrentMaxStamina; }

	UFUNCTION(BlueprintCallable)
	float GetLanternPercent() const { return LanternPercent; }

	UFUNCTION(BlueprintCallable)
	FInventorySlot GetCurrentEquippedItem() const { return CurrentSlot; }


// === AMULETS ===
public :
	UFUNCTION(BlueprintCallable)
	void ApplyContextualAmulet(EAmuletTriggerType Trigger);

	UPROPERTY(ReplicatedUsing=OnRep_Gold, BlueprintReadWrite)
	int32 SavedGold = 300;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ChangeGold(int NewGold);


// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY(BlueprintReadOnly)
	TSet<UAmuletData*> PossessedAmulets;

	UPROPERTY(ReplicatedUsing = OnRep_StaminaChange)
	float CurrentStamina;

	UPROPERTY(BlueprintReadOnly, Replicated)
	float CurrentMaxStamina;

	UPROPERTY(ReplicatedUsing = OnRep_HealthChange)
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Replicated)
	float CurrentMaxHealth;
	
	UPROPERTY(BlueprintReadOnly, Replicated)
	float CurseMaxHP;

	UPROPERTY(BlueprintReadOnly, Replicated)
	float MaxHealth;

	UPROPERTY(BlueprintReadOnly)
	float LanternPercent = 100;

	UPROPERTY(BlueprintReadOnly)
	FInventorySlot CurrentSlot;
	
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing=OnRep_Protected)
	bool bIsProtected;
	
	UFUNCTION()
	void OnRep_Gold();
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnRep_Protected();
	
	UPROPERTY(ReplicatedUsing=OnRep_Dead)
	bool bIsDead;
		
	UPROPERTY(Replicated)
	bool bIsPoisoned;

	UFUNCTION(BlueprintImplementableEvent)
	void OnRep_Dead();

	UPROPERTY(BlueprintReadWrite)
	bool bInSession;
	
	UPROPERTY(BlueprintReadWrite)
	int PlayerCount;
	
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGoldChanged);

	UPROPERTY(BlueprintAssignable)
	FOnGoldChanged OnGoldChanged;
};
