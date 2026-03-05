#include "Actors/Interactibles/Carriable.h"
#include "Components/BoxComponent.h"
#include "Interfaces/IPlayer.h"
#include "Net/UnrealNetwork.h"


void ACarriable::Interact_Implementation(AActor* Interactor)
{
	IPlayerInterface::Execute_StartCarryHeavyItem(Interactor, this);

	CarryActor = Interactor;
	bIsCarried = true;
	OnRep_IsCarried();
}

void ACarriable::StopCarry()
{
	CarryActor = nullptr;
	bIsCarried = false;

	InteractCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

TSubclassOf<AActor> ACarriable::GetTargetActorType()
{
	return TargetActor;
}

void ACarriable::PutInTargetActor_Implementation(AActor* Actor)
{
	this->Destroy();
}

void ACarriable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ACarriable, bIsCarried);
}

void ACarriable::OnRep_IsCarried()
{
	if (InteractCollider)
	{
		if (bIsCarried)
		{
			InteractCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		else
		{
			InteractCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
	}
}