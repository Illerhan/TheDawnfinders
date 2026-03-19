// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyRegistry.h"

void UEnemyRegistry::RegisterEnemy(ABaseEnemy* Enemy)
{
	if (!Enemy) return;

	int32 DangerLvl = Enemy->DangerLevel;
	AllEnemies.AddUnique(Enemy);

	EnemiesByDangerLevel.FindOrAdd(DangerLvl).Enemies.AddUnique(Enemy);
}

TArray<ABaseEnemy*> UEnemyRegistry::GetEnemiesByDangerLevel(int32 DangerLevel) const
{
	const FEnemyList* List = EnemiesByDangerLevel.Find(DangerLevel);
	return List ? List->Enemies : TArray<ABaseEnemy*>();
}

void UEnemyRegistry::UnregisterEnemy(ABaseEnemy* Enemy)
{
	if (!Enemy) return;

	int32 DangerLvl = Enemy->DangerLevel;
	AllEnemies.Remove(Enemy);

	if (FEnemyList* List = EnemiesByDangerLevel.Find(DangerLvl))
	{
		List->Enemies.Remove(Enemy);
		if (List->Enemies.IsEmpty())
		{
			EnemiesByDangerLevel.Remove(DangerLvl);
		}
	}
}