// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/UInventoryBarWidget.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Widgets/UInventorySlotWidget.h"

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

    // Bind TOUJOURS côté client
    InventoryComponentRef->OnInventoryChanging.AddUniqueDynamic(this, &UInventoryBarWidget::ActualiseWidget);

    // Forcer une update immédiate pour synchroniser avec l’état actuel
    ActualiseWidget(InventoryComponentRef->InventorySlots, InventoryComponentRef->CurrentSlotIndex);
}

void UInventoryBarWidget::NativeDestruct()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(BindDelayTimerHandle);
    }

    if (InventoryComponentRef)
    {
        InventoryComponentRef->OnInventoryChanging.RemoveDynamic(this, &UInventoryBarWidget::ActualiseWidget);
        InventoryComponentRef = nullptr;
    }

    Super::NativeDestruct();
}

void UInventoryBarWidget::ActualiseWidget_Implementation(const TArray<FInventorySlot>& Slots, int32 CurrentIndex)
{
    UE_LOG(LogTemp, Error, TEXT("════════════════════════════════════════════════════════"));
    UE_LOG(LogTemp, Error, TEXT("ActualiseWidget called! Slots: %d, CurrentIndex: %d"), Slots.Num(), CurrentIndex);
    
    for (int32 i = 0; i < InventorySlotsWidgets.Num(); i++)
    {
        if (!InventorySlotsWidgets[i])
        {
            continue;
        }

        if (Slots.IsValidIndex(i))
        {
            UE_LOG(LogTemp, Warning, TEXT("  Slot %d: %s (Qty: %d) [%s]"), 
                i,
                Slots[i].ItemData ? *Slots[i].ItemData->ItemName : TEXT("Empty"),
                Slots[i].Quantity,
                i == CurrentIndex ? TEXT("SELECTED") : TEXT(""));
            InventorySlotsWidgets[i]->ActualiseVisuals(Slots[i], i == CurrentIndex);
        }
        else
        {
            FInventorySlot EmptySlot;
            InventorySlotsWidgets[i]->ActualiseVisuals(EmptySlot, false);
        }
    }
    UE_LOG(LogTemp, Error, TEXT("════════════════════════════════════════════════════════"));
}
