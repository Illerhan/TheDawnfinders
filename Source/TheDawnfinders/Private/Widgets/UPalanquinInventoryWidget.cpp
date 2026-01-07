#include "Widgets/UPalanquinInventoryWidget.h"
#include "Widgets/UPalanquinInventorySlotWidget.h"
#include "DataAssets/ItemData.h"


void UPalanquinInventoryWidget::SetupSlots_Implementation()
{

}

void UPalanquinInventoryWidget::ActualiseSlots_Implementation(const TArray<FInventorySlot>& InventorySlots, int32 CurrentSlotIndex)
{
	for (int i = 0; i < SlotsWidgets.Num(); i++) {

		if (i >= InventorySlots.Num()) {
			SlotsWidgets[i]->ActualiseSlot(FInventorySlot());
			continue;
		}

		SlotsWidgets[i]->ActualiseSlot(InventorySlots[i]);
	}
}

void UPalanquinInventoryWidget::ShowWidget_Implementation()
{

}

void UPalanquinInventoryWidget::HideWidget_Implementation()
{

}
