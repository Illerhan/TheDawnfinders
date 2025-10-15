// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Player/APlayerCharacter.h"

#include "Actors/Interactibles/Interactible.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AAPlayerCharacter::AAPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
	AActor::SetReplicateMovement(true);

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

void AAPlayerCharacter::TryInteract(AInteractibleObjects* InteractibleObject, AAPlayerCharacter* Player)
{
	UE_LOG(LogTemp, Warning, TEXT("Hello"));
	if (InteractibleObject)
		ServerInteract(InteractibleObject,Player);
}

void AAPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	Inventory = FindComponentByClass<UInventoryComponent>();
	ensure(Inventory);
	GetCharacterMovement()->MaxWalkSpeed = 400.0f;
	
}

void AAPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateCharacter();
}

void AAPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


void AAPlayerCharacter::MoveCharacter(FVector2D Input)
{
	if (CurrentState == EPlayerState::UsingEquipment) return;

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
	if (CurrentState == EPlayerState::UsingEquipment) return;
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
	if (Input) {
		CurrentState = EPlayerState::Running;
		GetCharacterMovement()->MaxWalkSpeed = 700.0f;
	}
	else {
		if(CurrentState == EPlayerState::Running) CurrentState = EPlayerState::None;
		GetCharacterMovement()->MaxWalkSpeed = 400.0f;
	}
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
