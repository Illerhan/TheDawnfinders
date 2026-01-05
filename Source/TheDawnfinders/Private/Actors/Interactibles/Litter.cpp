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
	CapsuleCollider = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollider"));
	CapsuleCollider->SetupAttachment(CollisionBox);
	CapsuleCollider->SetCollisionResponseToAllChannels(ECR_Ignore);
	CapsuleCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CapsuleCollider->SetGenerateOverlapEvents(true);
	StaticMesh->SetupAttachment(RootComponent);

	Light = CreateDefaultSubobject<UPlayerLightComponent>(TEXT("LightSource"));
	Light->LightRoot->SetupAttachment(CollisionBox);
	//InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("LitterInventoryComponent"));
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

void ALitter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
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

    // ---- Mise à jour du light radius ----
    if (Light && Light->ProtectionZone)
    {
        float NewRadius = (Light->FuelRemaining / MaxFuel) * MaxRange;
        Light->ProtectionZone->SetSphereRadius(NewRadius);
    }

    if (!HasAuthority()) return;
    const float Now = GetWorld()->GetTimeSeconds();

    // ---- Nettoyage des porteurs inactifs ----
    TArray<TWeakObjectPtr<AAPlayerCharacter>> RemoveList;
    for (auto& Pair : ActivePushers)
        if (Now - Pair.Value.LastUdateTime > InputTimeout)
            RemoveList.Add(Pair.Key);
    for (auto& P : RemoveList)
        ActivePushers.Remove(P);

    if (ActivePushers.Num() == 0)
    {
        ServerVelocity = FVector::ZeroVector;
        return;
    }

    // ---- Somme des inputs ----
    FVector TotalInput = FVector::ZeroVector;
    for (auto& Pair : ActivePushers)
        TotalInput += Pair.Value.InputVector;

    if (TotalInput.IsNearlyZero())
    {
        ServerVelocity = FVector::ZeroVector;
        return;
    }

    FVector Dir = TotalInput.GetSafeNormal();
    float SpeedMultiplier = FMath::Clamp((float)ActivePushers.Num() / 4.f, 0.25f, 1.f);
    FVector Delta = Dir * MaxSpeed * SpeedMultiplier * DeltaTime;

    // ---- Paramètres step / slide ----
    float MaxStepHeight = 20.f;  // Hauteur max que l'on peut monter
    FHitResult Hit;

    // ---- SafeMoveUpdatedComponent ----
    if (CollisionBox)
    {
        // SafeMove gère automatiquement les collisions et les slides
        CollisionBox->MoveComponent(Delta, GetActorRotation(), true, &Hit);

        // Si bloqué, tente un step up
        if (Hit.IsValidBlockingHit() && Hit.Normal.Z < 0.99f)
        {
            FVector StepUp = FVector(0.f, 0.f, MaxStepHeight);
            FVector Slide = FVector::VectorPlaneProject(Delta, Hit.Normal);
            
            // Move avec step up
            CollisionBox->MoveComponent(Slide + StepUp, GetActorRotation(), true, &Hit);
        }
    }

    // ---- Gravité ----
    FVector Start = GetActorLocation();
    FVector End = Start - FVector(0.f, 0.f, 125.f);
    FHitResult GroundHit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (!GetWorld()->LineTraceSingleByChannel(GroundHit, Start, End, ECC_WorldStatic, Params))
    {
        FVector Gravity = FVector(0.f, 0.f, -980.f * DeltaTime);
        CollisionBox->MoveComponent(Gravity, GetActorRotation(), true, &Hit);
    }

    // ---- Mise à jour vitesse serveur ----
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
	if (UCapsuleComponent* PlayerCapsule = Player->GetCapsuleComponent())
	{
		// 1. Définir le mode de collision à QueryOnly (plus de Physics ou Blocking)
		PlayerCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly); 
       
		// 2. Définir la réponse par défaut : Ignorer les objets physiques (Block), Overlap tout le reste
		PlayerCapsule->SetCollisionResponseToAllChannels(ECR_Overlap);
       
		// 3. Cas spécifique : Assurez-vous que les objets du monde ou les acteurs ne vous bloquent pas :
		PlayerCapsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
		PlayerCapsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
		// Optionnel : Conserver l'overlap avec le Palanquin si nécessaire (mais normalement géré par l'attachement)
		// PlayerCapsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
       
		// 4. Important : Assurez-vous qu'elle peut encore détecter les "WorldDynamic" (pièges, etc.)
		// et générer les overlaps pour les ennemis/lumières.
	}

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
	if (UCapsuleComponent* PlayerCapsule = Player->GetCapsuleComponent())
	{
		// 1. Réappliquer le profil par défaut du Character (souvent 'Pawn' ou 'CharacterMesh')
		// Si vous utilisez le profil de collision par défaut d'un ACharacter, il est "Pawn".
		PlayerCapsule->SetCollisionProfileName(TEXT("Pawn")); 
        
		// 2. Réactiver la collision complète, qui est implicite dans le profil "Pawn".
		PlayerCapsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
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