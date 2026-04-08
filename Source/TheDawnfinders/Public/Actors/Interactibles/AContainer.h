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
	virtual void DoInteractionAnim_Implementation(AActor* Interactor) override;
	virtual bool GetCanBeUsed_Implementation(AActor* Interactor) override;
	//virtual void CancelInteraction_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintCallable)
	void SetupLoot();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void Multicast_SetupLoot(const TArray<UItemData*>& Items);

	UFUNCTION()
	void CloseContainerInventory();

	UFUNCTION(Server, Reliable)
	void Server_CloseContainerInventory();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ChangeStaticMesh();

	UFUNCTION(NetMulticast, Reliable)
	void DisableInterestPointVFX();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DoPlayerAutoMove(AAPlayerCharacter* PlayerToMove);


protected :
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container")
	UInventoryComponent* InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container")
	FName DataTableRowName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Container")
	UStaticMesh* OpenedMesh;

	UPROPERTY()
	AAPlayerCharacter* Player;

	UPROPERTY()
	bool bIsOpened;

	FTimerHandle DelayStartHandle;
};
