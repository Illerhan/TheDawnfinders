#include "Actors/Enemy/ABaseEnemy.h"

#include "AkComponent.h"
#include "AkGameplayStatics.h"
#include "BlueprintNodes/PostEventAsync.h"
#include "Components/WidgetComponent.h"
#include "Components/UEnemyAttackComponent.h"
#include "Widgets/UEnemyWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/Interactibles/AContainer.h"
#include "Interfaces/IDamageable.h"
#include "Others/BasicEnemyAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/EnemyRegistry.h"


ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    EnemyWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("EnemyWidgetComponent");
    EnemyWidgetComponent->SetupAttachment(RootComponent);

    AttackCollisionPosRef = CreateDefaultSubobject<USceneComponent>("AttackCollisionPosRef");
    AttackCollisionPosRef->SetupAttachment(GetMesh());
    AkComponent = CreateDefaultSubobject<UAkComponent>(TEXT("AkAudioComponent"));

    // 2. Attachement au Skeletal Mesh du personnage
    if (GetMesh())
    {
        AkComponent->SetupAttachment(GetMesh());
    }
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

    EnemyWidget = Cast<UEnemyWidget>(EnemyWidgetComponent->GetWidget());

    CurrentHealth = EnemyData->Health;
    EnableDistance = EnableDistance * EnableDistance;

    GetWorldTimerManager().SetTimer(
        EnableTimer,
        this,
        &ABaseEnemy::CheckEnableDistance,
        1.5f,   // time in seconds
        true    // looping
    );
    
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UEnemyRegistry* Registry = GI->GetSubsystem<UEnemyRegistry>())
        {
            Registry->RegisterEnemy(this);
        }
    }
}


void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (StunTimer > 0) {
        StunTimer -= DeltaTime;
        if (StunTimer <= 0) UnstunEnemy();
    }

    if(HasAuthority())
        CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed;
}

void ABaseEnemy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ABaseEnemy::CheckEnableDistance()
{
    float ClosestDistSq = TNumericLimits<float>::Max();

    if (!GetWorld()) return;
    if (!GetWorld()->GetPlayerControllerIterator()) return;

    // We go through all the players 
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (APawn* Pawn = It->Get()->GetPawn())
        {
            float DistSq = FVector::DistSquared((GetActorLocation()), Pawn->GetActorLocation());
            ClosestDistSq = FMath::Min(ClosestDistSq, DistSq);
        }
    }

    if (ClosestDistSq < EnableDistance)
    {    
        ShowEnemy();
    }
    else
    {
        HideEnemy();
    }
}


void ABaseEnemy::StunEnemy_Implementation(float Duration)
{
    StunTimer = Duration;
    bIsStuned = true;
}

void ABaseEnemy::UnstunEnemy_Implementation()
{
    bIsStuned = false;
}


void ABaseEnemy::ExitCrystal_Implementation()
{
    bIsCrystallized = false;
}

void ABaseEnemy::ShowEnemy_Implementation()
{
}

void ABaseEnemy::HideEnemy_Implementation()
{

}


void ABaseEnemy::StartAttack_Implementation(FEnemyActionData AttackData, AActor* Target)
{
    if (!GetMesh()) return;

    bIsAttacking = true;
    CurrentActionData = AttackData;

    //GetCharacterMovement()->MaxWalkSpeed = 0;
    MulticastPlayMontage(AttackData.Animation, AttackData.MontageSpeed);
}


#pragma region States Functions


void ABaseEnemy::EnterIdleState_Implementation()
{
}

void ABaseEnemy::EnterListeningState_Implementation()
{
}

void ABaseEnemy::EnterSuspicious_Implementation()
{
}


void ABaseEnemy::Multicast_HideEye_Implementation()
{
    EnemyWidget->HideAnim();
}

void ABaseEnemy::Multicast_EnterIdle_Implementation()
{
    EnterIdleState();
}

void ABaseEnemy::Multicast_EnterListening_Implementation()
{
    EnterListeningState();

    EnemyWidget->PlayListeningAnim();
}

void ABaseEnemy::Multicast_EnterSuspicious_Implementation()
{
    EnterSuspicious();

    EnemyWidget->PlaySuspiciousAnim();
}

void ABaseEnemy::Multicast_EnterAggressives_Implementation()
{
    EnemyWidget->PlayAggressiveAnim();
}

#pragma endregion


void ABaseEnemy::MulticastPlayMontage_Implementation(UAnimMontage* Montage, float Speed)
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->StopAllMontages(0.1f);
    AnimInstance->Montage_Play(Montage, Speed);

    AnimInstance->OnPlayMontageNotifyBegin.RemoveAll(this);

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &ABaseEnemy::OnMontageEnd);
    AnimInstance->Montage_SetBlendingOutDelegate(EndDelegate, Montage);
    AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ABaseEnemy::OnMontageNotifyBegin);
}

