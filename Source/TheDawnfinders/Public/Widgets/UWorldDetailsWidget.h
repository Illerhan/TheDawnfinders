#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DataAssets/ItemData.h"
#include "UWorldDetailsWidget.generated.h"


UCLASS()
class THEDAWNFINDERS_API UWorldDetailsWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
	UItemData* CurrentData;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Show(UItemData* data);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Hide();
};
