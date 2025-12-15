// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/UPlayerLightComponent.h"

#include <gsl/pointers>

#include "Actors/Player/APlayerCharacter.h"
#include "GameFramework/CustomPlayerState.h"
#include "Components/UHealthComponent.h"
#include "Interfaces/IFadeable.h"
#include "Net/UnrealNetwork.h"

UPlayerLightComponent::UPlayerLightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	LightRoot = CreateDefaultSubobject<USceneComponent>(FName("Root"));
	
	PointLight = CreateDefaultSubobject<UPointLightComponent>(FName("Light"));
	PointLight->SetupAttachment(LightRoot);

	ProtectionZone = CreateDefaultSubobject<USphereComponent>(FName("ProtectionZone"));
	ProtectionZone->SetGenerateOverlapEvents(true);
	ProtectionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProtectionZone->SetCollisionObjectType(ECC_WorldDynamic);
	ProtectionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	ProtectionZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	ProtectionZone->SetHiddenInGame(true);
	ProtectionZone->SetupAttachment(LightRoot);

	FogOfWarLightOn = CreateDefaultSubobject<USphereComponent>(FName("FogOfWarLightOn"));
	FogOfWarLightOn->SetupAttachment(LightRoot);

	FogOfWarLightOff = CreateDefaultSubobject<USphereComponent>(FName("FogOfWarLightOff"));
	FogOfWarLightOff->SetupAttachment(LightRoot);

	LightMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("LanternMesh"));
	LightMesh->SetupAttachment(LightRoot);

	bLightOn = false;
	FuelRemaining = 100.f;
	LightIntensity = 5000.f;
	LightRadius = 300.f;
	FuelConsumption = 5.f;

	SetIsReplicatedByDefault(true);
}

void UPlayerLightComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
		{
		if (LightRoot)
		{
			LightRoot->AttachToComponent(
				Owner->GetRootComponent(),
				FAttachmentTransformRules::KeepRelativeTransform
			);
		}
	}

	AActor* Owner = GetOwner();
	if (!Owner) return;
	
	if (Cast<AAPlayerCharacter>(Owner))
	{
		if (ProtectionZone)
		{
			ProtectionZone->SetGenerateOverlapEvents(false);
			ProtectionZone->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
		}
	}
	else
	{
		if (ProtectionZone)
		{
			ProtectionZone->SetGenerateOverlapEvents(true);
			ProtectionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly); 
		}
	}
	
	LightRoot->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	if (GetOwnerRole() == ROLE_Authority)
	{
		ProtectionZone->OnComponentBeginOverlap.AddDynamic(this, &UPlayerLightComponent::OnOverlapBegin);
		ProtectionZone->OnComponentEndOverlap.AddDynamic(this, &UPlayerLightComponent::OnOverlapEnd);

		FogOfWarLightOn->OnComponentBeginOverlap.AddDynamic(this, &UPlayerLightComponent::OnFogOfWarOverlapBegin);
		FogOfWarLightOn->OnComponentEndOverlap.AddDynamic(this, &UPlayerLightComponent::OnFogOfWarOverlapEnd);

		FogOfWarLightOff->OnComponentBeginOverlap.AddDynamic(this, &UPlayerLightComponent::OnFogOfWarOverlapBegin);
		FogOfWarLightOff->OnComponentEndOverlap.AddDynamic(this, &UPlayerLightComponent::OnFogOfWarOverlapEnd);
	}

	ApplyLightState();


	// Actualises the local UI
	if (!Owner) return;

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;

	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC || !PC->IsLocalController()) return;

	if (!PC->PlayerState) return;

	ACustomPlayerState* PSCustom = Cast<ACustomPlayerState>(PC->PlayerState);
	PSCustom->ActualiseLocalLantern(FuelRemaining, MaxFuel);
}


void UPlayerLightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bLightOn)
	{
		ConsumeFuel(DeltaTime);
	}
}

void UPlayerLightComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerLightComponent, bLightOn);
	DOREPLIFETIME_CONDITION_NOTIFY(UPlayerLightComponent, FuelRemaining, COND_None, REPNOTIFY_Always);
}

void UPlayerLightComponent::InitialiseComponent(float FuelRate, float MaxFuels)
{
	FuelConsumption= FuelRate;
	MaxFuel = MaxFuels;
	FuelRemaining = MaxFuel;
}


#pragma region Colliders

void UPlayerLightComponent::OnFogOfWarOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->Implements<UFadeable>()) return;

	OtherActor->SetOwner(GetOwner());
	IFadeable::Execute_FadeIn(OtherActor);
}

void UPlayerLightComponent::OnFogOfWarOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor->Implements<UFadeable>()) return;
	IFadeable::Execute_FadeOut(OtherActor);
}

