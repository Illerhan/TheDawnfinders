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
	
public :
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;


// === MAIN PROPERTIES ===
public :
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	void ActualiseWidget(const TArray<FInventorySlot>& Slots, int32 CurrentIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ShowWidget();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HideWidget();

	UFUNCTION(BlueprintCallable)
	void TryBindToInventory();


// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY()
	UInventoryComponent* InventoryComponentRef;

	UPROPERTY(BlueprintReadWrite)
	TArray<UInventorySlotWidget*> InventorySlotsWidgets;

	FTimerHandle BindDelayTimerHandle;
};
