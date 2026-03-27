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
