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

    SuccessPercent = FMath::FInterpTo(SuccessPercent, AimedSuccessPercent, InDeltaTime, 5.f);
}

void ULockpickQTEWidget::PlaySuccessAnim_Implementation()
{
    
}

void ULockpickQTEWidget::PlayFailAnim_Implementation()
{
    
}


void ULockpickQTEWidget::EnterQTE_Implementation(float SuccessStart, float SuccessEnd, float Speed, int Steps)
{
    if (IsDisplayed) return;

    IsDisplayed = true;
	SuccessPercent = SuccessStart;
    AimedSuccessPercent = SuccessStart;
    SuccessPercentStart = SuccessStart;
    SuccessPercentEnd = SuccessEnd;
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
    RecentlyPressedTimer = 0.1f;

    if (!(CurrentAngle > 360 - SuccessPercent * 0.5f) && !(CurrentAngle < SuccessPercent * 0.5f)) {
        PlayFailAnim();
        return false;
    }

    CurrentStep++;
    if (CurrentStep >= StepsCount) ExitQTE();

    AimedSuccessPercent = FMath::Lerp(SuccessPercentStart, SuccessPercentEnd, ((float)CurrentStep + 1) / StepsCount);

    PlaySuccessAnim();

    return CurrentStep >= StepsCount;
}
