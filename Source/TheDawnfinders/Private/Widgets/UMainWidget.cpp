#include "Widgets/UMainWidget.h"




UPalanquinHUDWidget* UMainWidget::GetPalanquinHUDWidget_Implementation()
{
	return nullptr;
}

void UMainWidget::OpenContainerInventory_Implementation(AActor* Container)
{
	bIsInContainer = true;
}

void UMainWidget::CloseContainerInventory_Implementation()
{
	bIsInContainer = false;
}

bool UMainWidget::GetIsInContainer()
{
	return bIsInContainer;
}
