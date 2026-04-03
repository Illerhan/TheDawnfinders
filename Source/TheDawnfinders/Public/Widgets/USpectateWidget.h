#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "USpectateWidget.generated.h"


UCLASS()
class THEDAWNFINDERS_API UUSpectateWidget : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DisplayWidget();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void HideWidget();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActualiseInfos(int PlayerIndex);
};
