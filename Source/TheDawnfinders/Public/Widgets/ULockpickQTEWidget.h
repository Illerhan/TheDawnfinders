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
	void EnterQTE(float Success, float Speed);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ExitQTE();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool ValidateQTE();


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
};
