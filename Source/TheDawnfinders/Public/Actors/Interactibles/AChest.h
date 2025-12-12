#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actors/Interactibles/Interactible.h"
#include "AChest.generated.h"

class AItem;

UCLASS()
class THEDAWNFINDERS_API AChest : public AInteractibleObjects
{
	GENERATED_BODY()
	
public:	
	AChest();
	virtual void Tick(float DeltaTime) override;


// === INTERFACE FUNCTIONS ===
public :
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void StopInteract_Implementation(AActor* Interactor) override;
	
	
protected :
	UFUNCTION()
	virtual void BP_OnInteractionFinished_Implementation() override;

	UFUNCTION(Server, Reliable)
	void SpawnLoot();

	UFUNCTION()
	FVector GetPossibleSpawnLocation();


// === PARAMETERS ===
protected :
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InteractionDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName DataTableRowName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AItem> LootActor;
	
};
