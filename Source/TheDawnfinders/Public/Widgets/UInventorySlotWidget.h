#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomStructs.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "UInventorySlotWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHoverWidget, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnhoverWidget, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClickWidget, int32, SlotIndex);

UCLASS()
class THEDAWNFINDERS_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	

// === MAIN FUNCTIONS ===
public :	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void InitialiseWidget(int Index);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ActualiseVisuals(FInventorySlot Data, bool IsSelected);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ActualiseSlotColor(FInventorySlot Data);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ActualiseSlotCounter(FInventorySlot Data);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ActualiseTextOrientations(float Angle);


// === OTHERS ===
public :
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PlayAppearAnimation(float Delay);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PlayPalanquinAppearAnimation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PlayLobbyAppearAnimation(UUserWidget* StashWidget);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PlayDisappearAnimation(float Delay);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetupNavigation(UWidget* TowardWidget);


// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnHoverWidget OnHoverWidget;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnUnhoverWidget OnUnhoverWidget;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnClickWidget OnClickWidget;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* IconImage;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UCanvasPanel* CanvasPanel;

	UPROPERTY(BlueprintReadWrite)
	FInventorySlot CurrentData;

	UPROPERTY(BlueprintReadWrite)
	bool bIsSlotSelected = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsInPalanquin = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsInLobby = false;

	UPROPERTY(BlueprintReadWrite)
	int SlotIndex;
};
