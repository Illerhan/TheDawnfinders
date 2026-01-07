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
    ResolveWallPenetration(DeltaTime);

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

    // --- DETECTION COLLISION JOUEURS ---
    FHitResult PlayerHit;
    
    // On fait 3 itérations max pour glisser le long des murs (coin de mur, couloir étroit)
    int MaxIterations = 3;
    
    // Copie de travail
        FVector ProposedMove = DeltaLoc;
        FRotator ProposedRot = DeltaRot;

        while (MaxIterations > 0)
        {
            // Si pas de collision, on est bon, on sort de la boucle
            if (!CheckPlayerCollision(ProposedMove, ProposedRot, PlayerHit))
            {
                break;
            }

            // --- COLLISION DETECTEE ---
            
            // 1. Debug : Visualiser où ça tape
            DrawDebugPoint(GetWorld(), PlayerHit.ImpactPoint, 10.f, FColor::Red, false, 0.1f);
            DrawDebugLine(GetWorld(), PlayerHit.ImpactPoint, PlayerHit.ImpactPoint + PlayerHit.ImpactNormal * 50.f, FColor::Yellow, false, 0.1f);

            // 2. Logique de Glissement (Slide)
            // On projette le mouvement restant sur le plan du mur
            FVector RestOfMove = FVector::VectorPlaneProject(ProposedMove, PlayerHit.Normal);
            ProposedMove = RestOfMove;

            // 3. Logique de Rotation bloquée
            // Si on tourne ET qu'on tape, la rotation nous fait rentrer dans le mur.
            // On réduit drastiquement la rotation.
            ProposedRot *= 0.2f; // On garde 20% pour essayer de "glisser", sinon 0.f pour stop net.
            CurrentAngularVelocityYaw *= 0.5f; // Perte d'énergie cinétique

            // 4. On tue la vélocité linéaire vers le mur pour ne pas accumuler de pression
            CurrentLinearVelocity = FVector::VectorPlaneProject(CurrentLinearVelocity, PlayerHit.Normal);

            MaxIterations--;
        }

        // --- APPLICATION FINALE ---
        // Si après itérations on est toujours bloqué (ex: coincé entre 2 murs), on annule tout.
        if (MaxIterations == 0 && CheckPlayerCollision(ProposedMove, ProposedRot, PlayerHit))
        {
            // Fallback ultime : on ne bouge pas cette frame pour éviter le clip
            ProposedMove = FVector::ZeroVector;
            ProposedRot = FRotator::ZeroRotator;
        }

        // Déplacement réel du Brancard (Physics Root)
        FHitResult Hit;
        RootCollision->MoveComponent(ProposedMove, GetActorRotation() + ProposedRot, true, &Hit);

        // Gestion collision du brancard lui-même (si le bois tape)
        if (Hit.IsValidBlockingHit())
        {
            FVector Slide = FVector::VectorPlaneProject(ProposedMove, Hit.Normal);
            RootCollision->MoveComponent(Slide, GetActorRotation(), true);
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

    // Trouve un slot libre
    int32 FreeSlot = -1;
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
        
        // On désactive le mouvement du CharacterMovement (WASD standard)
        // Mais on garde la Capsule Collision active !
        Player->GetCharacterMovement()->DisableMovement();
    }
}

void ALitter::Server_EndPushing_Implementation(AAPlayerCharacter* Player)
{
    if (!Player) return;

    DetachPlayer(Player);
    ActivePushers.Remove(Player);

    Player->Server_SetPushingState(this, false);
    
    // On rend le contrôle standard au joueur
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

    // Attache le joueur au point de portage
    // SnapToTargetNotIncludingScale garde la taille, mais force Pos/Rot
    Player->AttachToComponent(CarryPoints[SlotIndex], FAttachmentTransformRules::SnapToTargetNotIncludingScale);

    // CHANGEMENT : On NE touche PLUS aux collisions du joueur ici.
    // Le joueur reste en "Pawn", collide avec le World, Static, Dynamic, etc.
    // Il ne collide pas avec la Litter car le constructeur de la Litter ignore ECC_Pawn.
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

    // Détache le joueur tout en gardant sa position actuelle dans le monde
    Player->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // CHANGEMENT : Pas besoin de restaurer les collisions car on ne les a jamais enlevées.
}

