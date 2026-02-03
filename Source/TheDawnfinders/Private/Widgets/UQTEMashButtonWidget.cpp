#include "Widgets/UQTEMashButtonWidget.h"



void UQTEMashButtonWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bDisplayed) return;

	CurrentProgress -= InDeltaTime * ProgressLostPerSecond;
	CurrentProgress = FMath::Clamp(CurrentProgress, 0, QuantityToReach);
}

void UQTEMashButtonWidget::StartQTE_Implementation(float Quantity, float DecreasePerSecond, bool Cancellable)
{
	bDisplayed = true;
	CurrentProgress = 0;
	
	QuantityToReach = Quantity;
	ProgressLostPerSecond = DecreasePerSecond;
	bCanBeCanceled = Cancellable;
}

void UQTEMashButtonWidget::ExitQTE_Implementation()
{
	bDisplayed = false;
}

void UQTEMashButtonWidget::PressButtonFeedback_Implementation()
{
}

bool UQTEMashButtonWidget::PressButton()
{
	DoQTEStep();

	CurrentProgress += AddedProgressPerClick;
	if (CurrentProgress >= QuantityToReach) {
		ExitQTE();
		return true;
	}

	return false;
}
