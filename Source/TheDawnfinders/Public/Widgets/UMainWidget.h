#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/USpectateWidget.h"
#include "UMainWidget.generated.h"

class UPalanquinHUDWidget;
class UWorldDetailsWidget;

UCLASS()
class THEDAWNFINDERS_API UMainWidget : public UUserWidget
{
	GENERATED_BODY()

public :
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OpenContainerInventory(AActor* Container);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CloseContainerInventory();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EnterSpectate();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OpenMap(UTexture2D* MapSprite);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CloseMap();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ExitSpectate();

	UFUNCTION(BlueprintCallable)
	bool GetIsInContainer();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UPalanquinHUDWidget* GetPalanquinHUDWidget();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UUSpectateWidget* GetSpectateWidget();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UWorldDetailsWidget* GetDetailsWidget();


protected :
	UPROPERTY(BlueprintReadWrite)
	bool bIsInContainer;

	UPROPERTY(BlueprintReadWrite)
	AActor* CurrentContainer;
};
