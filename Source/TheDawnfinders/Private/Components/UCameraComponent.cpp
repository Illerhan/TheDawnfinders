#include "Components/UCameraComponent.h"
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

	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return;
	if (!Pawn->Controller) return;
	if (!Pawn->Controller->IsLocalController()) return;

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ActualiseEnemiesInfos();
	ActualiseEnviroInfos();

	UpdateOffset(DeltaTime);
	UpdateDistance(DeltaTime);

	SpringArmComponent->TargetArmLength = CurrentDist;
	SpringArmComponent->SetWorldLocation(GetOwner()->GetActorLocation() + CurrentOffset);
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

	CurrentOffset = FMath::Lerp(CurrentOffset, NewOffset, DeltaTime * CameraOffsetLerpSpeed);
}

void UPlayerCameraComponent::UpdateDistance(float DeltaTime)
{
	float NewDistance = CameraBaseDistance;

	for (int i = 0; i < EnemiesAtRange.Num(); i++) {
		float Distance = (GetOwner()->GetActorLocation() - EnemiesAtRange[i]->GetActorLocation()).Length();
		NewDistance -= FMath::Lerp(0, EnemiesDistanceMaxImpact, 1 - (Distance / EnemiesMaxRange));
	}

	CurrentDist = FMath::Lerp(CurrentDist, NewDistance, DeltaTime * CameraDistanceLerpSpeed);
}



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

	EnemiesAtRange = TArray<AActor*>();

	//if (!bHit) return;

	float BestDist = EnemiesMaxRange;
	for (int i = 0; i < HitResults.Num(); i++) {
		float Dist = (HitResults[i].GetActor()->GetActorLocation() - GetOwner()->GetActorLocation()).Length();

		AActor* Actor = HitResults[i].GetActor();

		if (!IFadeable::Execute_GetIsDisplayed(Actor)) continue;
		if (Dist > BestDist) continue;

		BestDist = Dist;
		EnemiesAtRange = TArray<AActor*>();
		EnemiesAtRange.Add(HitResults[i].GetActor());
	}
}

void UPlayerCameraComponent::ActualiseEnviroInfos()
{

}