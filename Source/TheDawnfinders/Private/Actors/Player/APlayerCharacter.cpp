#include "Actors/Player/APlayerCharacter.h"

#include "Actors/Interactibles/Litter.h"
#include "Actors/Interactibles/Carriable.h"
#include "Actors/Interactibles/ZiplineInteractible.h"
#include "Components/UStaminaComponent.h"
#include "Components/UHealthComponent.h"
#include "Components/UItemComponent.h"
#include "Components/UInteractionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/CustomHUD.h"
#include "Perception/AISense_Hearing.h"
#include "Net/UnrealNetwork.h"
#include "Components/DebugComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CustomPlayerController.h"
#include "GameFramework/CustomPlayerState.h"
#include "Widgets/UMainWidget.h"
#include "Widgets/UWorldPlayerWidget.h"
#include "Widgets/UWorldProgressBar.h"
#include "Widgets/UQTEMashButtonWidget.h"


AAPlayerCharacter::AAPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Réplication Actor + mouvement (utile pour ACharacter)
    bReplicates = true;
    SetReplicateMovement(true);
    //GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;
    //GetCharacterMovement()->bNetworkSmoothingComplete =  true;
    //GetCharacterMovement()->NetworkSimulatedSmoothLocationTime = 0.08f;
    //GetCharacterMovement()->NetworkSimulatedSmoothRotationTime = 0.08f;
    //GetCharacterMovement()->ListenServerNetworkSimulatedSmoothLocationTime = 0.2f;
    //GetCharacterMovement()->ListenServerNetworkSimulatedSmoothRotationTime = 0.2f;
    
    // Interpolation plus agressive
    //GetCharacterMovement()->NetworkMaxSmoothUpdateDistance = 128.0f;
    //GetCharacterMovement()->NetworkNoSmoothUpdateDistance = 256.0f;
    
    // Augmenter la fréquence pour les mouvements critiques
    SetNetUpdateFrequency(30.0f);
    SetMinNetUpdateFrequency(15.0f);

    // Components 
    InventoryComponent   = CreateDefaultSubobject<UInventoryComponent>(TEXT("AC_Inventory"));
    StaminaComponent     = CreateDefaultSubobject<UStaminaComponent>(TEXT("AC_Stamina"));
    HealthComponent      = CreateDefaultSubobject<UHealthComponent>(TEXT("AC_Health"));
    ItemComponent        = CreateDefaultSubobject<UItemComponent>(TEXT("AC_ItemUse"));
    InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("AC_Interaction"));
    LightComponent       = CreateDefaultSubobject<UPlayerLightComponent>(TEXT("AC_LightComponent"));
    
    PlayerWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerWidgetComponent"));
    PlayerWidgetComponent->SetupAttachment(GetMesh());

    WeaponMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    WeaponMeshComponent->SetupAttachment(GetMesh());
    GunMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunMeshComponent"));
    GunMeshComponent->SetupAttachment(GetMesh());

    ThrowablePreviewMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ThrowablePreviewMeshComponent"));
    ThrowablePreviewMeshComponent->SetupAttachment(GetMesh());
    DebugComponent = CreateDefaultSubobject<UDebugComponent>(TEXT("DebugComponent"));
    WeaponCollisionPosRef = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponCollisionPosRef"));
    WeaponCollisionPosRef->SetupAttachment(GetMesh());

    CarriablePosRef = CreateDefaultSubobject<USceneComponent>(TEXT("CarriablePosRef"));
    CarriablePosRef->SetupAttachment(GetMesh());

    ThrowStartPosRef = CreateDefaultSubobject<USceneComponent>(TEXT("ThrowStartPosRef"));
    ThrowStartPosRef->SetupAttachment(GetMesh());

    PointLight = CreateDefaultSubobject<UPointLightComponent>(FName("Light"));
    PointLight->SetupAttachment(RootComponent);

    ProtectionZone = CreateDefaultSubobject<USphereComponent>(FName("ProtectionZone"));
    ProtectionZone->SetGenerateOverlapEvents(true);
    ProtectionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ProtectionZone->SetCollisionObjectType(ECC_WorldDynamic);
    ProtectionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
    ProtectionZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    ProtectionZone->SetHiddenInGame(true);
    ProtectionZone->SetupAttachment(RootComponent);

    NoiseZone = CreateDefaultSubobject<USphereComponent>(FName("NoiseZone"));
    NoiseZone->SetupAttachment(GetMesh());

    LoudNoiseZone = CreateDefaultSubobject<USphereComponent>(FName("LoudNoiseZone"));
    LoudNoiseZone->SetupAttachment(GetMesh());

    FogOfWarLightOn = CreateDefaultSubobject<USphereComponent>(FName("FogOfWarLightOn"));
    FogOfWarLightOn->SetupAttachment(RootComponent);

    FogOfWarLightOff = CreateDefaultSubobject<USphereComponent>(FName("FogOfWarLightOff"));
    FogOfWarLightOff->SetupAttachment(RootComponent);
    
    AkComponent = CreateDefaultSubobject<UAkComponent>(TEXT("AkAudioComponent"));
    if (GetMesh())
    {
        AkComponent->SetupAttachment(GetMesh());
    }

    // ---------- ROTATION PAR DÉFAUT ----------
    
     bUseControllerRotationYaw = false;
    if (PlayerConfig == nullptr)
    {
        PlayerConfig = CreateDefaultSubobject<UPlayerData>(TEXT("PlayerConfig"));
    }
}


