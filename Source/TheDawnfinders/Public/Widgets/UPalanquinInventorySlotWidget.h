#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomStructs.h"
#include "UPalanquinInventorySlotWidget.generated.h"

class UItemData;

UCLASS()
class THEDAWNFINDERS_API UPalanquinInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActualiseSlot(FInventorySlot ItemData);

protected :
	UPROPERTY(BlueprintReadWrite)
	FInventorySlot CurrentData;
};
