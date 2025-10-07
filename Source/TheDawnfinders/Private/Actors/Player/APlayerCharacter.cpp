// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Player/APlayerCharacter.h"

// Sets default values
AAPlayerCharacter::AAPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AAPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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
	if (NoInputMovement) return;

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
	if (NoRotation) return;
	if (CurrentPlayerInput.Length() < 1.f) return;
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