void AAPlayerCharacter::ApplyPlayerData()
{
    SetPlayerSpeed(1500.f);

    HealthComponent->InitialiseComponent(
        PlayerConfig->MaxHealth,
        PlayerConfig->MinMaxHP,
        PlayerConfig->MinReviveHP,
        PlayerConfig->FallenTime,
        PlayerConfig->PoisonDmg,
        PlayerConfig->FallenTime
        );
    
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

    PlayerWidget = Cast<UWorldPlayerWidget>(PlayerWidgetComponent->GetWidget());

    ItemComponent->OnThrowPreviewDisplay.AddUniqueDynamic(this, &AAPlayerCharacter::DisplayThrowPreview);
    ItemComponent->OnThrowHidePreview.AddUniqueDynamic(this, &AAPlayerCharacter::HideThrowPreview);
    
    ProtectionZone->SetGenerateOverlapEvents(false);
    
    if (HasAuthority())
        InitPlayerNames();
    
}


void AAPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (NoMovementTimer > 0) {
        NoMovementTimer -= DeltaTime;
    }

    if (!IsLocallyControlled() && !bIsForcingRotation) return;

    if (bAutoLockIsActive) {
        ActualiseAutoLock();
    }
    ActualiseRotation();

    if (!IsLocallyControlled()) return;
    if (bIsAutoMoving) AutoMoveCharacter();

    if (LoudnessTimer > 0) {
        LoudnessTimer -= DeltaTime;
        if (LoudnessTimer <= 0) UILoudness = 0.05f;
    }

    if (bIsCarrying && CurrentPushedObject)
    {
        UpdatePushingMovement(DeltaTime);
    }
    
    switch (CurrentState) {
    case EPlayerState::Dodging :
        ActualiseDodge(DeltaTime);
        break;

    case EPlayerState::Blocking :
        if (!StaminaComponent->VerifyHasStamina()) ItemComponent->StopSecondaryAction();
        break;
    }
}

void AAPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // Laisse tes bindings BP gérer le Turn (AddControllerYawInput) et le Move.
}


void AAPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAPlayerCharacter, PlayerSpeed);
    DOREPLIFETIME(AAPlayerCharacter, bIsCarrying);
    DOREPLIFETIME(AAPlayerCharacter, CurrentPushedObject);
    DOREPLIFETIME(AAPlayerCharacter, bAutoLockIsActive);
    DOREPLIFETIME(AAPlayerCharacter, bIsForcingRotation);
    DOREPLIFETIME(AAPlayerCharacter, bIsAutoMoving);
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

void AAPlayerCharacter::StartMashButtonQTE_Implementation(float Quantity, float DecreasePerSecond, bool Cancellable)
{
    PlayerWidget->GetQTEMashButton()->StartQTE(Quantity, DecreasePerSecond, Cancellable);
}


void AAPlayerCharacter::DoCameraShake_Implementation(float Intensity) {}
void AAPlayerCharacter::DoDamagePostProcess_Implementation(float Duration) {}

void AAPlayerCharacter::ShowProgress_Implementation(float CurrentValue)
{
    if (!GetController()) { 
        ClientShowProgress(CurrentValue); return; 
    }
    if (!GetController()->IsLocalController()) {
        ClientShowProgress(CurrentValue); return;
    }

    if (!PlayerWidget->GetProgressBar()) return;

    UWorldProgressBar* ProgressBar = PlayerWidget->GetProgressBar();
    ProgressBar->ActualiseProgress(CurrentValue);
}

void AAPlayerCharacter::HideProgress_Implementation()
{
    if (!GetController()) {
        ClientHideProgress();
        return;
    }
    if (!GetController()->IsLocalController()) { 
        ClientHideProgress(); 
        return;
    }

    if (!PlayerWidget->GetProgressBar()) return;

    UE_LOG(LogTemp, Display, TEXT("STOOOOOP"));

    UWorldProgressBar* ProgressBar = PlayerWidget->GetProgressBar();
    ProgressBar->Hide();
}

void AAPlayerCharacter::ClientShowProgress_Implementation(float CurrentValue)
{
    if (!GetController()) return;
    if (!GetController()->IsLocalController()) return;

    if (!PlayerWidget->GetProgressBar()) return;

    UWorldProgressBar* ProgressBar = PlayerWidget->GetProgressBar();
    ProgressBar->ActualiseProgress(CurrentValue);
}

void AAPlayerCharacter::ClientHideProgress_Implementation()
{
    if (!GetController()) return;
    if (!GetController()->IsLocalController()) return;

    if (!PlayerWidget->GetProgressBar()) return;

    UE_LOG(LogTemp, Display, TEXT("STOOOOOP"));

    UWorldProgressBar* ProgressBar = PlayerWidget->GetProgressBar();
    ProgressBar->Hide();
}

void AAPlayerCharacter::SetEquippedMesh_Implementation(UStaticMesh* NewMesh, bool bIsRanged)
{
    WeaponMeshComponent->SetStaticMesh(nullptr);
    GunMeshComponent->SetStaticMesh(nullptr);

    if (bIsRanged) {
        GunMeshComponent->SetStaticMesh(NewMesh);
    }
    else {
        WeaponMeshComponent->SetStaticMesh(NewMesh);
    }
}

UInventoryComponent* AAPlayerCharacter::GetInventoryComponent_Implementation()
{
    return InventoryComponent;
}

EPlayerState AAPlayerCharacter::GetCurrentPlayerState_Implementation() 
{ 
    return CurrentState;
}

