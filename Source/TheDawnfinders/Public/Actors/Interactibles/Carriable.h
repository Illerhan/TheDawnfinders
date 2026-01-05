#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactibles/Interactible.h"
#include "Carriable.generated.h"

/**
 * 
 */
UCLASS()
class THEDAWNFINDERS_API ACarriable : public AInteractibleObjects
{
	GENERATED_BODY()
	
public :
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintCallable)
	void StopCarry();


protected :
	UPROPERTY()
	bool bIsCarried;

	UPROPERTY()
	AActor* CarryActor;
};
