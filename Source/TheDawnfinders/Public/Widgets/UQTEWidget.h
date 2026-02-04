#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UQTEWidget.generated.h"

class AInteractibleObjects;

UCLASS()
class THEDAWNFINDERS_API UQTEWidget : public UUserWidget
{
	GENERATED_BODY()
	
public :
	UFUNCTION(BlueprintCallable)
	virtual bool PressButton();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ExitQTE();

	UFUNCTION(BlueprintCallable)
	void SetLinkedInteractible(AInteractibleObjects* Interactible);

	UFUNCTION(BlueprintCallable)
	void FailQTEStep();

	UFUNCTION(BlueprintCallable)
	void DoQTEStep();

protected :
	UPROPERTY()
	AInteractibleObjects* LinkedInteractible;
};
