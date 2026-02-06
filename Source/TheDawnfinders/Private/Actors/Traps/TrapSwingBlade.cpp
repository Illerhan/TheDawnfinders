#include "Actors/Traps/TrapSwingBlade.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/IDamageable.h"
#include "Particles/ParticleSystemComponent.h"

ATrapSwingBlade::ATrapSwingBlade()
{
    USceneComponent* CeilingAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("CeilingAnchor"));
    RootComponent = CeilingAnchor;

    Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
    Pivot->SetupAttachment(RootComponent);

    PendulumArm = CreateDefaultSubobject<USceneComponent>(TEXT("PendulumArm"));
    PendulumArm->SetupAttachment(Pivot);
    PendulumArm->SetRelativeLocation(FVector(0, 0, -200.f));

    ChainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChainMesh"));
    ChainMesh->SetupAttachment(Pivot);
    ChainMesh->SetRelativeLocation(FVector(0, 0, -100.f));
    ChainMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    BladeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BladeMesh"));
    BladeMesh->SetupAttachment(PendulumArm);
    BladeMesh->SetRelativeLocation(FVector(0, 0, 0));
    BladeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    BladeCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BladeCollider"));
    BladeCollider->SetupAttachment(BladeMesh);
    BladeCollider->SetBoxExtent(FVector(50.f, 10.f, 100.f));
    BladeCollider->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    
    if (TrapCollider)
    {
        TrapCollider->DestroyComponent();
        TrapCollider = nullptr;
    }

    PrimaryActorTick.bCanEverTick = true;
}

void ATrapSwingBlade::BeginPlay()
{
    Super::BeginPlay();

    InitialRotation = GetActorRotation();
    
    PendulumArm->SetRelativeLocation(FVector(0, 0, -PendulumLength));
    
    if (BladeCollider)
    {
        BladeCollider->OnComponentBeginOverlap.AddDynamic(this, &ATrapSwingBlade::OnBladeOverlapBegin);
        BladeCollider->OnComponentEndOverlap.AddDynamic(this, &ATrapSwingBlade::OnBladeOverlapEnd);
    }
}

void ATrapSwingBlade::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeAccumulator += DeltaTime * SwingSpeed;

    if (HasAuthority()) {
        float Angle = FMath::Sin(TimeAccumulator) * MaxSwingAngle;
        Pivot->SetRelativeRotation(FRotator(Angle, 0.f, 0.f));
    }

    float AngularVelocity = FMath::Abs(FMath::Cos(TimeAccumulator) * SwingSpeed * MaxSwingAngle);
    CurrentSwingVelocity = AngularVelocity;

    float CurrentDirection = FMath::Sign(FMath::Cos(TimeAccumulator));
    if (CurrentDirection != LastSwingDirection)
    {
        bHasHitThisSwing = false;
        LastSwingDirection = CurrentDirection;
        ActorsHitThisSwing.Empty();
    }
}

void ATrapSwingBlade::OnBladeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || OtherActor == this) return;
    
    ACharacter* HitCharacter = Cast<ACharacter>(OtherActor);
    if (HitCharacter || OtherActor->ActorHasTag("Player") || OtherActor->ActorHasTag("Character"))
    {
        if (!ActorsHitThisSwing.Contains(OtherActor))
        {
            TrappedActor = OtherActor;
            DoTrapAction();
            ActorsHitThisSwing.Add(OtherActor);
        }
    }
}

void ATrapSwingBlade::OnBladeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (TrappedActor == OtherActor)
    {
        TrappedActor = nullptr;
    }
}

void ATrapSwingBlade::DoTrapAction(AActor* OtherActor)
{
    if (!TrappedActor) return;

    // Calculer les dégâts
    float VelocityMultiplier = FMath::Clamp(CurrentSwingVelocity / MaxSwingAngle, 0.5f, 1.5f);
    float FinalDamage = Damages * VelocityMultiplier * DamageMultiplier;
    
    FVector ImpactPoint = BladeCollider->GetComponentLocation();
    
    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this, 
            HitSound, 
            ImpactPoint,
            HitSoundVolume,
            1.0f
        );
    }

    if (HitParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            HitParticles,
            ImpactPoint,
            FRotator::ZeroRotator,
            ParticleScale
        );
    }

    ACharacter* HitCharacter = Cast<ACharacter>(TrappedActor);
    if (bApplyKnockback && HitCharacter)
    {
        FVector KnockbackDirection;
        
        if (bUseCustomKnockbackDirection)
        {
            // Utiliser la direction personnalisée
            KnockbackDirection = CustomKnockbackDirection.GetSafeNormal();
        }
        else
        {
            // Direction depuis la lame vers la cible
            KnockbackDirection = (TrappedActor->GetActorLocation() - BladeMesh->GetComponentLocation()).GetSafeNormal();
        }
        
        // Ajouter la force verticale
        KnockbackDirection.Z = KnockbackUpwardForce;
        KnockbackDirection.Normalize();
        
        // Appliquer le knockback (seulement si KnockbackStrength > 0)
        if (KnockbackStrength > 0.f)
        {
            HitCharacter->LaunchCharacter(
                KnockbackDirection * KnockbackStrength,
                true,
                true
            );
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Blade hit %s for %.1f damage! (Knockback: %s)"), 
        *TrappedActor->GetName(), 
        FinalDamage,
        bApplyKnockback ? TEXT("YES") : TEXT("NO"));

    OnBladeHitTarget(TrappedActor, FinalDamage);
    IDamageable::Execute_ReceiveDamage(TrappedActor,Damages,this);
}
