// Copyright ...
#include "Actors/Player/APlayerCharacter.h"
#include "Actors/Interactibles/ZiplineInteractible.h"

#include "Components/UStaminaComponent.h"
#include "Components/UHealthComponent.h"
#include "Components/UItemComponent.h"
#include "Components/UInteractionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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
    // LightComponent     = CreateDefaultSubobject<UPlayerLightComponent>(TEXT("AC_Light"));
    // LightComponent->SetupAttachment(GetMesh());

    // ---------- ROTATION PAR DÉFAUT ----------

    bUseControllerRotationYaw = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;

    // Orientation sur déplacement
     bUseControllerRotationYaw = false;
     GetCharacterMovement()->bOrientRotationToMovement = true;
     GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
}


void AAPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;

    ProgressBarWidget = Cast<UWorldProgressBar>(ProgressBarComponent->GetWidget());


    ItemComponent->OnThrowPreviewDisplay.AddUniqueDynamic(this, &AAPlayerCharacter::DisplayThrowPreview);
    ItemComponent->OnThrowHidePreview.AddUniqueDynamic(this, &AAPlayerCharacter::HideThrowPreview);
    

    UE_LOG(LogTemp, Display, TEXT("%d"), ProgressBarWidget != nullptr);
}


void AAPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);


    if (GetLocalRole() == ROLE_SimulatedProxy)
    {
        return;
    }

    // --- Plus aucune rotation Mesh/Actor ici ---
    // Tu gères la rotation en Blueprint (input Turn/Look et/ou CharacterMovement).

    if (CurrentState == EPlayerState::Dodging)
    {
        ActualiseDodge(DeltaTime);
    }
}


void AAPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAPlayerCharacter, CurrentState);
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

void AAPlayerCharacter::PlayAttackMontage_Implementation(UAnimMontage* AttackMontage)
{
    PlayMontage(AttackMontage);
}

void AAPlayerCharacter::ReceiveDamage_Implementation(float quantity, AActor* Origin)
{
    HealthComponent->TakeDamage(quantity);
}

#pragma endregion


void AAPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    // Laisse tes bindings BP gérer le Turn (AddControllerYawInput) et le Move.
}


#pragma region Movement / Run


void AAPlayerCharacter::MoveCharacter(FVector2D Input)
{
    if (CurrentState == EPlayerState::UsingEquipment || CurrentState == EPlayerState::Dodging)
        return;

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

    // Ton offset 30° (cam isométrique ?) conservé
    FRotator Rotation(0.0f, 30.0f - 90.0f, 0.0f);
    FinalVector = Rotation.RotateVector(FinalVector);

    AddMovementInput(FinalVector, 1.0f, true);
}


void AAPlayerCharacter::ManageRun(bool Input)
{
    if (CurrentState == EPlayerState::Dodging) return;

    // Si on est sur le serveur, on applique normalement
    if (HasAuthority())
    {
        if (Input)
        {
            CurrentState = EPlayerState::Running;
            GetCharacterMovement()->MaxWalkSpeed = 800.0f;
        }
        else
        {
            if (CurrentState == EPlayerState::Running)
                CurrentState = EPlayerState::None;

            GetCharacterMovement()->MaxWalkSpeed = 400.0f;
        }
    }
    else
    {
        // Client: uniquement prédiction visuelle locale (vitesse)
        // Ne PAS toucher à CurrentState, il sera répliqué par le serveur
        //GetCharacterMovement()->MaxWalkSpeed = Input ? 800.0f : 400.0f;
        
        // Demander au serveur
        ServerManageRun(Input);
    }
}


bool AAPlayerCharacter::IsProtectedFromCurse() const
{
    return ProtectionZoneAmount > 0;
}


void AAPlayerCharacter::AddProtectionZone()
{
    if (HasAuthority())
    {
        ProtectionZoneAmount++;
        // If need to add more logic
        //OnRep_ProtectionZoneChanged();
    }
}


void AAPlayerCharacter::RemoveProtectionZone()
{
    if (HasAuthority())
    {
        ProtectionZoneAmount--;
        // If need to add more logic
        //OnRep_ProtectionChanged();
    }
}


void AAPlayerCharacter::OnRep_CurrentPlayerState()
{
    UE_LOG(LogTemp, Warning, TEXT("[CLIENT] %s CurrentState replicated. Controller: %s"),
        *GetName(),
        GetController() ? *GetController()->GetName() : TEXT("None"));
    switch(CurrentState)
    {
    case EPlayerState::Running:
        GetCharacterMovement()->MaxWalkSpeed = 800.0f;
        break;
    case EPlayerState::None:
        GetCharacterMovement()->MaxWalkSpeed = 400.0f;
        break;
    case EPlayerState::Dodging:
        // Géré par ActualiseDodge
        break;
    default:
        GetCharacterMovement()->MaxWalkSpeed = 400.0f;
        break;
    }
    // You can add logic here to update animations, movement speed, etc.
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


#pragma region Dodge

void AAPlayerCharacter::StartDodge()
{
    if (CurrentState == EPlayerState::UsingEquipment || CurrentState == EPlayerState::Dodging) return;
    CurrentState = EPlayerState::Dodging;
    DodgeTimer = 0;
}


void AAPlayerCharacter::EndDodge()
{
    CurrentState = EPlayerState::None;
    //GetCharacterMovement()->MaxWalkSpeed = 400.0f;
}


void AAPlayerCharacter::ActualiseDodge(float DeltaTime)
{
    DodgeTimer += DeltaTime;
    GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(1400.0f, 100.0f, DodgeTimer * 0.9f);

    FVector FinalVector = PreviousPlayerInput;
    FinalVector.Normalize();
    FRotator Rotation(0.0f, 30.0f - 90.0f, 0.0f);
    FinalVector = Rotation.RotateVector(FinalVector);

    AddMovementInput(FinalVector, 1.0f, false);
}

#pragma endregion


#pragma region Montages

void AAPlayerCharacter::MulticastPlayMontage_Implementation(UAnimMontage* Montage)
{
    if (!Montage || !GetMesh()) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    AnimInstance->StopAllMontages(0.1f);
    AnimInstance->Montage_Play(Montage);

    AnimInstance->OnPlayMontageNotifyBegin.RemoveAll(this);

    FOnMontageEnded EndDelegate;
    EndDelegate.BindUObject(this, &AAPlayerCharacter::OnMontageEnded);
    AnimInstance->Montage_SetBlendingOutDelegate(EndDelegate, Montage);
    AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &AAPlayerCharacter::OnMontageNotifyBegin);
}


void AAPlayerCharacter::PlayMontage(UAnimMontage* Montage)
{
    if (!HasAuthority())
    {
        ServerPlayMontage(Montage);
    }
    else
    {
        MulticastPlayMontage(Montage);
    }
}


void AAPlayerCharacter::ServerPlayMontage_Implementation(UAnimMontage* Montage)
{
    if (Montage)
        MulticastPlayMontage(Montage);
}


void AAPlayerCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!Montage) return;

    UE_LOG(LogTemp, Log, TEXT("[%s] Montage %s ended. Interrupted: %s"),
        *GetName(),
        *Montage->GetName(),
        bInterrupted ? TEXT("true") : TEXT("false"));

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

