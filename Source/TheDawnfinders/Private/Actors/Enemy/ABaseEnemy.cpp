#include "Actors/Enemy/ABaseEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"


ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

    CurrentHealth = EnemyData->Health;
}

void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseEnemy::DoAttack(UEnemyAttackData* AttackData)
{
    if (!GetMesh()) return;

    GetCharacterMovement()->MaxWalkSpeed = 0.f;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->StopAllMontages(0.1f);
    AnimInstance->Montage_Play(AttackData->AttackAnimMontage);

    AnimInstance->OnPlayMontageNotifyBegin.RemoveAll(this);

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &ABaseEnemy::OnEndAttack);
    AnimInstance->Montage_SetBlendingOutDelegate(EndDelegate, AttackData->AttackAnimMontage);
    AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ABaseEnemy::OnMontageNotifyBegin);
}

void ABaseEnemy::OnEndAttack(UAnimMontage* Montage, bool bInterrupted)
{
    GetCharacterMovement()->MaxWalkSpeed = EnemyData->AggressiveSpeed;
}

void ABaseEnemy::OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
    BP_OnMontageNotifyBegin(NotifyName);
}


void ABaseEnemy::ReceiveDamage_Implementation(float Quantity, AActor* Origin) {
    CurrentHealth -= Quantity;

    UE_LOG(LogTemp, Display, TEXT("Enemy Health = %d"), CurrentHealth);

    if (CurrentHealth <= 0) {
        Die();
    }
}

void ABaseEnemy::Die() {
    Destroy();
}