// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Interactibles/Interactible.h"
#include "Widgets/ULockpickQTEWidget.h"
#include "Widgets/UWorldInteractibleWidget.h"
#include "Actors/Player/APlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

AInteractibleObjects::AInteractibleObjects()
{
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	InteractCollider = CreateDefaultSubobject<UBoxComponent>(FName("InteractCollider"));
	InteractCollider->SetupAttachment(RootComponent);
	InteractCollider->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractCollider->SetGenerateOverlapEvents(true);
	
	BoxCollider = CreateDefaultSubobject<UBoxComponent>(FName("CapsuleColliderComp"));
	BoxCollider->SetupAttachment(RootComponent);
	BoxCollider->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BoxCollider->SetGenerateOverlapEvents(true);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);
	StaticMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	StaticMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	StaticMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap);

	InteractibleWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(FName("InteractibleWidget"));
	InteractibleWidgetComponent->SetupAttachment(InteractCollider);
	
	bReplicates = true;
}

void AInteractibleObjects::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInteractibleObjects, bIsInteracting);
	DOREPLIFETIME(AInteractibleObjects, PlayerTemp);
	DOREPLIFETIME(AInteractibleObjects, InteractionTimer);
	DOREPLIFETIME(AInteractibleObjects, CurrentInteractActor);
}

void AInteractibleObjects::BeginPlay()
{
	Super::BeginPlay();
	if (InteractCollider)
	{
		InteractCollider->OnComponentBeginOverlap.AddDynamic(this, &AInteractibleObjects::OnOverlapBegin);
		InteractCollider->OnComponentEndOverlap.AddDynamic(this, &AInteractibleObjects::OnOverlapEnd);
	}

	EnableDistance = EnableDistance * EnableDistance;

	InteractibleWidget = Cast<UWorldInteractibleWidget>(InteractibleWidgetComponent->GetWidget());

	if (FloatCurve)
	{
		FOnTimelineFloat ProgressFunction{};
		ProgressFunction.BindUFunction(this, FName("HandleFadeProgress"));

		FadeTimeline.AddInterpFloat(FloatCurve, ProgressFunction);

		FadeTimeline.SetLooping(false);
	}

	if (!HasAuthority()) return;

	GetWorldTimerManager().SetTimer(
		EnableTimer,
		this,
		&AInteractibleObjects::CheckEnableDistance,
		1.5f,   // time in seconds
		true    // looping
	);
}

void AInteractibleObjects::Tick(float DeltaTime)
{
	FadeTimeline.TickTimeline(DeltaTime);

	Super::Tick(DeltaTime);
}

int AInteractibleObjects::GetPlayerCount_Implementation()
{
	return PlayerCount;
}

void AInteractibleObjects::SetPlayerCount_Implementation(int Value)
{
	PlayerCount+= Value;
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

void AInteractibleObjects::PlayNoise()
{
	ANoise* NoiseObj = GetWorld()->SpawnActor<ANoise>(NoiseActor, GetActorLocation(), GetActorRotation());
	NoiseObj->NoiseZone->SetSphereRadius(NoiseRange);
}

void AInteractibleObjects::CheckEnableDistance()
{
	float ClosestDistSq = TNumericLimits<float>::Max();

	// We go through all the players 
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APawn* Pawn = It->Get()->GetPawn())
		{
			float DistSq = FVector::DistSquared(GetActorLocation(), Pawn->GetActorLocation());
			ClosestDistSq = FMath::Min(ClosestDistSq, DistSq);
		}
	}

	if (ClosestDistSq < EnableDistance)
	{
		SetActorTickEnabled(true);
		SetActorHiddenInGame(false);
	}
	else
	{
		SetActorTickEnabled(false);
		SetActorHiddenInGame(true);
	}
}


#pragma region Others 

void AInteractibleObjects::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor);
	if (Player && Player->IsLocallyControlled())
	{
		Player->AddInteractibleAtRange_Implementation(this);
		UE_LOG(LogTemp, Log, TEXT("Added interactible locally on client"));
	}
}

void AInteractibleObjects::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(OtherActor);
	if (Player && Player->IsLocallyControlled())
	{
		Player->RemoveInteractibleAtRange_Implementation(this);
	}
}

void AInteractibleObjects::DisplayErrorMessage(const FString& Message)
{
	if (InteractibleWidget->GetVisibility() == ESlateVisibility::Collapsed) return;
	InteractibleWidget->DisplayErrorText(Message);
}

void AInteractibleObjects::Multicast_DisplayErrorMessage_Implementation(const FString& Message)
{
	if (InteractibleWidget->GetVisibility() == ESlateVisibility::Collapsed) return;
	InteractibleWidget->DisplayErrorText(Message);
}

void AInteractibleObjects::Server_DisplayErrorMessage_Implementation(const FString& Message)
{
	Multicast_DisplayErrorMessage(Message);

	if (InteractibleWidget->GetVisibility() == ESlateVisibility::Collapsed) return;
	InteractibleWidget->DisplayErrorText(Message);
}

#pragma endregion	


#pragma region Interface

void AInteractibleObjects::SelectInteractible_Implementation(AActor* Interactor)
{
	if (!InteractibleWidget) return;

	InteractibleWidget->DisplayText("Interact");
}

void AInteractibleObjects::UnselectInteractible_Implementation(AActor* Interactor)
{
	if (!InteractibleWidget) return;

	InteractibleWidget->HideText();
}

void AInteractibleObjects::Interact_Implementation(AActor* Interactor)
{
	if (!bCanBeUsed) return;

	BP_OnInteraction(Cast<AAPlayerCharacter>(Interactor));
}

void AInteractibleObjects::StopInteract_Implementation(AActor* Interactor)
{
	if (!bCanBeUsed) return;

	BP_OnStopInteraction(Cast<AAPlayerCharacter>(Interactor));
}

void AInteractibleObjects::BP_OnInteractionFinished_Implementation()
{
	if (NeededInteractItem == NULL || InteractItemConsumptionType != EInteractItemConsuptionType::ConsumeOnUse) return;

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			if (AAPlayerCharacter* PlayerCharacter = Cast<AAPlayerCharacter>(Pawn))
			{
				PlayerCharacter->InventoryComponent->RemoveCurrentItem();
			}
		}
	}
}

bool AInteractibleObjects::GetCanBeUsed_Implementation(AActor* Interactor)
{
	if (NeededInteractItem != nullptr) {

		if (!IPlayerInterface::Execute_GetInventoryComponent(Interactor)->VerifyHasItemInInventory(NeededInteractItem))
		{
			DisplayErrorMessage("You need a " + NeededInteractItem->ItemName);
			return false;
		}
	}

	if (bPlayerIsUsing) {
		DisplayErrorMessage("Already Used By Someone");
		return false;
	}

	return bCanBeUsed;
}

EQTEType AInteractibleObjects::GetNeededQTE_Implementation()
{
	return QTEType;
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

void AInteractibleObjects::OnQTESuccess()
{
}

void AInteractibleObjects::OnQTEFailed()
{
}

#pragma endregion

