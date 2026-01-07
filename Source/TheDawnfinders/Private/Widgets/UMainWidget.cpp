#include "Widgets/UMainWidget.h"



void UMainWidget::OpenPalanquinInventory_Implementation()
{
	bIsInPalanquin = true;
}

void UMainWidget::ClosePalanquinInventory_Implementation()
{
	bIsInPalanquin = false;
}

bool UMainWidget::GetIsInPalanquin()
{
	return bIsInPalanquin;
}
