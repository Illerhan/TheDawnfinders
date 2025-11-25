#pragma once

#include "CoreMinimal.h"
#include "Actors/Traps/ATrapBase.h"
#include "TrapSwingBlade.generated.h"

UCLASS()
class THEDAWNFINDERS_API ATrapSwingBlade : public ATrapBase
{
    GENERATED_BODY()

public:
    ATrapSwingBlade();
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

    // ========== Composants ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    USceneComponent* Pivot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    USceneComponent* PendulumArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UStaticMeshComponent* BladeMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    UStaticMeshComponent* ChainMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    class UBoxComponent* BladeCollider;

    // ========== Paramètres du Pendule (VISIBLE DANS VIEWPORT) ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pendulum", meta=(ClampMin="1.0", ClampMax="90.0", AllowPrivateAccess="true"))
    float MaxSwingAngle = 45.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pendulum", meta=(ClampMin="0.1", ClampMax="5.0", AllowPrivateAccess="true"))
    float SwingSpeed = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pendulum", meta=(ClampMin="50.0", ClampMax="500.0", AllowPrivateAccess="true"))
    float PendulumLength = 200.f;

    // ========== Paramètres de Dégâts (VISIBLE DANS VIEWPORT) ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage", meta=(ClampMin="0.0", AllowPrivateAccess="true"))
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UDamageType> DamageTypeClass;

    /** Enable or disable knockback for THIS specific trap */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage|Knockback", meta=(AllowPrivateAccess="true"))
    bool bApplyKnockback = true;

    /** Horizontal knockback force (set to 0 to disable horizontal push) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage|Knockback", 
        meta=(EditCondition="bApplyKnockback", EditConditionHides, ClampMin="0.0", ClampMax="5000.0", AllowPrivateAccess="true"))
    float KnockbackStrength = 1000.f;

    /** Vertical knockback force (0 = no upward force, 1 = full upward launch) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage|Knockback", 
        meta=(EditCondition="bApplyKnockback", EditConditionHides, ClampMin="0.0", ClampMax="1.0", AllowPrivateAccess="true"))
    float KnockbackUpwardForce = 0.3f;

    /** Override knockback direction (if enabled, uses this direction instead of blade direction) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage|Knockback", 
        meta=(EditCondition="bApplyKnockback", EditConditionHides, AllowPrivateAccess="true"))
    bool bUseCustomKnockbackDirection = false;

    /** Custom knockback direction (only if bUseCustomKnockbackDirection is true) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage|Knockback", 
        meta=(EditCondition="bApplyKnockback && bUseCustomKnockbackDirection", EditConditionHides, AllowPrivateAccess="true"))
    FVector CustomKnockbackDirection = FVector(1.f, 0.f, 0.f);

    // ========== Effets Audio/Visuels (VISIBLE DANS VIEWPORT) ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite,  meta=(AllowPrivateAccess="true"))
    USoundBase* HitSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effects", meta=(ClampMin="0.0", ClampMax="2.0", AllowPrivateAccess="true"))
    float HitSoundVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effects", meta=(AllowPrivateAccess="true"))
    UParticleSystem* HitParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effects", meta=(AllowPrivateAccess="true"))
    FVector ParticleScale = FVector(1.f, 1.f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effects", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UCameraShakeBase> CameraShakeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effects", meta=(ClampMin="0.0", ClampMax="2.0", AllowPrivateAccess="true"))
    float CameraShakeScale = 1.0f;

    // ========== Variables internes ==========
    FRotator InitialRotation;
    float TimeAccumulator = 0.f;
    float LastSwingDirection = 1.f;
    float CurrentSwingVelocity = 0.f;
    bool bHasHitThisSwing = false;
    TArray<AActor*> ActorsHitThisSwing;

    // ========== Fonctions ==========
    virtual void DoTrapAction() override;

    UFUNCTION()
    void OnBladeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnBladeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // Événement Blueprint pour gameplay custom
    UFUNCTION(BlueprintImplementableEvent, Category="Trap")
    void OnBladeHitTarget(AActor* HitActor, float DamageDealt);

};