bool ALitter::CheckPlayerCollision(const FVector& DeltaLoc, const FRotator& DeltaRot, FHitResult& OutHit)
{
    // 1. Préparer la transformation future du Brancard
    FTransform CurrentLitterTrans = GetActorTransform();
    FTransform FutureLitterTrans = CurrentLitterTrans;
    
    // Appliquer la rotation et la translation prévues
    FQuat RotQuat = DeltaRot.Quaternion();
    FutureLitterTrans.SetRotation(RotQuat * CurrentLitterTrans.GetRotation());
    FutureLitterTrans.AddToTranslation(DeltaLoc);

    for (int i = 0; i < 4; i++)
    {
        AAPlayerCharacter* Player = CarrySlots[i].Get();
        if (!Player) continue;

        UCapsuleComponent* PlayerCapsule = Player->GetCapsuleComponent();
        if (!PlayerCapsule) continue;

        // --- CORRECTIF 1 : Offset Réel ---
        // On ne suppose pas que le joueur est pile sur le CarryPoint.
        // On calcule où il est VRAIMENT par rapport au centre du brancard maintenant.
        FVector PlayerLocationWorld = Player->GetActorLocation();
        FVector PlayerRelativeLoc = CurrentLitterTrans.InverseTransformPosition(PlayerLocationWorld);

        // --- CALCUL DU MOUVEMENT ---
        FVector StartPos = PlayerLocationWorld;
        // La position de fin est : La future Transform du Litter + L'offset relatif du joueur
        FVector EndPos = FutureLitterTrans.TransformPosition(PlayerRelativeLoc);

        // --- CORRECTIF 2 : Ignorer le sol (Shrink Height) ---
        // Si la capsule fait 88cm de haut, on teste avec 80cm.
        // Ça évite que le bas de la capsule ne frotte le sol et bloque la rotation.
        float Radius = PlayerCapsule->GetScaledCapsuleRadius() * 1.1f; // +10% de marge en largeur
        float HalfHeight = PlayerCapsule->GetScaledCapsuleHalfHeight() - 5.0f; // -5 unités en hauteur (Lift feet)

        if (HalfHeight <= 0) HalfHeight = 10.f; // Sécurité

        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(Player);

        // Debug visuel pour voir le tunnel de collision (Optionnel)
        // DrawDebugCapsule(GetWorld(), (StartPos + EndPos) / 2, HalfHeight, Radius, FQuat::Identity, FColor::Cyan, false, -1);

        bool bHit = GetWorld()->SweepSingleByChannel(
            OutHit,
            StartPos,
            EndPos,
            FQuat::Identity,
            ECC_WorldStatic,
            FCollisionShape::MakeCapsule(Radius, HalfHeight),
            Params
        );

        if (bHit)
        {
            // --- CORRECTIF CRITIQUE POUR NE PAS RESTER BLOQUÉ ---
            
            // Cas 1 : On commence DÉJÀ dans le mur (bStartPenetrating)
            if (OutHit.bStartPenetrating)
            {
                // Si on est déjà dedans, on autorise le mouvement SEULEMENT s'il nous éloigne.
                // On calcule le produit scalaire entre la direction du mouvement et la normale de sortie.
                // Note : En bStartPenetrating, ImpactNormal est souvent inversée ou non fiable, 
                // mais Normal représente la direction pour sortir.
                
                float Dot = FVector::DotProduct(DeltaLoc.GetSafeNormal(), OutHit.Normal);
                
                if (Dot > 0.1f) 
                {
                    // On va dans le sens de la normale (vers le vide), donc ON LAISSE PASSER !
                    continue; 
                }
                else
                {
                    // On s'enfonce encore plus, on bloque.
                    return true;
                }
            }
            
            // Cas 2 : C'est un sol (comme vu avant)
            if (OutHit.ImpactNormal.Z > 0.7f) continue;

            return true;
        }
        if (bHit)
        {
            // Vérification ultime : Si on touche quelque chose qui est "sous" nous (le sol), on ignore.
            // Si la normale d'impact pointe vers le haut (Z > 0.7), c'est un sol/pente.
            if (OutHit.ImpactNormal.Z > 0.7f) 
            {
                continue; // C'est juste le sol, on continue
            }

            return true; // C'est un mur !
        }
    }

    return false;
}

void ALitter::ResolveWallPenetration(float DeltaTime)
{
    FVector TotalDepenetration = FVector::ZeroVector;
    int StuckCount = 0;

    for (int i = 0; i < 4; i++)
    {
        AAPlayerCharacter* Player = CarrySlots[i].Get();
        if (!Player) continue;

        UCapsuleComponent* Capsule = Player->GetCapsuleComponent();
        if (!Capsule) continue;

        // On cherche les overlaps réels (pas des sweeps, mais "suis-je dedans ?")
        TArray<FOverlapResult> Overlaps;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(Player);

        // On utilise une sphère légèrement plus petite que la capsule pour être sûr
        // de ne réagir que si on est VRAIMENT dedans.
        float TestRadius = Capsule->GetScaledCapsuleRadius() * 0.9f; 

        bool bOverlap = GetWorld()->OverlapMultiByChannel(
            Overlaps,
            Player->GetActorLocation(),
            FQuat::Identity,
            ECC_WorldStatic, // Ton canal de mur
            FCollisionShape::MakeSphere(TestRadius),
            Params
        );

        if (bOverlap)
        {
            for (const FOverlapResult& Res : Overlaps)
            {
                // Astuce Mathématique :
                // Si on est dans un mur, le moyen le plus simple de sortir est d'aller
                // vers le centre du Brancard (qui est supposé être dans le vide).
                // C'est une approximation robuste pour ce type de gameplay.
                
                FVector DirectionToCenter = GetActorLocation() - Player->GetActorLocation();
                DirectionToCenter.Z = 0.f; // On ne veut pas voler
                
                // Force de répulsion violente
                TotalDepenetration += DirectionToCenter.GetSafeNormal() * 50.0f; // 50cm de poussée
                StuckCount++;
                break; // Un mur suffit pour ce joueur
            }
        }
    }

    // Si des joueurs sont coincés, on applique une correction immédiate à la position
    if (StuckCount > 0)
    {
        // On déplace le Root sans vérifier la collision (Teleport) pour sortir de force
        // On utilise VInterp pour ne pas que ça "téléporte" trop violemment visuellement
        FVector Nudge = TotalDepenetration * DeltaTime * 5.0f; // Vitesse d'éjection
        RootCollision->AddWorldOffset(Nudge, false); // false = Teleport (ignore collision)
        
        // On tue la vélocité pour arrêter de foncer dans le mur
        CurrentLinearVelocity *= 0.1f;
        CurrentAngularVelocityYaw = 0.f;
    }
}