// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UInventoryComponent.h"


UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}




FInventorySlot UInventoryComponent::GetCurrentSlot_Implementation()
{
	return FInventorySlot();
}

FInventorySlot UInventoryComponent::ChangeCurrentSlot_Implementation(bool IndexGoUp)
{
	return FInventorySlot();
}

void UInventoryComponent::AddNewItem_Implementation(UItemData* NewItem)
{

}

void UInventoryComponent::Throw_Implementation()
{
	
}