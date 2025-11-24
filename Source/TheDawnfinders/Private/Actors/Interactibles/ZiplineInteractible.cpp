// AZiplinePoint.cpp

#include "Actors/Interactibles/ZiplineInteractible.h"

#include "Actors/Player/APlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"

AZiplineInteractible::AZiplineInteractible()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

}

void AZiplineInteractible::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		TryLinkToNearbyZipline();
	}
}

void AZiplineInteractible::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsTravelling)
		UpdateTravel(DeltaTime);
}

void AZiplineInteractible::TryLinkToNearbyZipline()
{
	if (!HasAuthority()) return;

	TArray<AActor*> FoundZiplines;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZiplineInteractible::StaticClass(), FoundZiplines);

	for (AActor* Actor : FoundZiplines)
	{
		AZiplineInteractible* Other = Cast<AZiplineInteractible>(Actor);
		if (!Other || Other == this) continue;
		if (Other->LinkedZipline != nullptr) continue;

		float Dist = FVector::Dist(GetActorLocation(), Other->GetActorLocation());
		if (Dist <= LinkRange)
		{
			LinkedZipline = Other;
			Other->LinkedZipline = this;

			UE_LOG(LogTemp, Log, TEXT("Ziplines linked: %s <-> %s"), *GetName(), *Other->GetName());
			return;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("%s found no zipline to link"), *GetName());
}

void AZiplineInteractible::Interact_Implementation(AActor* Interactor)
{
	Super::Interact_Implementation(Interactor);

	if (!LinkedZipline)
	{
		UE_LOG(LogTemp, Warning, TEXT("Zipline has no link!"));
		return;
	}

	if (!HasAuthority())
	{
		ServerStartTravel(Interactor);
		return;
	}

	StartTravel(Interactor);
}

void AZiplineInteractible::ServerStartTravel_Implementation(AActor* Player)
{
	if (!LinkedZipline) return;
	StartTravel(Player);
}

void AZiplineInteractible::StartTravel(AActor* Player)
{
	if (!LinkedZipline || !Player) return;

	bIsTravelling = true;
	TravellingPlayer = (ACharacter*)Player;
	TravelTimer = 0.f;

	StartLocation = Player->GetActorLocation() + TeleportOffset;
	EndLocation = LinkedZipline->GetActorLocation() + TeleportOffset;

	if (UCharacterMovementComponent* MoveComp = TravellingPlayer->GetCharacterMovement())
	{
		MoveComp->DisableMovement();
		MoveComp->SetComponentTickEnabled(false);
	}

	MulticastStartTravel(Player, StartLocation, EndLocation);
}

void AZiplineInteractible::UpdateTravel(float DeltaTime)
{
	if (!TravellingPlayer) return;

	TravelTimer += DeltaTime;
	float Alpha = FMath::Clamp(TravelTimer / TravelDuration, 0.f, 1.f);
	float SmoothAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.f);

	FVector NewLoc = FMath::Lerp(StartLocation, EndLocation, SmoothAlpha);
	TravellingPlayer->SetActorLocation(NewLoc, true);

	if (Alpha >= 1.f)
		EndTravel();
}

void AZiplineInteractible::EndTravel()
{
	bIsTravelling = false;

	if (TravellingPlayer)
	{
		if (UCharacterMovementComponent* MoveComp = TravellingPlayer->GetCharacterMovement())
		{
			MoveComp->SetComponentTickEnabled(true);
			MoveComp->SetMovementMode(MOVE_Walking);
		}
	}

	TravellingPlayer = nullptr;
}

void AZiplineInteractible::ClientPlayTravelEffects_Implementation()
{
	// TODO: ajouter VFX/SFX pour le déplacement
}

void AZiplineInteractible::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AZiplineInteractible, LinkedZipline);
}

void AZiplineInteractible::MulticastStartTravel_Implementation(AActor* Player, FVector Start, FVector End)
{
	TravellingPlayer = (ACharacter*)Player;
	StartLocation = Start;
	EndLocation = End;
	TravelTimer = 0.f;
	bIsTravelling = true;

	ClientPlayTravelEffects();
}