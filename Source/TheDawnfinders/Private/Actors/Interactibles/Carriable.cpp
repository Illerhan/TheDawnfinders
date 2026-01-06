#include "Actors/Interactibles/Carriable.h"
#include "Interfaces/IPlayer.h"


void ACarriable::Interact_Implementation(AActor* Interactor)
{
	CarryActor = Interactor;
	bIsCarried = true;

	IPlayerInterface::Execute_StartCarryHeavyItem(Interactor, this);

	CapsuleCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACarriable::StopCarry()
{
	CarryActor = nullptr;
	bIsCarried = false;

	CapsuleCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

TSubclassOf<AActor> ACarriable::GetTargetActorType()
{
	return TargetActor;
}

void ACarriable::PutInTargetActor_Implementation(AActor* Actor)
{


	Destroy();
}

