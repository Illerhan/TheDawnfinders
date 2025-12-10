#include "Litter.h"
#include "Actors/Player/APlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ALitter::ALitter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	// ===== Collision principale (Root) =====
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	CollisionBox->SetBoxExtent(FVector(80.f, 80.f, 50.f));
	
	// ===== Collider interaction =====
	CapsuleCollider->SetupAttachment(CollisionBox);
	CapsuleCollider->SetCollisionResponseToAllChannels(ECR_Ignore);
	CapsuleCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CapsuleCollider->SetGenerateOverlapEvents(true);

	// ===== Light =====
	Light = CreateDefaultSubobject<UPlayerLightComponent>(TEXT("Light"));
	Light->LightRoot->SetupAttachment(RootComponent);

	// ===== Carry points =====
	
	CarryPoints.SetNum(4);
	for (int i = 0; i < 4; i++)
	{
		FString Name = FString::Printf(TEXT("CarryPoint_%d"), i);

		CarryPoints[i] = CreateDefaultSubobject<USceneComponent>(*Name);
		CarryPoints[i]->SetupAttachment(RootComponent);
	}
	CarrySlots.SetNum(4);
}

void ALitter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALitter, ServerVelocity);
}

void ALitter::Interact_Implementation(AActor* Interactor)
{
	Super::Interact_Implementation(Interactor);
	Server_StartPushing(Cast<AAPlayerCharacter>(Interactor));
	PlayerTemp = Interactor;
}

void ALitter::StopInteract_Implementation(AActor* Interactor)
{
	Super::StopInteract_Implementation(Interactor);
	Server_EndPushing(Cast<AAPlayerCharacter>(Interactor));
}

void ALitter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
	
	if (true)
	{
		float NewRadius = (Light->FuelRemaining/MaxFuel) * MaxRange;
		Light->ProtectionZone->SetSphereRadius(NewRadius);
	}
	
    if (!HasAuthority()) return;

    const float Now = GetWorld()->GetTimeSeconds();

    // ------------------ Nettoyage des porteurs inactifs ------------------
    TArray<TWeakObjectPtr<AAPlayerCharacter>> RemoveList;
    for (auto& Pair : ActivePushers)
    {
        if (Now - Pair.Value.LastUdateTime > InputTimeout)
            RemoveList.Add(Pair.Key);
    }
    for (auto& P : RemoveList)
        ActivePushers.Remove(P);

    if (ActivePushers.Num() == 0)
    {
        ServerVelocity = FVector::ZeroVector;
        return;
    }

    // ------------------ Somme des inputs ------------------
    FVector TotalInput = FVector::ZeroVector;
    for (auto& Pair : ActivePushers)
        TotalInput += Pair.Value.InputVector;

    if (TotalInput.IsNearlyZero())
    {
        ServerVelocity = FVector::ZeroVector;
        return;
    }

    FVector Dir = TotalInput.GetSafeNormal();

    // Vitesse proportionnelle au nombre de porteurs
    float SpeedMultiplier = FMath::Clamp((float)ActivePushers.Num() / 4.f, 0.25f, 1.f);
    FVector Delta = Dir * MaxSpeed * SpeedMultiplier * DeltaTime;

    // ------------------ Déplacement principal ------------------
	FHitResult Hit;
	CollisionBox->MoveComponent(Delta, GetActorRotation(), true, &Hit);

	// ------------------ Correction de fin de pente ------------------
	if (Hit.IsValidBlockingHit())
	{
		// Si on touche un "coin" (arête de rampe)
		bool bIsRampEdge = Hit.Normal.Z > 0.1f && Hit.Normal.Z < 0.9f;

		if (bIsRampEdge)
		{
			FVector Lift = FVector(0.f, 0.f, 15.f); // petit lift horizontal
			CollisionBox->MoveComponent(Lift, GetActorRotation(), true);
		}
	}

	// ------------------ Slide si pente réelle ------------------
	if (Hit.IsValidBlockingHit() && Hit.Normal.Z < 0.99f)
	{
		FVector Slide = FVector::VectorPlaneProject(Delta, Hit.Normal);
		CollisionBox->MoveComponent(Slide, GetActorRotation(), true);
	}
    // ------------------ Vérification du sol et gravité ------------------
    FVector Start = GetActorLocation();
    FVector End = Start - FVector(0.f, 0.f, 70.f); // distance pour vérifier le sol
    FHitResult GroundHit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bOnGround = GetWorld()->LineTraceSingleByChannel(GroundHit, Start, End, ECC_WorldStatic, Params);

    if (!bOnGround)
    {
        // Appliquer gravité si en l'air
        FVector Gravity = FVector(0.f, 0.f, -980.f * DeltaTime);
        CollisionBox->MoveComponent(Gravity, GetActorRotation(), true, &Hit);
    }

    // ------------------ Mise à jour de la vitesse serveur ------------------
    ServerVelocity = Dir * MaxSpeed * SpeedMultiplier;

	
}

void ALitter::BeginPlay()
{
	Super::BeginPlay();
	Light->InitialiseComponent(FuelRate,MaxFuel);
	Light->ProtectionZone->SetSphereRadius(MaxFuel);
}


// ==================== Attach / Detach ====================

void ALitter::AttachPlayer(AAPlayerCharacter* Player)
{
	if (!Player) return;
	if (CarryPoints.Num() == 0) return;

	int32 FreeIndex = -1;
	for (int i = 1; i < 4; i++)
	{
		if (!CarrySlots[i].IsValid())
		{
			FreeIndex = i;
			break;
		}
	}

	if (FreeIndex == -1) return;
	if (!CarryPoints.IsValidIndex(FreeIndex)) return;
	
	CarrySlots[FreeIndex] = Player;
	
	Player->AttachToComponent(
		CarryPoints[FreeIndex],
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);

	Player->bIsCarrying = true;
	Player->CurrentPushedObject = this;
}

void ALitter::DetachPlayer(AAPlayerCharacter* Player)
{
	if (!Player) return;

	for (int i = 0; i < 4; i++)
	{
		if (CarrySlots[i].Get() == Player)
		{
			CarrySlots[i] = nullptr;
			break;
		}
	}

	Player->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Player->bIsCarrying = false;
	Player->CurrentPushedObject = nullptr;
}

// ==================== RPC ====================

void ALitter::Server_StartPushing_Implementation(AAPlayerCharacter* Player)
{
	if (!HasAuthority() || !Player) return;

	FPusherData& Data = ActivePushers.FindOrAdd(Player);
	Data.InputVector = FVector::ZeroVector;
	Data.LastUdateTime = GetWorld()->GetTimeSeconds();

	AttachPlayer(Player);
	Player->GetCharacterMovement()->DisableMovement();
	Player->Server_SetPushingState(this, true);
}

void ALitter::Server_EndPushing_Implementation(AAPlayerCharacter* Player)
{
	if (!HasAuthority() || !Player) return;

	ActivePushers.Remove(Player);
	DetachPlayer(Player);
	Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	Player->Server_SetPushingState(this, false);
}

void ALitter::Server_UpdateInputs_Implementation(AAPlayerCharacter* Player, FVector Input)
{
	if (!HasAuthority() || !Player) return;

	if (FPusherData* Data = ActivePushers.Find(Player))
	{
		Data->InputVector = Input.GetClampedToMaxSize(1.f);
		Data->LastUdateTime = GetWorld()->GetTimeSeconds();
	}
}