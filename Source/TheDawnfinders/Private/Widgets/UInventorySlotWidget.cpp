#include "Widgets/UInventorySlotWidget.h"


#pragma region Main Functions

void UInventorySlotWidget::ActualiseVisuals_Implementation(FInventorySlot Data, bool IsSelected)
{
	bIsSlotSelected = IsSelected;
	UE_LOG(LogTemp, Log, TEXT("ActualiseVisuals C++ - IsSelected: %s, bISlotSelected: %s, HasItemData: %s"),
		IsSelected ? TEXT("true") : TEXT("false"),
		bIsSlotSelected ? TEXT("true") : TEXT("false"),
		Data.ItemData ? TEXT("true") : TEXT("false"));
}

void UInventorySlotWidget::ActualiseSlotColor_Implementation(FInventorySlot Data)
{
}

void UInventorySlotWidget::InitialiseWidget_Implementation(int Index)
{
}

void UInventorySlotWidget::ActualiseSlotCounter_Implementation(FInventorySlot Data)
{
}


#pragma endregion


#pragma region Others

void UInventorySlotWidget::PlayAppearAnimation_Implementation(float Delay)
{
}

void UInventorySlotWidget::PlayDisappearAnimation_Implementation(float Delay)
{
}

#pragma endregion
