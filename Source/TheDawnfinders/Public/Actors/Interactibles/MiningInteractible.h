#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactibles/Interactible.h"
#include "Widgets/UWorldHealthBar.h"
#include "Interfaces/IDamageable.h"
#include "MiningInteractible.generated.h"


UCLASS()
class THEDAWNFINDERS_API AMiningInteractible : public AInteractibleObjects, public IDamageable
{
	GENERATED_BODY()

public :
	AMiningInteractible();
	virtual void BeginPlay() override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION()
	FVector GetPossibleSpawnLocation();

	virtual void ReceiveDamage_Implementation(float Quantity, AActor* Origin) override;

	UFUNCTION(NetMulticast, Reliable)
	void DamageFeedback(float HealthProgress);


// === PARAMETERS === 
protected :
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MinItemToSpawnCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxItemToSpawnCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ItemToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UWidgetComponent* HealthBarWidgetComponent;


// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY(BlueprintReadOnly)
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UWorldHealthBar* HealthBarWidget;
};
