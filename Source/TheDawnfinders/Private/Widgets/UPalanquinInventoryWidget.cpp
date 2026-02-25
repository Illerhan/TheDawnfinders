#include "Widgets/UPalanquinInventoryWidget.h"
#include "Widgets/UPalanquinInventorySlotWidget.h"
#include "DataAssets/ItemData.h"


void UPalanquinInventoryWidget::SetupSlots_Implementation()
{

}

void UPalanquinInventoryWidget::ActualiseSlots_Implementation(const TArray<FInventorySlot>& InventorySlots, int32 CurrentSlotIndex)
{
	SlotCount = InventorySlots.Num();

	for (int i = 0; i < SlotsWidgets.Num(); i++) {

		if (i >= InventorySlots.Num()) {
			SlotsWidgets[i]->SetVisibility(ESlateVisibility::Collapsed);
			SlotsWidgets[i]->ActualiseSlot(FInventorySlot(), i);
			continue;
		}

		SlotsWidgets[i]->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		SlotsWidgets[i]->ActualiseSlot(InventorySlots[i], i);
	}
}

void UPalanquinInventoryWidget::ShowWidget_Implementation(UInventoryComponent* LinkedInventory)
{

}

void UPalanquinInventoryWidget::HideWidget_Implementation()
{

}
