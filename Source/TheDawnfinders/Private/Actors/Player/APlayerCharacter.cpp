// Copyright ...
#include "Actors/Player/APlayerCharacter.h"

#include "Actors/Interactibles/Litter.h"
#include "Actors/Interactibles/Lock.h"
#include "Actors/Interactibles/ZiplineInteractible.h"

#include "Components/UStaminaComponent.h"
#include "Components/UHealthComponent.h"
#include "Components/UItemComponent.h"
#include "Components/UInteractionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/UWorldProgressBar.h"


AAPlayerCharacter::AAPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Réplication Actor + mouvement (utile pour ACharacter)
    bReplicates = true;
    SetReplicateMovement(true);
    GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;
    GetCharacterMovement()->bNetworkSmoothingComplete = false;
    GetCharacterMovement()->NetworkSimulatedSmoothLocationTime = 0.100f;
    GetCharacterMovement()->NetworkSimulatedSmoothRotationTime = 0.033f;
    GetCharacterMovement()->ListenServerNetworkSimulatedSmoothLocationTime = 0.040f;
    GetCharacterMovement()->ListenServerNetworkSimulatedSmoothRotationTime = 0.033f;
    
    // Interpolation plus agressive
    GetCharacterMovement()->NetworkMaxSmoothUpdateDistance = 128.0f;
    GetCharacterMovement()->NetworkNoSmoothUpdateDistance = 256.0f;
    
    // Augmenter la fréquence pour les mouvements critiques
    SetNetUpdateFrequency(100.0f);
    SetMinNetUpdateFrequency(50.0f);

    // Components 
    InventoryComponent   = CreateDefaultSubobject<UInventoryComponent>(TEXT("AC_Inventory"));
    StaminaComponent     = CreateDefaultSubobject<UStaminaComponent>(TEXT("AC_Stamina"));
    HealthComponent      = CreateDefaultSubobject<UHealthComponent>(TEXT("AC_Health"));
    ItemComponent        = CreateDefaultSubobject<UItemComponent>(TEXT("AC_ItemUse"));
    InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("AC_Interaction"));
    ProgressBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("ProgressBarComponent"));
    ProgressBarComponent->SetupAttachment(GetMesh());
    WeaponMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    WeaponMeshComponent->SetupAttachment(GetMesh());
    ThrowablePreviewMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ThrowablePreviewMeshComponent"));
    ThrowablePreviewMeshComponent->SetupAttachment(GetMesh());
    WeaponCollisionPosRef = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponCollisionPosRef"));
    WeaponCollisionPosRef->SetupAttachment(GetMesh());

    // ---------- ROTATION PAR DÉFAUT ----------

    bUseControllerRotationYaw = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;

    // Orientation sur déplacement
     bUseControllerRotationYaw = false;
     GetCharacterMovement()->bOrientRotationToMovement = true;
    if (PlayerConfig == nullptr)
    {
        PlayerConfig = CreateDefaultSubobject<UPlayerData>(TEXT("PlayerConfig"));
    }
    if (PlayerConfig)
     GetCharacterMovement()->RotationRate = FRotator(0.f, PlayerConfig->RotationRate, 0.f);
}


void AAPlayerCharacter::ApplyPlayerData()
{
    GetCharacterMovement()->MaxWalkSpeed = PlayerConfig->WalkSpeed;
    TargetMaxSpeed = PlayerConfig->WalkSpeed;
    HealthComponent->InitialiseComponent(
        PlayerConfig->MaxHealth,
        PlayerConfig->MinMaxHP,
        PlayerConfig->MinReviveHP,
        PlayerConfig->InjureDecreaseSpeed,
        PlayerConfig->CurseRatio
        );
    
    LightComponent->InitialiseComponent(
        PlayerConfig->FuelConsumption, PlayerConfig->MaxFuel);
    
    StaminaComponent->InitialiseComponent(PlayerConfig->MaxStamina,
    PlayerConfig->ReloadSpeed,
    PlayerConfig->ReloadDelay,
    PlayerConfig->StaminaConsumptionRun,
    PlayerConfig->StaminaConsumptionDodge);
    
}

void AAPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (PlayerConfig)
    ApplyPlayerData();

    ProgressBarWidget = Cast<UWorldProgressBar>(ProgressBarComponent->GetWidget());


    ItemComponent->OnThrowPreviewDisplay.AddUniqueDynamic(this, &AAPlayerCharacter::DisplayThrowPreview);
    ItemComponent->OnThrowHidePreview.AddUniqueDynamic(this, &AAPlayerCharacter::HideThrowPreview);
    
    
    UE_LOG(LogTemp, Display, TEXT("%d"), ProgressBarWidget != nullptr);
}


void AAPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAutoLockIsActive) {
        ActualiseAutoLock();
    }
    if (IsLocallyControlled() && bIsCarrying && CurrentPushedObject)
    {
        UpdatePushingMovement(DeltaTime);
    }
    else if (HasAuthority() && bIsCarrying && CurrentPushedObject)
        {
            UpdatePushingMovement(DeltaTime);
        }

    if (IsLocallyControlled() && CurrentPushedObject)
    {
        static float LastSendTime = 0.f;
        FVector Input = GetLastMovementInputVector();
        if (GetWorld()->GetTimeSeconds() - LastSendTime > 0.05f) // 20 Hz
        {
            Server_SendPushInput(CurrentPushedObject, Input);
            LastSendTime = GetWorld()->GetTimeSeconds();
        }
    }
    
    if (GetLocalRole() == ROLE_SimulatedProxy)
    {
        return;
    }
    
    switch (CurrentState) {
    case EPlayerState::Dodging :
        ActualiseDodge(DeltaTime);
        break;

    case EPlayerState::Blocking :
        StaminaComponent->UseStamina(PlayerConfig->BlockStaminaDrainPerSecond * DeltaTime);
        if (!StaminaComponent->VerifyHasStamina()) ItemComponent->StopSecondaryAction();
        break;
    }
}


void AAPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAPlayerCharacter, CurrentState);
    DOREPLIFETIME(AAPlayerCharacter, PlayerSpeed);
}


#pragma region Interface Functions

void AAPlayerCharacter::AddInteractibleAtRange_Implementation(AActor* Interactible)
{
    InteractionComponent->AddInteractible(Interactible);
}

void AAPlayerCharacter::RemoveInteractibleAtRange_Implementation(AActor* Interactible)
{
    InteractionComponent->RemoveInteractible(Interactible);
}

void AAPlayerCharacter::DoCameraShake_Implementation(float Intensity)
{
}

void AAPlayerCharacter::DoDamagePostProcess_Implementation(float Duration)
{
}

void AAPlayerCharacter::ShowProgress_Implementation(float CurrentValue)
{
    ProgressBarWidget->ActualiseProgress(CurrentValue);
}

void AAPlayerCharacter::HideProgress_Implementation()
{
    ProgressBarWidget->Hide();
}

void AAPlayerCharacter::SetEquippedMesh_Implementation(UStaticMesh* NewMesh)
{
    WeaponMeshComponent->SetStaticMesh(NewMesh);
}

UItemData* AAPlayerCharacter::GetEquippedItem_Implementation()
{
    return ItemComponent->GetEquippedItem();
}

EPlayerState AAPlayerCharacter::GetCurrentPlayerState_Implementation()
{
    return CurrentState;
}

void AAPlayerCharacter::SetCurrentPlayerState_Implementation(EPlayerState NewState)
{
    CurrentState = NewState;
}

void AAPlayerCharacter::PlayAttackMontage_Implementation(UAnimMontage* AttackMontage, float Speed)
{
    PlayMontage(AttackMontage, Speed);
}