#pragma endregion


#pragma region Turn On / Off

void UPlayerLightComponent::TurnLightOn()
{
	if (bLightOn) return;

	// Server
	if (GetOwner()->HasAuthority())
	{
		bLightOn = true;
		ApplyLightState();
	}
	else   // Client
	{
		Server_TurnLightOn();
	}
}

void UPlayerLightComponent::TurnLightOff()
{
	if (!bLightOn) return;

	// Server
	if (GetOwner()->HasAuthority())
	{
		bLightOn = false;
		ApplyLightState();
	}
	else  // Client
	{
		Server_TurnLightOff();
	}
}


void UPlayerLightComponent::Server_TurnLightOn_Implementation()
{
	TurnLightOn();
}

void UPlayerLightComponent::Server_TurnLightOff_Implementation()
{
	TurnLightOff();
}


void UPlayerLightComponent::ApplyLightState_Implementation()
{
	if (!PointLight) return;

	PointLight->SetVisibility(bLightOn);
	PointLight->SetIntensity(bLightOn ? LightIntensity : 0.f);
	PointLight->SetSourceRadius(bLightOn ? LightRadius : 0.f);
	ProtectionZone->SetHiddenInGame(!bLightOn);

	if (!GetOwner()->HasAuthority()) return;    // Continue only if is server
	
	if (!ProtectionZone) return;

	TArray<AActor*> OverlappingActors;
	ProtectionZone->GetOverlappingActors(OverlappingActors, AAPlayerCharacter::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor == GetOwner()) continue;

		if (AAPlayerCharacter* Player = Cast<AAPlayerCharacter>(Actor))
		{
			if (bLightOn) Player->HealthComponent->AddProtectionZone();
			else Player->HealthComponent->RemoveProtectionZone();
		}
	}
}

void UPlayerLightComponent::OnRep_LightOn()
{
	ApplyLightState();
}

void UPlayerLightComponent::FuelUpdate(float NewFuel)
{

	if (GetOwner()->HasAuthority()) 
	{
		FuelRemaining += NewFuel;
		FuelRemaining = FMath::Clamp(FuelRemaining, 0.f, MaxFuel);

		if (FuelRemaining <= 0.f && bLightOn)
		{
			TurnLightOff();
		}
	}
	
	// Actualises the local UI 
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;

	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC || !PC->IsLocalController()) return;

	if (!PC->PlayerState) return;

	ACustomPlayerState* PSCustom = Cast<ACustomPlayerState>(PC->PlayerState);
	PSCustom->ActualiseLocalLantern(FuelRemaining, MaxFuel);
}

void UPlayerLightComponent::OnRep_FuelRemaining()
{
	// Update UI local
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;

	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC || !PC->IsLocalController()) return;

	if (!PC->PlayerState) return;

	ACustomPlayerState* PSCustom = Cast<ACustomPlayerState>(PC->PlayerState);
	if (PSCustom)
	{
		PSCustom->ActualiseLocalLantern(FuelRemaining, MaxFuel);
	}
}


#pragma endregion


#pragma region Fuel + Protection

void UPlayerLightComponent::ConsumeFuel(float DeltaTime)
{
	// Server Changes values
	if (GetOwner()->HasAuthority()) 
	{
		FuelRemaining -= FuelConsumption * DeltaTime;
		FuelRemaining = FMath::Max(FuelRemaining, 0.f);

		if (FuelRemaining <= 0.f && bLightOn)
		{
			TurnLightOff();
		}
	}

	// Actualises the local UI 
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* PawnOwner = Cast<APawn>(Owner);
	if (!PawnOwner) return;

	APlayerController* PC = Cast<APlayerController>(PawnOwner->GetController());
	if (!PC || !PC->IsLocalController()) return;

	if (!PC->PlayerState) return;

	ACustomPlayerState* PSCustom = Cast<ACustomPlayerState>(PC->PlayerState);
	PSCustom->ActualiseLocalLantern(FuelRemaining, MaxFuel);
}


void UPlayerLightComponent::Server_RequestFuelUpdate_Implementation(float Amount)
{
	FuelUpdate(Amount);
}

void UPlayerLightComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!GetOwner()->HasAuthority() || !bLightOn) return;
	if (OtherActor == GetOwner()) return;

	if (OtherActor->Implements<UPlayerInterface>())
	{
		IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(OtherActor);

		PlayerInterface->AddProtectionZone_Implementation();
	}
}

void UPlayerLightComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!GetOwner()->HasAuthority() || !bLightOn) return;
	if (OtherActor == GetOwner()) return;

	if (OtherActor->Implements<UPlayerInterface>())
	{
		IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(OtherActor);

		PlayerInterface->RemoveProtectionZone_Implementation();
	}
}

#pragma endregion

