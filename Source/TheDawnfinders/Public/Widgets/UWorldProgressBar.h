#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UWorldProgressBar.generated.h"


UCLASS()
class THEDAWNFINDERS_API UWorldProgressBar : public UUserWidget
{
	GENERATED_BODY()
	

// === MAIN FUNCTIONS ===
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Show(float CurrentValue);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActualiseProgress(float CurrentValue);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Hide();


// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY(BlueprintReadOnly)
	float StartValue;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDisplayed;
};
