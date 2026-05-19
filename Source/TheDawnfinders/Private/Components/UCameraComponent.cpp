#include "Components/UCameraComponent.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Interfaces/IFadeable.h"


UPlayerCameraComponent::UPlayerCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerCameraComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (!bIsInitialised) return;

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ActualiseEnemiesInfos();
	ActualiseEnviroInfos();
	ActualisePlayerInfos(DeltaTime);

	if (!bIsOnForcedSize) {
		ForcedPositionDistanceProgress = FMath::Lerp(ForcedPositionDistanceProgress, 0, DeltaTime * (ForcedDistanceLerpSpeed != 0 ? ForcedDistanceLerpSpeed : CameraDistanceLerpSpeed));
		ForcedPositionDistanceProgress = FMath::Clamp(ForcedPositionDistanceProgress, 0, 1);

		UpdateDistance(DeltaTime);

		SpringArmComponent->TargetArmLength = FMath::Lerp(CurrentTotalDist, ForcedDist, ForcedPositionDistanceProgress);
	}
	else {
		ForcedPositionDistanceProgress = FMath::Lerp(ForcedPositionDistanceProgress, 1, DeltaTime * (ForcedDistanceLerpSpeed != 0 ? ForcedDistanceLerpSpeed : CameraDistanceLerpSpeed));
		ForcedPositionDistanceProgress = FMath::Clamp(ForcedPositionDistanceProgress, 0, 1);

		SpringArmComponent->TargetArmLength = FMath::Lerp(StartForcedDist, ForcedDist, ForcedPositionDistanceProgress);
	}


	if(!bIsOnForcedPosition) {
		ForcedPositionOffsetProgress = FMath::Lerp(ForcedPositionOffsetProgress, 0, DeltaTime * (ForcedOffsetLerpSpeed != 0 ? ForcedOffsetLerpSpeed : CameraOffsetLerpSpeed));
		ForcedPositionOffsetProgress = FMath::Clamp(ForcedPositionOffsetProgress, 0, 1);

		UpdateOffset(DeltaTime);

		SpringArmComponent->SetWorldLocation(FMath::Lerp(GetOwner()->GetActorLocation() + CurrentTotalOffset, ForcedPosition, ForcedPositionOffsetProgress));
	}

	else {
		ForcedPositionOffsetProgress = FMath::Lerp(ForcedPositionOffsetProgress, 1, DeltaTime * (ForcedOffsetLerpSpeed != 0 ? ForcedOffsetLerpSpeed : CameraOffsetLerpSpeed));
		ForcedPositionOffsetProgress = FMath::Clamp(ForcedPositionOffsetProgress, 0, 1);

		SpringArmComponent->SetWorldLocation(FMath::Lerp(StartForcedOffset, ForcedPosition, ForcedPositionOffsetProgress));
	}
}



void UPlayerCameraComponent::InitialiseComponent(USpringArmComponent* SpringArm)
{
	bIsInitialised = true;
	SpringArmComponent = SpringArm;

	Player = Cast<AAPlayerCharacter>(GetOwner());

	if (Player) {
		if (!Player->GetController()) bIsInitialised = false;
	}
	else bIsInitialised = false;
}

void UPlayerCameraComponent::UpdateOffset(float DeltaTime)
{
	FVector NewOffset = FVector(0, 0, 0);

	for (int i = 0; i < EnemiesAtRange.Num(); i++) {
		if (!IFadeable::Execute_GetIsDisplayed(EnemiesAtRange[i])) continue;

		FVector Offset = GetOwner()->GetActorLocation() - EnemiesAtRange[i]->GetActorLocation();
		Offset.Normalize();
		NewOffset -= Offset * FMath::Lerp(0, EnemiesOffsetMaxImpact, 1 - (Offset.Length() / EnemiesMaxRange));
	}

	if(EnemiesAtRange.Num() != 0)
		NewOffset /= EnemiesAtRange.Num();

	FVector AveragePos;
	for (int i = 0; i < NearbyWallsLocations.Num(); i++)
	{
		AveragePos += NearbyWallsLocations[i];
	}
	AveragePos /= NearbyWallsLocations.Num();
	NewOffset -= (GetOwner()->GetActorLocation() - AveragePos) * EnviroOffsetMaxImpact;

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

		if(Distance < 800)
			AverageDist += Distance;

		else
			AverageDist += Distance;
	}
	AverageDist /= NearbyWallsLocations.Num();
	NewDistance -= FMath::Lerp(0, EnviroDistanceMaxImpact, 1 - (AverageDist / EnviroRaycastsMaxRange));

	CurrentEnviroDist = FMath::Lerp(CurrentEnviroDist, NewDistance, DeltaTime * CameraDistanceLerpSpeed);
	CurrentTotalDist = CurrentEnviroDist + CurrentPlayerDist;
}


void UPlayerCameraComponent::StartForcePosition(FVector NewPos, float Dist, float LerpDistSpeedOverride, float LerpOffsetSpeedOverride, bool bOnlySize)
{
	ForcedPosition = NewPos;
	ForcedDist = Dist;

	StartForcedDist = CurrentTotalDist;
	StartForcedOffset = CurrentTotalOffset + GetOwner()->GetActorLocation();

	ForcedDistanceLerpSpeed = LerpDistSpeedOverride;
	ForcedOffsetLerpSpeed = LerpOffsetSpeedOverride;

	bIsOnForcedSize = true;
	if (!bOnlySize) {
		bIsOnForcedPosition = true;
	}
}

void UPlayerCameraComponent::StartAutomaticControl()
{
	bIsOnForcedPosition = false;
	bIsOnForcedSize = false;

	ForcedOffsetLerpSpeed *= 1.5f;
	ForcedDistanceLerpSpeed *= 1.5f;
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
	FVector BasePos = GetOwner()->GetActorLocation() + FVector(0, 0, 200.f);

	for (float CurrentAngle = 0; CurrentAngle <= 360; CurrentAngle += 5)
	{
		FVector Dir = FVector(FMath::Cos(CurrentAngle), FMath::Sin(CurrentAngle), 0);
		FVector EndPos = BasePos + Dir * EnviroRaycastsMaxRange;

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(GetOwner());

		bool bHit = GetWorld()->LineTraceSingleByObjectType(
			HitResult,
			BasePos,
			EndPos,
			ECC_WorldStatic
		);

		FColor LineColor = bHit ? FColor::Green : FColor::Red;
		if (bHit) EndPos = HitResult.ImpactPoint;

		/*DrawDebugLine(
			GetWorld(),
			BasePos,      // Début
			EndPos,       // Fin
			LineColor,    // Couleur
			false,        // Persistent (reste-t-il indéfiniment ?)
			0.05f,         // Durée de vie (en secondes)
			0,            // Priorité de profondeur
			2.0f          // Épaisseur de la ligne
		);*/

		if (!bHit) {
			NearbyWallsLocations.Add(EndPos - FVector(0, 0, 200.f));
			continue;
		}

		NearbyWallsLocations.Add(HitResult.ImpactPoint - FVector(0, 0, 200.f));
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
