#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/UQTEWidget.h"
#include "UQTEMashButtonWidget.generated.h"


UCLASS()
class THEDAWNFINDERS_API UQTEMashButtonWidget : public UQTEWidget
{
	GENERATED_BODY()

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
public : 
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void StartQTE(float Quantity, float DecreasePerSecond, bool CanBeCanceled);

	virtual void ExitQTE_Implementation() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PressButtonFeedback();

	virtual bool PressButton() override;        // Returns true if QTE is finished


protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AddedProgressPerClick = 1.f;

	UPROPERTY(BlueprintReadWrite)
	float QuantityToReach = 20.f;

	UPROPERTY(BlueprintReadWrite)
	float ProgressLostPerSecond = 2.f;

	UPROPERTY(BlueprintReadWrite)
	bool bCanBeCanceled = true;

	UPROPERTY(BlueprintReadWrite)
	float CurrentProgress = 0.f;

	UPROPERTY(BlueprintReadWrite)
	bool bDisplayed = false;
};
