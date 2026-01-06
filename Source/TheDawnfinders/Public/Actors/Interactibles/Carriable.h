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

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PutInTargetActor(AActor* Actor);

	UFUNCTION(BlueprintCallable)
	TSubclassOf<AActor> GetTargetActorType();


protected :
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> TargetActor;

	UPROPERTY()
	bool bIsCarried;

	UPROPERTY()
	AActor* CarryActor;
};