void AAPlayerCharacter::RequestStateChange_Implementation(EPlayerState NewState, bool bOverrideClient)
{
    if (CurrentState == NewState) return;

    if (HealthComponent->bIsFallen) {
        return;
    }

    switch (CurrentState) {
    case EPlayerState::Carrying :
        if (NewState == EPlayerState::Running || NewState == EPlayerState::Sneaking) return;
        break;
    }

    SetCurrentPlayerState_Implementation(NewState, bOverrideClient);
}

void AAPlayerCharacter::SetCurrentPlayerState_Implementation(EPlayerState NewState, bool bOverrideClient)
{
    CurrentState = NewState;

    switch (CurrentState)
    {
    case EPlayerState::None :
        StopAutoLock();
        break;

    case EPlayerState::Carrying:
        break;

    case EPlayerState::Fallen :
        StopAutoLock();
        ItemComponent->StopAim();
        ItemComponent->CancelReload();
        ItemComponent->StopMainAction();
        InteractionComponent->CancelInteraction();
        StopAutoMoveCharacter(true);
        StopMovementForDuration(4.5f);
        
        break;

    case EPlayerState::Immobilized :
        break;
        
    case EPlayerState::Trapped :
        ItemComponent->StopAim();
        break;

    case EPlayerState::Dead :
        InterruptMontage();
        break;
    }

    UE_LOG(LogTemp, Display, TEXT("%s"), *UEnum::GetValueAsString(CurrentState));

    if (!HasAuthority()) {
        Server_SetCurrentPlayerState(CurrentState, false);
    }
    else {
        Multicast_SetCurrentPlayerState(CurrentState, bOverrideClient);
    }
}

void AAPlayerCharacter::Server_SetCurrentPlayerState_Implementation(EPlayerState NewState, bool bOverrideClient)
{
    CurrentState = NewState;

    Multicast_SetCurrentPlayerState(NewState, bOverrideClient);
}

void AAPlayerCharacter::Multicast_SetCurrentPlayerState_Implementation(EPlayerState NewState, bool bOverrideClient)
{
    if (GetController() && !bOverrideClient) return;

    CurrentState = NewState;

    if (!GetController()) return;

    switch (CurrentState)
    {
    case EPlayerState::None:
        StopAutoLock();
        break;

    case EPlayerState::Carrying:
        break;

    case EPlayerState::Fallen:
        StopAutoLock();
        ItemComponent->StopAim();
        ItemComponent->CancelReload();
        ItemComponent->StopMainAction();
        InteractionComponent->CancelInteraction();
        InteractionComponent->ServerCancelHelp();
        StopAutoMoveCharacter(true);
        StopMovementForDuration(4.5f);
        break;

    case EPlayerState::Immobilized:
        break;

    case EPlayerState::Trapped:
        ItemComponent->StopAim();
        break;

    case EPlayerState::Dead:
        break;
    }
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
    if (SoundTag == "Walk") return PlayerConfig->WalkSoundAlertness * GetWorld()->GetDeltaSeconds();
    if (SoundTag == "Run") return PlayerConfig->RunSoundAlertness * GetWorld()->GetDeltaSeconds();
    if (SoundTag == "Dodge") return PlayerConfig->DodgeSoundAlertness;
    if (SoundTag == "Attack") return PlayerConfig->AttackSoundAlertness;
    if (SoundTag == "Sneak") return PlayerConfig->SneakSoundAlertness;
    if (SoundTag == "Distraction") return PlayerConfig->DistractionSoundAlertness;   
    return .0f;
}

void AAPlayerCharacter::PlaySoundOnServer_Implementation(FName SoundTag, float Range, float WaveStrength, FVector Loc, bool bLoudNoise)
{
    LoudnessTimer = 0.8f;
    UILoudness = WaveStrength;

    CurrentNoise = Range * NoiseModifier;

    if(!HasAuthority())
        Server_PlayNoise(SoundTag, CurrentNoise, Loc, bLoudNoise);
    
    else
        Server_PlayNoise_Implementation(SoundTag, CurrentNoise, Loc, bLoudNoise);
}

UWorldPlayerWidget* AAPlayerCharacter::GetPlayerWidget_Implementation()
{
    return PlayerWidget;
}

void AAPlayerCharacter::Server_AskOwnershipPermission_Implementation(AActor* Target, AController* Origin)
{
    Target->SetOwner(Origin);
}

void AAPlayerCharacter::ReceiveDamage_Implementation(float quantity, AActor* Origin)
{
    if (!GetController()) return;
    if (!GetController()->IsLocalController()) return;
    if (CurrentState == EPlayerState::Fallen || CurrentState == EPlayerState::Dead) return;

    if(quantity > 0)
        HealthComponent->TakeDamage(quantity);
}

#pragma endregion


#pragma region Interactible Interface

void AAPlayerCharacter::SelectInteractible_Implementation(AActor* Interactor)
{

}

void AAPlayerCharacter::UnselectInteractible_Implementation(AActor* Interactor)
{

}

#pragma endregion


#pragma region Movement / Run

void AAPlayerCharacter::SetPlayerSpeed(float NewSpeed, bool bInstant)
{
    if (HasAuthority())
    {
        PlayerSpeed = NewSpeed;
        TargetMaxSpeed = NewSpeed;

        GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
    }
    else
    {
        ServerSetPlayerSpeed(NewSpeed, bInstant);
        PlayerSpeed = NewSpeed;
        TargetMaxSpeed = NewSpeed;

        GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
    }

    //if (bInstant) GetCharacterMovement()->MaxAcceleration = 6000;
    //else GetCharacterMovement()->MaxAcceleration = 4000;
}

