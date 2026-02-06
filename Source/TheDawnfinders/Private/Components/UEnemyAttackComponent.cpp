// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UEnemyAttackComponent.h"
#include "Interfaces/IPlayer.h"
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

	for (int i = 0; i < CurrentAttacksCooldowns.Num(); i++) {
		if (CurrentAttacksCooldowns[i] <= 0) continue;

		CurrentAttacksCooldowns[i] -= DeltaTime;
	}
}


void UEnemyAttackComponent::SortPossibleAttacks(TArray<FName> PossibleAttacksRowNames)
{
	SortedPossibleAttacks.Reset();
	CurrentAttacksCooldowns.Reset();

	UDataTable* ActionsDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_EnemyActions.DT_EnemyActions"));
	if (!ActionsDataTable)
		UE_LOG(LogTemp, Error, TEXT("Failed to load DataTable"));

	TArray<FEnemyActionData> PossibleAttacks;
	for (int i = 0; i < PossibleAttacksRowNames.Num(); i++) {
		PossibleAttacks.Add(*ActionsDataTable->FindRow<FEnemyActionData>(PossibleAttacksRowNames[i], " "));
	}

	for (int i = 0; i < PossibleAttacksRowNames.Num(); i++) {
		bool Found = false;

		for (int j = 0; j < SortedPossibleAttacks.Num(); j++) {
			if (SortedPossibleAttacks[j].AttackPriority <= PossibleAttacks[i].AttackPriority) {
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

FEnemyActionData UEnemyAttackComponent::GetCurrentAttack(TArray<AActor*> PlayersAtRange)
{
	for (int i = 0; i < SortedPossibleAttacks.Num(); i++) {
		if (CurrentAttacksCooldowns[i] > 0) continue;   // If the skill is on cooldown we skip

		// We verify all the triggers to see if they are all valid
		bool TriggerValid = true;
		for (int j = 0; j < SortedPossibleAttacks[i].Triggers.Num(); j++) {
			if (VerifyTrigger(SortedPossibleAttacks[i].Triggers[j], PlayersAtRange)) continue;

			TriggerValid = false;
			break;
		}

		if (!TriggerValid) continue;

		// We send the valid attack + Setup it's cooldown
		CurrentAttacksCooldowns[i] = SortedPossibleAttacks[i].AttackCooldown + 1;

		LastAttackUsed = SortedPossibleAttacks[i];
		return SortedPossibleAttacks[i];
	}

	return FEnemyActionData();
}

FEnemyActionData UEnemyAttackComponent::GetLastAttackUsed()
{
	return LastAttackUsed;
}

bool UEnemyAttackComponent::VerifyTrigger(FEnemyAttackTrigger Trigger, TArray<AActor*> PlayersAtRange)
{
	switch (Trigger.EnemyAttackTriggerType) {
	case EEnemyAttackTriggerType::DistanceMin:
		for (AActor* Player : PlayersAtRange) {
			if (!Player->Implements<UPlayerInterface>()) continue;
			if (IPlayerInterface::Execute_GetCurrentPlayerState(Player) == EPlayerState::Fallen ||
				IPlayerInterface::Execute_GetCurrentPlayerState(Player) == EPlayerState::Dead) continue;

			float Dist = (Player->GetActorLocation() - GetOwner()->GetAttachParentActor()->GetActorLocation()).Length();

			if (Dist > Trigger.Value) return true;
		}
		break;

	case EEnemyAttackTriggerType::DistanceMax:
		for (AActor* Player : PlayersAtRange) {
			if (!Player->Implements<UPlayerInterface>()) continue;
			if (IPlayerInterface::Execute_GetCurrentPlayerState(Player) == EPlayerState::Fallen ||
				IPlayerInterface::Execute_GetCurrentPlayerState(Player) == EPlayerState::Dead) continue;

			AAIController* AI = Cast<AAIController>(GetOwner());
			float Dist = (Player->GetActorLocation() - AI->GetPawn()->GetActorLocation()).Length();

			if (Dist < Trigger.Value) return true;
		}
		break;

	case EEnemyAttackTriggerType::HealthMin:
		break;

	case EEnemyAttackTriggerType::HealthMax:
		break;
	}

	return false;
}