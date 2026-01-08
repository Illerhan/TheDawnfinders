#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UMainWidget.generated.h"


UCLASS()
class THEDAWNFINDERS_API UMainWidget : public UUserWidget
{
	GENERATED_BODY()

public :
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OpenPalanquinInventory();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ClosePalanquinInventory();

	UFUNCTION(BlueprintCallable)
	bool GetIsInPalanquin();


protected :
	UPROPERTY(BlueprintReadWrite)
	bool bIsInPalanquin;
};
