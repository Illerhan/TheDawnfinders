// Fill out your copyright notice in the Description page of Project Settings.


#include "GrapplePoint.h"

#include "Actors/Player/APlayerCharacter.h"
#include "Components/UInventoryComponent.h"


// Sets default values
AGrapplePoint::AGrapplePoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GrappleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrappleMesh"));
	GrappleMesh->SetupAttachment(RootComponent);
	GrappleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGrapplePoint::StartSmoothGrapple(AAPlayerCharacter* Player)
{
	if (!Player) return;

	bIsGrappling = true;
	GrapplingPlayer = Player;
	GrappleElapsedTime = 0.f;

	GrappleStartLocation = Player->GetActorLocation();
	GrappleTargetLocation = GetActorLocation() + TeleportOffset;

	UE_LOG(LogTemp, Log, TEXT("GrapplePoint: Smooth grapple started"));
}

void AGrapplePoint::UpdateSmoothGrapple(float DeltaTime)
{
	if (!bIsGrappling || !GrapplingPlayer)
		return;

	GrappleElapsedTime += DeltaTime;

	float Alpha = FMath::Clamp(GrappleElapsedTime / GrappleMoveDuration, 0.f, 1.f);
	float SmoothAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.f); // adoucit le mouvement

	FVector NewLocation = FMath::Lerp(GrappleStartLocation, GrappleTargetLocation, SmoothAlpha);
	GrapplingPlayer->SetActorLocation(NewLocation, true);

	if (Alpha >= 1.f)
	{
		bIsGrappling = false;
		GrapplingPlayer = nullptr;
		UE_LOG(LogTemp, Log, TEXT("GrapplePoint: Grapple complete"));
	}
}

void AGrapplePoint::Interaction(AAPlayerCharacter* Player)
{
	Super::Interaction(Player);

	if (!Player) return;

	if (!HasGrappleInInventory(Player))
	{
		OnGrappleFailed(Player, TEXT("Grappin required"));
		UE_LOG(LogTemp, Warning, TEXT("GrapplePoint: Player doesn't have grapple item"));
		return;
	}

	float Distance = FVector::Dist(Player->GetActorLocation(),GetActorLocation());
	if (Distance>MaxGrappleDistance)
	{
		OnGrappleFailed(Player, TEXT("Grapple out of range"));
		UE_LOG(LogTemp, Warning, TEXT("GrapplePoint: Grapple out of range"));
		return;
	}

	if (!HasLineOfSight(Player))
	{
		OnGrappleFailed(Player, TEXT("Obstacle on the way"));
		UE_LOG(LogTemp, Warning, TEXT("GrapplePoint: No line of sight"));
		return;
	}

	ServerTeleportPlayer(Player);

	BP_OnInteraction(Player);
}

// Called when the game starts or when spawned
void AGrapplePoint::BeginPlay()
{
	Super::BeginPlay();
	
}

bool AGrapplePoint::HasGrappleInInventory(AAPlayerCharacter* Player) const
{
	if (!Player || !RequiredGrappleItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("GrapplePoint: Invalid Player or RequiredGrappleItem"));
		return false;
	}

	UInventoryComponent* InventoryComp = Player->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("GrapplePoint: Player has no InventoryComponent"));
		return false;
	}

	if (InventoryComp->GetCurrentSlot().ItemData == RequiredGrappleItem && InventoryComp->GetCurrentSlot().Quantity>0)
	{
		UE_LOG(LogTemp, Log, TEXT("GrapplePoint: Player has grapple item"));
		return true;
	}
	UE_LOG(LogTemp, Warning, TEXT("GrapplePoint: Grapple item not found in inventory"));
	return false;
}

bool AGrapplePoint::HasLineOfSight(AAPlayerCharacter* Player) const
{

	if (!Player || !GetWorld()) return false;

	FVector Start = Player->GetActorLocation() + FVector(0, 0, 80.f);
	FVector End = GetActorLocation();

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Player);
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

void AGrapplePoint::ServerTeleportPlayer_Implementation(AAPlayerCharacter* Player)
{
	if (!Player)
	{
		UE_LOG(LogTemp, Error, TEXT("GrapplePoint: ServerTeleportPlayer - Player is null"));
		return;
	}

	// Double vérification côté serveur (sécurité)
	if (!HasGrappleInInventory(Player))
	{
		UE_LOG(LogTemp, Warning, TEXT("GrapplePoint: Server rejected teleport - no grapple"));
		return;
	}

	if (!HasLineOfSight(Player))
	{
		UE_LOG(LogTemp, Warning, TEXT("GrapplePoint: Server rejected teleport - no LOS"));
		return;
	}

	StartSmoothGrapple(Player);

	OnGrappleUsed(Player);
	ClientPlayGrappleEffects();
	
}

void AGrapplePoint::ClientPlayGrappleEffects_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("GrapplePoint: Playing grapple effects on client"));
}

// Called every frame
void AGrapplePoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsGrappling)
	{
		UpdateSmoothGrapple(DeltaTime);
	}
	
}

