#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IFadeable.h"
#include "Components/TimelineComponent.h" 
#include "Curves/CurveFloat.h"
#include "ATrapBase.generated.h"

UCLASS()
class THEDAWNFINDERS_API ATrapBase : public AActor, public IFadeable
{
	GENERATED_BODY()

public:
	ATrapBase();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY()
	AActor* TrappedActor;
	
	/** Damage value (server side only) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trap")
	int Damages = 10;

	/** Cooldown time (in seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trap")
	float Cooldown = 1.f;

	/** Is trap active ? (replicated) */
	UPROPERTY(ReplicatedUsing=OnRep_Enabled, EditAnywhere, BlueprintReadWrite, Category="Trap")
	bool bEnable = true;

	/** Trigger collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
	UBoxComponent* TrapCollider;

	/** Sound played for ALL players on activation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision")
	USoundBase* Sound;

	/** Called when overlap happens (SERVER ONLY handles it) */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						bool bFromSweep, const FHitResult& SweepResult);


// == FADEABLE ===
public :
	virtual void FadeIn_Implementation() override;
	virtual void FadeOut_Implementation() override;

	UFUNCTION(NetMulticast, Reliable)
	void FadeIn_Multicast();

	UFUNCTION(NetMulticast, Reliable)
	void FadeOut_Multicast();

	UFUNCTION()
	void HandleFadeProgress(float Value);

	UPROPERTY()
	FTimeline FadeTimeline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* FloatCurve;

	UPROPERTY(BlueprintReadWrite)
	int FogOfWarCount = 0;

	UPROPERTY(BlueprintReadWrite)
	TArray<UMaterialInstanceDynamic*> Materials;


protected : 
	/** Cooldown timer (server only) */
	float CurrentCooldown = 0.f;

	/** Called when bEnable changes on clients */
	UFUNCTION()
	void OnRep_Enabled();

	/** Disable trap */
	UFUNCTION(BlueprintCallable)
	void DisableTrap();

	/** Function children override to define effects (server only) */
	UFUNCTION(BlueprintCallable)
	virtual void DoTrapAction();

	/** Play FX/SFX on all machines */
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayEffects();

	/** Needed for replication */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
