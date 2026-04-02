#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UWorldHealthBar.generated.h"

UCLASS()
class THEDAWNFINDERS_API UWorldHealthBar : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Setup(int Parts);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void TakeDamage(float NewPercent, bool bIsPoisoned);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void Heal(float NewPercent);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActualiseCurse(float NewPercent);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DisplayFallenIcon();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActualiseFallenIcon(float NewPercent);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void HideFallenIcon();


protected :
	UPROPERTY(BlueprintReadWrite)
	int PartCount;

	UPROPERTY(BlueprintReadWrite)
	bool bHealthBarDisplayed;

	UPROPERTY(BlueprintReadWrite)
	float CurrentProgress;
};