void AAPlayerCharacter::SetPlayerAcceleration(float NewAcceleration)
{
    GetCharacterMovement()->MaxAcceleration = NewAcceleration;

    if (!HasAuthority()) {
        ServerSetPlayerAcceleration(NewAcceleration);
    }
}

void AAPlayerCharacter::ServerSetPlayerAcceleration_Implementation(float NewAcceleration)
{
    GetCharacterMovement()->MaxAcceleration = NewAcceleration;
}

bool AAPlayerCharacter::ServerSetPlayerSpeed_Validate(float NewSpeed, bool bInstant)
{
    return NewSpeed >= 0.0f && NewSpeed <= 2000.0f;
}

void AAPlayerCharacter::ServerSetPlayerSpeed_Implementation(float NewSpeed, bool bInstant)
{
    PlayerSpeed = NewSpeed;
    TargetMaxSpeed = NewSpeed;
    GetCharacterMovement()->MaxWalkSpeed = NewSpeed;

    //if (bInstant) GetCharacterMovement()->MaxAcceleration = 6000;
    //else GetCharacterMovement()->MaxAcceleration = 4000;
}

void AAPlayerCharacter::OnRep_PlayerSpeed()
{
    GetCharacterMovement()->MaxWalkSpeed = PlayerSpeed;
}

void AAPlayerCharacter::MoveCharacter(FVector2D Input)
{
    if (bIsAutoMoving) return;
    if (CurrentState == EPlayerState::Dodging || CurrentState == EPlayerState::Immobilized || CurrentState == EPlayerState::Dead) {
        bMoveInputActive = false;
        if (!HasAuthority()) Server_SetMoveInputActive(false);
        return;
    }

    if (NoMovementTimer > 0) {
        if (bMoveInputActive && CurrentState == EPlayerState::Fallen) {
            InterruptMontage();
        }

        bMoveInputActive = false;

        if (!HasAuthority()) Server_SetMoveInputActive(false);
        return;
    }

    if (CurrentState == EPlayerState::Blocking) {
        AddMovementInput(FVector(0, 0, 0), 1.0f, false);
        return;
    }

    // --- 1. HANDLE LITTER MOVEMENT ---
    if (bIsCarrying && CurrentPushedObject)
    {
        FVector PushDir = FVector(-Input.X, Input.Y, 0);

        // IMPORTANT : tester AVANT normalize
        if (PushDir.SquaredLength() > 0.05f)
        {
            PushDir = PushDir.GetClampedToMaxSize(1);

            FRotator CameraRotation(0.0f, -45.0f, 0.0f);
            PushDir = CameraRotation.RotateVector(PushDir);
        }
        else
        {
            PushDir = FVector::ZeroVector;
        }

        if (IsLocallyControlled())
        {
            Server_SendPushInput(CurrentPushedObject, PushDir);
        }

        return;
    }

    // --- 2. STANDARD CHARACTER MOVEMENT ---
    CurrentPlayerInput = FVector(-Input.X, Input.Y, 0);

    if (CurrentPlayerInput.SquaredLength() > 0.05f) {
        PreviousPlayerInput = CurrentPlayerInput;

        if (!bMoveInputActive && CurrentState == EPlayerState::Fallen) {
            PlayMontageLoop(CrawlMontage, 1);
        }

        bMoveInputActive = true;
        if (!HasAuthority()) Server_SetMoveInputActive(true);
    }
    else {
        AddMovementInput(FVector(0, 0, 0), 1.0f, true);

        if (bMoveInputActive && CurrentState == EPlayerState::Fallen) {
            InterruptMontage();
        }

        bMoveInputActive = false;
        if (!HasAuthority()) Server_SetMoveInputActive(false);
        return;
    }

    FVector FinalVector = FVector(-Input.X, Input.Y, 0);
    FinalVector = FinalVector.GetClampedToMaxSize(1);

    FRotator Rotation(0.0f, -45.0f, 0.0f);
    FinalVector = Rotation.RotateVector(FinalVector);

    if (CurrentState == EPlayerState::None) {
        FVector NormRotInput = CurrentRotationInput;
        NormRotInput.Normalize();

        FVector NormMoveInput = FinalVector;
        NormMoveInput.Normalize();

        float DotProd = NormRotInput.Dot(NormMoveInput);
        DotProd = 1 - FMath::Clamp(DotProd, 0, 1);

        if (!bIsForcingRotation) DotProd = 0;

        AddMovementInput(FinalVector, FMath::Clamp(PlayerConfig->WalkSpeed / 1500.f, 0, PlayerConfig->WalkSpeed / 1500.f * (1-(1-PlayerConfig->SideWalkModifier) * DotProd)), true);
    }

    else if(CurrentState == EPlayerState::Running)
        AddMovementInput(FinalVector, PlayerConfig->RunSpeed / 1500.f, true);

    else if(CurrentState == EPlayerState::Sneaking)
        AddMovementInput(FinalVector, PlayerConfig->SneakSpeed / 1500.f, true);

    else if (CurrentState == EPlayerState::Fallen)
        AddMovementInput(FinalVector, PlayerConfig->FallenSpeed / 1500.f, true);

    else if (CurrentState == EPlayerState::UsingEquipment)
        AddMovementInput(FinalVector, PlayerConfig->WalkSpeed * 0.4f / 1500.f, true);

    else if (CurrentState == EPlayerState::Carrying)
        AddMovementInput(FinalVector, PlayerConfig->CarrySpeed / 1500.f, true);
}

