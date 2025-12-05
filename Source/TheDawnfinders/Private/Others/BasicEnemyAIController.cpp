#include "Others/BasicEnemyAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"


ABasicEnemyAIController::ABasicEnemyAIController()
{
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
    HearingConfig->HearingRange = 2000.f;
    HearingConfig->LoSHearingRange = 2500.f;

    // Pour s'assurer que l’AI utilise le sens auditif
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(UAISense_Hearing::StaticClass());
}




void ABasicEnemyAIController::AddAlertness(float Quantity)
{
	Alertness += Quantity;

	if (Alertness >= PossessedPawn->EnemyData->AlertnessThreshold) {
		SetEnemyState(EEnemyState::Aggressive);
	}
}

void ABasicEnemyAIController::SetHearingRange(float NewRange)
{
    if (HearingConfig)
    {
        HearingConfig->HearingRange = NewRange;
        HearingConfig->LoSHearingRange = NewRange * 1.2f; 

        AIPerceptionComponent->RequestStimuliListenerUpdate();
    }
}

void ABasicEnemyAIController::SetEnemyState_Implementation(EEnemyState NewEnemyState)
{

}

