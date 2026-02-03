#include "Widgets/UQTEWidget.h"
#include "Actors/Interactibles/Interactible.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Kismet/GameplayStatics.h"


bool UQTEWidget::PressButton()
{
	return false;
}

void UQTEWidget::SetLinkedInteractible(AInteractibleObjects* Interactible)
{
	LinkedInteractible = Interactible;
}

void UQTEWidget::FailQTEStep()
{
	if (LinkedInteractible->GetNeededInteractItem() == NULL
		|| LinkedInteractible->GetInteractItemConsumptionType() != EInteractItemConsuptionType::ConsumeOnQTEFail) return;

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			if (AAPlayerCharacter* PlayerCharacter = Cast<AAPlayerCharacter>(Pawn))
			{
				PlayerCharacter->InventoryComponent->UseDurability(1);
			}
		}
	}
}

void UQTEWidget::DoQTEStep()
{
	if (LinkedInteractible->GetNeededInteractItem() == NULL
		|| LinkedInteractible->GetInteractItemConsumptionType() != EInteractItemConsuptionType::ConsumeOnQTEInput) return;

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			if (AAPlayerCharacter* PlayerCharacter = Cast<AAPlayerCharacter>(Pawn))
			{
				PlayerCharacter->InventoryComponent->UseDurability(1);
			}
		}
	}
}
