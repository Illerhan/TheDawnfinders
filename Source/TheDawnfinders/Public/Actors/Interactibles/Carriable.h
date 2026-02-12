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
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(Replicated,EditAnywhere, BlueprintReadWrite)
    	bool bUsed;


protected :
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> TargetActor;

	UPROPERTY(ReplicatedUsing = OnRep_IsCarried, BlueprintReadOnly, Category = "Carriable")
	bool bIsCarried;
	

	UFUNCTION()
	void OnRep_IsCarried();
	
	UPROPERTY()
	AActor* CarryActor;
};
