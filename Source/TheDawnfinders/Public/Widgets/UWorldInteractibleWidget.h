#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UWorldInteractibleWidget.generated.h"


UCLASS()
class THEDAWNFINDERS_API UWorldInteractibleWidget : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DisplayText(const FString& Text);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void HideText();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DisplayErrorText(const FString& Text);
};
