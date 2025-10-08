// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UInventoryComponent.h"


UUInventoryComponent::UUInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UUInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UUInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UUInventoryComponent::Throw()
{
	
}