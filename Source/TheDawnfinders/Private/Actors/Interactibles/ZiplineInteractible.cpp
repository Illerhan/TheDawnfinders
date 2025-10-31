// Fill out your copyright notice in the Description page of Project Settings.


#include "ZiplineInteractible.h"


// Sets default values
AZiplineInteractible::AZiplineInteractible()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AZiplineInteractible::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AZiplineInteractible::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AZiplineInteractible::CheckZiplineInteractibleInRange()
{
}

bool AZiplineInteractible::IsInLigneOfSight(AZiplineInteractible* otherZipline)
{

	if (!otherZipline || !GetWorld()) return false;

	FVector Start = otherZipline->GetActorLocation() + FVector(0, 0, 80.f);
	FVector End = GetActorLocation();

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(otherZipline);
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
		);

#if WITH_EDITOR
	if (GEngine)
	{
		DrawDebugLine(
			GetWorld(),
			Start,
			End,
			bHit ? FColor::Red : FColor::Green,
			false,
			2.0f,
			0,
			2.0f
		);

		if (bHit && HitResult.GetActor())
		{
			DrawDebugSphere(
				GetWorld(),
				HitResult.ImpactPoint,
				25.f,
				12,
				FColor::Orange,
				false,
				2.0f
			);
			UE_LOG(LogTemp, Warning, TEXT("GrapplePoint: Line trace blocked by %s"), 
				   *HitResult.GetActor()->GetName());
		}
	}
#endif
	
	return !bHit;
}

