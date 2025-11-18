#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IPlayer.generated.h"


UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	None UMETA(DisplayName = "None"),
	Running UMETA(DisplayName = "Running"),
	Crouching UMETA(DisplayName = "Crouching"),
	UsingEquipment UMETA(DisplayName = "Using Equipment"),
	Dodging UMETA(DisplayName = "Dodging"),
	Fallen UMETA(DisplayName = "Fallen"),
	Dead UMETA(DisplayName = "Dead")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMontageEnd);

class UItemData;


UINTERFACE(MinimalAPI, Blueprintable)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

class IPlayerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Main")
	EPlayerState GetCurrentPlayerState();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Main")
	void SetCurrentPlayerState(EPlayerState NewState);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void AddInteractibleAtRange(AActor* Interactible);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void RemoveInteractibleAtRange(AActor* Interactible);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ProgressBar")
	void ShowProgress(float CurrentValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ProgressBar")
	void HideProgress();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipment")
	void SetEquippedMesh(UStaticMesh* NewMesh);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipment")
	UItemData* GetEquippedItem();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipment")
	void PlayAttackMontage(UAnimMontage* AttackMontage);
};
