#include "Widgets/ULockpickQTEWidget.h"

#include "Actors/Interactibles/Interactible.h"
#include "Actors/Player/APlayerCharacter.h"


#pragma region Initialisation

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

#pragma endregion


#pragma region Main Functions

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

bool ULockpickQTEWidget::PressButton()
{
    if (RecentlyPressedTimer > 0.f) return false;
    RecentlyPressedTimer = 0.1f;

    if (!(CurrentAngle > 360 - SuccessPercent * 0.5f) && !(CurrentAngle < SuccessPercent * 0.5f)) {
        PlayFailAnim();
        return false;
    }

    CurrentStep++;
    if (CurrentStep >= StepsCount)
        ExitQTE();

    AimedSuccessPercent = FMath::Lerp(SuccessPercentStart, SuccessPercentEnd, ((float)CurrentStep + 1) / StepsCount);

    PlaySuccessAnim();

    return CurrentStep >= StepsCount;
}

#pragma endregion


#pragma region Others 

void ULockpickQTEWidget::PlaySuccessAnim_Implementation()
{

}

void ULockpickQTEWidget::PlayFailAnim_Implementation()
{

}

#pragma endregion