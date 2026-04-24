#include "Widgets/UMainWidget.h"
#include "Actors/Interactibles/Litter.h"
#include "Actors/Interactibles/AContainer.h"


UPalanquinHUDWidget* UMainWidget::GetPalanquinHUDWidget_Implementation()
{
	return nullptr;
}

void UMainWidget::OpenContainerInventory_Implementation(AActor* Container)
{
	bIsInContainer = true;

	CurrentContainer = Container;
}

void UMainWidget::CloseContainerInventory_Implementation()
{
	bIsInContainer = false;

	//ALitter* Litter = Cast<ALitter>(CurrentContainer);
	//if(Litter) Litter->ClosePalanquinInventory();

	//AContainer* Container = Cast<AContainer>(CurrentContainer);
	//if (Container) Container->CloseContainerInventory();
}

void UMainWidget::EnterSpectate_Implementation()
{
}

void UMainWidget::ExitSpectate_Implementation()
{
}

bool UMainWidget::GetIsInContainer()
{
	return bIsInContainer;
}

UUSpectateWidget* UMainWidget::GetSpectateWidget_Implementation()
{
	return nullptr;
}


void UMainWidget::OpenMap_Implementation(UTexture2D* MapSprite)
{

}

void UMainWidget::CloseMap_Implementation()
{

}