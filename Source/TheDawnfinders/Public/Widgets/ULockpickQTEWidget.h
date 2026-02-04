#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/UQTEWidget.h"
#include "ULockpickQTEWidget.generated.h"

class AInteractibleObjects;

UCLASS()
class THEDAWNFINDERS_API ULockpickQTEWidget : public UQTEWidget
{
	GENERATED_BODY()

public :
	ULockpickQTEWidget(const FObjectInitializer& ObjectInitializer);
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	

// === MAIN FUNCTIONS ===
public :
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EnterQTE(float SuccessStart, float SuccessEnd, float Speed, int Steps);

	virtual void ExitQTE_Implementation() override;

	virtual bool PressButton() override;


// === OTHERS ===
public : 
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PlaySuccessAnim();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PlayFailAnim();


// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY(BlueprintReadOnly)
	float SuccessPercent;

	UPROPERTY(BlueprintReadOnly)
	float AimedSuccessPercent;

	UPROPERTY(BlueprintReadOnly)
	float SuccessPercentStart;

	UPROPERTY(BlueprintReadOnly)
	float SuccessPercentEnd;

	UPROPERTY(BlueprintReadOnly)
	float RotationSpeed;

	UPROPERTY(BlueprintReadOnly)
	float CurrentAngle;

	UPROPERTY(BlueprintReadOnly)
	bool IsDisplayed;

	UPROPERTY(BlueprintReadOnly)
	int StepsCount;

	UPROPERTY(BlueprintReadOnly)
	int CurrentStep;

	UPROPERTY(BlueprintReadWrite)
	float RecentlyPressedTimer;
};
