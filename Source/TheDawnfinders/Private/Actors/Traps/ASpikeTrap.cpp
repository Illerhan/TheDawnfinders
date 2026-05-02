#include "ASpikeTrap.h"
#include "Interfaces/IDamageable.h"
#include "Net/UnrealNetwork.h"


AASpikeTrap::AASpikeTrap()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AASpikeTrap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AASpikeTrap, SpikeAnimValue);
}

void AASpikeTrap::DoTrapAction(AActor* OtherActor)
{
	Super::DoTrapAction(OtherActor);

}

void AASpikeTrap::Multicast_PlayTrapAnim_Implementation()
{
	DoSpikeAnim();
}

void AASpikeTrap::OnRep_SpikeAnimValue()
{
	ApplySpikePosition(SpikeAnimValue);
}

void AASpikeTrap::SetSpikeAnimValue(float Value)
{
	if (HasAuthority())
	{
		SpikeAnimValue = Value;
	}
}

void AASpikeTrap::DoSpikeAnim_Implementation()
{

}

