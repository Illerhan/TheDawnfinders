#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomStructs.h"
#include "UPalanquinInventorySlotWidget.generated.h"

class UItemData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotHovered, int, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotUnhovered, int, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotClicked, int, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotRevealed, int, SlotIndex);

UCLASS()
class THEDAWNFINDERS_API UPalanquinInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	

public :
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActualiseSlot(FInventorySlot ItemData, int Index);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ResetSlot();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RevealSlot(float Duration);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CancelSlotReveal();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RevealSlotInstant();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PlayAppearAnimation(float Delay);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PlayHideAnim(float Delay);

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnSlotClicked OnSlotClicked;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnSlotClicked FOnSlotHovered;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnSlotClicked FOnSlotUnhovered;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnSlotRevealed FOnSlotRevealed;


protected :
	UPROPERTY(BlueprintReadWrite)
	FInventorySlot CurrentData;

	UPROPERTY(BlueprintReadWrite)
	bool bIsSlotSelected;

	UPROPERTY(BlueprintReadWrite)
	bool bIsSlotRevealed;

	UPROPERTY(BlueprintReadWrite)
	bool bIsRevealing;

	UPROPERTY(BlueprintReadWrite)
	float RevealDuration;

	UPROPERTY(BlueprintReadWrite)
	float RevealTimer;

	UPROPERTY(BlueprintReadWrite)
	int SlotIndex;
};
