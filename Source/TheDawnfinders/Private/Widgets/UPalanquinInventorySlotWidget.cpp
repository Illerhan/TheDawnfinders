#include "Widgets/UPalanquinInventorySlotWidget.h"
#include "DataAssets/ItemData.h"


void UPalanquinInventorySlotWidget::ActualiseSlot_Implementation(UItemData* ItemData)
{
	CurrentData = ItemData;
}
