#include "Widgets/UInventorySlotWidget.h"


#pragma region Main Functions

void UInventorySlotWidget::ActualiseVisuals_Implementation(FInventorySlot Data, bool IsSelected)
{
	//bIsSlotSelected = IsSelected;
	CurrentData = Data;

	//UE_LOG(LogTemp, Log, TEXT("ActualiseVisuals C++ - IsSelected: %s, bISlotSelected: %s, HasItemData: %s"),
	//	IsSelected ? TEXT("true") : TEXT("false"), bIsSlotSelected ? TEXT("true") : TEXT("false"), Data.ItemData ? TEXT("true") : TEXT("false"));
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
	bIsInPalanquin = false;
}

void UInventorySlotWidget::PlayPalanquinAppearAnimation_Implementation()
{
	bIsInPalanquin = true;
}

void UInventorySlotWidget::PlayDisappearAnimation_Implementation(float Delay)
{

}

void UInventorySlotWidget::SetupNavigation_Implementation(UWidget* TowardWidget)
{

}

#pragma endregion
