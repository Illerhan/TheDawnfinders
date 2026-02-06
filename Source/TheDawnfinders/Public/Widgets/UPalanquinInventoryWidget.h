#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomStructs.h"
#include "Components/UInventoryComponent.h"
#include "UPalanquinInventoryWidget.generated.h"

class UPalanquinInventorySlotWidget;
class UItemData;

UCLASS()
class THEDAWNFINDERS_API UPalanquinInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	

public :
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetupSlots();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActualiseSlots(const TArray<FInventorySlot>& InventorySlots, int32 CurrentSlotIndex);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ShowWidget(UInventoryComponent* LinkedInventory);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void HideWidget();


// === PARAMETERS ===
protected :
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int SlotCount;


// === PROTECTED PROPERTIES ===
protected:
	UPROPERTY(BlueprintReadWrite)
	TArray<UPalanquinInventorySlotWidget*> SlotsWidgets;

	UPROPERTY(BlueprintReadWrite)
	UInventoryComponent* PalanquinInventory;

	UPROPERTY(BlueprintReadWrite)
	UInventoryComponent* OtherInventory;
};
