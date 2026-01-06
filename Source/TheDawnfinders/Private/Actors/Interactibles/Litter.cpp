#include "Litter.h"
#include "Actors/Player/APlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

ALitter::ALitter()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true); // Let Unreal sync the final Transform

    // 1. Root Collision (Physics box)
    RootCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RootCollision"));
    RootComponent = RootCollision;

    RootCollision->SetMobility(EComponentMobility::Movable); 
    RootCollision->SetSimulatePhysics(false);
    
    RootCollision->SetBoxExtent(FVector(80.f, 120.f, 50.f)); // Adjust to palanquin size
    RootCollision->SetCollisionProfileName(TEXT("BlockAllDynamic")); 
    // Ensure it blocks world/physics but doesn't get stuck on pawns easily
    RootCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore); 

    // 2. Visual Mesh
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionProfileName(TEXT("NoCollision")); // Visual only
    
    // *** FIX: Rotate mesh -90 degrees if it faces the wrong way ***
    MeshComponent->SetRelativeRotation(FRotator(0.f, -90.f, 0.f)); 

    // 3. Interaction Trigger
    CapsuleCollider->SetupAttachment(RootComponent);

    // 4. Create Carry Points (Corners)
    CarryPoints.SetNum(4);
    for (int i = 0; i < 4; i++)
    {
        FString Name = FString::Printf(TEXT("CarryPoint_%d"), i);
        CarryPoints[i] = CreateDefaultSubobject<USceneComponent>(*Name);
        CarryPoints[i]->SetupAttachment(RootComponent);

        // Setup Positions relative to center:
        // 0: Front Left, 1: Front Right, 2: Back Left, 3: Back Right
        float XPos = (i < 2) ? 80.f : -80.f;  // Front vs Back
        float YPos = (i % 2 == 0) ? -50.f : 50.f; // Left vs Right
        
        CarryPoints[i]->SetRelativeLocation(FVector(XPos, YPos, 0.f));
    }

    CarrySlots.SetNum(4);
    CurrentLinearVelocity = FVector::ZeroVector;
    CurrentAngularVelocityYaw = 0.f;
}

void ALitter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALitter, CurrentLinearVelocity);
    DOREPLIFETIME(ALitter, CurrentAngularVelocityYaw);
}

void ALitter::BeginPlay()
{
    Super::BeginPlay();
}

// ============================================================================
//                                PHYSICS LOOP
// ============================================================================

void ALitter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Only Server calculates physics. Clients just receive the Transform update.
    if (HasAuthority())
    {
        ResolvePhysics(DeltaTime);
    }
}

