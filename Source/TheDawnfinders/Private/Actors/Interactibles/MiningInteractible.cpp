// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Interactibles/MiningInteractible.h"

void AMiningInteractible::Interact_Implementation(AActor* Interactor)
{
	int ItemCount = FMath::RandRange(MinItemToSpawnCount, MaxItemToSpawnCount);
	for (int i = 0; i < ItemCount; i++) {
		FVector ItemLocation = GetPossibleSpawnLocation();

		AActor* NewActor = GetWorld()->SpawnActor<AActor>(ItemToSpawn, ItemLocation, FRotator());
	}

	Destroy();
}

FVector AMiningInteractible::GetPossibleSpawnLocation()
{
	int antiCrashCount = 0;

	while (antiCrashCount++ < 100) {
		FVector Origin = GetActorLocation();
		FVector RandomOffset = FVector(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f), 0.f);
		RandomOffset.Normalize();
		RandomOffset *= FMath::RandRange(-250.f, 250.f);

		FVector FinalPos = Origin + RandomOffset;

		FVector TraceStart = FinalPos + FVector(0, 0, 50);
		FVector TraceEnd = FinalPos - FVector(0, 0, 100);

		FHitResult Hit;
		FHitResult Hit2;
		FCollisionQueryParams Params;

		if (!GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params)) continue;
		if (GetWorld()->LineTraceSingleByChannel(Hit2, GetActorLocation() + FVector(0.f, 0.f, 60.f), Hit.ImpactPoint, ECC_Visibility, Params)) {
			continue;
		}


		return Hit.ImpactPoint + FVector(0.f, 0.f, 75.f);
	}

	return FVector();
}
