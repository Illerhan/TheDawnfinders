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
	void SelectInteractible(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void UnselectInteractible(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void StopInteract(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool GetCanBeUsed(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void DoInteractionAnim(AActor* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QTE")
	EQTEType GetNeededQTE();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QTE")
	bool GetQTEDone();
};
