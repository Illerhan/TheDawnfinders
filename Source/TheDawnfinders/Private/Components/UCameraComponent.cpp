#include "Components/UCameraComponent.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Interfaces/IFadeable.h"


UPlayerCameraComponent::UPlayerCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerCameraComponent::BeginPlay()
{
	Player = Cast<AAPlayerCharacter>(GetOwner());

	if (Player) {
		bIsInitialised = true;

		if(!Player->GetController()) bIsInitialised = false;
	}
	else bIsInitialised = false;


	Super::BeginPlay();
}

void UPlayerCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (!bIsInitialised) return;

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsOnForcedPosition) {
		ActualiseEnemiesInfos();
		ActualiseEnviroInfos();
		ActualisePlayerInfos(DeltaTime);

		UpdateOffset(DeltaTime);
		UpdateDistance(DeltaTime);

		SpringArmComponent->TargetArmLength = CurrentTotalDist;
		SpringArmComponent->SetWorldLocation(GetOwner()->GetActorLocation() + CurrentTotalOffset);
	}
	else {
		CurrentEnviroDist = FMath::Lerp(CurrentEnviroDist, ForcedDist, DeltaTime * (ForcedDistanceLerpSpeed != 0 ? ForcedDistanceLerpSpeed : CameraDistanceLerpSpeed));
		CurrentEnviroOffset = FMath::Lerp(CurrentEnviroOffset, ForcedPosition - GetOwner()->GetActorLocation(), 
			DeltaTime * (ForcedOffsetLerpSpeed != 0 ? ForcedOffsetLerpSpeed : CameraOffsetLerpSpeed));

		SpringArmComponent->TargetArmLength = CurrentEnviroDist;
		SpringArmComponent->SetWorldLocation(GetOwner()->GetActorLocation() + CurrentEnviroOffset);
	}
}



void UPlayerCameraComponent::InitialiseComponent(USpringArmComponent* SpringArm)
{
	bIsInitialised = true;
	SpringArmComponent = SpringArm;
}

void UPlayerCameraComponent::UpdateOffset(float DeltaTime)
{
	FVector NewOffset = FVector(0, 0, 0);

	for (int i = 0; i < EnemiesAtRange.Num(); i++) {
		FVector Offset = GetOwner()->GetActorLocation() - EnemiesAtRange[i]->GetActorLocation();
		Offset.Normalize();
		NewOffset -= Offset * FMath::Lerp(0, EnemiesOffsetMaxImpact, 1 - (Offset.Length() / EnemiesMaxRange));
	}

	if(EnemiesAtRange.Num() != 0)
		NewOffset /= EnemiesAtRange.Num();

	CurrentEnviroOffset = FMath::Lerp(CurrentEnviroOffset, NewOffset, DeltaTime * CameraOffsetLerpSpeed);
	CurrentTotalOffset = CurrentEnviroOffset + CurrentPlayerOffset;
}

void UPlayerCameraComponent::UpdateDistance(float DeltaTime)
{
	float NewDistance = CameraBaseDistance;
	float Distance = 0;

	if (FarestEnemy) {
		Distance = (GetOwner()->GetActorLocation() - FarestEnemy->GetActorLocation()).Length();
		NewDistance -= FMath::Lerp(0, EnemiesDistanceMaxImpact, 1 - (Distance / EnemiesMaxRange));
	}

	float AverageDist = 0;
	for (int i = 0; i < NearbyWallsLocations.Num(); i++) 
	{
		Distance = (GetOwner()->GetActorLocation() - NearbyWallsLocations[i]).Length();
		AverageDist += Distance;
	}
	AverageDist /= NearbyWallsLocations.Num();
	NewDistance -= FMath::Lerp(0, EnviroDistanceMaxImpact, 1 - (AverageDist / 2000.f));

	CurrentEnviroDist = FMath::Lerp(CurrentEnviroDist, NewDistance, DeltaTime * CameraDistanceLerpSpeed);
	CurrentTotalDist = CurrentEnviroDist + CurrentPlayerDist;
}