void AAPlayerCharacter::StartAutoMoveCharacter(FVector TargetPos, FRotator TargetRot, AActor* Target)
{
    bIsAutoMoving = true;

    AutoMoveTargetPos = TargetPos;
    AutoMoveTargetRot = TargetRot;
    AutoMoveTarget = Target;
}

void AAPlayerCharacter::AutoMoveCharacter()
{
    FVector Dir = AutoMoveTargetPos - GetActorLocation();
    Dir.Z = 0;

    if (Dir.SquaredLength() < 50) {
        StopAutoMoveCharacter(false);
        return;
    }

    Dir.Normalize();

    FVector InputDir = FVector(Dir.X, Dir.Y, 0);
    FRotator Rotation(0.0f, 45.0f, 0.0f);

    InputDir = Rotation.RotateVector(InputDir);
    CurrentPlayerInput = InputDir;
    PreviousPlayerInput = InputDir;

    AddMovementInput(Dir, PlayerConfig->WalkSpeed / 1500.f, true);
}

void AAPlayerCharacter::StopAutoMoveCharacter(bool bCancel)
{
    bIsAutoMoving = false;
    RequestStateChange_Implementation(EPlayerState::Immobilized, false);

    if (HasAuthority()) {
        Server_StopAutoMoveCharacter_Implementation(bCancel);
    }
    else {
        Server_StopAutoMoveCharacter(bCancel);
    }
}

void AAPlayerCharacter::Server_StopAutoMoveCharacter_Implementation(bool bCancel)
{
    bIsAutoMoving = false;

    if (bCancel) return;

    ReachAutoMoveDestination();
}

void AAPlayerCharacter::ServerManageRun_Implementation(bool Input)
{
    ManageRun(Input);
}

void AAPlayerCharacter::StopMovementForDuration(float Duration)
{
    NoMovementTimer = Duration;
    PlaySoundOnServer_Implementation("", 0, 0);
}

void AAPlayerCharacter::RestartMovement()
{
    NoMovementTimer = 0;
}


void AAPlayerCharacter::ManageRun(bool Input)
{
    if (Input)
    {
        if (PlayerConfig->RunSpeed == GetCharacterMovement()->MaxWalkSpeed) return;

        //SetPlayerSpeed(PlayerConfig->RunSpeed);
        RequestStateChange_Implementation(EPlayerState::Running);
    }
    else
    {
        if (PlayerConfig->WalkSpeed == GetCharacterMovement()->MaxWalkSpeed) return;

        //SetPlayerSpeed(PlayerConfig->WalkSpeed);
        if (CurrentState == EPlayerState::Running)
            RequestStateChange_Implementation(EPlayerState::None);
    }
}

void AAPlayerCharacter::GetShopItems_Implementation()
{
}

bool AAPlayerCharacter::IsProtectedFromCurse() const
{
    return ProtectionZoneAmount > 0;
}

#pragma endregion


#pragma region Litter

void AAPlayerCharacter::Server_SendPushInput_Implementation(ALitter* Obj, FVector Input)
{
    if (Obj)
        Obj->Server_UpdateInputs(this,Input);
}

void AAPlayerCharacter::UpdatePushingMovement(float DeltaTime)
{
    if (!bIsCarrying || !CurrentPushedObject) return;
}

void AAPlayerCharacter::Server_SetPushingState_Implementation(ALitter* Obj, bool bCarrying)
{
    bIsCarrying = bCarrying;
    CurrentPushedObject = bIsCarrying ? Obj : nullptr;
}

#pragma endregion


#pragma region Rotation / Auto Lock

void AAPlayerCharacter::ActualiseRotation()
{
    PreviousPlayerInput.Normalize();

    if (!bIsForcingRotation && CurrentForcedRotationRatio > 0) {
        CurrentForcedRotationRatio -= GetWorld()->GetDeltaSeconds() * PlayerConfig->NormalToForcedSpeed * 10.f;
        CurrentForcedRotationRatio = FMath::Clamp(CurrentForcedRotationRatio, 0, 1);

        //Server_StopForceRotation(CurrentForcedRotationRatio);
    }

    float angle = FMath::Atan2(PreviousPlayerInput.Y, PreviousPlayerInput.X);
    angle -= FMath::DegreesToRadians(50);
    FVector RotatedVector = FVector(FMath::Cos(angle), FMath::Sin(angle), 0);

    FRotator TargetRotation = RotatedVector.Rotation();
    FRotator MovementRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), PlayerConfig->NormalRotationSpeed);

    ItemComponent->ActualisePreviewThrow(CurrentRotationInput);

    FRotator NewRotation = FQuat::Slerp(MovementRotation.Quaternion(), CurrentForcedRotation.Quaternion(), CurrentForcedRotationRatio).Rotator();
    if (CurrentPlayerInput.SquaredLength() < 0.2f && CurrentForcedRotation != FRotator(0, 0, 0)) NewRotation = CurrentForcedRotation;
    else if (!bIsForcingRotation && CurrentForcedRotationRatio <= 0) CurrentForcedRotation = FRotator(0, 0, 0);

    SetActorRotation(NewRotation);
}


