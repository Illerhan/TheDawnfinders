// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DataAssets/EnemyData.h"
#include "Interfaces/IDamageable.h"
#include "Interfaces/IFadeable.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "ABaseEnemy.generated.h"


class UEnemyWidget;
class UWidgetComponent;
class ABasicEnemyAIController;


UENUM(BlueprintType)
enum class EEnemyState : uint8 {
	Idle UMETA(DisplayName = "Idle"),
	Listening UMETA(DisplayName = "Listening"),
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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UWidgetComponent* EnemyWidgetComponent;


public :
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UEnemyData* EnemyData;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	UEnemyWidget* EnemyWidget;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	USceneComponent* AttackCollisionPosRef;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void StartAttack(FEnemyActionData AttackData, AActor* Target);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EnterListeningState();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EnterIdleState();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EnterSuspicious();

	UFUNCTION(BlueprintCallable)
	void DoAttackCollision();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_HideEye();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_EnterIdle();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_EnterListening();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_EnterSuspicious();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_EnterAggressives();

	UFUNCTION(BlueprintCallable)
	void SetLockRotation(bool Locked);

	UFUNCTION(BlueprintCallable)
	void CheckEnableDistance();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ExitCrystal();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ShowEnemy();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HideEnemy();


// === MONTAGES ===
public :
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayMontage(UAnimMontage* Montage, float Speed);

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
	
	UFUNCTION(Server, Reliable)
	void Server_TakeDamages(float Quantity, AActor* Origin);


// === FADE === 
public :
	void FadeIn_Implementation();
	void FadeOut_Implementation();
	bool GetIsDisplayed_Implementation();



// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EnableDistance = 3500.f;

	FTimerHandle EnableTimer;

	UPROPERTY(BlueprintReadWrite)
	bool IsDisplayed;

	UPROPERTY(Replicated, BlueprintReadWrite)
	float CurrentSpeed;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bIsCrystallized;

	UPROPERTY(BlueprintReadWrite)
	bool bLockRotation;

	UPROPERTY(BlueprintReadWrite)
	bool bIsAttacking;

	UPROPERTY(BlueprintReadWrite)
	FEnemyActionData CurrentActionData;

	UPROPERTY(BlueprintReadWrite, Replicated)
	EEnemyState CurrentEnemyState;

	UPROPERTY(BlueprintReadWrite, Replicated)
	TEnumAsByte<EPathFollowingStatus::Type> FollowingStatus;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ABasicEnemyAIController* AIController;
};
