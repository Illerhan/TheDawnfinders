#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UMainWidget.generated.h"

class UPalanquinHUDWidget;


UCLASS()
class THEDAWNFINDERS_API UMainWidget : public UUserWidget
{
	GENERATED_BODY()

public :
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OpenContainerInventory(AActor* Container);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CloseContainerInventory();

	UFUNCTION(BlueprintCallable)
	bool GetIsInContainer();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UPalanquinHUDWidget* GetPalanquinHUDWidget();


protected :
	UPROPERTY(BlueprintReadWrite)
	bool bIsInContainer;

	UPROPERTY(BlueprintReadWrite)
	AActor* CurrentContainer;
};
