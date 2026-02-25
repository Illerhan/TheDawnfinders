#include "Litter.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Interfaces/IPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SoundManager.h"
#include "GameFramework/UDoorRegistry.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/UWorldInteractibleWidget.h"

ALitter::ALitter()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    // 1. Root Collision (Physics box)
    RootCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RootCollision"));
    RootComponent = RootCollision;
    RootCollision->SetCollisionProfileName(TEXT("NoCollision"));

    RootCollision->SetMobility(EComponentMobility::Movable); 
    RootCollision->SetSimulatePhysics(false);
    
    PhysicsCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("PhysicsCapsule"));
    PhysicsCapsule->SetupAttachment(RootCollision);
    PhysicsCapsule->SetCapsuleSize(100.f, 55.f);
    PhysicsCapsule->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    PhysicsCapsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    PhysicsCapsule->SetRelativeLocation(FVector::ZeroVector);
    
    RootCollision->SetBoxExtent(FVector(80.f, 120.f, 50.f)); // Adjust to palanquin size
    RootCollision->SetCollisionProfileName(TEXT("BlockAllDynamic")); 
    // Ensure it blocks world/physics but doesn't get stuck on pawns easily
    RootCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    
    BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
    BoxCollider->SetupAttachment(RootCollision);

    PointLight = CreateDefaultSubobject<UPointLightComponent>(FName("Light"));
    PointLight->SetupAttachment(RootComponent);

    ProtectionZone = CreateDefaultSubobject<USphereComponent>(FName("ProtectionZone"));
    ProtectionZone->SetupAttachment(RootComponent);

    FogOfWarLightOn = CreateDefaultSubobject<USphereComponent>(FName("FogOfWarLightOn"));
    FogOfWarLightOn->SetupAttachment(RootComponent);

    FogOfWarLightOff = CreateDefaultSubobject<USphereComponent>(FName("FogOfWarLightOff"));
    FogOfWarLightOff->SetupAttachment(RootComponent);

    LightMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("LanternMesh"));
    LightMesh->SetupAttachment(RootComponent);

    // 2. Visual Mesh
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionProfileName(TEXT("NoCollision")); // Visual only

    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

    LightComponent = CreateDefaultSubobject<UPlayerLightComponent>(TEXT("AC_LightComponent"));
    
    // *** FIX: Rotate mesh -90 degrees if it faces the wrong way ***
    MeshComponent->SetRelativeRotation(FRotator(0.f, -90.f, 0.f)); 

    // 3. Interaction Trigger
    if (BoxCollider)
    {
        BoxCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    InteractibleWidgetComponent->SetupAttachment(RootCollision);

    // --- 1. Zone AVANT (Portage) ---
    FrontTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("FrontTrigger"));
    FrontTrigger->SetupAttachment(RootComponent);
    FrontTrigger->SetBoxExtent(FVector(60.f, 100.f, 60.f)); 
    FrontTrigger->SetRelativeLocation(FVector(130.f, 0.f, 0.f)); // Décalé vers l'avant (+X)
    FrontTrigger->SetCollisionProfileName(TEXT("Trigger"));

    // --- 2. Zone ARRIÈRE (Portage) ---
    BackTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("BackTrigger"));
    BackTrigger->SetupAttachment(RootComponent);
    BackTrigger->SetBoxExtent(FVector(60.f, 100.f, 60.f));
    BackTrigger->SetRelativeLocation(FVector(-130.f, 0.f, 0.f)); // Décalé vers l'arrière (-X)
    BackTrigger->SetCollisionProfileName(TEXT("Trigger"));

    // --- 3. Zone CENTRALE 1 (Inventaire) ---
    InventoryTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InventoryTrigger"));
    InventoryTrigger->SetupAttachment(RootComponent);
    InventoryTrigger->SetBoxExtent(FVector(50.f, 80.f, 60.f));
    InventoryTrigger->SetRelativeLocation(FVector(0.f, 0.f, 0.f)); // Au centre
    InventoryTrigger->SetCollisionProfileName(TEXT("Trigger"));
    
    // --- 4. Zone CENTRALE 2 (Light) ---
    LightTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("LightTrigger"));
    LightTrigger->SetupAttachment(RootComponent);
    LightTrigger->SetBoxExtent(FVector(50.f, 80.f, 60.f));
    LightTrigger->SetRelativeLocation(FVector(0.f, 0.f, 0.f)); // Au centre
    LightTrigger->SetCollisionProfileName(TEXT("Trigger"));

    // 4. Create Carry Points (Corners)
    CarryPoints.SetNum(2);
    for (int i = 0; i < 2; i++)
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

    CarrySlots.SetNum(2);
    CurrentLinearVelocity = FVector::ZeroVector;
    CurrentAngularVelocityYaw = 0.f;
    
   // RootCollision->SetIsReplicated(true);
    
    if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(RootComponent))
    {
        Prim->SetIsReplicated(true);
    }

    RootCollision->SetEnableGravity(false);
    RootCollision->SetSimulatePhysics(false);
    
}