void AAPlayerCharacter::ForceRotation(FVector Input)
{
    if (bIsAutoMoving) return;
    if (CurrentState == EPlayerState::Immobilized || CurrentState == EPlayerState::Fallen || CurrentState == EPlayerState::Dead) return;

    if(CurrentForcedRotationRatio < 1)
        CurrentForcedRotationRatio += GetWorld()->GetDeltaSeconds() * PlayerConfig->NormalToForcedSpeed;

    CurrentForcedRotationRatio = FMath::Clamp(CurrentForcedRotationRatio, 0, 1);
    bIsForcingRotation = true;

    if (Input.Length() > 0.05f)
        PreviousPlayerInput = FVector(-Input.X, -Input.Y, 0);

    float Length = Input.Length();
    Input.Normalize();

    float angle = FMath::Atan2(Input.Y, Input.X);

    if(ItemComponent->GetIsPreviewingThrow()) angle -= FMath::DegreesToRadians(50 + 180);
    else angle -= FMath::DegreesToRadians(50 + 180);

    FVector RotatedVector = FVector(FMath::Cos(angle), FMath::Sin(angle), 0);

    FRotator TargetRotation = RotatedVector.Rotation();
    FRotator NewRotation = FMath::RInterpTo(CurrentForcedRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), PlayerConfig->ForceRotationSpeed);

    CurrentRotationInput = RotatedVector * Length;
    CurrentForcedRotation = NewRotation;

    if (!HasAuthority()) {
        Server_ForceRotation(CurrentForcedRotation, CurrentRotationInput, CurrentForcedRotationRatio);
    }
    else {
        Multicast_ForceRotation(CurrentForcedRotation, CurrentRotationInput, CurrentForcedRotationRatio, true);
    }
}

void AAPlayerCharacter::ForceRotationInstant(FRotator Rotation)
{
    CurrentForcedRotationRatio = 1;
    bIsForcingRotation = true;
    CurrentForcedRotation = Rotation;

    if (!HasAuthority()) {
        Server_ForceRotation(CurrentForcedRotation, CurrentRotationInput, CurrentForcedRotationRatio);
    }
    else {
        Multicast_ForceRotation(CurrentForcedRotation, CurrentRotationInput, CurrentForcedRotationRatio, true);
    }
}

void AAPlayerCharacter::Server_ForceRotation_Implementation(FRotator Rotation, FVector Input, float Progress)
{
    bIsForcingRotation = true;
    CurrentForcedRotation = Rotation;
    CurrentRotationInput = Input;

    CurrentForcedRotationRatio = Progress;
    GetCharacterMovement()->bOrientRotationToMovement = false;

    Multicast_ForceRotation(Rotation, Input, Progress, false);
}

void AAPlayerCharacter::Multicast_ForceRotation_Implementation(FRotator Rotation, FVector Input, float Progress, bool bOverrideClient)
{
    if (IsLocallyControlled() && !bOverrideClient) return;

    bIsForcingRotation = true;
    CurrentForcedRotation = Rotation;
    CurrentRotationInput = Input;

    CurrentForcedRotationRatio = Progress;
    GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AAPlayerCharacter::StopForceRotation(float Progress)
{
    if (bIsAutoMoving) return;
    if (CurrentState == EPlayerState::Immobilized) return;

    bIsForcingRotation = false;
    CurrentForcedRotationRatio = Progress;

    GetCharacterMovement()->bOrientRotationToMovement = false;

    if (HasAuthority()) {
        Multicast_StopForceRotation(CurrentForcedRotationRatio);
    }
    else {
        Server_StopForceRotation(CurrentForcedRotationRatio);
    }
}


void AAPlayerCharacter::Multicast_StopForceRotation_Implementation(float Progress)
{
    if (IsLocallyControlled()) return;

    bIsForcingRotation = false;
    CurrentForcedRotationRatio = Progress;

    GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AAPlayerCharacter::Server_StopForceRotation_Implementation(float Progress)
{
    bIsForcingRotation = false;
    CurrentForcedRotationRatio = Progress;

    GetCharacterMovement()->bOrientRotationToMovement = true;

    Multicast_StopForceRotation(Progress);
}


void AAPlayerCharacter::StartAutoLock(float AutoLockStrength)
{
    CurrentAutoLockStrength = PlayerConfig->AutoLockStrength;
    bAutoLockIsActive = true;

    CurrentAutoLockTarget = nullptr;

    TArray<FOverlapResult> Overlaps;
    FCollisionObjectQueryParams ObjectQueryParams;

    ObjectQueryParams.AddObjectTypesToQuery(ECC_Destructible);
    ObjectQueryParams.AddObjectTypesToQuery(ECC_GameTraceChannel1);
    ObjectQueryParams.AddObjectTypesToQuery(ECC_GameTraceChannel2);

    bool bHit = GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, ObjectQueryParams, FCollisionShape::MakeSphere(350.f));
    if (!bHit) return;

    float BestDist = 350.f;

    for (auto& Result : Overlaps) {
        AActor* Actor = Result.GetActor();
        if (!Actor || (!Actor->ActorHasTag("Enemy") && !Actor->ActorHasTag("Destructible"))) continue;
        if (Actor->ActorHasTag("Enemy") && Cast<ABaseEnemy>(Actor)->bIsDead) continue;

        float CurrentDist = (GetActorLocation() - Actor->GetActorLocation()).Length();
        if (CurrentDist < BestDist) 
        {
            CurrentAutoLockTarget = Actor; 
            BestDist = CurrentDist; 
        }
    }
}

void AAPlayerCharacter::ActualiseAutoLock()
{
    if (!IsValid(CurrentAutoLockTarget)) 
    { 
        StopAutoLock();
        return; 
    }

    FVector AimedForward = CurrentAutoLockTarget->GetActorLocation() - GetActorLocation();
    FRotator TargetRotation = AimedForward.Rotation();
    FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), CurrentAutoLockStrength);
    NewRotation.SetComponentForAxis(EAxis::Y, 0);
    NewRotation.SetComponentForAxis(EAxis::X, 0);

    bIsForcingRotation = true;
    CurrentForcedRotationRatio = 1;
    CurrentForcedRotation = NewRotation;

    FVector InputDir = GetActorForwardVector();
    float angle = FMath::Atan2(InputDir.Y, InputDir.X);
    angle -= FMath::DegreesToRadians(50 + 180);
    FVector RotatedVector = FVector(FMath::Cos(angle), FMath::Sin(angle), 0);

    CurrentRotationInput = RotatedVector;

    if (!HasAuthority()) {
        Server_ForceRotation(CurrentForcedRotation, CurrentRotationInput, 1);
    }
    else {
        Server_ForceRotation_Implementation(CurrentForcedRotation, CurrentRotationInput, 1);
    }
}

