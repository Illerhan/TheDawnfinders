#include "Widgets/UWorldProgressBar.h"


void UWorldProgressBar::Show_Implementation(float CurrentValue)
{
	StartValue = CurrentValue;
	bIsDisplayed = true;
}

void UWorldProgressBar::ActualiseProgress_Implementation(float CurrentValue)
{

}

void UWorldProgressBar::Hide_Implementation()
{
	bIsDisplayed = false;
}