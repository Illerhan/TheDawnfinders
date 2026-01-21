#include "Others/BasicEnemyAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/UEnemyAttackComponent.h"
#include "Perception/AISenseConfig_Hearing.h"


ABasicEnemyAIController::ABasicEnemyAIController()
{
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
    HearingConfig->HearingRange = 2000.f;

    EnemyAttackComponent = CreateDefaultSubobject<UEnemyAttackComponent>(TEXT("AC_EnemyAttack"));

    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(UAISense_Hearing::StaticClass());

    PrimaryActorTick.bCanEverTick = true;
}


void ABasicEnemyAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (EnemyState != EEnemyState::Idle) return;

    if (AlertnessTimer > 0) {
        AlertnessTimer -= DeltaTime;
        return;
    }

    if (Alertness > 0) {
        Alertness -= AlertnessDecreaseSpeed * DeltaTime;
        PossessedPawn->Multicast_ActualiseSuspicionProgress(Alertness / PossessedPawn->EnemyData->AlertnessThreshold);
    }
}


void ABasicEnemyAIController::AddAlertness(float Quantity, FVector Location)
{
    if (EnemyState != EEnemyState::Idle) return;

	Alertness += Quantity;
    Alertness = FMath::Clamp(Alertness, 0, PossessedPawn->EnemyData->AlertnessThreshold);

    PossessedPawn->Multicast_ActualiseSuspicionProgress(Alertness / PossessedPawn->EnemyData->AlertnessThreshold);

	if (Alertness >= PossessedPawn->EnemyData->AlertnessThreshold) {
		SetEnemyState(EEnemyState::Suspicious);
        AlertnessTimer = 0;
	}
}

void ABasicEnemyAIController::ResetAlertness()
{
    Alertness = 0;

    //PossessedPawn->Multicast_ActualiseSuspicionProgress(0);
}

void ABasicEnemyAIController::EndAttack_Implementation()
{

}

void ABasicEnemyAIController::SetHearingRange(float NewRange)
{
    if (HearingConfig)
    {
        HearingConfig->HearingRange = NewRange;

        AIPerceptionComponent->RequestStimuliListenerUpdate();
    }
}

void ABasicEnemyAIController::DisableAIController_Implementation()
{

}

void ABasicEnemyAIController::EnableAIController_Implementation()
{

}

UEnemyAttackComponent* ABasicEnemyAIController::GetEnemyAttackComponent()
{
    return EnemyAttackComponent;
}

void ABasicEnemyAIController::SetEnemyState_Implementation(EEnemyState NewEnemyState)
{

}

