#include "Actors/Traps/CurseStatue.h"


ACurseStatue::ACurseStatue()
{

}

void ACurseStatue::DoTrapAction(AActor* OtherActor)
{
	OpenStatueEyes(OpenedEyesDuration);
}

void ACurseStatue::OpenStatueEyes_Implementation(float Duration)
{

}

void ACurseStatue::CloseStatueEyes_Implementation()
{

}

void ACurseStatue::DeployCurse_Implementation()
{
	
}
