#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UQTEWidget.generated.h"

UCLASS()
class THEDAWNFINDERS_API UQTEWidget : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UFUNCTION(BlueprintCallable)
	virtual bool PressButton();
};
