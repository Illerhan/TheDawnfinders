#include "Widgets/ULockpickQTEWidget.h"


ULockpickQTEWidget::ULockpickQTEWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    
}

void ULockpickQTEWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!IsDisplayed) return;

    CurrentAngle += RotationSpeed * InDeltaTime;
}


void ULockpickQTEWidget::EnterQTE_Implementation(float Success, float Speed)
{
    IsDisplayed = true;
	SuccessPercent = Success;
    RotationSpeed = Speed;
    CurrentAngle = 0;
}

void ULockpickQTEWidget::ExitQTE_Implementation()
{
    IsDisplayed = false;
}

bool ULockpickQTEWidget::ValidateQTE_Implementation()
{
    IsDisplayed = false;

    return CurrentAngle > SuccessPercent * -0.5f && CurrentAngle < SuccessPercent * 0.5f;
}
