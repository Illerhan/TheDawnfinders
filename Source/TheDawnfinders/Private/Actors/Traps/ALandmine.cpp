#include "Actors/Traps/ALandmine.h"

void ALandmine::Explose_Implementation()
{
}

void ALandmine::DoTrapAction(AActor* OtherActor)
{
	Super::DoTrapAction(OtherActor);
	Explose();
}
