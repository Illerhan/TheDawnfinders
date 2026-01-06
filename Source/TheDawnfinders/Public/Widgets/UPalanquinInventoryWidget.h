#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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
	void ActualiseSlots(const TArray<UItemData*>& InventoryItems);


// === PARAMETERS ===
protected :
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int SlotCount;


// === PROTECTED PROPERTIES ===
protected:
	UPROPERTY(BlueprintReadWrite)
	TArray<UPalanquinInventorySlotWidget*> InventorySlots;
};
