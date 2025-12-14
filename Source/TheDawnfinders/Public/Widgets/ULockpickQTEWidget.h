#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ULockpickQTEWidget.generated.h"

UCLASS()
class THEDAWNFINDERS_API ULockpickQTEWidget : public UUserWidget
{
	GENERATED_BODY()

public :
	ULockpickQTEWidget(const FObjectInitializer& ObjectInitializer);
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	

// === MAIN FUNCTIONS ===
public :
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EnterQTE(float Success, float Speed, int Steps);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ExitQTE();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool ValidateQTE();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PlaySuccessAnim();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PlayFailAnim();


// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY(BlueprintReadOnly)
	float SuccessPercent;

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
