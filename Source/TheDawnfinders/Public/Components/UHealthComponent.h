#pragma once

#include "CoreMinimal.h"
#include "AkAudioEvent.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "UHealthComponent.generated.h"

class UStaminaComponent;
class UWorldHealthBar;


UENUM()
enum EVFXType
{
	Blood UMETA(DisplayName = "Blood"),
	Poison UMETA(DisplayName = "Poison")
};

UCLASS( ClassGroup=(Custom), Blueprintable, meta=(BlueprintSpawnableComponent) )
class THEDAWNFINDERS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


// === MAIN FUNCTIONS ===
public : 
	UFUNCTION(BlueprintCallable)
	void InitialiseComponent(float MaxHP, float MinMaxHP, float ReviveHP,float FallenSpeed,float DmgPoison, float FallenTime);

	UFUNCTION(BlueprintCallable)
	void Heal(float quantity);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_Heal(float quantity);

	UFUNCTION(BlueprintCallable)
	void TakeDamage(float quantity, EVFXType DamageType = EVFXType::Blood);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_TakeDamage(float quantity, AActor* Origin);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerChangeHealth(float newHealth);

	UFUNCTION(BlueprintCallable)
	void LocalChangeHealth();

	void RequestMaxHealthChange(float Amount);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ChangeCurrentMaxHealth(float newValue);

	UFUNCTION(BlueprintCallable)
	void ActualiseHurtPostProcess(float DeltaTime);


// === CURSE === 
public :
	UFUNCTION(BlueprintCallable)
	bool IsProtectedFromCurse() const;

	UFUNCTION(BlueprintCallable)
	void AddProtectionZone();

	UFUNCTION(BlueprintCallable)
	void RemoveProtectionZone();

	UFUNCTION(BlueprintCallable)
	void ApplyCurse(float DeltaTime);


// === POISON ===
public :
	UFUNCTION(BlueprintCallable)
	bool IsPoisoned() const
	{
		return bIsPoisoned;
	}

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void SetIsPoisoned(bool isPoisoned);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void StartPoisonEffects();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EndPoisonEffects();

	UFUNCTION(BlueprintCallable)
	void ActualisePoisonPostProcess(float DeltaTime);


// === DEATH ===
public : 
	UFUNCTION()
	void Fallen();
	
	UFUNCTION(Client,Unreliable)
	void Client_HeartbeatSound();

	UFUNCTION(BlueprintCallable)
	void FallenLoseHP(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void Die();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_Die();

	UFUNCTION(Server,Reliable,BlueprintCallable)
	void Server_Revive();

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_Revive();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_DisplayFallen();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_ActualiseFallen(float Percent);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_HideFallen();


// === INVINCIBILITY ===
public :
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StartInvincibilityFrames(float Duration);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void EndInvincibilityFrames();

	FTimerHandle InvincibilityTimerHandle;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UAkAudioEvent* DieBreath;
	
	UPROPERTY()
	int32 DieSoundID;


	// === NETWORK ===
public :
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnRep_IsDead();

	UFUNCTION()
	void OnRep_IsFallen();
	
	UFUNCTION()
	void OnRep_ProtectionZoneAmount();


// === PUBLIC PROPERTIES ===
public :
	UPROPERTY(Replicated)
	float CurrentHealth = 100.f;

	UPROPERTY()
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_IsDead, BlueprintReadWrite)
	bool bIsDead;
	
	UPROPERTY()
	int CurseZone = 0;


// === FALLEN PROPERTIES ===
public : 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FallenDuration;

	UPROPERTY(BlueprintReadOnly)
	float FallenTimer;

	UPROPERTY(ReplicatedUsing = OnRep_IsFallen, BlueprintReadWrite)
	bool bIsFallen;

	UPROPERTY(EditAnywhere)
	float PostProcessFallenOpacity = 2500.f;


// === PROTECTED PROPERTIES ===
protected :
	UPROPERTY(Replicated)
	float CurrentMaxHealth = 100.f;

	UPROPERTY(Replicated)
	float CurseMaxHealth = 100.f;

	UPROPERTY(BlueprintReadOnly)
	float MinReviveHP = 20.f;

	UPROPERTY(EditAnywhere)
	float InjureDecreaseSpeed = 0.02f;

	UPROPERTY(EditAnywhere)
	float PostProcessMaxOpacity = 2500.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_ProtectionZoneAmount)
	int32 ProtectionZoneAmount = 0;

	UPROPERTY()
	float MinimumMaxHP = 20.f;

	UPROPERTY()
	float CurseRatio= 1;

	UPROPERTY()
	UStaminaComponent* StaminaComponent;

	UPROPERTY()
	bool IsInvincible;
	
	UPROPERTY(Blueprintable,EditAnywhere, Replicated)
	bool bIsPoisoned = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PoisonDmg;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UMaterialInstanceDynamic* HurtMaterial;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UMaterialInstanceDynamic* PoisonMaterial;

	UPROPERTY(BlueprintReadOnly)
	float CurrentHurtVolumeStrength;

	UPROPERTY(BlueprintReadOnly)
	float CurrentPoisonVolumeStrength;

	UPROPERTY()
	APlayerController* OwnerController;

	UPROPERTY()
	UWorldHealthBar* WorldHealthBar;
	
	UPROPERTY()
	int32 BreathSoundID;
	
	UPROPERTY()
	int32 HeartBeatSoundID;
	
	UPROPERTY()
	int32 HealingSoundID;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UAkAudioEvent* FallenBreath;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UAkAudioEvent* HeartBeatFallen;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UAkAudioEvent* HealingSound;
};
