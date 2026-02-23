#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomStructs.h"
#include "UPalanquinInventorySlotWidget.generated.h"

class UItemData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotHovered, int, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotUnhovered, int, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotClicked, int, SlotIndex);

UCLASS()
class THEDAWNFINDERS_API UPalanquinInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	

public :
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActualiseSlot(FInventorySlot ItemData, int Index);

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnSlotClicked OnSlotClicked;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnSlotClicked FOnSlotHovered;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnSlotClicked FOnSlotUnhovered;


protected :
	UPROPERTY(BlueprintReadWrite)
	FInventorySlot CurrentData;

	UPROPERTY(BlueprintReadWrite)
	int SlotIndex;
};
