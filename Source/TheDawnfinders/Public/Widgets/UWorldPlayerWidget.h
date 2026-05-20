#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UWorldPlayerWidget.generated.h"

class UQTEMashButtonWidget;
class UWorldProgressBar;
class ULockpickQTEWidget;
class UWorldHealthBar;


UCLASS()
class THEDAWNFINDERS_API UWorldPlayerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public: 
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UWorldProgressBar* GetProgressBar();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	ULockpickQTEWidget* GetQTERotative();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UQTEMashButtonWidget* GetQTEMashButton();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UWorldHealthBar* GetHealthBar();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void BreakWeapon();
};