void AAPlayerCharacter::StopAutoLock()
{
    bAutoLockIsActive = false;

    bIsForcingRotation = false;
    CurrentForcedRotationRatio = 0;

    if (!HasAuthority()) {
        Server_StopForceRotation(CurrentForcedRotationRatio);
    }
    else {
        Server_StopForceRotation_Implementation(CurrentForcedRotationRatio);
    }
}

#pragma endregion


#pragma region Dodge

void AAPlayerCharacter::StartDodge()
{
    if (bIsAutoMoving) return;

    if (CurrentState == EPlayerState::Dodging) return;
    if (CurrentState == EPlayerState::UsingEquipment) 
    { 
        GetMesh()->GetAnimInstance()->StopAllMontages(false); 
    }

    SetPlayerAcceleration(6000);

    SetCurrentPlayerState_Implementation(EPlayerState::Dodging);
    DodgeTimer = 0;
}

void AAPlayerCharacter::EndDodge()
{
    if (CurrentState == EPlayerState::Fallen || CurrentState == EPlayerState::Dead) return;
    if (!GetController()) return;

    SetPlayerAcceleration(4000);

    SetCurrentPlayerState_Implementation(EPlayerState::None);
    //SetPlayerSpeed(PlayerConfig->WalkSpeed);
    PlaySoundOnServer_Implementation("", 0, 0, FVector(0, 0, 0));
}

void AAPlayerCharacter::ActualiseDodge(float DeltaTime)
{
    DodgeTimer += DeltaTime;
    //if(DodgeTimer < 0.9f)
        //SetPlayerSpeed(FMath::Clamp(FMath::Lerp(PlayerConfig->DodgeStartSpeed, PlayerConfig->DodgeEndSpeed, FMath::Clamp(DodgeTimer / 0.9f, 0, 1)), 0, 2000), true);

    FVector FinalVector = PreviousPlayerInput;
    FinalVector.Normalize();

    FRotator Rotation(0.0f, -45.0f, 0.0f);
    FinalVector = Rotation.RotateVector(FinalVector);

    AddMovementInput(FinalVector, FMath::Clamp(FMath::Lerp(PlayerConfig->DodgeStartSpeed, PlayerConfig->DodgeEndSpeed, 
        FMath::Clamp(DodgeTimer / 0.9f, 0, 1)), 0, 2000) / 1500.f, false);
}

#pragma endregion


#pragma region Montages

void AAPlayerCharacter::PlayMontage(UAnimMontage* Montage, float Speed) 
{ 
    if (!HasAuthority()) ServerPlayMontage(Montage, Speed); 
    else MulticastPlayMontage(Montage, Speed); 
}


void AAPlayerCharacter::PlayMontageLoop(UAnimMontage* Montage, float Speed)
{
    if (!HasAuthority()) ServerPlayMontageLoop(Montage, Speed);
    else MulticastPlayMontageLoop(Montage, Speed);
}

void AAPlayerCharacter::ServerPlayMontageLoop_Implementation(UAnimMontage* Montage, float Speed)
{
    if (Montage) MulticastPlayMontageLoop(Montage, Speed);
}

void AAPlayerCharacter::MulticastPlayMontageLoop_Implementation(UAnimMontage* Montage, float Speed)
{
    if (!Montage || !GetMesh()) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->StopAllMontages(0.1f);
    AnimInstance->Montage_Play(Montage, Speed);

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &AAPlayerCharacter::OnLoopMontageEnded);

    AnimInstance->Montage_SetBlendingOutDelegate(EndDelegate, Montage);
}

void AAPlayerCharacter::InterruptMontage()
{
    if (HasAuthority()) Multicast_InterruptMontage();
    else Server_InterruptMontage();
}

void AAPlayerCharacter::Server_InterruptMontage_Implementation()
{
    Multicast_InterruptMontage();
}

void AAPlayerCharacter::Multicast_InterruptMontage_Implementation()
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;
    AnimInstance->StopAllMontages(0.1f);
}

void AAPlayerCharacter::OnLoopMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (bInterrupted) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->StopAllMontages(0.1f);
    AnimInstance->Montage_Play(Montage, 1);

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &AAPlayerCharacter::OnLoopMontageEnded);

    AnimInstance->Montage_SetBlendingOutDelegate(EndDelegate, Montage);
}


void AAPlayerCharacter::ServerPlayMontage_Implementation(UAnimMontage* Montage, float Speed) 
{ 
    if (Montage) MulticastPlayMontage(Montage, Speed); 
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
        if (InventoryComponent->GetCurrentSlot().CurrentInfos.ItemData && !InventoryComponent->GetCurrentSlot().CurrentInfos.ItemData->bIsRangedWeapon) {
            SetCurrentPlayerState_Implementation(EPlayerState::None);
            ItemComponent->AttackAnimEnd();
        }
    }
    BP_OnMontageEnded(Montage, bInterrupted);
}