void ALitter::ClampToGround()
{
    float HalfHeight = PhysicsCapsule->GetScaledCapsuleHalfHeight() + 10;
    
    FVector Start = GetActorLocation() + FVector(0,0,50);
    FVector End   = GetActorLocation() - FVector(0,0,500);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
    {
        FVector Loc = GetActorLocation();
        Loc.Z = Hit.ImpactPoint.Z + HalfHeight;
        SetActorLocation(Loc, false, nullptr, ETeleportType::TeleportPhysics);
    }
}

void ALitter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ALitter, CurrentLinearVelocity);
    DOREPLIFETIME(ALitter, CurrentAngularVelocityYaw);
    DOREPLIFETIME(ALitter, ServerTransform);
}

void ALitter::BeginPlay()
{
    PrimaryActorTick.bStartWithTickEnabled = false;

    FTimerHandle Handle;
    GetWorldTimerManager().SetTimer(Handle, [this]()
    {
        SetActorTickEnabled(true);
    }, 0.0f, false);
    
    Super::BeginPlay();
    if (FrontTrigger)
    {
        FrontTrigger->OnComponentBeginOverlap.AddDynamic(this, &ALitter::OnZoneOverlapBegin);
        FrontTrigger->OnComponentEndOverlap.AddDynamic(this, &ALitter::OnZoneOverlapEnd);
    }
    if (BackTrigger)
    {
        BackTrigger->OnComponentBeginOverlap.AddDynamic(this, &ALitter::OnZoneOverlapBegin);
        BackTrigger->OnComponentEndOverlap.AddDynamic(this, &ALitter::OnZoneOverlapEnd);
    }
    if (InventoryTrigger)
    {
        InventoryTrigger->OnComponentBeginOverlap.AddDynamic(this, &ALitter::OnZoneOverlapBegin);
        InventoryTrigger->OnComponentEndOverlap.AddDynamic(this, &ALitter::OnZoneOverlapEnd);
    }
    
    if (LightTrigger)
    {
        LightTrigger->OnComponentBeginOverlap.AddDynamic(this, &ALitter::OnZoneOverlapBegin);
        LightTrigger->OnComponentEndOverlap.AddDynamic(this, &ALitter::OnZoneOverlapEnd);
    }
    AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASoundManager::StaticClass());
    SoundManagerInstance = Cast<ASoundManager>(FoundActor);
    if (!SoundManagerInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("ALitter: Attention, aucun ASoundManager trouvé dans le niveau !"));
    }

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
        ClampToGround();
    }
    else
    {
        SmoothClientTransform(DeltaTime);
    }
}

void ALitter::SmoothClientTransform(float DeltaTime)
{
    FTransform Current = GetActorTransform();

    FVector NewLoc = FMath::VInterpTo(
        Current.GetLocation(),
        ServerTransform.GetLocation(),
        DeltaTime,
        10.0f
    );

    FRotator NewRot = FMath::RInterpTo(
        Current.Rotator(),
        ServerTransform.Rotator(),
        DeltaTime,
        10.0f
    );

    SetActorLocationAndRotation(NewLoc, NewRot, false, nullptr, ETeleportType::None);
}