void AAPlayerCharacter::AddProtectionZone_Implementation()
{
    HealthComponent->AddProtectionZone();
}

void AAPlayerCharacter::RemoveProtectionZone_Implementation()
{
    HealthComponent->RemoveProtectionZone();
}

float AAPlayerCharacter::GetSoundAlertness_Implementation(FName SoundTag)
{
    if (SoundTag == "Run") {
        return PlayerConfig->RunSoundAlertness * GetWorld()->GetDeltaSeconds();
    }
    else if (SoundTag == "Dodge") {
        return PlayerConfig->DodgeSoundAlertness;
    }
    else if (SoundTag == "Attack") {
        return PlayerConfig->AttackSoundAlertness;
    }

    return 1.0f;
}

void AAPlayerCharacter::ReceiveDamage_Implementation(float quantity, AActor* Origin)
{
    if (!GetController()) return;
    if (!GetController()->IsLocalController()) return;

    HealthComponent->TakeDamage(quantity);
}

#pragma endregion


void AAPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // Laisse tes bindings BP gérer le Turn (AddControllerYawInput) et le Move.
}


#pragma region Movement / Run

void AAPlayerCharacter::SetPlayerSpeed(float NewSpeed)
{
    if (HasAuthority())
    {
        // Sur le serveur, on change directement
        PlayerSpeed = NewSpeed;
        TargetMaxSpeed = NewSpeed;
        GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
        
        UE_LOG(LogTemp, Log, TEXT("[SERVER] %s speed set to %.0f"), *GetName(), NewSpeed);
    }
    else
    {
        // Sur le client, on demande au serveur
        ServerSetPlayerSpeed(NewSpeed);

        PlayerSpeed = NewSpeed;
        TargetMaxSpeed = NewSpeed;
        
        // Prédiction locale optionnelle (pour réactivité)
        GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
    }
}

bool AAPlayerCharacter::ServerSetPlayerSpeed_Validate(float NewSpeed)
{
    // Validation : empêcher les valeurs absurdes
    return NewSpeed >= 0.0f && NewSpeed <= 2000.0f;
}


void AAPlayerCharacter::ServerSetPlayerSpeed_Implementation(float NewSpeed)
{
    PlayerSpeed = NewSpeed;
    TargetMaxSpeed = NewSpeed;
    GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}


void AAPlayerCharacter::OnRep_PlayerSpeed()
{
    // Appliqué automatiquement sur tous les clients quand PlayerSpeed change
    GetCharacterMovement()->MaxWalkSpeed = PlayerSpeed;
}



void AAPlayerCharacter::MoveCharacter(FVector2D Input)
{
    if (CurrentState == EPlayerState::Dodging || CurrentState == EPlayerState::Immobilized)
        return;

    if (CurrentState == EPlayerState::Blocking) {
        AddMovementInput(FVector(0, 0, 0), 1.0f, false);
        return;
    }

    CurrentPlayerInput = FVector(-Input.X, Input.Y, 0);

    if (CurrentPlayerInput.Length() > 0.5f) {
        PreviousPlayerInput = CurrentPlayerInput;
    }
    else {
        AddMovementInput(FVector(0, 0, 0), 1.0f, true);
        return;
    }

    FVector FinalVector = FVector(-Input.X, Input.Y, 0);
    FinalVector.Normalize();

    FRotator Rotation(0.0f, 30.0f - 90.0f, 0.0f);
    FinalVector = Rotation.RotateVector(FinalVector);

    AddMovementInput(FinalVector, 1.0f, true);
}


