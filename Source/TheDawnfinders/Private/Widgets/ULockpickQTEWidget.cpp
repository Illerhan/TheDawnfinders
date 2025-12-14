#include "Widgets/ULockpickQTEWidget.h"


ULockpickQTEWidget::ULockpickQTEWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    UpdateCanTick();
}

void ULockpickQTEWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!IsDisplayed) return;

    CurrentAngle += RotationSpeed * InDeltaTime;
    CurrentAngle = FMath::Modulo(CurrentAngle, 360.f);
}

void ULockpickQTEWidget::PlaySuccessAnim_Implementation()
{
    
}

void ULockpickQTEWidget::PlayFailAnim_Implementation()
{
    
}


void ULockpickQTEWidget::EnterQTE_Implementation(float Success, float Speed, int Steps)
{
    IsDisplayed = true;
	SuccessPercent = Success;
    RotationSpeed = Speed;
    CurrentAngle = 0;
    CurrentStep = 0;
    StepsCount = Steps;
}

void ULockpickQTEWidget::ExitQTE_Implementation()
{
    IsDisplayed = false;
}

bool ULockpickQTEWidget::ValidateQTE_Implementation()
{
    if (RecentlyPressedTimer > 0.f) return false;
    RecentlyPressedTimer = 0.05f;

    if (!(CurrentAngle > 360 - SuccessPercent * 0.5f) && !(CurrentAngle < SuccessPercent * 0.5f)) {
        PlayFailAnim();
        return false;
    }

    PlaySuccessAnim();
    CurrentStep++;
    if (CurrentStep >= StepsCount) ExitQTE();

    UE_LOG(LogTemp, Display, TEXT("%d"), StepsCount);

    return CurrentStep >= StepsCount;
}