void ALitter::StartExtraction_Implementation()
{
    LightComponent->Server_TurnLightOn();
    LightComponent->FuelRemaining = LightComponent->MaxFuel;
    LightComponent->FuelConsumption = 0;
    if (!HasAuthority()) return;

    UE_LOG(LogTemp, Warning, TEXT("[EXTRACTION] Mechanism activated! Opening all extraction doors..."));

    // Récupérer le registre et ouvrir TOUTES les portes
    UUDoorRegistry* Registry = GetGameInstance()->GetSubsystem<UUDoorRegistry>();
    if (Registry)
    {
        Registry->OpenAllExtractionDoors();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[EXTRACTION] Failed to get DoorRegistry!"));
    }
    
    
}

void ALitter::ResolvePhysics(float DeltaTime)
{
    const float TimeNow = GetWorld()->GetTimeSeconds();
    ResolveWallPenetration(DeltaTime);

    // --- 1. Cleanup Stale Inputs ---
    TArray<AAPlayerCharacter*> ToRemove;
    InputTimeout = 0.1f;

    // --- 2. Calculate Forces & Torques ---
    FVector TotalForce = FVector::ZeroVector;
    float TotalTorqueZ = 0.0f;
    int32 ActiveCount = 0;

    for (int i = 0; i < 2; i++)
    {
        if (SoundManagerInstance && ActiveCount == 1)
            SoundManagerInstance->MultiPlaySound(SoloSound,GetActorLocation(),800,1000,true);
        if (ActiveCount == 1 && InventoryComponent->CurrentWeight >= InventoryComponent->SoloMaxWeight)
        {
            Mass += InventoryComponent->CurrentWeight;
        }else if (ActiveCount == 2 && InventoryComponent->CurrentWeight>= InventoryComponent->DuoMaxWeight)
        {
            Mass += InventoryComponent->CurrentWeight;
        }
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

    Mass = BaseMass; 

    // 2. Vérifier si on est en surcharge
    bool bIsOverloaded = false;

    if (ActiveCount == 1 && InventoryComponent->CurrentWeight >= InventoryComponent->SoloMaxWeight)
    {
        // On est tout seul et c'est trop lourd
        Mass += InventoryComponent->CurrentWeight; // On ajoute le poids réel à la masse physique
        bIsOverloaded = true;
    }
    else if (ActiveCount == 2 && InventoryComponent->CurrentWeight >= InventoryComponent->DuoMaxWeight)
    {
        // On est deux mais c'est quand même trop lourd
        Mass += InventoryComponent->CurrentWeight;
        bIsOverloaded = true;
    }

    // 3. Gestion du Son (Avec un délai de 2 secondes entre chaque cri/bruit)
    if (ActiveCount == 1 &&  SoundManagerInstance)
    {
        // On vérifie si on pousse (inutile de crier si on est à l'arrêt)
        if (!CurrentLinearVelocity.IsZero() || !TotalForce.IsZero())
        {
            // Anti-Spam : On joue le son seulement si 2 secondes sont passées
            if (TimeNow - LastStrainSoundTime > 2.0f)
            {
                // J'assume que SoloSound est le bruit d'effort
                SoundManagerInstance->MultiPlaySound(SoloSound, GetActorLocation(), 0.01f, 100.f, true);
                LastStrainSoundTime = TimeNow;
            }
        }
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
        // Si après itérations on est toujours bloqué (ex: coincé entre deux murs), on annule tout.
        if (MaxIterations == 0 && CheckPlayerCollision(ProposedMove, ProposedRot, PlayerHit))
        {
            // Fallback ultime : on ne bouge pas cette frame pour éviter le clip
            ProposedMove = FVector::ZeroVector;
            ProposedRot = FRotator::ZeroRotator;
        }

        // Déplacement réel du Brancard (Physics Root)
        FHitResult Hit;
        AddActorWorldOffset(ProposedMove, true, &Hit);
        SetActorRotation(GetActorRotation() + ProposedRot);

        if (Hit.IsValidBlockingHit())
        {
            FVector Slide = FVector::VectorPlaneProject(ProposedMove, Hit.Normal);
            AddActorWorldOffset(Slide, true);
            CurrentLinearVelocity = FVector::VectorPlaneProject(CurrentLinearVelocity, Hit.Normal);
        }
    }
    if (HasAuthority())
    {
        ServerTransform = GetActorTransform();
    }
}


// ============================================================================
//                                INTERACTION
// ============================================================================

void ALitter::Interact_Implementation(AActor* Interactor)
{

    AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Interactor);
    if (!Player) return;

    // Vérifie dans quelle zone se trouve le joueur
    bool bInFront = FrontTrigger->IsOverlappingActor(Player);
    bool bInBack  = BackTrigger->IsOverlappingActor(Player);
    bool bInventory= InventoryTrigger->IsOverlappingActor(Player);
    bool bInLight = LightTrigger->IsOverlappingActor(Player);

    // CAS 1 : Light (Zone Centrale)
    if (bInLight)
    {
        if (LightComponent && !Player->bIsCarrying  && !bIsExtracting)
        {
            if (!HasAuthority()) return;
            IPlayerInterface::Execute_Server_AskOwnershipPermission(Interactor, this, Player->GetController());
            LightComponent->FuelUpdate();
            LightComponent->Server_TurnLightOn();
            return;
        }
        if (bIsExtracting && !Player->bIsCarrying)
        {
            IPlayerInterface::Execute_Server_AskOwnershipPermission(Interactor, this, Player->GetController());
            StartExtraction();
            return;
        }
    }
    if (Player->bIsCarrying) return;
    
    // CAS 2 : Inventaire (Zone Centrale)
    if (bInventory)
    {
        if (InventoryComponent && !Player->bIsCarrying && !bPlayerIsUsing)
        {
            if (!HasAuthority()) return;

            bPlayerIsUsing = true;

            Player->Client_OpenInteractionUI(EInteractionUI::LitterInventory, this);
            IPlayerInterface::Execute_Server_AskOwnershipPermission(Interactor, this, Player->GetController());

            return;
        }

        if (bPlayerIsUsing) {
            return;
        }
    }

    // CAS 3 : Portage (Zone Avant OU Arrière)
    int32 TargetSlot = -1;

    if (bInFront)
    {
        TargetSlot = 0; // 0 est défini comme l'avant
    }
    else if (bInBack)
    {
        TargetSlot = 1; // 1 est défini comme l'arrière
    }

    // Si on a trouvé un slot valide, on demande au serveur
    if (TargetSlot != -1)
    {
        Server_StartPushing(Player, TargetSlot);
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

void ALitter::Server_StartPushing_Implementation(AAPlayerCharacter* Player, int32 SlotIndex)
{
    if (!Player) return;

    // 1. Vérification de sécurité : L'index est-il valide (0 ou 1) ?
    if (!CarryPoints.IsValidIndex(SlotIndex)) return;

    // 2. Vérification : Le slot est-il déjà occupé ?
    if (CarrySlots[SlotIndex].IsValid())
    {
        // Optionnel : Feedback visuel "Place occupée" ou son
        UE_LOG(LogTemp, Warning, TEXT("Ce slot est déjà pris !"));
        return;
    }

    // 3. Attachement
    AttachPlayerToSlot(Player, SlotIndex);

    // Init Data
    FPusherData& Data = ActivePushers.FindOrAdd(Player);
    Data.InputVector = FVector::ZeroVector;
    Data.LastUpdateTime = GetWorld()->GetTimeSeconds();

    // Notify Player State
    Player->Server_SetPushingState(this, true);
    
    // Désactiver mouvement joueur
    Player->GetCharacterMovement()->DisableMovement();
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

        if (Data->InputVector.SizeSquared() < 0.001f)
        {
            Data->InputVector = FVector::ZeroVector;
        }else
        {
            Data->InputVector = WorldInputDirection.GetClampedToMaxSize(1.0f);
        }

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
    
    Player->AttachToComponent(CarryPoints[SlotIndex], FAttachmentTransformRules::SnapToTargetNotIncludingScale);

}

void ALitter::DetachPlayer(AAPlayerCharacter* Player)
{
    if (!Player) return;

    // Remove from slot array
    for (int i = 0; i < 2; i++)
    {
        if (CarrySlots[i] == Player)
        {
            CarrySlots[i] = nullptr;
            break;
        }
    }
    
    Player->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

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

    for (int i = 0; i < 2; i++)
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
                float Dot = FVector::DotProduct(DeltaLoc.GetSafeNormal(), OutHit.Normal);
                
                if (Dot > 0.1f) 
                {
                    continue; 
                }
                else
                {
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

void ALitter::ClosePalanquinInventory()
{
    if (HasAuthority()) 
    {
        Server_ClosePalanquinInventory_Implementation();
    }
    else 
    {
        Server_ClosePalanquinInventory();
    }
}

void ALitter::Server_ClosePalanquinInventory_Implementation()
{
    bPlayerIsUsing = false;
}

void ALitter::ResolveWallPenetration(float DeltaTime)
{
    FVector TotalDepenetration = FVector::ZeroVector;
    int StuckCount = 0;

    for (int i = 0; i < 2; i++)
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
        FVector Nudge = TotalDepenetration * DeltaTime * 5.0f; // Vitesse d'éjection
        RootCollision->AddWorldOffset(Nudge, false); // false = Teleport (ignore collision)
        
        // On tue la vélocité pour arrêter de foncer dans le mur
        CurrentLinearVelocity *= 0.1f;
        CurrentAngularVelocityYaw = 0.f;
    }
}

void ALitter::OnZoneOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor);
    if (Player && Player->IsLocallyControlled())
    {
        // Ajoute l'interactible au joueur pour qu'il puisse appuyer sur E
        Player->AddInteractibleAtRange_Implementation(this);

        // Mise à jour du texte selon la zone touchée
        if (InteractibleWidget)
        {
            if (OverlappedComp == InventoryTrigger)
            {
                    InteractibleWidget->DisplayText("Ouvrir Inventaire",InputIcon);
            }
            else  if (OverlappedComp == LightTrigger)
            {
                if (Player->CurrentState == EPlayerState::Carrying)
                    InteractibleWidget->DisplayText("Remplir le palanquin",InputIcon);
                else if (IsIsExtracting())  InteractibleWidget->DisplayText("Demarrer l'extraction",InputIcon);
                else InteractibleWidget->DisplayText("Activer la lumiere",InputIcon);
            }
            // Front ou Back
            else
            {
                InteractibleWidget->DisplayText("Porter",InputIcon);
            }
        }
    }
}

void ALitter::OnZoneOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!OtherActor || OtherActor == this) return;

    AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor);
    InventoryComponent->CloseInventory();
    // On ne fait rien si ce n'est pas le joueur local
    if (!Player || !Player->IsLocallyControlled()) return;

    // --- SECURITE CRITIQUE : Vérifier que nos composants existent ---
    // Si pour une raison quelconque le composant est détruit ou pas encore prêt, on arrête.
    if (!FrontTrigger || !BackTrigger || !InventoryTrigger || !LightTrigger) return;

    // 2. Vérification : Est-ce qu'on touche encore UNE AUTRE zone ?
    bool bStillOverlapping = false;

    // On utilise "&&" pour vérifier que le pointeur existe AVANT de l'utiliser
    if (FrontTrigger && FrontTrigger->IsOverlappingActor(Player)) bStillOverlapping = true;
    if (BackTrigger && BackTrigger->IsOverlappingActor(Player)) bStillOverlapping = true;
    if (InventoryTrigger && InventoryTrigger->IsOverlappingActor(Player)) bStillOverlapping = true;
    if (LightTrigger && LightTrigger->IsOverlappingActor(Player)) bStillOverlapping = true;

    // 3. Logique d'affichage
    if (!bStillOverlapping)
    {
        // CAS A : On a tout quitté
        Player->RemoveInteractibleAtRange_Implementation(this);
        
        if (InteractibleWidget)
        {
            InteractibleWidget->HideText();
        }
    }
    
}