void ALitter::ResolvePhysics(float DeltaTime)
{
    const float TimeNow = GetWorld()->GetTimeSeconds();

    // --- 1. Cleanup Stale Inputs ---
    TArray<AAPlayerCharacter*> ToRemove;
    for (auto& Pair : ActivePushers)
    {
        if (TimeNow - Pair.Value.LastUpdateTime > InputTimeout)
        {
            // If player stopped sending input, assume zero input but keep attached
            Pair.Value.InputVector = FVector::ZeroVector;
        }
    }

    // --- 2. Calculate Forces & Torques ---
    FVector TotalForce = FVector::ZeroVector;
    float TotalTorqueZ = 0.0f;
    int32 ActiveCount = 0;

    for (int i = 0; i < 4; i++)
    {
        // Check if we have a player in this slot
        AAPlayerCharacter* Pusher = CarrySlots[i].Get();
        if (!Pusher) continue;

        // Get their input data
        if (!ActivePushers.Contains(Pusher)) continue;
        FVector InputDir = ActivePushers[Pusher].InputVector;

        // --- VISUAL DEBUG: Draw Arrow for Player Input ---
        if (!InputDir.IsZero())
        {
            ActiveCount++;
            FVector PPos = CarryPoints[i]->GetComponentLocation();
            DrawDebugDirectionalArrow(GetWorld(), PPos, PPos + InputDir * 100.f, 
                5.0f, FColor::Green, false, -1, 0, 3.0f);
        }
        // -------------------------------------------------

        if (InputDir.IsNearlyZero()) continue;

        // Force Calculation: Direction * Strength
        FVector AppliedForce = InputDir.GetSafeNormal() * PushForce;

        // A. Add to Linear Force
        TotalForce += AppliedForce;

        // B. Add to Torque (Cross Product)
        // LeverArm = Position of CarryPoint relative to Center (Rotated to World Space)
        FVector LeverArmLocal = CarryPoints[i]->GetRelativeLocation();
        FVector LeverArmWorld = GetActorRotation().RotateVector(LeverArmLocal);

        // Torque = r x F.  We only care about Z-axis rotation.
        FVector Torque3D = FVector::CrossProduct(LeverArmWorld, AppliedForce);
        TotalTorqueZ += Torque3D.Z;
    }

    // --- VISUAL DEBUG: Print status ---
    if (ActiveCount > 0 || !CurrentLinearVelocity.IsZero())
    {
        FString Msg = FString::Printf(TEXT("Pushers: %d | Force: %s | Velocity: %s"), 
            ActiveCount, *TotalForce.ToString(), *CurrentLinearVelocity.ToString());
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, Msg);

        // Draw Total Force Arrow from Center
        DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), 
            GetActorLocation() + TotalForce * 0.001f, 10.f, FColor::Red, false, -1, 0, 5.f);
    }
    // ----------------------------------

    bool bIsPushing = !TotalForce.IsZero();

    if (bIsPushing)
    {
        // CAS A : ON POUSSE
        // On applique l'accélération normale
        FVector Acceleration = TotalForce / Mass;
        CurrentLinearVelocity += Acceleration * DeltaTime;
        
        // On applique une friction légère (Damping) pour limiter la vitesse max
        float LinearDrag = 1.0f - (LinearDamping * DeltaTime);
        CurrentLinearVelocity *= FMath::Clamp(LinearDrag, 0.f, 1.f);
    }
    else
    {
        // CAS B : ON LACHE TOUT (FREINAGE)
        // On réduit la vitesse vers 0 rapidement
        float CurrentSpeed = CurrentLinearVelocity.Size();
        
        if (CurrentSpeed > 0.f)
        {
            // On calcule combien de vitesse on perd ce tour-ci
            float SpeedDrop = BrakingDeceleration * DeltaTime;
            
            // Nouvelle vitesse (on ne descend pas en dessous de 0)
            float NewSpeed = FMath::Max(0.f, CurrentSpeed - SpeedDrop);
            
            // On réapplique la direction
            CurrentLinearVelocity = CurrentLinearVelocity.GetSafeNormal() * NewSpeed;
        }

        // Si la vitesse est ridicule, on force l'arrêt total (Snapping)
        if (CurrentSpeed < StopThreshold)
        {
            CurrentLinearVelocity = FVector::ZeroVector;
        }
    }

    // Gestion de la Rotation (Même logique : freinage fort si pas de force)
    if (FMath::IsNearlyZero(TotalTorqueZ))
    {
         // Freinage angulaire rapide
         float AngSpeed = FMath::Abs(CurrentAngularVelocityYaw);
         float AngDrop = (BrakingDeceleration * 0.1f) * DeltaTime; // 10% du freinage linéaire
         float NewAngSpeed = FMath::Max(0.f, AngSpeed - AngDrop);
         
         CurrentAngularVelocityYaw = (CurrentAngularVelocityYaw > 0 ? 1 : -1) * NewAngSpeed;
    }
    else
    {
        float AngularAcc = TotalTorqueZ / RotationalInertia;
        CurrentAngularVelocityYaw += AngularAcc * DeltaTime;
        float AngularDrag = 1.0f - (AngularDamping * DeltaTime);
        CurrentAngularVelocityYaw *= FMath::Clamp(AngularDrag, 0.f, 1.f);
    }

    // --- 5. Apply Movement ---
    if (!CurrentLinearVelocity.IsZero() || !FMath::IsNearlyZero(CurrentAngularVelocityYaw))
    {
        FVector DeltaLoc = CurrentLinearVelocity * DeltaTime;
        FRotator DeltaRot(0.f, CurrentAngularVelocityYaw * DeltaTime, 0.f);

        FHitResult Hit;
        RootCollision->MoveComponent(DeltaLoc, GetActorRotation() + DeltaRot, true, &Hit);

        // --- 6. Handle Collision (Slide) ---
        if (Hit.IsValidBlockingHit())
        {
            DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 10.f, FColor::Red, false, 0.1f);
            // Slide along wall
            FVector SlideVector = FVector::VectorPlaneProject(DeltaLoc, Hit.Normal);
            RootCollision->MoveComponent(SlideVector, GetActorRotation(), true);
            
            // Kill velocity perpendicular to wall (simple bounce/stop)
            CurrentLinearVelocity = FVector::VectorPlaneProject(CurrentLinearVelocity, Hit.Normal);
        }
    }
}