void ABaseEnemy::Multi_PlayHitSound_Implementation()
{
    HitSoundID = UAkGameplayStatics::PostEvent(HitSound,Owner,0,FOnAkPostEventCallback(), false);
}

void ABaseEnemy::OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
    BP_OnMontageNotifyBegin(NotifyName);
}


void ABaseEnemy::DoAttackCollision()
{
    //if (!HasAuthority()) return;
    //FEnemyActionData EnemyAction = AIController->GetEnemyAttackComponent()->GetLastAttackUsed();

    //GetCharacterMovement()->MaxWalkSpeed = 0;

    TArray<FHitResult> HitResults;
    FVector Start = AttackCollisionPosRef->GetComponentLocation();
    FCollisionQueryParams Params;
    FCollisionShape Box = FCollisionShape::MakeBox(FVector(EnemyData->AttacksRadius, EnemyData->AttacksRadius, EnemyData->AtttacksRange));

    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        Start,
        Start,
        AttackCollisionPosRef->GetComponentRotation().Quaternion(),
        ECC_PhysicsBody,
        Box,
        Params,
        FCollisionResponseParams::DefaultResponseParam
    );

    if (EnemyData->DisplayCollisionDebug) 
    {
        DrawDebugBox(
            GetWorld(),
            Start,
            Box.GetExtent(),
            AttackCollisionPosRef->GetComponentRotation().Quaternion(),
            FColor::Blue,
            false,
            1.0f,
            0,
            1.5f
        );
    }

    if (!bHit) return;

    Multi_PlayHitSound();
    TSet<AActor*> AlreadyHitActors;
    for (int i = 0; i < HitResults.Num(); i++) {

        if (!HitResults[i].GetActor()) continue;
        if (!HitResults[i].GetActor()->ActorHasTag("Player")) continue;
        if (AlreadyHitActors.Contains(HitResults[i].GetActor())) continue;

        AlreadyHitActors.Add(HitResults[i].GetActor());

        IDamageable::Execute_ReceiveDamage(HitResults[i].GetActor(), EnemyData->Damages, this);
    }
}


void ABaseEnemy::SetLockRotation(bool Locked)
{
    bLockRotation = Locked;
}

void ABaseEnemy::OnMontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
    GetCharacterMovement()->MaxWalkSpeed = EnemyData->AggressiveBaseSpeed;

    bIsAttacking = false;

    BP_OnMontageEnd(bInterrupted);
}


void ABaseEnemy::ReceiveDamage_Implementation(float Quantity, AActor* Origin) 
{
    if (bIsDead) return;

    CurrentHealth -= Quantity;

    Multicast_DisplayDamageBar(CurrentHealth / EnemyData->Health);
    if (CurrentHealth <= 0) {
        Die();
    }

    DoHitEffect();
}

void ABaseEnemy::Multicast_DisplayDamageBar_Implementation(float Percent)
{
    HealthBarWidget->TakeDamage(Percent, false);
}

void ABaseEnemy::Server_TakeDamages_Implementation(float Quantity, AActor* Origin)
{
    if (bIsDead) return;

    UE_LOG(LogTemp, Display, TEXT("%f"), CurrentHealth);

    CurrentHealth -= Quantity;
    HealthBarWidget->TakeDamage(CurrentHealth / EnemyData->Health, false);

    if (CurrentHealth <= 0) {
        Die();
    }

    DoHitEffect();
}

void ABaseEnemy::Die() {

    FVector StartLocation = GetActorLocation();
    FVector EndLocation = StartLocation + (FVector::DownVector * 2000);

    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Visibility, 
        CollisionParams
    );

    // DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.0f, 0, 2.0f);

    if (bHit)
    {
        FVector SpawnLocation = HitResult.ImpactPoint + FVector(0, 0, 60);
        FRotator SpawnRotation = FRotator();

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AContainer* Container = Cast<AContainer>(GetWorld()->SpawnActor<AActor>(ContainerToSpawn, SpawnLocation, SpawnRotation, SpawnParams));
    }

    MulticastPlayMontage_Implementation(DeathMontage, 1);

    bIsDead = true;
}


void ABaseEnemy::StartInvincibilityFrames(float Duration)
{
    if (IsInvincible) return;

    IsInvincible = true;

    GetWorld()->GetTimerManager().SetTimer(
        InvincibilityTimerHandle,
        this,
        &ABaseEnemy::EndInvincibilityFrames,
        Duration,
        false
    );
}

void ABaseEnemy::EndInvincibilityFrames()
{
    IsInvincible = false;
}

void ABaseEnemy::DoHitEffect_Implementation()
{

}


void ABaseEnemy::FadeIn_Implementation()
{
    IsDisplayed = true;
}

void ABaseEnemy::FadeOut_Implementation()
{
    IsDisplayed = false;
}

bool ABaseEnemy::GetIsDisplayed_Implementation()
{
    return IsDisplayed;
}
