#include "Widgets/UPalanquinInventorySlotWidget.h"
#include "DataAssets/ItemData.h"


void UPalanquinInventorySlotWidget::ActualiseSlot_Implementation(FInventorySlot ItemData, int Index)
{
	CurrentData = ItemData;
	SlotIndex = Index;
}

void UPalanquinInventorySlotWidget::RevealSlot_Implementation(float Duration)
{

}

void UPalanquinInventorySlotWidget::CancelSlotReveal_Implementation()
{

}

void UPalanquinInventorySlotWidget::ResetSlot_Implementation()
{
}

void UPalanquinInventorySlotWidget::RevealSlotInstant_Implementation()
{

}

void UPalanquinInventorySlotWidget::PlayAppearAnimation_Implementation(float Delay)
{

}

void UPalanquinInventorySlotWidget::PlayHideAnim_Implementation(float Delay)
{

}
