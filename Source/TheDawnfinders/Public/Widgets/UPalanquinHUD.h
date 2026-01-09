#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UPalanquinHUD.generated.h"


UCLASS()
class THEDAWNFINDERS_API UPalanquinHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	

public :
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ActualiseWidget(float VivianiteFactor);
};
