#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IPlayer.generated.h"


UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	None UMETA(DisplayName = "None"),
	Running UMETA(DisplayName = "Running"),
	Carrying UMETA(DisplayName = "Carrying"),
	UsingEquipment UMETA(DisplayName = "Using Equipment"),
	Blocking UMETA(DisplayName = "Blocking"),
	Dodging UMETA(DisplayName = "Dodging"),
	Fallen UMETA(DisplayName = "Fallen"),
	Immobilized UMETA(DisplayName = "Immobilized"),
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

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Main")
	void RequestStateChange(EPlayerState NewState);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Main")
	void DoCameraShake(float Intensity);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Main")
	void DoDamagePostProcess(float Speed);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Main")
	void PlaySoundOnServer(FName SoundTag, float Range, float WaveStrength);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Main")
	float GetSoundAlertness(FName SoundTag);


public :
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void AddInteractibleAtRange(AActor* Interactible);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void RemoveInteractibleAtRange(AActor* Interactible);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void StartCarryHeavyItem(AActor* Interactible);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void EndCarryHeavyItem(AActor* Interactible);


public :
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ProgressBar")
	void ShowProgress(float CurrentValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ProgressBar")
	void HideProgress();


public :
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipment")
	void SetEquippedMesh(UStaticMesh* NewMesh);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipment")
	UItemData* GetEquippedItem();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Equipment")
	void PlayAttackMontage(UAnimMontage* AttackMontage, float Speed);


public :
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Protection")
	void AddProtectionZone();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Protection")
	void RemoveProtectionZone();


public :
	UFUNCTION(Server, Reliable, Category = "Others")
	virtual void Server_AskOwnershipPermission(AActor* Target, AController* Origin) = 0;
};
