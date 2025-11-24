#include "Actors/Interactibles/GrapplePoint.h"

#include "Actors/Player/APlayerCharacter.h"
#include "Components/UInventoryComponent.h"


AGrapplePoint::AGrapplePoint()
{
	PrimaryActorTick.bCanEverTick = true;

	GrappleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrappleMesh"));
	GrappleMesh->SetupAttachment(RootComponent);
	GrappleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGrapplePoint::BeginPlay()
{
	Super::BeginPlay();
}

void AGrapplePoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsGrappling)
	{
		UpdateSmoothGrapple(DeltaTime);
	}

}


void AGrapplePoint::StartSmoothGrapple(AActor* Player)
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


void AGrapplePoint::Interact_Implementation(AActor* Interactor)
{
	Super::Interact_Implementation(Interactor);

	if (!Interactor) return;

	if (IPlayerInterface::Execute_GetEquippedItem(Interactor) == nullptr)
	{
		OnGrappleFailed(Interactor, TEXT("Grappin required"));
		return;
	}

	if (!HasLineOfSight(Interactor))
	{
		OnGrappleFailed(Interactor, TEXT("Obstacle on the way"));
		return;
	}

	ServerTeleportPlayer(Interactor);

	Super::Interact_Implementation(Interactor);
}


bool AGrapplePoint::HasLineOfSight(AActor* Player) const
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


void AGrapplePoint::ServerTeleportPlayer_Implementation(AActor* Player)
{
	if (!Player)
	{
		return;
	}

	// Double vérification côté serveur (sécurité)
	if (IPlayerInterface::Execute_GetEquippedItem(Player) == nullptr)
	{
		return;
	}

	if (!HasLineOfSight(Player))
	{
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