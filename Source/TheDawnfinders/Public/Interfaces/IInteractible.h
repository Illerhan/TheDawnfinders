#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IInteractible.generated.h"


UINTERFACE(MinimalAPI, Blueprintable)
class UInteractible : public UInterface
{
	GENERATED_BODY()
};


class IInteractible
{
	GENERATED_BODY()

public :
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void StopInteract(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool GetCanBeUsed();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QTE")
	EQTEType GetNeededQTE();
};
