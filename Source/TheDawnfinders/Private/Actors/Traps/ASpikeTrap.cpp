#include "ASpikeTrap.h"
#include "Interfaces/IDamageable.h"


AASpikeTrap::AASpikeTrap()
{
	PrimaryActorTick.bCanEverTick = true;
}


void AASpikeTrap::DoTrapAction(AActor* OtherActor)
{
	//if (!TrappedActor) return;

	Super::DoTrapAction();

	DoSpikeAnim();
}

void AASpikeTrap::DoSpikeAnim_Implementation()
{
}

