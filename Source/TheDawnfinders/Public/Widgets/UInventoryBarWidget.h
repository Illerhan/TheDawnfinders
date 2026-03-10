#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomStructs.h"
#include "UInventorySlotWidget.h"
#include "UInventoryBarWidget.generated.h"

class UInventoryComponent;


UCLASS()
class THEDAWNFINDERS_API UInventoryBarWidget : public UUserWidget
{
	GENERATED_BODY()
	

// === INITIALISATION / DESTRUCTION ===
public :
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetupInventory(int SlotCount);


// === MAIN FUNCTIONS ===
public :

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	void ActualiseWidget(const TArray<FInventorySlot>& Slots, int32 CurrentIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ActualiseTreasuresWidgets(const TArray<FInventorySlot>& Slots);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ShowWidget();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ShowWidgetFromPalanquin();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HideWidget();

	UFUNCTION(BlueprintCallable)
	void TryBindToInventory();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetupSlotsNavigation();


// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY(BlueprintReadWrite)
	UInventoryComponent* InventoryComponentRef;

	UPROPERTY(BlueprintReadWrite)
	UInventoryComponent* PalanquinInventoryComponent;

	UPROPERTY(BlueprintReadWrite)
	TArray<UInventorySlotWidget*> InventorySlotsWidgets;

	UPROPERTY(BlueprintReadWrite)
	TArray<UInventorySlotWidget*> InventoryTreasureSlotsWidgets;

	UPROPERTY(BlueprintReadWrite)
	bool bIsInContainer;

	FTimerHandle BindDelayTimerHandle;
};
