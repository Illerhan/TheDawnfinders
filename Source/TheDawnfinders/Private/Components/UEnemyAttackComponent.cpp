// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UEnemyAttackComponent.h"
#include "AIController.h"



UEnemyAttackComponent::UEnemyAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UEnemyAttackComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UEnemyAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UEnemyAttackComponent::SortPossibleAttacks(TArray<UEnemyAttackData*> PossibleAttacks)
{
	SortedPossibleAttacks.Reset();
	CurrentAttacksCooldowns.Reset();

	for (int i = 0; i < PossibleAttacks.Num(); i++) {
		bool Found = false;

		for (int j = 0; j < SortedPossibleAttacks.Num(); j++) {
			if (SortedPossibleAttacks[j]->AttackPriority <= PossibleAttacks[i]->AttackPriority) {
				Found = true;

				SortedPossibleAttacks.Insert(PossibleAttacks[i], j);
				CurrentAttacksCooldowns.Add(0);

				break;
			}
		}

		if (Found) continue;

		SortedPossibleAttacks.Add(PossibleAttacks[i]);
		CurrentAttacksCooldowns.Add(0);
	}
}

UEnemyAttackData* UEnemyAttackComponent::GetCurrentAttack(TArray<AActor*> PlayersAtRange)
{
	for (int i = 0; i < SortedPossibleAttacks.Num(); i++) {
		if (CurrentAttacksCooldowns[i] != 0) continue;   // If the skill is on cooldown we skip

		// We verify all the triggers to see if they are all valid
		bool TriggerValid = true;
		for (int j = 0; j < SortedPossibleAttacks[i]->AttackTriggers.Num(); j++) {
			if (VerifyTrigger(SortedPossibleAttacks[i]->AttackTriggers[j], PlayersAtRange)) continue;

			TriggerValid = false;
			break;
		}

		if (!TriggerValid) continue;

		// We send the valid attack + Setup it's cooldown
		CurrentAttacksCooldowns[i] = SortedPossibleAttacks[i]->AttackCooldown + 1;
		return SortedPossibleAttacks[i];
	}

	return nullptr;
}

void UEnemyAttackComponent::ActualiseAttacksCooldowns()
{
	for (int i = 0; i < CurrentAttacksCooldowns.Num(); i++) {
		if (CurrentAttacksCooldowns[i] == 0) continue;

		CurrentAttacksCooldowns[i]--;
	}
}

bool UEnemyAttackComponent::VerifyTrigger(FEnemyAttackTrigger Trigger, TArray<AActor*> PlayersAtRange)
{
	switch (Trigger.EnemyAttackTriggerType) {
	case EEnemyAttackTriggerType::DistanceMin:
		for (int i = 0; i < PlayersAtRange.Num(); i++) {
			float Dist = (PlayersAtRange[i]->GetActorLocation() - GetOwner()->GetAttachParentActor()->GetActorLocation()).Length();

			if (Dist > Trigger.Value) return true;
		}

	case EEnemyAttackTriggerType::DistanceMax:
		for (int i = 0; i < PlayersAtRange.Num(); i++) {

			AAIController* AI = Cast<AAIController>(GetOwner());
			float Dist = (PlayersAtRange[i]->GetActorLocation() - AI->GetPawn()->GetActorLocation()).Length();

			if (Dist < Trigger.Value) return true;
		}

	case EEnemyAttackTriggerType::HealthMin:
		break;

	case EEnemyAttackTriggerType::HealthMax:
		break;
	}

	return false;
}