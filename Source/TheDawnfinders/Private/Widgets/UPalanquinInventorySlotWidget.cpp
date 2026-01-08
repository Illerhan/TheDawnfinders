#include "Widgets/UPalanquinInventorySlotWidget.h"
#include "DataAssets/ItemData.h"


void UPalanquinInventorySlotWidget::ActualiseSlot_Implementation(FInventorySlot ItemData, int Index)
{
	CurrentData = ItemData;
	SlotIndex = Index;
}
