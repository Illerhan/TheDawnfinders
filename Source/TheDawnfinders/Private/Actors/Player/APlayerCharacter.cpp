// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Player/APlayerCharacter.h"

#include "Actors/Interactibles/Interactible.h"
#include "Actors/Interactibles/Lever.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AAPlayerCharacter::AAPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
	AActor::SetReplicateMovement(true);

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("AC_Inventory"));
	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("AC_Stamina"));
	//InventoryComponent->SetupAttachment(RootComponent);
}

void AAPlayerCharacter::AddInteractibleAtRange_Implementation(AActor* Interactible)
{
	InteractiblesAtRange.Add(Interactible);
}

void AAPlayerCharacter::RemoveInteractibleAtRange_Implementation(AActor* Interactible)
{
	InteractiblesAtRange.Remove(Interactible);
}

void AAPlayerCharacter::ServerInteract_Implementation(AInteractibleObjects* Interactible,AAPlayerCharacter* Player)
{
	if (!Interactible || !Interactible->bCanBeUsed)
		return;
	
	if (Interactible)
	{
		Interactible->Interaction(Player);
	}
}

void AAPlayerCharacter::ServerStopInteract_Implementation(ALever* Lever, AAPlayerCharacter* Player)
{
	if (!Lever || !Player)
		return;
    
	Lever->StopHoldInteraction(Player);
	UE_LOG(LogTemp, Warning, TEXT("[SERVER] Stop interaction called for %s"), *Lever->GetName());
}

void AAPlayerCharacter::TryInteract(AInteractibleObjects* InteractibleObject, AAPlayerCharacter* Player)
{
	UE_LOG(LogTemp, Warning, TEXT("Hello"));
	if (InteractibleObject)
		ServerInteract(InteractibleObject,Player);
}

void AAPlayerCharacter::StartInteract()
{
	AActor* NereastInteractible  = GetNearestInteractible();
	AInteractibleObjects* Interactible = Cast<AInteractibleObjects>(NereastInteractible);

	if (Interactible && Interactible->bCanBeUsed)
	{
		CurrentInteractible = Interactible;

		ALever* Lever = Cast<ALever>(Interactible);
		if (Lever && Lever->bCanBeUsed)
		{
			TryInteract(Interactible,this);
			UE_LOG(LogTemp, Log, TEXT("[CLIENT] Started hold interaction with %s"), *Interactible->GetName());
		}
		else
		{
			TryInteract(Interactible, this);
			CurrentInteractible = nullptr; // Pas besoin de tracker pour toggle
			UE_LOG(LogTemp, Log, TEXT("[CLIENT] Single interaction with %s"), *Interactible->GetName());
	
		}
	}
	
}

void AAPlayerCharacter::StopInteract()
{
	if (CurrentInteractible)
	{
		ALever* Lever = Cast<ALever>(CurrentInteractible);
		if (Lever && Lever->bRequiresHold)
		{
			// Arrête l'interaction maintenue
			ServerStopInteract(Lever, this);
			UE_LOG(LogTemp, Log, TEXT("[CLIENT] Stopped hold interaction with %s"), *Lever->GetName());
		}
        
		CurrentInteractible = nullptr;
	}
}

void AAPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = 400.0f;
	
}

void AAPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateCharacter();

	if (CurrentState == EPlayerState::Dodging) {
		ActualiseDodge(DeltaTime);
	}
}

void AAPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


void AAPlayerCharacter::MoveCharacter(FVector2D Input)
{
	if (CurrentState == EPlayerState::UsingEquipment || CurrentState == EPlayerState::Dodging) return;

	CurrentPlayerInput = FVector(-Input.X, Input.Y, 0);

	FVector FinalVector = FVector(-Input.X, Input.Y, 0);
	FinalVector.Normalize();
	//FinalVector = GetActorTransform().TransformVector(FinalVector);

	FRotator Rotation(0.0f, 30.0f - 90.0f, 0.0f);
	FinalVector = Rotation.RotateVector(FinalVector);

	AddMovementInput(FinalVector, 1.0f, true);
}


void AAPlayerCharacter::RotateCharacter()
{
	if (CurrentState == EPlayerState::UsingEquipment || CurrentState == EPlayerState::Dodging) return;
	if (CurrentPlayerInput.Length() < 10.f) return;
	if (GetVelocity().Length() < 100.f) return;

	FVector Direction = GetVelocity();
	Direction.Normalize();

	double radAngle = atan2(Direction.Y, Direction.X);
	FRotator AimedRotation = FRotator(0, FMath::RadiansToDegrees(radAngle), 0);

	if (GetMesh()) {
		FRotator CurrentRotation = GetMesh()->GetRelativeRotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, AimedRotation, 1.f, 1.f);

		SetActorRelativeRotation(NewRotation);
	}
}

void AAPlayerCharacter::ManageRun(bool Input)
{
	if (CurrentState == EPlayerState::Dodging) return;

	if (Input) {
		CurrentState = EPlayerState::Running;
		GetCharacterMovement()->MaxWalkSpeed = 800.0f;
	}
	else {
		if(CurrentState == EPlayerState::Running) CurrentState = EPlayerState::None;
		GetCharacterMovement()->MaxWalkSpeed = 400.0f;
	}
}

void AAPlayerCharacter::Dodge()
{
	if (CurrentState == EPlayerState::UsingEquipment || CurrentState == EPlayerState::Dodging) return;

	CurrentState = EPlayerState::Dodging;
	DodgeTimer = 0;
}

void AAPlayerCharacter::ActualiseDodge(float DeltaTime)
{
	DodgeTimer += DeltaTime;
	if (DodgeTimer > 0.5f) {
		CurrentState = EPlayerState::None;
		GetCharacterMovement()->MaxWalkSpeed = 400.0f;
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = 1000.0f;

	FVector FinalVector = CurrentPlayerInput;
	FinalVector.Normalize();

	FRotator Rotation(0.0f, 30.0f - 90.0f, 0.0f);
	FinalVector = Rotation.RotateVector(FinalVector);

	AddMovementInput(FinalVector, 1.0f, false);
}

AActor* AAPlayerCharacter::GetNearestInteractible()
{
	float bestDist = FLT_MAX;
	AActor* pickedInteractible = nullptr;

	for (AActor* Interactible : InteractiblesAtRange) {
		float currentDist = (Interactible->GetActorLocation() - GetActorLocation()).Length();

		if (currentDist < bestDist) {
			pickedInteractible = Interactible;
			bestDist = currentDist;
		}
	}
	
	return pickedInteractible;
}

void AAPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
    
	UE_LOG(LogTemp, Warning, TEXT("[SERVER] %s POSSESSED by %s"), 
		*GetName(), 
		NewController ? *NewController->GetName() : TEXT("None"));
    
	// Verify RPC readiness
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
	// For Server RPCs to work, the pawn must:
	// 1. Have a valid controller
	// 2. The controller must be a PlayerController (for client->server RPCs)
	return GetController() != nullptr && 
		   Cast<APlayerController>(GetController()) != nullptr;
}