void AAPlayerCharacter::ManageRun(bool Input)
{
    if (!HasAuthority())
    {
        ServerManageRun(Input);
        return;
    }

    if (Input)
    {
        SetPlayerSpeed(PlayerConfig->RunSpeed);

        CurrentState = EPlayerState::Running;
        // Ajuster friction si besoin
        //GetCharacterMovement()->BrakingFrictionFactor = 2.f;
    }
    else
    {
        SetPlayerSpeed(PlayerConfig->WalkSpeed);

        if (CurrentState == EPlayerState::Running)
            CurrentState = EPlayerState::None;
        //GetCharacterMovement()->BrakingFrictionFactor = 2.0f;
        //GetCharacterMovement()->BrakingDecelerationWalking = 1500.f;
    }
}

bool AAPlayerCharacter::IsProtectedFromCurse() const
{
    return ProtectionZoneAmount > 0;
}

void AAPlayerCharacter::OnFallen()
{
    if (!HasAuthority())
    {
        Server_OnFallen();
    }

    CurrentState = EPlayerState::Fallen;
    SetPlayerSpeed(PlayerConfig->FallenSpeed);

    //TargetMaxSpeed = PlayerConfig->FallenSpeed;
    GetCharacterMovement()->MaxWalkSpeed = PlayerConfig->FallenSpeed; // Force immédiate
    PlayerSpeed = PlayerConfig->FallenSpeed;
    
}

void AAPlayerCharacter::OnTrapped()
{
    if (!HasAuthority())
        Server_OnTrapped();
    CurrentState = EPlayerState::Immobilized;
    TargetMaxSpeed = 0.f;
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->MaxWalkSpeed = 0.f; // Force immédiate
    PlayerSpeed = 0.f;
}

void AAPlayerCharacter::Server_OnTrapped_Implementation()
{
    OnTrapped();
}

void AAPlayerCharacter::Server_OnFallen_Implementation()
{
    OnFallen(); // Re-appelle la version serveur
}

void AAPlayerCharacter::OnRevive()
{
    if (!HasAuthority())
    {
        Server_OnRevive();
    }
    CurrentState = EPlayerState::None;
    TargetMaxSpeed = PlayerConfig->WalkSpeed;
    GetCharacterMovement()->MaxWalkSpeed = PlayerConfig->WalkSpeed; // Force immédiate
    PlayerSpeed = PlayerConfig->WalkSpeed;
    GetPlayerState()->GetPlayerController()->SetViewTargetWithBlend(this);
}

void AAPlayerCharacter::Server_OnRevive_Implementation()
{
    OnRevive();
}


void AAPlayerCharacter::OnDeath()
{
    if (!HasAuthority())
    {
        Server_OnDied();
    }

    CurrentState = EPlayerState::Dead;
    TargetMaxSpeed = 0.f;
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->MaxWalkSpeed = 0.f; // Force immédiate
    PlayerSpeed = 0.f;
}

void AAPlayerCharacter::Server_OnDied_Implementation()
{
    OnDeath();
}

bool AAPlayerCharacter::ServerManageRun_Validate(bool Input)
{
    return true;
}


void AAPlayerCharacter::ServerManageRun_Implementation(bool Input)
{
    ManageRun(Input); // Call the same logic on the server
}

#pragma endregion


#pragma region Auto Lock

void AAPlayerCharacter::Server_SendPushInput_Implementation(ALitter* Obj, FVector Input)
{
    if (Obj)
        Obj->Server_UpdateInputs(this,Input);
}

void AAPlayerCharacter::UpdatePushingMovement(float DeltaTime)
{
    if (!bIsCarrying || !CurrentPushedObject) return;

    FVector ObjVelocity = CurrentPushedObject->GetVelocity();
    ObjVelocity.Z = 0.0f;

    const float Speed = ObjVelocity.Size();
    if (Speed <= KINDA_SMALL_NUMBER) return;

    FRotator TargetRotation = ObjVelocity.Rotation();
    FRotator NewRot = FMath::RInterpTo(GetActorRotation(),TargetRotation,DeltaTime, 12.f);
    SetActorRotation(NewRot);
    
}

