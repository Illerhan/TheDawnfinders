// Fill out your copyright notice in the Description page of Project Settings.


#include "WolfTrap.h"

#include "Actors/Enemy/ABaseEnemy.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Interfaces/IDamageable.h"


void AWolfTrap::DoTrapAction()
{
	Super::DoTrapAction();
	IPlayerInterface::Execute_RequestStateChange(TrappedActor, EPlayerState::Immobilized);
}

