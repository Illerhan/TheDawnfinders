#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactibles/Interactible.h"
#include "AContainer.generated.h"

class UInventoryComponent;

UCLASS()
class THEDAWNFINDERS_API AContainer : public AInteractibleObjects
{
	GENERATED_BODY()
	
public :
	AContainer();
	virtual void BeginPlay() override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual void StopInteract_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void Multicast_SetupLoot(const TArray<UItemData*>& Items);

protected :
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container")
	UInventoryComponent* InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container")
	FName DataTableRowName;
};
