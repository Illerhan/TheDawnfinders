// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DataAssets/EnemyData.h"
#include "Interfaces/IDamageable.h"
#include "Interfaces/IFadeable.h"
#include "ABaseEnemy.generated.h"


class UEnemyWidget;
class UWidgetComponent;


UENUM(BlueprintType)
enum class EEnemyState : uint8 {
	Idle UMETA(DisplayName = "Idle"),
	Suspicious UMETA(DisplayName = "Suspicious"),
	Aggressive UMETA(DisplayName = "Aggressive")
};


UCLASS()
class THEDAWNFINDERS_API ABaseEnemy : public ACharacter, public IDamageable, public IFadeable
{
	GENERATED_BODY()
	
public:	
	ABaseEnemy();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UWidgetComponent* EnemyWidgetComponent;

public :
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UEnemyData* EnemyData;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UEnemyWidget* EnemyWidget;

	UFUNCTION(BlueprintCallable)
	void DoAttack(FEnemyActionData AttackData);

	UFUNCTION()
	void OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnMontageNotifyBegin(FName NotifyName);

	UFUNCTION()
	void OnMontageEnd(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnMontageEnd(bool bInterrupted);


// === HEALTH ===
public :
	UPROPERTY(BlueprintReadOnly)
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly)
	bool IsInvincible;

	UFUNCTION(BlueprintCallable)
	void Die();

	UFUNCTION(BlueprintNativeEvent)
	void DoHitEffect();

	UFUNCTION()
	void StartInvincibilityFrames(float Duration);

	UFUNCTION()
	void EndInvincibilityFrames();

	FTimerHandle InvincibilityTimerHandle;

	void ReceiveDamage_Implementation(float Quantity, AActor* Origin);



// === FADE === 
public :
	void FadeIn_Implementation();
	void FadeOut_Implementation();
	bool GetIsDisplayed_Implementation();

protected :
	UPROPERTY(BlueprintReadWrite)
	bool IsDisplayed;
};
