#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actors/Interactibles/Interactible.h"
#include "Actors/Traps/CurseZone.h"
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

	UFUNCTION()
	void DoTrapEffect();

	UFUNCTION(Server, Reliable)
	void SpawnLoot();

	UFUNCTION()
	FVector GetPossibleSpawnLocation();


// === PARAMETERS ===
protected :
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest")
	float InteractionDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trapped Chest")
	float TrapProba;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trapped Chest")
	float TrapRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trapped Chest")
	float TrapDelay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trapped Chest")
	float TrapDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trapped Chest")
	TSubclassOf<ACurseZone> TrapCurseZone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest")
	FName DataTableRowName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest")
	TSubclassOf<AItem> LootActor;
};
