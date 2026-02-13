// Fill out your copyright notice in the Description page of Project Settings.


#include "UDoorRegistry.h"

#include "Actors/MovableObjects/Doors.h" // 🔥 INCLUDE COMPLET dans le .cpp

void UUDoorRegistry::RegisterExtractionDoor(ADoors* Door)
{
	if (Door)
	{
		ExtractDoors.Add(Door);
		UE_LOG(LogTemp, Warning, TEXT("[REGISTRY] Door '%s' registered (Total: %d)"), 
			   *Door->GetName(), ExtractDoors.Num());
	}
}

void UUDoorRegistry::OpenAllExtractionDoors()
{
	UE_LOG(LogTemp, Warning, TEXT("[REGISTRY] Opening ALL %d extraction doors"), ExtractDoors.Num());
    
	int32 OpenedCount = 0;
    
	for (auto DoorPtr : ExtractDoors)
	{
		if (DoorPtr.IsValid())
		{
			DoorPtr->OpenPermanently();
			OpenedCount++;
		}
	}
    
	UE_LOG(LogTemp, Warning, TEXT("[REGISTRY] Successfully opened %d doors"), OpenedCount);
}
