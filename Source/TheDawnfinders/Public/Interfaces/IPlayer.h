#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IPlayer.generated.h"


UINTERFACE(MinimalAPI, Blueprintable)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

class IPlayerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void AddInteractibleAtRange(AActor* Interactible);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void RemoveInteractibleAtRange(AActor* Interactible);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ProgressBar")
	void ShowProgress(float CurrentValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ProgressBar")
	void HideProgress();
};