// ============================================================================
//                                INTERACTION
// ============================================================================

void ALitter::Interact_Implementation(AActor* Interactor)
{
    AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Interactor);
    if (Player)
    {
        Server_StartPushing(Player);
    }
}

void ALitter::StopInteract_Implementation(AActor* Interactor)
{
    AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Interactor);
    if (Player)
    {
        Server_EndPushing(Player);
    }
}

// ============================================================================
//                                SERVER RPCs
// ============================================================================

void ALitter::Server_StartPushing_Implementation(AAPlayerCharacter* Player)
{
    if (!Player) return;

    // 1. Find a free slot
    int32 FreeSlot = -1;
    // Simple heuristic: Try to find nearest slot, or just first available
    // Here we just pick first available for simplicity
    for (int i = 0; i < 4; i++)
    {
        if (!CarrySlots[i].IsValid())
        {
            FreeSlot = i;
            break;
        }
    }

    if (FreeSlot != -1)
    {
        AttachPlayerToSlot(Player, FreeSlot);

        // Init Data
        FPusherData& Data = ActivePushers.FindOrAdd(Player);
        Data.InputVector = FVector::ZeroVector;
        Data.LastUpdateTime = GetWorld()->GetTimeSeconds();

        // Notify Player State
        Player->Server_SetPushingState(this, true);
        Player->GetCharacterMovement()->DisableMovement();
    }
}

void ALitter::Server_EndPushing_Implementation(AAPlayerCharacter* Player)
{
    if (!Player) return;

    DetachPlayer(Player);
    ActivePushers.Remove(Player);

    Player->Server_SetPushingState(this, false);
    Player->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void ALitter::Server_UpdateInputs_Implementation(AAPlayerCharacter* Player, FVector WorldInputDirection)
{
    if (FPusherData* Data = ActivePushers.Find(Player))
    {
        Data->InputVector = WorldInputDirection.GetClampedToMaxSize(1.0f);
        Data->LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

// ============================================================================
//                                HELPER FUNCTIONS
// ============================================================================

void ALitter::AttachPlayerToSlot(AAPlayerCharacter* Player, int32 SlotIndex)
{
    if (!Player || !CarryPoints.IsValidIndex(SlotIndex)) return;

    CarrySlots[SlotIndex] = Player;

    // Snap player to the CarryPoint
    Player->AttachToComponent(CarryPoints[SlotIndex], FAttachmentTransformRules::SnapToTargetNotIncludingScale);

    // Disable Player Collision so they don't push the litter physically via Capsule
    if (UCapsuleComponent* Cap = Player->GetCapsuleComponent())
    {
        Cap->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Cap->SetCollisionResponseToAllChannels(ECR_Ignore);
        // Keep visibility/camera checks if needed
        Cap->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
    }
}

void ALitter::DetachPlayer(AAPlayerCharacter* Player)
{
    if (!Player) return;

    // Remove from slot array
    for (int i = 0; i < 4; i++)
    {
        if (CarrySlots[i] == Player)
        {
            CarrySlots[i] = nullptr;
            break;
        }
    }

    Player->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // Restore Collision
    if (UCapsuleComponent* Cap = Player->GetCapsuleComponent())
    {
        Cap->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Cap->SetCollisionProfileName(TEXT("Pawn"));
    }
}