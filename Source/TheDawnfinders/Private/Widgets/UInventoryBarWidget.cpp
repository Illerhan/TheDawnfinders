#include "Widgets/UInventoryBarWidget.h"
#include "Actors/Player/APlayerCharacter.h"
#include "GameFramework/CustomPlayerState.h"
#include "Widgets/UInventorySlotWidget.h"


#pragma region Initialisation / Destruction

void UInventoryBarWidget::NativeConstruct()
{
    Super::NativeConstruct();

    APlayerController* PC = GetOwningPlayer();
    if (!PC || !PC->IsLocalController())
    {
        return;
    }

    APawn* ControlledPawn = PC->GetPawn();
    if (ControlledPawn)
    {
        TryBindToInventory();
    }
    else
    {
        // Poll for pawn if not immediately available
        GetWorld()->GetTimerManager().SetTimer(BindDelayTimerHandle, this, &UInventoryBarWidget::TryBindToInventory, 0.1f, true);
    }
}

void UInventoryBarWidget::SetupInventory_Implementation(int SlotCount)
{

}

void UInventoryBarWidget::NativeDestruct()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(BindDelayTimerHandle);
    }

    if (InventoryComponentRef)
    {
        InventoryComponentRef->OnInventoryChange.RemoveDynamic(this, &UInventoryBarWidget::ActualiseWidget);
        InventoryComponentRef = nullptr;
    }

    Super::NativeDestruct();
}

#pragma endregion


#pragma region Main Functions

void UInventoryBarWidget::ShowWidget_Implementation()
{
    bIsInContainer = false;
}

void UInventoryBarWidget::ShowWidgetFromPalanquin_Implementation()
{
    bIsInContainer = true;
}

void UInventoryBarWidget::HideWidget_Implementation()
{

}


void UInventoryBarWidget::TryBindToInventory()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC || !PC->IsLocalController())
    {
        return;
    }

    APawn* Pawn = PC->GetPawn();
    if (!Pawn)
    {
        return;
    }

    AAPlayerCharacter* PlayerCharacter = Cast<AAPlayerCharacter>(Pawn);
    if (!PlayerCharacter || !PlayerCharacter->InventoryComponent)
    {
        return;
    }

    InventoryComponentRef = PlayerCharacter->InventoryComponent;
    GetWorld()->GetTimerManager().ClearTimer(BindDelayTimerHandle);

    // Bind TOUJOURS côté client
    InventoryComponentRef->OnInventoryChange.AddUniqueDynamic(this, &UInventoryBarWidget::ActualiseWidget);

    UE_LOG(LogTemp, Display, TEXT("%d"), InventoryComponentRef->InventorySlots.Num());

    // Forcer une update immédiate pour synchroniser avec l’état actuel
    SetupInventory(InventoryComponentRef->InventorySlotCount);
    UE_LOG(LogTemp, Error, TEXT("=== TryBind: InventorySlots au moment du bind ==="));
    for (int i = 0; i < InventoryComponentRef->InventorySlots.Num(); i++)
    {
        UE_LOG(LogTemp, Error, TEXT("Slot %d: %s"), i,
            InventoryComponentRef->InventorySlots[i].CurrentInfos.ItemData
                ? *InventoryComponentRef->InventorySlots[i].CurrentInfos.ItemData->ItemName
                : TEXT("VIDE"));
    }
    bool bHasItems = InventoryComponentRef->InventorySlots.ContainsByPredicate(
       [](const FInventorySlot& S){ return S.CurrentInfos.ItemData != nullptr; }
   );
    
    if (bHasItems)
    {
        ActualiseWidget(InventoryComponentRef->InventorySlots, InventoryComponentRef->CurrentSlotIndex);
    }
}

void UInventoryBarWidget::SetupSlotsNavigation_Implementation()
{
    for (int i = 0; i < InventorySlotsWidgets.Num(); i++) {
        if (i == 0) {
            InventorySlotsWidgets[i]->SetupNavigation(InventorySlotsWidgets[i + 1]);
            InventorySlotsWidgets[i]->SetupNavigation(InventorySlotsWidgets[InventorySlotsWidgets.Num() - 1]);
        }
        else if (i == InventorySlotsWidgets.Num() - 1) {
            InventorySlotsWidgets[i]->SetupNavigation(InventorySlotsWidgets[0]);
            InventorySlotsWidgets[i]->SetupNavigation(InventorySlotsWidgets[i - 1]);
        }
        else {
            InventorySlotsWidgets[i]->SetupNavigation(InventorySlotsWidgets[i + 1]);
            InventorySlotsWidgets[i]->SetupNavigation(InventorySlotsWidgets[i - 1]);
        }
    }
}

void UInventoryBarWidget::ActualiseWidget_Implementation(const TArray<FInventorySlot>& Slots, int32 CurrentIndex)
{
    UE_LOG(LogTemp, Error, TEXT("ActualiseWidget: %d slots dispo, %d slot widgets créés"),
       Slots.Num(), InventorySlotsWidgets.Num()); // ← ce chiffre est probablement 0
    
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    ACustomPlayerState* PSCustom = PC->GetPlayerState<ACustomPlayerState>();
    if (!PSCustom) return;

    PSCustom->ActualiseEquippedItem(Slots[CurrentIndex]);


    for (int32 i = 0; i < InventorySlotsWidgets.Num(); i++)
    {
        if (!InventorySlotsWidgets[i]) continue;

        if (Slots.IsValidIndex(i))
        {
            InventorySlotsWidgets[i]->ActualiseVisuals(Slots[i], i == CurrentIndex);
        }
        else
        {
            FInventorySlot EmptySlot;
            InventorySlotsWidgets[i]->ActualiseVisuals(EmptySlot, false);
        }
    }
}

#pragma endregion

