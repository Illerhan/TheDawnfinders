#include "Others/BasicEnemyAIController.h"

void ABasicEnemyAIController::AddAlertness(float Quantity)
{
	Alertness += Quantity;

	if (Alertness >= PossessedPawn->EnemyData->AlertnessThreshold) {
		SetEnemyState(EEnemyState::Aggressive);
	}
}

void ABasicEnemyAIController::SetEnemyState_Implementation(EEnemyState NewEnemyState)
{

}
