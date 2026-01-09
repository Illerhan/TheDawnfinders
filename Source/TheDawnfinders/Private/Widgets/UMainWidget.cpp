#include "Widgets/UMainWidget.h"



void UMainWidget::OpenPalanquinInventory_Implementation()
{
	bIsInPalanquin = true;
}

void UMainWidget::ClosePalanquinInventory_Implementation()
{
	bIsInPalanquin = false;
}


UPalanquinHUDWidget* UMainWidget::GetPalanquinHUDWidget_Implementation()
{
	return nullptr;
}

bool UMainWidget::GetIsInPalanquin()
{
	return bIsInPalanquin;
}