void AAPlayerCharacter::OnMontageNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{ 
    BP_OnMontageNotifyBegin(NotifyName); 
}

#pragma endregion


#pragma region Carry

void AAPlayerCharacter::StartCarryHeavyItem_Implementation(AActor* Interactible)
{
    ACarriable* Carriable = Cast<ACarriable>(Interactible);

    InteractionComponent->StartCarryHeavyItem(Carriable);

    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, false);
    Carriable->StaticMesh->AttachToComponent(CarriablePosRef, AttachRules);
}

void AAPlayerCharacter::EndCarryHeavyItem_Implementation(AActor* Interactible)
{
    if (HasAuthority()) {
        Server_EndCarryHeavyItem_Implementation();
    }
    else {
        Server_EndCarryHeavyItem();
    }
}

void AAPlayerCharacter::Server_EndCarryHeavyItem_Implementation()
{
    InteractionComponent->EndCarryHeavyItem();

}
#pragma endregion


#pragma region Others

void AAPlayerCharacter::OnRevive()
{
    if (!HasAuthority()) Server_OnRevive(); 
    SetCurrentPlayerState_Implementation(EPlayerState::None, true);
    Client_ResetCamera();

    StopMovementForDuration(2.25f);
}

void AAPlayerCharacter::Client_ResetCamera_Implementation()
{
    ACustomPlayerController* PC = Cast<ACustomPlayerController>(GetController());
    if (!PC) return;
    PC->SetViewTargetWithBlend(this);
}

void AAPlayerCharacter::Server_OnRevive_Implementation()
{
    OnRevive();

    StopMovementForDuration(2.25f);
}

bool AAPlayerCharacter::IsReadyForRPCs() const
{
    return GetController() != nullptr && Cast<APlayerController>(GetController()) != nullptr;
}

void AAPlayerCharacter::Server_PlayNoise_Implementation(FName SoundTag, float Range, FVector Loc, bool bLoudNoise)
{
    if (bLoudNoise) {
        LoudNoiseZone->SetSphereRadius(Range);

        if (Range <= 0) {
            LoudNoiseZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        else {
            LoudNoiseZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
    }

    else {
        NoiseZone->SetSphereRadius(Range);

        if (Range <= 0) {
            NoiseZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        else {
            NoiseZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
    }
}

void AAPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
}

void AAPlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    ACustomPlayerState* PS = GetPlayerState<ACustomPlayerState>();
    if (PS && InventoryComponent)
    {
        if (PS->ShopItems.Num() > 0)
        {
            for (const FItemInfos& Item : PS->ShopItems)
            {
                InventoryComponent->AddNewItem(Item); 
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Items du Shop transférés dans l'inventaire après Travel"));
        }
    }
    InitPlayerNames();
}

void AAPlayerCharacter::ServerUseZiplineItem_Implementation(UItemData* ZiplineItem)
{
    if (!ZiplineItem || !ZiplineItem->ZiplineClass) return;
    FVector  SpawnLoc = GetActorLocation() + GetMesh()->GetForwardVector() * 50.f + FVector(0, 0, 0.f);
    FRotator SpawnRot = GetActorRotation();
    FActorSpawnParameters Params; Params.Owner = this; Params.Instigator = this;
    AZiplineInteractible* NewZip = GetWorld()->SpawnActor<AZiplineInteractible>(ZiplineItem->ZiplineClass, SpawnLoc, SpawnRot, Params);
    if (NewZip) InventoryComponent->RemoveCurrentItem();
}

void AAPlayerCharacter::DisplayThrowPreview_Implementation(FVector Direction, float Strength)
{
    //ThrowablePreviewMeshComponent->SetWorldLocation(FVector(Position.X, Position.Y, Position.Z));
    //ThrowablePreviewMeshComponent->SetHiddenInGame(false);
    //ThrowablePreviewMeshComponent->SetRelativeScale3D(FVector(Range, Range, 1) * 0.01f);
}

void AAPlayerCharacter::HideThrowPreview()
{
    ThrowablePreviewMeshComponent->SetHiddenInGame(true);
}

void AAPlayerCharacter::PlayVibrations_Implementation(bool bStrong, bool bLong)
{
}

void AAPlayerCharacter::Server_SetMoveInputActive_Implementation(bool bActive)
{
    bMoveInputActive = bActive;
}

void AAPlayerCharacter::ReachAutoMoveDestination_Implementation()
{

}


void AAPlayerCharacter::Client_OpenInteractionUI_Implementation(EInteractionUI UIType, AActor* Context)
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    ACustomHUD* HUD = Cast<ACustomHUD>(PC->GetHUD());
    if (!HUD || !HUD->MainWidget) return;

    switch (UIType)
    {
    case EInteractionUI::LitterInventory:
        HUD->MainWidget->OpenContainerInventory(Context);
        break;

    case EInteractionUI::ContainerInventory:
        HUD->MainWidget->OpenContainerInventory(Context);
        break;
    }
}

void AAPlayerCharacter::Client_CloseInteractionUI_Implementation(EInteractionUI UIType, AActor* Context)
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    ACustomHUD* HUD = Cast<ACustomHUD>(PC->GetHUD());
    if (!HUD || !HUD->MainWidget) return;

    switch (UIType)
    {
    case EInteractionUI::LitterInventory:
        HUD->MainWidget->CloseContainerInventory();
        break;

    case EInteractionUI::ContainerInventory:
        HUD->MainWidget->CloseContainerInventory();
        break;
    }
}


#pragma endregion