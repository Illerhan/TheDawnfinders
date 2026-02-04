#pragma once

#include "CoreMinimal.h"
#include "Actors/Interactibles/Interactible.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IFadeable.h"
#include "Components/TimelineComponent.h" 
#include "Curves/CurveFloat.h"
#include "ATrapBase.generated.h"


UENUM(BlueprintType)
enum class ETrapTriggerType : uint8 {
	OnColliderEnter,
	OnDuration
};


UCLASS()
class THEDAWNFINDERS_API ATrapBase : public AInteractibleObjects
{
	GENERATED_BODY()

public:
	ATrapBase();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	virtual void OnRep_TrappedActor();

	UFUNCTION()
	void OnTrapOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRep_Enabled();

	UFUNCTION(BlueprintCallable)
	void DisableTrap();

	UFUNCTION(BlueprintCallable)
	virtual void DoTrapAction();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayEffects();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


// === TRAP PARAMETERS ===
protected : 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	int Damages = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float Cooldown = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	ETrapTriggerType TrapTriggerType;

	UPROPERTY(ReplicatedUsing = OnRep_Enabled, EditAnywhere, BlueprintReadWrite, Category = "Trap")
	bool bEnable = true;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Duration Trap")
	float TriggerWaitDuration = 2.0f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Duration Trap")
	float StartOffsetDuration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	UBoxComponent* TrapCollider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	USoundBase* Sound;


// === TRAP PROPERTIES ===
protected :
	UPROPERTY()
	float CurrentCooldown = 0.f;

	UPROPERTY()
	float TriggerTimer = 0.f;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_TrappedActor)
	AActor* TrappedActor;
};