void UPlayerCameraComponent::StartForcePosition(FVector NewPos, float Dist, float LerpDistSpeedOverride, float LerpOffsetSpeedOverride)
{
	ForcedPosition = NewPos;
	ForcedDist = Dist;

	ForcedDistanceLerpSpeed = LerpDistSpeedOverride;
	ForcedOffsetLerpSpeed = LerpOffsetSpeedOverride;

	bIsOnForcedPosition = true;
}

void UPlayerCameraComponent::StartAutomaticControl()
{
	bIsOnForcedPosition = false;

	ForcedOffsetLerpSpeed = 0;
	ForcedDistanceLerpSpeed = 0;
}


#pragma region Actualise Modificators

void UPlayerCameraComponent::ActualiseEnemiesInfos()
{
	TArray<FHitResult> HitResults;
	FVector Start = GetOwner()->GetActorLocation();
	float Radius = EnemiesMaxRange;
	FCollisionQueryParams Params;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Start,
		Start,
		FQuat::Identity,
		ECC_GameTraceChannel1,
		Sphere,
		Params,
		FCollisionResponseParams::DefaultResponseParam
	);

	EnemiesAtRange.Reset();
	FarestEnemy = nullptr;

	if (!bHit) return;

	float BestDist = 0;
	for (int i = 0; i < HitResults.Num(); i++)
	{
		AActor* Actor = HitResults[i].GetActor();

		if (!IsValid(Actor)) continue;
		if (!Actor->Implements<UFadeable>()) continue;
		if (!IFadeable::Execute_GetIsDisplayed(Actor)) continue;
		
		// Le reste de la logique peut continuer
		float Dist = (Actor->GetActorLocation() - GetOwner()->GetActorLocation()).Length();

		EnemiesAtRange.Add(Actor);

		if (Dist < BestDist) continue;

		FarestEnemy = Actor;
		BestDist = Dist;
	}
}

void UPlayerCameraComponent::ActualiseEnviroInfos()
{
	NearbyWallsLocations.Reset();
	FVector BasePos = GetOwner()->GetActorLocation();

	for (float CurrentAngle = 0; CurrentAngle <= 360; CurrentAngle += 20)
	{
		FVector Dir = FVector(FMath::Cos(CurrentAngle), FMath::Sin(CurrentAngle), 0);
		FVector EndPos = BasePos + Dir * 2000.f;

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(GetOwner());

		bool bHit = GetWorld()->LineTraceSingleByObjectType(
			HitResult,
			BasePos,
			EndPos,
			ECC_WorldStatic
		);

		if (!bHit) {
			NearbyWallsLocations.Add(EndPos);
			continue;
		}

		NearbyWallsLocations.Add(HitResult.ImpactPoint);
	}
}

void UPlayerCameraComponent::ActualisePlayerInfos(float DeltaTime)
{
	EPlayerState CurrentState = IPlayerInterface::Execute_GetCurrentPlayerState(GetOwner());
	float TargetDist = 0;

	switch (CurrentState) {
	case EPlayerState::None :
		if (GetOwner()->GetVelocity().SquaredLength() > 1) TargetDist = PlayerWalkDistance;
		else TargetDist = PlayerIdleDistance;
		break;

	case EPlayerState::Running:
		TargetDist = PlayerRunDistance;
		break;

	case EPlayerState::Sneaking:
		TargetDist = PlayerCrouchDistance;
		break;

	case EPlayerState::Dodging:
		TargetDist = PlayerRunDistance;
		break;

	case EPlayerState::Fallen:
		TargetDist = PlayerFallenDistance;
		break;

	case EPlayerState::Dead:
		TargetDist = PlayerDeadDistance;
		break;
	}

	if (!Player->GetIsForcingRotation() || Player->GetAutoLockIsActive() || Player->InventoryComponent->GetIsOpened()) {
		CurrentPlayerOffset = FMath::Lerp(CurrentPlayerOffset, FVector(0, 0, 0), DeltaTime * PlayerOffsetSpeed);
	}
	else {
		CurrentPlayerOffset = FMath::Lerp(CurrentPlayerOffset, Player->GetCurrentRotationInput() * PlayerForceRotationOffset, DeltaTime * PlayerOffsetSpeed);
		TargetDist += PlayerForceRotationDistance;
	}

	CurrentPlayerDist = FMath::Lerp(CurrentPlayerDist, TargetDist, DeltaTime * PlayerDistanceSpeed);
}


#pragma endregion
