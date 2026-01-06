#include "Widgets/UPalanquinInventoryWidget.h"
#include "Widgets/UPalanquinInventorySlotWidget.h"
#include "DataAssets/ItemData.h"


void UPalanquinInventoryWidget::SetupSlots_Implementation()
{

}

void UPalanquinInventoryWidget::ActualiseSlots_Implementation(const TArray<UItemData*>& InventoryItems)
{
	for (int i = 0; i < InventorySlots.Num(); i++) {

		if (i >= InventoryItems.Num()) {
			InventorySlots[i]->ActualiseSlot(nullptr);
			continue;
		}

		InventorySlots[i]->ActualiseSlot(InventoryItems[i]);
	}
}
