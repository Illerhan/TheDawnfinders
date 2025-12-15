#include "ASpikeTrap.h"
#include "Interfaces/IDamageable.h"


AASpikeTrap::AASpikeTrap()
{
	PrimaryActorTick.bCanEverTick = true;
}


void AASpikeTrap::DoTrapAction()
{
	if (!TrappedActor) return;

	Super::DoTrapAction();
	DoSpikeAnim();
	//IDamageable::Execute_ReceiveDamage(TrappedActor, Damages, this);
}

void AASpikeTrap::DoSpikeAnim_Implementation()
{
}

