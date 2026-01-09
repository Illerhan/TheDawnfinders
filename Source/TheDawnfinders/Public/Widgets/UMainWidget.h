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
	void OpenPalanquinInventory();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ClosePalanquinInventory();

	UFUNCTION(BlueprintCallable)
	bool GetIsInPalanquin();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UPalanquinHUDWidget* GetPalanquinHUDWidget();


protected :
	UPROPERTY(BlueprintReadWrite)
	bool bIsInPalanquin;

public:
	void SetPalanquin(bool bIsOpen)
	{
		bIsInPalanquin = bIsOpen;
	}
};
