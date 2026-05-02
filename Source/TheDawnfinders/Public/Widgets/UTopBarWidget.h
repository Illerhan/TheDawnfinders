#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UTopBarWidget.generated.h"


UCLASS()
class THEDAWNFINDERS_API UTopBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ActualiseSelectedMenu(int CurrentMenuIndex);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ActualiseGoldCount(int GoldCount);
};
