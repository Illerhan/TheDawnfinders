// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Interactibles/Interactible.h"
#include "Widgets/ULockpickQTEWidget.h"
#include "Widgets/UWorldInteractibleWidget.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AInteractibleObjects::AInteractibleObjects()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(FName("Root"));

	CapsuleCollider = CreateDefaultSubobject<UCapsuleComponent>(FName("SphereCollider"));
	CapsuleCollider->SetupAttachment(RootComponent);
	CapsuleCollider->SetCollisionResponseToAllChannels(ECR_Ignore);
	CapsuleCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CapsuleCollider->SetGenerateOverlapEvents(true);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);
	StaticMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	StaticMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	StaticMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);

	InteractQTEWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(FName("LockpickWidget"));
	InteractQTEWidgetComponent->SetupAttachment(CapsuleCollider);

	InteractibleWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(FName("InteractibleWidget"));
	InteractibleWidgetComponent->SetupAttachment(CapsuleCollider);
	
	bReplicates = true;
}

void AInteractibleObjects::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInteractibleObjects, bIsInteracting);
	DOREPLIFETIME(AInteractibleObjects, PlayerTemp);
	DOREPLIFETIME(AInteractibleObjects, InteractionTimer);
}

void AInteractibleObjects::BeginPlay()
{
	Super::BeginPlay();

	CapsuleCollider->OnComponentBeginOverlap.AddDynamic(this, &AInteractibleObjects::OnOverlapBegin);
	CapsuleCollider->OnComponentEndOverlap.AddDynamic(this, &AInteractibleObjects::OnOverlapEnd);

	InteractQTEWidget = Cast<ULockpickQTEWidget>(InteractQTEWidgetComponent->GetWidget());
	InteractibleWidget = Cast<UWorldInteractibleWidget>(InteractibleWidgetComponent->GetWidget());

	if (FloatCurve)
	{
		FOnTimelineFloat ProgressFunction{};
		ProgressFunction.BindUFunction(this, FName("HandleFadeProgress"));

		FadeTimeline.AddInterpFloat(FloatCurve, ProgressFunction);

		FadeTimeline.SetLooping(false);
	}
}

void AInteractibleObjects::Tick(float DeltaTime)
{
	FadeTimeline.TickTimeline(DeltaTime);

	Super::Tick(DeltaTime);
}

void AInteractibleObjects::HoldTimer(float DeltaTime)
{
	IPlayerInterface::Execute_ShowProgress(PlayerTemp, InteractionTimer);
	InteractionTimer = InteractionTimer - DeltaTime;

	if (InteractionTimer <= 0) {
		IPlayerInterface::Execute_HideProgress(PlayerTemp);
		
		BP_OnInteractionFinished();
	}
}


#pragma region Colliders 

void AInteractibleObjects::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor);
	if (Player && Player->IsLocallyControlled())
	{
		Player->AddInteractibleAtRange_Implementation(this);
		UE_LOG(LogTemp, Log, TEXT("Added interactible locally on client"));

		if(InteractibleWidget)
			InteractibleWidget->DisplayText("[E] Interact");
	}
}

void AInteractibleObjects::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor);
	if (Player && Player->IsLocallyControlled())
	{
		Player->RemoveInteractibleAtRange_Implementation(this);

		if (InteractibleWidget)
			InteractibleWidget->HideText();
	}
}

#pragma endregion	


#pragma region Interface

void AInteractibleObjects::Interact_Implementation(AActor* Interact)
{
	if (!bCanBeUsed) return;

	BP_OnInteraction(Cast<AAPlayerCharacter>(Interact));
}

void AInteractibleObjects::StopInteract_Implementation(AActor* Interactor)
{
	if (!bCanBeUsed) return;

	BP_OnStopInteraction(Cast<AAPlayerCharacter>(Interactor));
}

void AInteractibleObjects::BP_OnInteractionFinished_Implementation()
{
}

bool AInteractibleObjects::GetCanBeUsed_Implementation()
{
	return bCanBeUsed;
}

bool AInteractibleObjects::GetQTENeeded_Implementation()
{
	return bDoQTE;
}

void AInteractibleObjects::StartQTE_Implementation()
{
	InteractQTEWidget->EnterQTE(QTESuccessRange, 400.f);
}

void AInteractibleObjects::StopQTE_Implementation()
{
	InteractQTEWidget->ExitQTE();
}

bool AInteractibleObjects::ValidateQTE_Implementation()
{
	InteractQTEWidget->ExitQTE();

	return InteractQTEWidget->ValidateQTE();
}

void AInteractibleObjects::FadeIn_Implementation()
{
	FadeIn_Multicast();
}

void AInteractibleObjects::FadeOut_Implementation()
{
	FadeOut_Multicast();
}

void AInteractibleObjects::FadeIn_Multicast_Implementation()
{
	FogOfWarCount++;
	if (FogOfWarCount != 1) return;

	FadeTimeline.PlayFromStart();
}

void AInteractibleObjects::FadeOut_Multicast_Implementation()
{
	FogOfWarCount--;
	if (FogOfWarCount != 0) return;

	FadeTimeline.ReverseFromEnd();
}

void AInteractibleObjects::HandleFadeProgress(float Value)
{
	for (int i = 0; i < Materials.Num(); i++) {
		Materials[i]->SetScalarParameterValue("Opacity", Value);
	}
}


#pragma endregion