void AAPlayerCharacter::Server_SetPushingState_Implementation(ALitter* Obj, bool bCarrying)
{
    bIsCarrying = bCarrying;
    CurrentPushedObject = bIsCarrying ? Obj : nullptr;
}

void AAPlayerCharacter::StartAutoLock(float AutoLockStrength)
{
    CurrentAutoLockStrength = PlayerConfig->AutoLockStrength;
    bAutoLockIsActive = true;

    // Get the nearest enemy as a target
    TArray<FOverlapResult> Overlaps;
    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

    bool bHit = GetWorld()->OverlapMultiByObjectType(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ObjectQueryParams,
        FCollisionShape::MakeSphere(1000.f)
    );

    if (!bHit) return;
    
    float BestDist = 10000.f;
    for (auto& Result : Overlaps)
    {
        AActor* Actor = Result.GetActor();
        if (!Actor || !Actor->ActorHasTag("Enemy")) continue;
            
        float CurrentDist = (GetActorLocation() - Actor->GetActorLocation()).Length();
        if (CurrentDist > BestDist) continue;

        CurrentAutoLockTarget = Actor;
        BestDist = CurrentDist;
    }
}

void AAPlayerCharacter::ActualiseAutoLock()
{
    if (!CurrentAutoLockTarget) {
        GetCharacterMovement()->bOrientRotationToMovement = true;
        return;
    }

    GetCharacterMovement()->bOrientRotationToMovement = false;
    FVector AimedForward = CurrentAutoLockTarget->GetActorLocation() - GetActorLocation();

    FRotator TargetRotation = AimedForward.Rotation();
    
    FRotator NewRotation = FMath::RInterpTo(
        GetActorRotation(),
        TargetRotation,
        GetWorld()->GetDeltaSeconds(),
        CurrentAutoLockStrength
    );

    SetActorRotation(NewRotation);
}

void AAPlayerCharacter::StopAutoLock()
{
    bAutoLockIsActive = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
}

#pragma endregion


#pragma region Dodge

void AAPlayerCharacter::StartDodge()
{
    if (CurrentState == EPlayerState::Dodging) return;

    if (CurrentState == EPlayerState::UsingEquipment) {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        AnimInstance->StopAllMontages(false);
    }

    CurrentState = EPlayerState::Dodging;
    DodgeTimer = 0;
}


void AAPlayerCharacter::EndDodge()
{
    CurrentState = EPlayerState::None;

    SetPlayerSpeed(PlayerConfig->WalkSpeed);
}


void AAPlayerCharacter::ActualiseDodge(float DeltaTime)
{
    DodgeTimer += DeltaTime;

    SetPlayerSpeed(FMath::Lerp(PlayerConfig->DodgeStartSpeed, PlayerConfig->DodgeEndSpeed, DodgeTimer));

    FVector FinalVector = PreviousPlayerInput;
    FinalVector.Normalize();
    FRotator Rotation(0.0f, 30.0f - 90.0f, 0.0f);
    FinalVector = Rotation.RotateVector(FinalVector);

    AddMovementInput(FinalVector, 1.0f, false);
}

#pragma endregion


#pragma region Montages

void AAPlayerCharacter::PlayMontage(UAnimMontage* Montage, float Speed)
{
    if (!HasAuthority())
    {
        ServerPlayMontage(Montage, Speed);
    }
    else
    {
        MulticastPlayMontage(Montage, Speed);
    }
}


void AAPlayerCharacter::ServerPlayMontage_Implementation(UAnimMontage* Montage, float Speed)
{
    if (Montage)
        MulticastPlayMontage(Montage, Speed);
}


void AAPlayerCharacter::MulticastPlayMontage_Implementation(UAnimMontage* Montage, float Speed)
{
    if (!Montage || !GetMesh()) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->StopAllMontages(0.1f);
    AnimInstance->Montage_Play(Montage, Speed);

    AnimInstance->OnPlayMontageNotifyBegin.RemoveAll(this);

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &AAPlayerCharacter::OnMontageEnded);
    AnimInstance->Montage_SetBlendingOutDelegate(EndDelegate, Montage);
    AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &AAPlayerCharacter::OnMontageNotifyBegin);
}


void AAPlayerCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!Montage) return;

    if (CurrentState == EPlayerState::UsingEquipment)
    {
        CurrentState = EPlayerState::None;
        ItemComponent->AttackAnimEnd();
    }

    BP_OnMontageEnded(Montage, bInterrupted);
}


void AAPlayerCharacter::OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
    BP_OnMontageNotifyBegin(NotifyName);
}

#pragma endregion


#pragma region Others


void AAPlayerCharacter::OnRep_CurrentPlayerState()
{
    UE_LOG(LogTemp, Warning, TEXT("[CLIENT] %s CurrentState replicated. Controller: %s"),
        *GetName(),
        GetController() ? *GetController()->GetName() : TEXT("None"));

    // Ne pas changer TargetMaxSpeed ou MaxWalkSpeed côté client.
    // Laisser le serveur gérer TargetMaxSpeed et répliquer PlayerSpeed.
    // Ici, tu peux jouer des animations / effets visuels en fonction de CurrentState:
    switch (CurrentState)
    {
    case EPlayerState::Running:
        // jouer anim run
        break;
    case EPlayerState::None:
        // jouer idle/walk
        break;
    case EPlayerState::Dodging:
        // ...
        break;
    case EPlayerState::Fallen:
        // ...
        break;
    case EPlayerState::Dead:
        // ...
        break;
    case EPlayerState::Immobilized:
        break;
    default:
        break;
    }
}


void AAPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    UE_LOG(LogTemp, Warning, TEXT("[SERVER] %s POSSESSED by %s"),
        *GetName(),
        NewController ? *NewController->GetName() : TEXT("None"));

    if (IsReadyForRPCs())
    {
        UE_LOG(LogTemp, Log, TEXT("[SERVER] %s is ready for RPCs"), *GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[SERVER] %s is NOT ready for RPCs!"), *GetName());
    }
}

void AAPlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    UE_LOG(LogTemp, Warning, TEXT("[CLIENT] %s PlayerState replicated. Controller: %s"),
        *GetName(),
        GetController() ? *GetController()->GetName() : TEXT("None"));
}

bool AAPlayerCharacter::IsReadyForRPCs() const
{
    return GetController() != nullptr &&
        Cast<APlayerController>(GetController()) != nullptr;
}


void AAPlayerCharacter::ServerUseZiplineItem_Implementation(UItemData* ZiplineItem)
{
    if (!ZiplineItem || !ZiplineItem->ZiplineClass)
        return;

    FVector  SpawnLoc = GetActorLocation() + GetMesh()->GetForwardVector() * 50.f + FVector(0, 0, 0.f);
    FRotator SpawnRot = GetActorRotation();

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.Instigator = this;

    AZiplineInteractible* NewZip = GetWorld()->SpawnActor<AZiplineInteractible>(
        ZiplineItem->ZiplineClass,
        SpawnLoc,
        SpawnRot,
        Params
    );

    if (NewZip)
    {
        UE_LOG(LogTemp, Log, TEXT("Zipline placed by %s"), *GetName());
        InventoryComponent->RemoveCurrentItem();
    }
}


void AAPlayerCharacter::DisplayThrowPreview(FVector Position, float Range)
{
    ThrowablePreviewMeshComponent->SetWorldLocation(FVector(Position.X, Position.Y, Position.Z));

    ThrowablePreviewMeshComponent->SetHiddenInGame(false);
    ThrowablePreviewMeshComponent->SetRelativeScale3D(FVector(Range, Range, 1) * 0.01f);
}

void AAPlayerCharacter::HideThrowPreview()
{
    ThrowablePreviewMeshComponent->SetHiddenInGame(true);
}

#pragma endregion

