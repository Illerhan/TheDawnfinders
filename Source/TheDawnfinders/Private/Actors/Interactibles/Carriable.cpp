#include "Actors/Interactibles/Carriable.h"
#include "Components/BoxComponent.h"
#include "Interfaces/IPlayer.h"
#include "Net/UnrealNetwork.h"


ACarriable::ACarriable()
{
	SetRootComponent(StaticMesh);

	InteractCollider->SetupAttachment(StaticMesh);
	BoxCollider->SetupAttachment(StaticMesh);
	InterestPointVFXComponent->SetupAttachment(StaticMesh);
}

void ACarriable::Interact_Implementation(AActor* Interactor)
{
	StaticMesh->SetSimulatePhysics(false);

	IPlayerInterface::Execute_StartCarryHeavyItem(Interactor, this);

	CarryActor = Interactor;
	bIsCarried = true;
	OnRep_IsCarried();
}

void ACarriable::StopCarry()
{
	StaticMesh->SetSimulatePhysics(true);

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
			StaticMesh->SetSimulatePhysics(false);
			InteractCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		else
		{
			StaticMesh->SetSimulatePhysics(true);
			InteractCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
	}
}