#include "Widgets/UPalanquinInventorySlotWidget.h"
#include "DataAssets/ItemData.h"


void UPalanquinInventorySlotWidget::ActualiseSlot_Implementation(FInventorySlot ItemData)
{
	CurrentData = ItemData;
}
